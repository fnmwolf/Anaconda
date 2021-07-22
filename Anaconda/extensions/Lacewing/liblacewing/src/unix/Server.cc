
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
#include "../QueuedSend.h"
#include "SendFile.h"

struct ServerInternal;

struct ServerClientInternal
{
    ServerInternal &Server;

    ServerClientInternal(ServerInternal &_Server) : Server(_Server)
    {
        Public.InternalTag = this;
        Public.Tag = 0;

        Context = 0;
        SSLReadWhenWriteReady = false;

        Transfer = 0;
    }

    ~ServerClientInternal()
    {
        delete Address;

        if(Context)
            SSL_free(Context);
    }

    Lacewing::Server::Client Public;
    Lacewing::Address * Address;

    List <ServerClientInternal *>::Element * Element;

    int Socket;
    void * GoneKey;
    
    SSL * Context;
    BIO * SSLBio;

    bool SSLReadWhenWriteReady;

    QueuedSendManager QueuedSends;

    bool Send         (QueuedSend * Queued, const char * Data, int Size);
    bool SendFile     (bool AllowQueue, const char * Filename, lw_i64 Offset, lw_i64 Size);
    bool SendWritable (bool AllowQueue, char * Data, int Size);
    
    void DoNextQueued()
    {
        for(;;)
        {
            QueuedSend * First = QueuedSends.First;

            if(First && First->Type == QueuedSendType::Data)
            {
                bool SentImmediately = Send(First, First->Data.Buffer + First->DataOffset,
                                                     First->Data.Size - First->DataOffset);

                if(SentImmediately)
                {
                    QueuedSends.First = First->Next;
                    delete First;

                    if(!QueuedSends.First)
                        QueuedSends.Last = 0;
                }
                else
                {
                    /* The new Send() didn't complete immediately, so we left it in the queue.
                       Relying on the completion of that to send the rest of the queued files/data.  */
                    
                    break;
                }
            }
            
            if(First = QueuedSends.First)
            {
                if (First->Type == QueuedSendType::Disconnect)
                {
                    Terminate ();
                    break;
                }
                else if (First->Type == QueuedSendType::File)
                {
                    bool SentImmediately = SendFile(false, First->Filename, First->FileOffset, First->FileSize);
                    
                    QueuedSends.First = First->Next;
                    delete First;

                    if(!QueuedSends.First)
                        QueuedSends.Last = 0;
                        
                    if(!SentImmediately)
                    {
                        /* The new SendFile() didn't complete immediately, and is now in the queue.
                           Relying on the completion of that to send the rest of the queued files/data. */
                        
                        break;
                    }
                }

                continue;
            }

            /* Nothing left */

            Public.Flush();
            break;
        }
    }
    
    FileTransfer * Transfer;

    void Terminate ();
};

struct ServerInternal
{
    Lacewing::Server &Server;

    int Port;
    int Socket;

    PumpInternal &EventPump;
    
    Lacewing::Server::HandlerConnect    HandlerConnect;
    Lacewing::Server::HandlerDisconnect HandlerDisconnect;
    Lacewing::Server::HandlerReceive    HandlerReceive;
    Lacewing::Server::HandlerError      HandlerError;

    ServerInternal(Lacewing::Server &_Server, PumpInternal &_EventPump)
            : Server(_Server), EventPump(_EventPump)
    {
        Socket  = -1;
        Context =  0;
        
        HandlerConnect    = 0;
        HandlerDisconnect = 0;
        HandlerReceive    = 0;
        HandlerError      = 0;

        Nagle = true;

        BytesReceived = 0;
    }

    ~ServerInternal()
    {
    }
    
    Backlog <ServerInternal, ServerClientInternal>
        ClientStructureBacklog;
        
    List <ServerClientInternal *> Clients;

    SSL_CTX * Context;

    char Passphrase[128];
    bool Nagle;

    lw_i64 BytesReceived;

    /* When multithreading support is added, there will be one ReceiveBuffer per client.
       Until then, we can save RAM by having a single ReceiveBuffer global to the server. */

    ReceiveBuffer Buffer;
};
    
void ServerClientInternal::Terminate ()
{
    DebugOut ("Terminate %d", &Public);

    shutdown (Socket, SHUT_RDWR);
    close (Socket);

    Server.EventPump.Remove (GoneKey);
    Socket = -1;
    
    if(Server.HandlerDisconnect)
        Server.HandlerDisconnect(Server.Server, Public);

    Server.Clients.Erase (Element);
    Server.ClientStructureBacklog.Return (*this);
}

