
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../Common.h"

#include "FrameReader.h"
#include "FrameBuilder.h"
#include "IDPool.h"

#include "../webserver/Common.h"

struct RelayServerInternal;

void ServerMessageHandler (void * Tag, unsigned char Type, char * Message, int Size);
void ServerTimerTick      (Lacewing::Timer &Timer);

struct RelayServerInternal
{
    Lacewing::RelayServer &Server;
    Lacewing::Timer Timer;

    Lacewing::RelayServer::HandlerConnect           HandlerConnect;
    Lacewing::RelayServer::HandlerDisconnect        HandlerDisconnect;
    Lacewing::RelayServer::HandlerError             HandlerError;
    Lacewing::RelayServer::HandlerServerMessage     HandlerServerMessage;
    Lacewing::RelayServer::HandlerChannelMessage    HandlerChannelMessage;
    Lacewing::RelayServer::HandlerPeerMessage       HandlerPeerMessage;
    Lacewing::RelayServer::HandlerJoinChannel       HandlerJoinChannel;
    Lacewing::RelayServer::HandlerLeaveChannel      HandlerLeaveChannel;
    Lacewing::RelayServer::HandlerSetName           HandlerSetName;

    RelayServerInternal(Lacewing::RelayServer &_Server, Lacewing::Pump &Pump)
            : Server(_Server), Builder(false), Timer(Pump)
    {
        HandlerConnect          = 0;
        HandlerDisconnect       = 0;
        HandlerError            = 0;
        HandlerServerMessage    = 0;
        HandlerChannelMessage   = 0;
        HandlerPeerMessage      = 0;
        HandlerJoinChannel      = 0;
        HandlerLeaveChannel     = 0;
        HandlerSetName          = 0;

        WelcomeMessage = Lacewing::Version();
    
        Timer.Tag = this;
        Timer.onTick (ServerTimerTick);

        ChannelListingEnabled = true;
    }

    IDPool ClientIDs;
    IDPool ChannelIDs;

    struct Channel;

    struct Client
    {
        Lacewing::RelayServer::Client Public;

        Lacewing::Server::Client &Socket;
        RelayServerInternal &Server;
        
        Client(Lacewing::Server::Client &_Socket)
                : Server(*(RelayServerInternal *) Socket.Tag), Socket(_Socket),
                    UDPAddress(Socket.GetAddress())
        {
            Public.InternalTag    = this;
            Public.Tag            = 0;

            Reader.Tag            = this;
            Reader.MessageHandler = ServerMessageHandler;

            ID = Server.ClientIDs.Borrow();

            Handshook      = false;
            Ponged         = true;
            GotFirstByte   = false;
        }

        ~Client()
        {
            Server.ClientIDs.Return(ID);  
        }

        FrameReader Reader;
        
        void MessageHandler (unsigned char Type, char * Message, int Size, bool Blasted);

        List <Channel *> Channels;

        String Name;
        bool NameAltered;

        bool CheckName (const char * Name);

        unsigned short ID;
    
        Channel * ReadChannel(MessageReader &Reader);
    
        bool Handshook;
        bool GotFirstByte;
        bool Ponged;

        Lacewing::Address UDPAddress;

    };

    struct Channel
    {
        Lacewing::RelayServer::Channel Public;
        
        RelayServerInternal &Server;

        List <RelayServerInternal::Channel *>::Element * Element;

        Channel(RelayServerInternal &_Server) : Server(_Server)
        {
            Public.InternalTag    = this;
            Public.Tag            = 0;

            ID = Server.ChannelIDs.Borrow();
        }

        ~Channel()
        {
            Server.ChannelIDs.Return(ID);
        }

        List <RelayServerInternal::Client *> Clients;

        String Name;
    
        unsigned short ID;

        bool Hidden;
        bool AutoClose;
    
        Client * ChannelMaster;
        Client * ReadPeer(MessageReader &Reader);
        
        void RemoveClient(Client &);
        void Close();
    };
    
    Backlog<Lacewing::Server::Client, Client>
        ClientBacklog;

    Backlog<RelayServerInternal, Channel>
        ChannelBacklog;

    FrameBuilder Builder;

    String WelcomeMessage;

    List <Channel *> Channels;

    bool ChannelListingEnabled;