Lacewing::Server::Server(Lacewing::Pump &Pump)
{
    LacewingInitialise();
    
    InternalTag = new ServerInternal(*this, *(PumpInternal *) Pump.InternalTag);
    Tag = 0;
}

Lacewing::Server::~Server()
{
    Unhost();

    delete ((ServerInternal *) InternalTag);
}

void ClientSocketReadReady (ServerClientInternal &Client)
{
    ServerInternal &Internal = Client.Server;
    
    if(Client.Transfer)
    {
        if(!Client.Transfer->ReadReady())
        {
            delete Client.Transfer;
            Client.Transfer = 0;

            Client.DoNextQueued();
        }
    }

    /* Data is ready to receive */

    int Received;
    bool Disconnected = false;
    
    for(;;)
    {
        Internal.Buffer.Prepare();

        if(!Internal.Context)
        {
            /* Normal receive */

            Received = recv(Client.Socket, Internal.Buffer, Internal.Buffer.Size, MSG_DONTWAIT);
            
            if(Received < 0)
            {
                Disconnected = (errno != EAGAIN && errno != EWOULDBLOCK);
                break;
            }
            
            if (Received == 0)
            {
                Disconnected = true;
                break;
            }
        }
        else
        {
            /* SSL receive - first we need to check if a previous SSL_write gave an SSL_ERROR_WANT_READ */
            
            {   QueuedSend * First = Client.QueuedSends.First;
            
                if(First && First->Type == QueuedSendType::SSLWriteWhenReadable)
                {
                    int Error = SSL_write(Client.Context, First->Data.Buffer, First->Data.Size);
                
                    if(Error == SSL_ERROR_NONE)
                    {
                        Client.DoNextQueued();
                        break;
                    }

                    switch(SSL_get_error(Client.Context, Error))
                    {
                        case SSL_ERROR_WANT_READ:
                        
                            /* Not enough incoming data to do some SSL stuff.  The outgoing data will
                               have to wait until next time the socket is readable. */

                            break;
                        
                        case SSL_ERROR_WANT_WRITE:
                        
                            /* Although we've now satisfied the "readable" condition, the socket isn't
                               actually writable.  Change the queued type to regular data, which will
                               be written next time the socket is writable. */
                            
                            First->Type = QueuedSendType::Data;
                            break;
                            
                        default:
                            
                            Disconnected = true;
                            break;
                    };                    
                }                   
            }
            
            /* Now do the actual read */

            Received = SSL_read(Client.Context, Internal.Buffer, Internal.Buffer.Size);

            if(Received == 0)
            {
                close (Client.Socket);
                Disconnected = true;

                break;
            }
            
            if(Received < 0)
            {
                int Error = SSL_get_error(Client.Context, Received);
                
                if(Error == SSL_ERROR_WANT_READ)
                {
                    /* We'll call SSL_read again as soon as more data is available
                       anyway, so there's nothing left to do here. */

                    return;
                }

                if(Error == SSL_ERROR_WANT_WRITE)
                {
                    /* SSL_read wants to send some data, but the socket isn't ready
                       for writing at the moment. */

                    Client.SSLReadWhenWriteReady = true;
                    return;
                }

                /* Unknown error */
                
                close (Client.Socket);
                Disconnected = true;

                break;
            }
        }

        Internal.Buffer.Received(Received);
        Internal.BytesReceived += Received;

        if(Internal.HandlerReceive)
            Internal.HandlerReceive(Internal.Server, Client.Public, Internal.Buffer, Received);
    }
    
    if (Disconnected)
    {
        Client.Terminate ();
    }
}

void ClientSocketWriteReady(ServerClientInternal &Client)
{
    if(Client.SSLReadWhenWriteReady)
    {
        Client.SSLReadWhenWriteReady = false;
        ClientSocketReadReady(Client);
    }

    ServerInternal &Internal = Client.Server;

    if(Client.Transfer)
    {
        if(!Client.Transfer->WriteReady())
        {
            delete Client.Transfer;
            Client.Transfer = 0;

            Client.DoNextQueued();
        }
    }
    else
    {
        if(Client.QueuedSends.First && Client.QueuedSends.First->Type == QueuedSendType::Data && !Client.Transfer)
        {
            /* Data has been queued because it couldn't be sent immediately,
               not because of a currently executing file transfer */

            Client.DoNextQueued();
        }
    }
}

void ListenSocketReadReady(ServerInternal &Internal, bool)
{
    sockaddr_in Address;
    socklen_t AddressLength = sizeof(Address);
    
    for(;;)
    {
        int Socket = accept(Internal.Socket, (sockaddr *) &Address, &AddressLength);
        
        if(Socket == -1)
            break; 
        
        fcntl(Socket, F_SETFL, fcntl(Socket, F_GETFL, 0) | O_NONBLOCK);
        
        DisableSigPipe (Socket);

        if (!Internal.Nagle)
            DisableNagling (Socket);

        ServerClientInternal &Client = Internal.ClientStructureBacklog.Borrow(Internal);

        Client.Socket = Socket;
        Client.Address = new Lacewing::Address(Address.sin_addr.s_addr);

        if(Internal.Context)
        {
            Client.Context = SSL_new(Internal.Context);

            Client.SSLBio = BIO_new_socket(Client.Socket, BIO_NOCLOSE);
            SSL_set_bio(Client.Context, Client.SSLBio, Client.SSLBio);
            
            SSL_set_accept_state(Client.Context);
        }
        
        if(Internal.HandlerConnect)
            Internal.HandlerConnect(Internal.Server, Client.Public);
        
        Client.GoneKey = Internal.EventPump.AddReadWrite(Client.Socket, &Client,
                        (void *) ClientSocketReadReady, (void *) ClientSocketWriteReady);
        
        Client.Element = Internal.Clients.Push (&Client);
    }
}

void Lacewing::Server::Host(int Port, bool ClientSpeaksFirst)
{
    Lacewing::Filter Filter;
    Filter.LocalPort(Port);

    Host(Filter, ClientSpeaksFirst);
}

void Lacewing::Server::Host(Lacewing::Filter &Filter, bool)
{
    Unhost();
    
    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    if(!(Internal.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        Lacewing::Error Error;

        Error.Add(errno);
        Error.Add("Error creating socket");
        
        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    DisableSigPipe (Internal.Socket);

    if((!CertificateLoaded()) && (!Internal.Nagle))
        ::DisableNagling(Internal.Socket);

    fcntl(Internal.Socket, F_SETFL, fcntl(Internal.Socket, F_GETFL, 0) | O_NONBLOCK);
       
    {   int reuse = Filter.Reuse() ? 1 : 0;
        setsockopt(Internal.Socket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));
    }

    sockaddr_in Address;
    memset(&Address, 0, sizeof(Address));
    
    Address.sin_family = AF_INET;
    Address.sin_port = htons(Filter.LocalPort() ? Filter.LocalPort() : 0);
    Address.sin_addr.s_addr = Filter.LocalIP() ? Filter.LocalIP() : htonl(INADDR_ANY);
    
    if(bind(Internal.Socket, (sockaddr *) &Address, sizeof(Address)) == -1)
    {
        Lacewing::Error Error;
        
        Error.Add(errno);
        Error.Add("Error binding port");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);
        
        return;
    }

    if(listen(Internal.Socket, SOMAXCONN) == -1)
    {
        Lacewing::Error Error;
        
        Error.Add(errno);
        Error.Add("Error listening");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);
        
        return;
    }
    
    Internal.EventPump.AddRead(Internal.Socket, &Internal, (void *) ListenSocketReadReady);
    
    socklen_t AddressLength = sizeof(sockaddr_in);
    getsockname(Internal.Socket, (sockaddr *) &Address, &AddressLength);

    Internal.Port = ntohs(Address.sin_port);
}

void Lacewing::Server::Unhost()
{
    if(!Hosting())
        return;

    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    close(Internal.Socket);
    Internal.Socket = -1;
}

bool Lacewing::Server::Hosting()
{
    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    return Internal.Socket != -1;
}

int Lacewing::Server::ClientCount()
{
    return ((ServerInternal *) InternalTag)->Clients.Size;
}

lw_i64 Lacewing::Server::BytesSent()
{
    ServerInternal &Internal = *(ServerInternal *) InternalTag;

    return 0; /* TODO */
}

lw_i64 Lacewing::Server::BytesReceived()
{
    ServerInternal &Internal = *(ServerInternal *) InternalTag;

    return Internal.BytesReceived;
}

void Lacewing::Server::DisableNagling()
{
    ServerInternal &Internal = *(ServerInternal *) InternalTag;

    if(Internal.Socket != -1)
    {
        Lacewing::Error Error;
        Error.Add("DisableNagling() can only be called when the server is not hosting");

        if(Internal.HandlerError)
            Internal.HandlerError(*this, Error);

        return;
    }

    Internal.Nagle = false;
}