    void TimerTick()
    {
        Lacewing::Server &Socket = Server.Socket;
        List <RelayServerInternal::Client *> ToDisconnect;

        Builder.AddHeader(11, 0); /* Ping */
        
        for (Lacewing::Server::Client * ClientSocket = Socket.FirstClient (); ClientSocket; ClientSocket = ClientSocket->Next ())
        {
            RelayServerInternal::Client &Client = *(RelayServerInternal::Client *) ClientSocket->Tag;
            
            if (!Client.Ponged)
            {
                ToDisconnect.Push (&Client);
                continue;
            }

            Client.Ponged = false;
            Builder.Send (Client.Socket, false);
        }

        Builder.FrameReset();

        for(List <RelayServerInternal::Client *>::Element * E = ToDisconnect.First; E; E = E->Next)
            (** E)->Socket.Disconnect();
    }
};

void ServerMessageHandler (void * Tag, unsigned char Type, char * Message, int Size)
{   ((RelayServerInternal::Client *) Tag)->MessageHandler(Type, Message, Size, false);
}

void ServerTimerTick (Lacewing::Timer &Timer)
{   ((RelayServerInternal *) Timer.Tag)->TimerTick();
}

RelayServerInternal::Channel * RelayServerInternal::Client::ReadChannel(MessageReader &Reader)
{
    int ChannelID = Reader.Get <unsigned short> ();

    if(Reader.Failed)
        return 0;

    for(List <RelayServerInternal::Channel *>::Element * E = Channels.First; E; E = E->Next)
        if((** E)->ID == ChannelID)
            return ** E;
     
    Reader.Failed = true;
    return 0;
}

RelayServerInternal::Client * RelayServerInternal::Channel::ReadPeer(MessageReader &Reader)
{
    int PeerID = Reader.Get <unsigned short> ();

    if(Reader.Failed)
        return 0;

    for(List <RelayServerInternal::Client *>::Element * E = Clients.First; E; E = E->Next)
        if((** E)->ID == PeerID)
            return ** E;
     
    Reader.Failed = true;
    return 0;
}

void HandlerConnect(Lacewing::Server &Server, Lacewing::Server::Client &ClientSocket)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) Server.Tag;
     
    ClientSocket.Tag = &Internal;
    ClientSocket.Tag = &Internal.ClientBacklog.Borrow (ClientSocket);   
}

void HandlerDisconnect(Lacewing::Server &Server, Lacewing::Server::Client &ClientSocket)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) Server.Tag;
    RelayServerInternal::Client &Client  = *(RelayServerInternal::Client *) ClientSocket.Tag;

    for(List <RelayServerInternal::Channel *>::Element * E = Client.Channels.First; E; E = E->Next)
        (** E)->RemoveClient (Client);
    
    if(Client.Handshook && Internal.HandlerDisconnect)
        Internal.HandlerDisconnect(Internal.Server, Client.Public);

    Internal.ClientBacklog.Return(Client);
}

void HandlerReceive(Lacewing::Server &Server, Lacewing::Server::Client &ClientSocket, char * Data, int Size)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) Server.Tag;
    RelayServerInternal::Client &Client = *(RelayServerInternal::Client *) ClientSocket.Tag;
    
    if (!Client.GotFirstByte)
    {
        Client.GotFirstByte = true;

        ++ Data;

        if (!-- Size)
            return;
    }

    Client.Reader.Process (Data, Size);
}

void HandlerError(Lacewing::Server &Server, Lacewing::Error &Error)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) Server.Tag;

    Error.Add("Socket error");

    if(Internal.HandlerError)
        Internal.HandlerError(Internal.Server, Error);
}

void HandlerUDPReceive(Lacewing::UDP &UDP, Lacewing::Address &Address, char * Data, int Size)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) UDP.Tag;

    if(Size < (sizeof(unsigned short) + 1))
        return;

    unsigned char Type = *(unsigned char  *) Data;
    unsigned short ID  = *(unsigned short *) (Data + 1);

    Data += sizeof(unsigned short) + 1;
    Size -= sizeof(unsigned short) + 1;

    Lacewing::Server &Socket = Internal.Server.Socket;

    for (Lacewing::Server::Client * ClientSocket = Socket.FirstClient (); ClientSocket; ClientSocket = ClientSocket->Next ())
    {
        RelayServerInternal::Client &Client = *(RelayServerInternal::Client *) ClientSocket->Tag;
        
        if(Client.ID == ID)
        {
            if(Client.Socket.GetAddress().IP() != Address.IP())
                return;

            Client.UDPAddress.Port(Address.Port());
            Client.MessageHandler(Type, Data, Size, true);

            break;
        }
    }
}

void HandlerUDPError(Lacewing::UDP &UDP, Lacewing::Error &Error)
{
    RelayServerInternal &Internal = *(RelayServerInternal *) UDP.Tag;

    Error.Add("UDP socket error");

    if(Internal.HandlerError)
        Internal.HandlerError(Internal.Server, Error);
}