int Lacewing::Server::Port()
{
    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    if(!Hosting())
        return 0;

    return Internal.Port;
}

bool Lacewing::Server::CertificateLoaded()
{
    return ((ServerInternal *) InternalTag)->Context != 0;
}

int PasswordCallback(char * Buffer, int, int, void * InternalTag)
{
    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    strcpy(Buffer, Internal.Passphrase);
    return strlen(Internal.Passphrase);
}

bool Lacewing::Server::LoadCertificateFile(const char * Filename, const char * Passphrase)
{
    ServerInternal &Internal = *((ServerInternal *) InternalTag);
  
    SSL_load_error_strings();

    Internal.Context = SSL_CTX_new(SSLv23_server_method());

    strcpy(Internal.Passphrase, Passphrase);

    SSL_CTX_set_mode(Internal.Context, 
        SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
            
        #if HAVE_DECL_SSL_MODE_RELEASE_BUFFERS
             | SSL_MODE_RELEASE_BUFFERS
        #endif
    );

    SSL_CTX_set_quiet_shutdown (Internal.Context, 1);

    SSL_CTX_set_default_passwd_cb(Internal.Context, PasswordCallback);
    SSL_CTX_set_default_passwd_cb_userdata(Internal.Context, &Internal);

    if(SSL_CTX_use_certificate_chain_file(Internal.Context, Filename) != 1)
    {
        DebugOut("Failed to load certificate chain file: %s", ERR_error_string(ERR_get_error(), 0));

        Internal.Context = 0;
        return false;
    }

    if(SSL_CTX_use_PrivateKey_file(Internal.Context, Filename, SSL_FILETYPE_PEM) != 1)
    {
        DebugOut("Failed to load private key file: %s", ERR_error_string(ERR_get_error(), 0));

        Internal.Context = 0;
        return false;
    }

    return true;
}

bool Lacewing::Server::LoadSystemCertificate(const char * StoreName, const char * CommonName, const char * Location)
{
    ServerInternal &Internal = *((ServerInternal *) InternalTag);

    Lacewing::Error Error;
    Error.Add("System certificates are only supported on Windows");

    if(Internal.HandlerError)
        Internal.HandlerError(*this, Error);

    return false;
}

bool ServerClientInternal::Send(QueuedSend * Queued, const char * Buffer, int Size)
{
    if(Size == -1)
        Size = strlen(Buffer);

    if(!Size)
        return true;
    
    if((Transfer || QueuedSends.First) && !Queued)
    {
        QueuedSends.Add(Buffer, Size);
        return false;
    }

    if(!Context)
    {
        int Sent = send (Socket, Buffer, Size, LacewingNoSignal);

        if(Sent == Size)
        {
            /* Sent immediately */
            return true;
        }

        if(Sent == -1)
        {
            if(errno == EAGAIN)
            {
                /* Can't send now, queue it for later */

                if(!Queued)
                    QueuedSends.Add(Buffer, Size);

                return false;
            }

            return true;
        }

        /* send() didn't fail, but it didn't send everything either */

        if(!Queued)
        {
            QueuedSends.Add(Buffer + Sent, Size - Sent);
            return false;
        }
            
        Queued->DataOffset += Sent;
        return false;
    }
    else
    {
        /* SSL send */

        for(;;)
        {
            int Error = SSL_write(Context, Buffer, Size);

            if(Error > 0)
                return true;

            switch(SSL_get_error(Context, Error))
            {
                case SSL_ERROR_WANT_READ:

                    /* More data from the remote is required before we can write. */
                    
                    if(!Queued)
                        QueuedSends.AddSSLWriteWhenReadable(Buffer, Size);
                    
                    return false;

                case SSL_ERROR_WANT_WRITE:

                    /* The socket isn't ready for writing to right now. */
                    
                    if(!Queued)
                        QueuedSends.Add(Buffer, Size);
                    
                    return false;

                default:

                    /* Unknown error */
                    return true;
            };
            
            break;
        }
    }

    return true;
}

void Lacewing::Server::Client::Send(const char * Buffer, int Size)
{
    ((ServerClientInternal *) InternalTag)->Send(0, Buffer, Size);
}

void Lacewing::Server::Client::SendWritable(char * Buffer, int Size)
{
    /* This only differs for Windows */

    ((ServerClientInternal *) InternalTag)->Send(0, Buffer, Size);
}

bool ServerClientInternal::SendFile(bool AllowQueue, const char * Filename, lw_i64 Offset, lw_i64 Size)
{   
    if(AllowQueue && (QueuedSends.First || Transfer))
    {        
        QueuedSends.Add(Filename, Offset, Size);
        return false;
    }
    
    int File = open(Filename, O_RDONLY, 0);

    if(File == -1)
    {
        DebugOut("Failed to open %s", Filename);

        Public.Flush();
        return true;
    }
    
    if(!Size)
    {
        struct stat FileStat;
        
        if(fstat(File, &FileStat) == -1)
        {
            DebugOut("Failed to stat %s", Filename);

            close(File);
            Public.Flush();

            return true;
        }
        
        Size = FileStat.st_size;
    }
    
    if(Context)
    {
        /* SSL - have to send the file manually */

        lseek(File, Offset, SEEK_SET);
        
        Transfer = new SSLFileTransfer(File, Context, Size);

        if(!Transfer->WriteReady())
        {
            /* Either completed immediately or failed */

            DebugOut("SSL file transfer completed immediately or failed");

            delete Transfer;
            Transfer = 0;
            
            Public.Flush();
            return true;
        }

        return false;
    }
    else
    {
        /* Non SSL - can use kernel sendfile.  First try to send whatever we can of the file with
           LacewingSendFile, and then if that doesn't fail or complete immediately, create a RawFileTransfer. */
    
        off_t _Offset = Offset;
        off_t _Size   = Size;

        if((!LacewingSendFile(File, Socket, _Offset, _Size)) || _Size == 0)
        {
            /* Failed or whole file sent immediately */
            
            close(File);        
            Public.Flush();

            return true;
        }

        Transfer = new RawFileTransfer(File, Socket, _Offset, _Size);
        return false;
    }
    
    return true;
}

void Lacewing::Server::Client::SendFile(const char * Filename, lw_i64 Offset, lw_i64 Size)
{
    ((ServerClientInternal *) InternalTag)->SendFile(true, Filename, Offset, Size);
}

bool Lacewing::Server::Client::CheapBuffering()
{
    return true;
}

void Lacewing::Server::Client::StartBuffering()
{
    ServerClientInternal &Internal = *(ServerClientInternal *) InternalTag;

    if(Internal.Context)
        return;

    #ifdef LacewingAllowCork
        int Enabled = 1;
        setsockopt(Internal.Socket, IPPROTO_TCP, LacewingCork, &Enabled, sizeof(Enabled));
    #endif
}

void Lacewing::Server::Client::Flush()
{
    ServerClientInternal &Internal = *(ServerClientInternal *) InternalTag;

    if(Internal.Context)
        return;

    #ifdef LacewingAllowCork
        int Enabled = 0;
        setsockopt(Internal.Socket, IPPROTO_TCP, LacewingCork, &Enabled, sizeof(Enabled));
    #endif
}

void Lacewing::Server::Client::Disconnect()
{
    /* TODO : Is it safe to remove the client immediately in multithreaded mode? */

    ServerClientInternal &Internal = *((ServerClientInternal *) InternalTag);

    if (Internal.QueuedSends.First || Internal.Transfer)
    {
        Internal.QueuedSends.Add (new QueuedSend (QueuedSendType::Disconnect));
        return;
    }
    else
    {
        if (Internal.Context)
            SSL_shutdown (Internal.Context);
        else
            shutdown (Internal.Socket, SHUT_RD);
    }
}

Lacewing::Address &Lacewing::Server::Client::GetAddress()
{
    ServerClientInternal &Internal = *(ServerClientInternal *) InternalTag;
    
    return *Internal.Address;
}

Lacewing::Server::Client * Lacewing::Server::Client::Next ()
{
    return ((ServerClientInternal *) InternalTag)->Element->Next ?
        &(** ((ServerClientInternal *) InternalTag)->Element->Next)->Public : 0;
}

Lacewing::Server::Client * Lacewing::Server::FirstClient ()
{
    return ((ServerInternal *) InternalTag)->Clients.First ?
            &(** ((ServerInternal *) InternalTag)->Clients.First)->Public : 0;
}

AutoHandlerFunctions(Lacewing::Server, ServerInternal, Connect)
AutoHandlerFunctions(Lacewing::Server, ServerInternal, Disconnect)
AutoHandlerFunctions(Lacewing::Server, ServerInternal, Receive)
AutoHandlerFunctions(Lacewing::Server, ServerInternal, Error)