Lacewing::RelayServer::RelayServer(Lacewing::Pump &Pump) : Socket(Pump), UDP(Pump)
{
    LacewingInitialise();

    Socket.onConnect     (::HandlerConnect);
    Socket.onDisconnect  (::HandlerDisconnect);
    Socket.onReceive     (::HandlerReceive);
    Socket.onError       (::HandlerError);

    UDP.onReceive  (::HandlerUDPReceive);
    UDP.onError    (::HandlerUDPError);

    Socket.Tag = UDP.Tag = InternalTag = new RelayServerInternal(*this, Pump);
    
    Socket.DisableNagling ();
}

Lacewing::RelayServer::~RelayServer()
{
    Unhost();

    delete ((RelayServerInternal *) InternalTag);
}

void Lacewing::RelayServer::Host(int Port)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    Host(Filter);
}

void Lacewing::RelayServer::Host(Lacewing::Filter &_Filter)
{
    Lacewing::Filter Filter(_Filter);

    if(!Filter.LocalPort())
        Filter.LocalPort(6121);

    Socket.Host (Filter, true);
    UDP.Host    (Filter);

    ((RelayServerInternal *) InternalTag)->Timer.Start(5000);
}

void Lacewing::RelayServer::Unhost()
{
    Socket.Unhost();
    UDP.Unhost();

    ((RelayServerInternal *) InternalTag)->Timer.Stop();
}

bool Lacewing::RelayServer::Hosting()
{
    return Socket.Hosting();
}

int Lacewing::RelayServer::Port()
{
    return Socket.Port();
}

void RelayServerInternal::Channel::Close()
{
    FrameBuilder &Builder = Server.Builder;

    /* Tell all the clients that they've left, and remove this channel from their channel lists. */

    Builder.AddHeader   (0, 0, false); /* Response */
    Builder.Add <unsigned char>   (3); /* LeaveChannel */
    Builder.Add <unsigned char>   (1); /* Success */
    Builder.Add <unsigned short> (ID);

    for(List <RelayServerInternal::Client *>::Element * E = Clients.First;
            E; E = E->Next)
    {
        RelayServerInternal::Client &Client = *** E;
        Builder.Send(Client.Socket, false);

        for(List <RelayServerInternal::Channel *>::Element * E2 = Client.Channels.First;
                E2; E2 = E2->Next)
        {
            if(** E2 == this)
            {
                Client.Channels.Erase (E2);
                break;
            }
        }
    }

    Builder.FrameReset();

    
    /* Remove this channel from the channel list and return it to the backlog. */

    for(List <RelayServerInternal::Channel *>::Element * E = Server.Channels.First;
            E; E = E->Next)
    {
        if(** E == this)
        {
            Server.Channels.Erase(E);
            break;
        }
    }
    
    Server.ChannelBacklog.Return(*this);
}

void RelayServerInternal::Channel::RemoveClient(RelayServerInternal::Client &Client)
{
    for(List <RelayServerInternal::Client *>::Element * E = Clients.First;
            E; E = E->Next)
    {
        if(** E == &Client)
        {
            Clients.Erase (E);
            break;
        }
    }

    if((!Clients.Size) || (ChannelMaster == &Client && AutoClose))
    {   
        Close ();
        return;
    }

    if(ChannelMaster == &Client)
        ChannelMaster = 0;


    FrameBuilder &Builder = Server.Builder;

    /* Notify all the other peers that this client has left the channel */

    Builder.AddHeader (9, 0); /* Peer */
    
    Builder.Add <unsigned short> (ID);
    Builder.Add <unsigned short> (Client.ID);

    for(List <RelayServerInternal::Client *>::Element * E = Clients.First;
            E; E = E->Next)
    {
        Builder.Send((** E)->Socket, false);
    }

    Builder.FrameReset();
}

bool RelayServerInternal::Client::CheckName (const char * Name)
{
    for(List <RelayServerInternal::Channel *>::Element * E = Channels.First;
            E; E = E->Next)
    {
        RelayServerInternal::Channel * Channel = ** E;

        for(List <RelayServerInternal::Client *>::Element * E2 = Channel->Clients.First;
                E2; E2 = E2->Next)
        {
            if ((** E2) == this)
                continue;

            if(!strcasecmp((** E2)->Name, Name))
            {
                FrameBuilder &Builder = Server.Builder;

                Builder.AddHeader        (0, 0);  /* Response */
                Builder.Add <unsigned char> (1);  /* SetName */
                Builder.Add <unsigned char> (0);  /* Failed */

                Builder.Add <unsigned char> (strlen(Name));
                Builder.Add (Name, -1);

                Builder.Add ("Name already taken", -1);

                Builder.Send(Socket);

                return false;
            }
        }
    }

    return true;
}

void RelayServerInternal::Client::MessageHandler(unsigned char Type, char * Message, int Size, bool Blasted)
{
    unsigned char MessageTypeID  = (Type >> 4);
    unsigned char Variant        = (Type << 4);

    Variant >>= 4;

    MessageReader Reader (Message, Size);
    FrameBuilder &Builder = Server.Builder;

    if(MessageTypeID != 0 && !Handshook)
    {
        Socket.Disconnect();
        return;
    }

    switch(MessageTypeID)
    {
        case 0: /* Request */
        {
            unsigned char RequestType = Reader.Get <unsigned char> ();

            if(Reader.Failed)
                break;

            if(RequestType != 0 && !Handshook)
            {
                Reader.Failed = true;
                break;
            }

            switch(RequestType)
            {
                case 0: /* Connect */
                {
                    const char * Version = Reader.GetRemaining ();

                    if(Reader.Failed)
                        break;

                    if(Handshook)
                    {
                        Reader.Failed = true;
                        break;
                    }

                    if(strcmp(Version, "revision 3"))
                    {
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (0);  /* Connect */
                        Builder.Add <unsigned char> (0);  /* Failed */
                        Builder.Add ("Version mismatch", -1);

                        Builder.Send(Socket);

                        Reader.Failed = true;
                        break;
                    }

                    if(Server.HandlerConnect && !Server.HandlerConnect(Server.Server, Public))
                    {
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (0);  /* Connect */
                        Builder.Add <unsigned char> (0);  /* Failed */
                        Builder.Add ("Connection refused by server", -1);

                        Builder.Send(Socket);

                        Reader.Failed = true;
                        break;
                    }

                    Handshook = true;

                    Builder.AddHeader          (0, 0);  /* Response */
                    Builder.Add <unsigned char>   (0);  /* Connect */
                    Builder.Add <unsigned char>   (1);  /* Success */
                    
                    Builder.Add <unsigned short> (ID);
                    Builder.Add (Server.WelcomeMessage);

                    Builder.Send(Socket);

                    break;
                }

                case 1: /* SetName */
                {
                    const char * Name = Reader.GetRemaining (false);

                    if(Reader.Failed)
                        break;

                    if (!CheckName (Name))
                        break;

                    String NameCopy = this->Name;

                    /* The .Name() setter will also set NameAltered to true.  This means that if the
                       handler sets the name explicitly, the default behaviour of setting the name to
                       the requested one will be skipped. */

                    NameAltered = false;

                    if(Server.HandlerSetName && !Server.HandlerSetName(Server.Server, Public, Name))
                    {
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (1);  /* SetName */
                        Builder.Add <unsigned char> (0);  /* Failed */
                        
                        Builder.Add <unsigned char> (strlen(Name));
                        Builder.Add (Name, -1);

                        Builder.Add ("Name refused by server", -1);

                        Builder.Send(Socket);

                        break;
                    }

                    if (!NameAltered)
                    {
                        this->Name = Name;
                    }
                    else
                    {
                        /* Check the new name provided by the handler */

                        if (!CheckName (this->Name))
                        {
                            this->Name = NameCopy;
                            break;
                        }
                    }

                    Builder.AddHeader        (0, 0);  /* Response */
                    Builder.Add <unsigned char> (1);  /* SetName */
                    Builder.Add <unsigned char> (1);  /* Success */
                
                    Builder.Add <unsigned char> (this->Name.Length);
                    Builder.Add (this->Name);

                    Builder.Send(Socket);

                    for(List <RelayServerInternal::Channel *>::Element * E = Channels.First;
                            E; E = E->Next)
                    {
                        RelayServerInternal::Channel * Channel = ** E;

                        Builder.AddHeader (9, 0); /* Peer */
                        
                        Builder.Add <unsigned short> (Channel->ID);
                        Builder.Add <unsigned short> (ID);
                        Builder.Add <unsigned char>  (this == Channel->ChannelMaster ? 1 : 0);
                        Builder.Add (this->Name);

                        for(List <RelayServerInternal::Client *>::Element * E2 = Channel->Clients.First;
                                E2; E2 = E2->Next)
                        {
                            if(** E2 == this)
                                continue;

                            Builder.Send((** E2)->Socket, false);
                        }

                        Builder.FrameReset ();
                    }

                    break;
                }

                case 2: /* JoinChannel */
                {            
                    if(!this->Name.Length)
                        Reader.Failed = true;

                    unsigned char Flags = Reader.Get <unsigned char> ();
                    char *        Name  = Reader.GetRemaining(false);
                    
                    if(Reader.Failed)
                        break;

                    RelayServerInternal::Channel * Channel = 0;

                    for(List <RelayServerInternal::Channel *>::Element * E = Server.Channels.First;
                            E; E = E->Next)
                    {
                        if(!strcasecmp ((** E)->Name, Name))
                        {
                            Channel = ** E;
                            break;
                        }
                    }
                    
                    if(Channel)
                    {
                        /* Joining an existing channel */

                        bool NameTaken = false;

                        for(List <RelayServerInternal::Client *>::Element * E = Channel->Clients.First;
                                E; E = E->Next)
                        {
                            RelayServerInternal::Client * Client = ** E;
    
                            if(!strcasecmp (Client->Name, this->Name))
                            {
                                NameTaken = true;
                                break;
                            }
                        }

                        if(NameTaken)
                        {
                            Builder.AddHeader        (0, 0);  /* Response */
                            Builder.Add <unsigned char> (2);  /* JoinChannel */
                            Builder.Add <unsigned char> (0);  /* Failed */

                            Builder.Add <unsigned char> (strlen(Name));
                            Builder.Add (Name, -1);

                            Builder.Add ("Name already taken", -1);

                            Builder.Send(Socket);

                            break;
                        }

                        if(Server.HandlerJoinChannel && !Server.HandlerJoinChannel(Server.Server, Public, Channel->Public))
                        {
                            Builder.AddHeader        (0, 0);  /* Response */
                            Builder.Add <unsigned char> (2);  /* JoinChannel */
                            Builder.Add <unsigned char> (0);  /* Failed */

                            Builder.Add <unsigned char> (Channel->Name.Length);
                            Builder.Add (Channel->Name);

                            Builder.Add ("Join refused by server", -1);

                            Builder.Send(Socket);
                            
                            break;
                        }
                    
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (2);  /* JoinChannel */
                        Builder.Add <unsigned char> (1);  /* Success */
                        Builder.Add <unsigned char> (0);  /* Not the channel master */

                        Builder.Add <unsigned char> (Channel->Name.Length);
                        Builder.Add (Channel->Name);

                        Builder.Add <unsigned short> (Channel->ID);
                        
                        for(List <RelayServerInternal::Client *>::Element * E = Channel->Clients.First;
                                E; E = E->Next)
                        {
                            RelayServerInternal::Client * Client = ** E;

                            Builder.Add <unsigned short> (Client->ID);
                            Builder.Add <unsigned char>  (Channel->ChannelMaster == Client ? 1 : 0);
                            Builder.Add <unsigned char>  (Client->Name.Length);
                            Builder.Add (Client->Name);
                        }

                        Builder.Send(Socket);


                        Builder.AddHeader (9, 0); /* Peer */
                        
                        Builder.Add <unsigned short> (Channel->ID);
                        Builder.Add <unsigned short> (ID);
                        Builder.Add <unsigned char>  (0);
                        Builder.Add (this->Name);

                        /* Notify the other clients on the channel that this client has joined */

                        for(List <RelayServerInternal::Client *>::Element * E = Channel->Clients.First;
                                E; E = E->Next)
                        {
                            Builder.Send((** E)->Socket, false);
                        }

                        Builder.FrameReset();


                        /* Add this client to the channel */

                        Channels.Push (Channel);
                        Channel->Clients.Push (this);

                        break;
                    }

                    /* Creating a new channel */

                    Channel = &Server.ChannelBacklog.Borrow(Server);

                    Channel->Name          =  Name;
                    Channel->ChannelMaster =  this;
                    Channel->Hidden        =  (Flags & 1) != 0;
                    Channel->AutoClose     =  (Flags & 2) != 0;

                    if(Server.HandlerJoinChannel && !Server.HandlerJoinChannel(Server.Server, Public, Channel->Public))
                    {
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (2);  /* JoinChannel */
                        Builder.Add <unsigned char> (0);  /* Failed */

                        Builder.Add <unsigned char> (Channel->Name.Length);
                        Builder.Add (Channel->Name);

                        Builder.Add ("Join refused by server", -1);

                        Builder.Send(Socket);
                        
                        Server.ChannelBacklog.Return(*Channel);
                        break;
                    }

                    Channel->Element = Server.Channels.Push (Channel);

                    Channel->Clients.Push (this);
                    Channels.Push (Channel);

                    Builder.AddHeader        (0, 0);  /* Response */
                    Builder.Add <unsigned char> (2);  /* JoinChannel */
                    Builder.Add <unsigned char> (1);  /* Success */
                    Builder.Add <unsigned char> (1);  /* Channel master */

                    Builder.Add <unsigned char> (Channel->Name.Length);
                    Builder.Add (Channel->Name);

                    Builder.Add <unsigned short> (Channel->ID);

                    Builder.Send(Socket);

                    break;
                }

                case 3: /* LeaveChannel */
                {
                    RelayServerInternal::Channel * Channel = ReadChannel(Reader);

                    if(Reader.Failed)
                        break;

                    if(Server.HandlerLeaveChannel && !Server.HandlerLeaveChannel(Server.Server, Public, Channel->Public))
                    {
                        Builder.AddHeader         (0, 0);  /* Response */
                        Builder.Add <unsigned char>  (3);  /* LeaveChannel */
                        Builder.Add <unsigned char>  (0);  /* Failed */
                        Builder.Add <unsigned short> (Channel->ID);

                        Builder.Add ("Leave refused by server", -1);

                        Builder.Send(Socket);

                        break;
                    }

                    for(List <RelayServerInternal::Channel *>::Element * E = Channels.First; E; E = E->Next)
                    {
                        if(** E == Channel)
                        {
                            Channels.Erase(E);
                            break;
                        }
                    } 

                    Builder.AddHeader         (0, 0);  /* Response */
                    Builder.Add <unsigned char>  (3);  /* LeaveChannel */
                    Builder.Add <unsigned char>  (1);  /* Success */
                    Builder.Add <unsigned short> (Channel->ID);

                    Builder.Send(Socket);

                    /* Do this last, because it might delete the channel */

                    Channel->RemoveClient(*this);

                    break;
                }

                case 4: /* ChannelList */

                    if (!Server.ChannelListingEnabled)
                    {
                        Builder.AddHeader        (0, 0);  /* Response */
                        Builder.Add <unsigned char> (4);  /* ChannelList */
                        Builder.Add <unsigned char> (0);  /* Failed */
                        
                        Builder.Add ("Channel listing is not enabled on this server");

                        Builder.Send (Socket);

                        break;
                    }

                    Builder.AddHeader        (0, 0);  /* Response */
                    Builder.Add <unsigned char> (4);  /* ChannelList */
                    Builder.Add <unsigned char> (1);  /* Success */

                    for(List <RelayServerInternal::Channel *>::Element * E = Server.Channels.First;
                            E; E = E->Next)
                    {
                        RelayServerInternal::Channel * Channel = ** E;

                        if(Channel->Hidden)
                            continue;

                        Builder.Add <unsigned short> (Channel->Clients.Size);
                        Builder.Add <unsigned char>  (Channel->Name.Length);
                        Builder.Add (Channel->Name);
                    }

                    Builder.Send(Socket);

                    break;

                default:
                    
                    Reader.Failed = true;
                    break;
            }

            break;
        }

        case 1: /* BinaryServerMessage */
        {
            unsigned char Subchannel = Reader.Get <unsigned char> ();
            
            char * Message;
            unsigned int Size;

            Reader.GetRemaining(Message, Size);
            
            if(Reader.Failed)
                break;

            if(Server.HandlerServerMessage)
                Server.HandlerServerMessage(Server.Server, Public, Blasted, Subchannel, Message, Size, Variant);

            break;
        }

        case 2: /* BinaryChannelMessage */
        {
            unsigned char Subchannel = Reader.Get <unsigned char> ();
            RelayServerInternal::Channel * Channel = ReadChannel (Reader);
            
            char * Message;
            unsigned int Size;

            Reader.GetRemaining(Message, Size);
            
            if(Reader.Failed)
                break;

            if(Server.HandlerChannelMessage && !Server.HandlerChannelMessage(Server.Server, Public, Channel->Public,
                        Blasted, Subchannel, Message, Size, Variant))
            {
                break;
            }

            Builder.AddHeader (2, Variant, Blasted); /* BinaryChannelMessage */
            
            Builder.Add <unsigned char>  (Subchannel);
            Builder.Add <unsigned short> (Channel->ID);
            Builder.Add <unsigned short> (ID);
            Builder.Add (Message, Size);

            for(List <RelayServerInternal::Client *>::Element * E = Channel->Clients.First; E; E = E->Next)
            {
                if(** E == this)
                    continue;

                if(Blasted)
                    Builder.Send(Server.Server.UDP, (** E)->UDPAddress, false);
                else
                    Builder.Send((** E)->Socket, false);
            }

            Builder.FrameReset();

            break;
        }

        case 3: /* BinaryPeerMessage */
        {
            unsigned char Subchannel          = Reader.Get <unsigned char> ();
            RelayServerInternal::Channel * Channel = ReadChannel      (Reader);
            RelayServerInternal::Client  * Peer    = Channel->ReadPeer(Reader);

            if(Peer == this)
            {
                Reader.Failed = true;
                break;
            }

            char * Message;
            unsigned int Size;

            Reader.GetRemaining(Message, Size);
            
            if(Reader.Failed)
                break;

            if(Server.HandlerPeerMessage && !Server.HandlerPeerMessage(Server.Server, Public, Channel->Public,
                Peer->Public, Blasted, Subchannel, Message, Size, Variant))
            {
                break;
            }

            Builder.AddHeader (3, Variant, Blasted); /* BinaryPeerMessage */
            
            Builder.Add <unsigned char>  (Subchannel);
            Builder.Add <unsigned short> (Channel->ID);
            Builder.Add <unsigned short> (ID);
            Builder.Add (Message, Size);

            if(Blasted)
                Builder.Send(Server.Server.UDP, Peer->UDPAddress);
            else
                Builder.Send(Peer->Socket);

            break;
        }
            
        case 4: /* ObjectServerMessage */

            break;
            
        case 5: /* ObjectChannelMessage */

            break;
            
        case 6: /* ObjectPeerMessage */

            break;
            
        case 7: /* UDPHello */

            if(!Blasted)
            {
                Reader.Failed = true;
                break;
            }

            Builder.AddHeader (10, 0); /* UDPWelcome */
            Builder.Send      (Server.Server.UDP, UDPAddress);

            break;
            
        case 8: /* ChannelMaster */

            break;

        case 9: /* Ping */

            Ponged = true;
            break;

        default:

            Reader.Failed = true;
            break;
    };

    if(Reader.Failed)
    {
        /* Socket.Disconnect(); */
    }
}

void Lacewing::RelayServer::Client::Send(int Subchannel, const char * Message, int Size, int Variant)
{
    RelayServerInternal::Client &Internal = *(RelayServerInternal::Client *) InternalTag;
    FrameBuilder &Builder = Internal.Server.Builder;

    Builder.AddHeader (1, Variant); /* BinaryServerMessage */
    
    Builder.Add <unsigned char> (Subchannel);
    Builder.Add (Message, Size);

    Builder.Send (Internal.Socket);
}

void Lacewing::RelayServer::Client::Blast(int Subchannel, const char * Message, int Size, int Variant)
{
    RelayServerInternal::Client &Internal = *(RelayServerInternal::Client *) InternalTag;
    FrameBuilder &Builder = Internal.Server.Builder;

    Builder.AddHeader (1, Variant, true); /* BinaryServerMessage */
    
    Builder.Add <unsigned char> (Subchannel);
    Builder.Add (Message, Size);

    Builder.Send (Internal.Server.Server.UDP, Internal.UDPAddress);
}

void Lacewing::RelayServer::Channel::Send(int Subchannel, const char * Message, int Size, int Variant)
{
    RelayServerInternal::Channel &Internal = *(RelayServerInternal::Channel *) InternalTag;
    FrameBuilder &Builder = Internal.Server.Builder;

    Builder.AddHeader (4, Variant); /* BinaryServerChannelMessage */
    
    Builder.Add <unsigned char> (Subchannel);
    Builder.Add <unsigned short> (Internal.ID);
    Builder.Add (Message, Size);

    for (List <RelayServerInternal::Client *>::Element *
                E = Internal.Clients.First; E; E = E->Next)
    {
        Builder.Send ((** E)->Socket, false);
    }

    Builder.FrameReset ();
}

void Lacewing::RelayServer::Channel::Blast(int Subchannel, const char * Message, int Size, int Variant)
{
    RelayServerInternal::Channel &Internal = *(RelayServerInternal::Channel *) InternalTag;
    FrameBuilder &Builder = Internal.Server.Builder;

    Builder.AddHeader (4, Variant, true); /* BinaryServerChannelMessage */
    
    Builder.Add <unsigned char> (Subchannel);
    Builder.Add <unsigned short> (Internal.ID);
    Builder.Add (Message, Size);

    for (List <RelayServerInternal::Client *>::Element *
                E = Internal.Clients.First; E; E = E->Next)
    {
        Builder.Send (Internal.Server.Server.UDP, (** E)->UDPAddress, false);
    }

    Builder.FrameReset ();
}

int Lacewing::RelayServer::Client::ID()
{
    return ((RelayServerInternal::Client *) InternalTag)->ID;
}

const char * Lacewing::RelayServer::Channel::Name()
{
    return ((RelayServerInternal::Channel *) InternalTag)->Name;
}

void Lacewing::RelayServer::Channel::Name(const char * Name)
{
    ((RelayServerInternal::Channel *) InternalTag)->Name = Name;
}

bool Lacewing::RelayServer::Channel::Hidden()
{
    return ((RelayServerInternal::Channel *) InternalTag)->Hidden;
}

bool Lacewing::RelayServer::Channel::AutoCloseEnabled()
{
    return ((RelayServerInternal::Channel *) InternalTag)->AutoClose;
}

void Lacewing::RelayServer::SetWelcomeMessage(const char * Message)
{
    ((RelayServerInternal *) InternalTag)->WelcomeMessage = Message;
}

void Lacewing::RelayServer::SetChannelListing (bool Enabled)
{
    ((RelayServerInternal *) InternalTag)->ChannelListingEnabled = Enabled;
}

Lacewing::RelayServer::Client * Lacewing::RelayServer::Channel::ChannelMaster()
{
    RelayServerInternal::Client * Client = ((RelayServerInternal::Channel *) InternalTag)->ChannelMaster;

    return Client ? &Client->Public : 0;
}

void Lacewing::RelayServer::Channel::Close()
{
    ((RelayServerInternal::Channel *) InternalTag)->Close();
}

void Lacewing::RelayServer::Client::Disconnect()
{
    ((RelayServerInternal::Client *) InternalTag)->Socket.Disconnect();
}

Lacewing::Address &Lacewing::RelayServer::Client::GetAddress()
{
    return ((RelayServerInternal::Client *) InternalTag)->Socket.GetAddress();
}

const char * Lacewing::RelayServer::Client::Name()
{
    return ((RelayServerInternal::Client *) InternalTag)->Name;
}

void Lacewing::RelayServer::Client::Name(const char * Name)
{
    RelayServerInternal::Client &Internal = *(RelayServerInternal::Client *) InternalTag;

    Internal.Name = Name;
    Internal.NameAltered = true;
}

int Lacewing::RelayServer::ChannelCount()
{
    return ((RelayServerInternal *) InternalTag)->Channels.Size;
}

int Lacewing::RelayServer::Channel::ClientCount()
{
    return ((RelayServerInternal::Channel *) InternalTag)->Clients.Size;
}

int Lacewing::RelayServer::Client::ChannelCount()
{
    return ((RelayServerInternal::Client *) InternalTag)->Channels.Size;
}

int Lacewing::RelayServer::ClientCount()
{
    return Socket.ClientCount ();
}

Lacewing::RelayServer::Client * Lacewing::RelayServer::FirstClient ()
{
    return Socket.FirstClient () ?
        (Lacewing::RelayServer::Client *) Socket.FirstClient ()->Tag : 0;
}

Lacewing::RelayServer::Client * Lacewing::RelayServer::Client::Next ()
{
    Lacewing::Server::Client * NextSocket =
        ((RelayServerInternal::Client *) InternalTag)->Socket.Next ();

    return NextSocket ? (Lacewing::RelayServer::Client *) NextSocket->Tag : 0;
}

Lacewing::RelayServer::Channel * Lacewing::RelayServer::Channel::Next ()
{
    return ((RelayServerInternal::Channel *) InternalTag)->Element->Next ?
        &(** ((RelayServerInternal::Channel *) InternalTag)->Element->Next)->Public : 0;
}

Lacewing::RelayServer::Channel * Lacewing::RelayServer::FirstChannel ()
{
    return ((RelayServerInternal *) InternalTag)->Channels.First ?
            &(** ((RelayServerInternal *) InternalTag)->Channels.First)->Public : 0;
}

Lacewing::RelayServer::Channel::ClientIterator::ClientIterator
    (Lacewing::RelayServer::Channel &Channel)
{
    InternalTag = ((RelayServerInternal::Channel *) Channel.InternalTag)->Clients.First;
}

Lacewing::RelayServer::Client * Lacewing::RelayServer::Channel::ClientIterator::Next ()
{
    List <Lacewing::RelayServer::Client *>::Element * E =
        (List <Lacewing::RelayServer::Client *>::Element *) InternalTag;

    if (!E)
        return 0;

    InternalTag = E->Next;
    return ** E;
}

Lacewing::RelayServer::Client::ChannelIterator::ChannelIterator
    (Lacewing::RelayServer::Client &Client)
{
    InternalTag = ((RelayServerInternal::Client *) Client.InternalTag)->Channels.First;
}

Lacewing::RelayServer::Channel * Lacewing::RelayServer::Client::ChannelIterator::Next ()
{
    List <Lacewing::RelayServer::Channel *>::Element * E =
        (List <Lacewing::RelayServer::Channel *>::Element *) InternalTag;

    if (!E)
        return 0;

    InternalTag = E->Next;
    return ** E;
}

AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, Connect)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, Disconnect)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, Error)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, ServerMessage)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, ChannelMessage)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, PeerMessage)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, JoinChannel)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, LeaveChannel)
AutoHandlerFunctions(Lacewing::RelayServer, RelayServerInternal, SetName)

