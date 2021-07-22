/*
Code by Jamie.
zlib license.
*/

#include "LacewingMMF.h"

LacewingMMF * LacewingMMF::first;

void LacewingMMF::init ()
{
    LacewingMMF::first = 0;
}

LacewingMMF * LacewingMMF::create (void * ext, EditData * edPtr)
{
    if (!edPtr->global)
    {
        return new LacewingMMF (edPtr, ext);
    }
    
    LacewingMMF * inst;
    
    for (inst = LacewingMMF::first; inst; inst = inst->next)
    {
        if (!strcasecmp (inst->globalID, edPtr->globalID))
        {
            inst->addExt (ext);
            return inst;
        }
    }
    
    inst = new LacewingMMF (edPtr, ext);
    inst->next = 0;
    
    strcpy (inst->globalID, edPtr->globalID);
    
    LacewingMMF * last = first;
    
    if (!last)
        return (LacewingMMF::first = inst);
    
    while (last->next)
        last = last->next;
        
    last->next = inst;
    last = inst;
    
    return inst;
}
    
void onConnect (Lacewing::RelayClient &client, const char * welcomeMessage)
{   ((LacewingMMF *) client.Tag)->onConnect ();
}
void onConnectionDenied (Lacewing::RelayClient &client, const char * denyReason)
{   ((LacewingMMF *) client.Tag)->onConnectionDenied (denyReason);
}
void onDisconnect (Lacewing::RelayClient &client)
{   ((LacewingMMF *) client.Tag)->onDisconnect ();
}
void onJoinChannel (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel)
{   ((LacewingMMF *) client.Tag)->onJoinChannel (channel);
}
void onJoinDenied (Lacewing::RelayClient &client, const char * channelName, const char * denyReason)
{   ((LacewingMMF *) client.Tag)->onJoinDenied (channelName, denyReason);
}
void onNameSet (Lacewing::RelayClient &client)
{   ((LacewingMMF *) client.Tag)->onNameSet ();
}
void onNameChanged (Lacewing::RelayClient &client, const char * oldName)
{   ((LacewingMMF *) client.Tag)->onNameChanged (oldName);
}
void onNameDenied (Lacewing::RelayClient &client, const char * name, const char * denyReason)
{   ((LacewingMMF *) client.Tag)->onNameDenied (name, denyReason);
}
void onLeave (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel)
{   ((LacewingMMF *) client.Tag)->onLeave (channel);
}
void onLeaveDenied (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel, const char * denyReason)
{   ((LacewingMMF *) client.Tag)->onLeaveDenied (channel, denyReason);
}
void onChannelListReceived (Lacewing::RelayClient &client)
{   ((LacewingMMF *) client.Tag)->onChannelListReceived ();
}
void onError (Lacewing::RelayClient &client, Lacewing::Error &error)
{   ((LacewingMMF *) client.Tag)->onError (error);
}

void onPeerConnect (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
                    Lacewing::RelayClient::Channel::Peer &peer)
{
    ((LacewingMMF *) client.Tag)->onPeerConnect (channel, peer);
}

void onPeerDisconnect (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
                    Lacewing::RelayClient::Channel::Peer &peer)
{
    ((LacewingMMF *) client.Tag)->onPeerDisconnect (channel, peer);
}

void onPeerChangeName (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
                    Lacewing::RelayClient::Channel::Peer &peer, const char * oldName)
{
    ((LacewingMMF *) client.Tag)->onPeerChangeName (channel, peer, oldName);
}

void onServerMessage (Lacewing::RelayClient &client, bool blasted,
            int subchannel, char * data, int size, int type)
{
    ((LacewingMMF *) client.Tag)->onServerMessage
        (blasted, subchannel, data, size, type);
}

void onChannelMessage (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
            Lacewing::RelayClient::Channel::Peer &peer, bool blasted,  int subchannel,
                char * data, int size, int type)
{
    ((LacewingMMF *) client.Tag)->onChannelMessage
        (channel, peer, blasted, subchannel, data, size, type);
}

void onServerChannelMessage (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
                bool blasted,  int subchannel, char * data, int size, int type)
{
    ((LacewingMMF *) client.Tag)->onServerChannelMessage
        (channel, blasted, subchannel, data, size, type);
}

void onPeerMessage (Lacewing::RelayClient &client, Lacewing::RelayClient::Channel &channel,
            Lacewing::RelayClient::Channel::Peer &peer, bool blasted,  int subchannel,
                char * data, int size, int type)
{
    ((LacewingMMF *) client.Tag)->onPeerMessage
        (channel, peer, blasted, subchannel, data, size, type);
}

Lacewing::EventPump eventPump;

LacewingMMF::LacewingMMF (LacewingMMF::EditData * edPtr, void * ext) : client (eventPump)
{
    extCount = 0;
    addExt (ext);
    
    client.Tag = this;
    
    client.onConnect               ((Lacewing::RelayClient::HandlerConnect)::onConnect);
    client.onConnectionDenied      (::onConnectionDenied);
    client.onDisconnect            (::onDisconnect);
    client.onServerMessage         (::onServerMessage);
    client.onChannelMessage        (::onChannelMessage);
    client.onPeerMessage           (::onPeerMessage);
    client.onServerChannelMessage  (::onServerChannelMessage);
    client.onJoin                  (::onJoinChannel);
    client.onJoinDenied            (::onJoinDenied);
    client.onLeave                 (::onLeave);
    client.onLeaveDenied           (::onLeaveDenied);
    client.onNameSet               (::onNameSet);
    client.onNameChanged           (::onNameChanged);
    client.onNameDenied            (::onNameDenied);
    client.onPeerConnect           (::onPeerConnect);
    client.onPeerDisconnect        (::onPeerDisconnect);
    client.onPeerChangeName        (::onPeerChangeName);
    client.onChannelListReceived   (::onChannelListReceived);
    client.onError                 (::onError);

    denyReason = "";
    
    this->autoClearBinary = edPtr->autoClearBinary;
    this->global = edPtr->global;
    
    selectedChannel = 0;
    selectedPeer = 0;
    
    binary = 0;
    binaryAllocated = 0;
    binarySize = 0;
    
    channelListing = 0;
    
    received = 0;
    receivedSize = cursor = 0;
    
    freeReceived = 0;
    
    lastError = 0;
}

LacewingMMF::~LacewingMMF ()
{    
}

int LacewingMMF::handleRunObject (void * ext)
{
    curExt = ext;
    
    eventPump.Tick ();
    return 0;
}

void LacewingMMF::onConnect ()
{
    generateEventGlobal (1);
}

void LacewingMMF::onConnectionDenied (const char * denyReason)
{
    this->denyReason = denyReason;
    generateEventGlobal (2);
    this->denyReason = "";
}

void LacewingMMF::onDisconnect ()
{
    selectChannel (0);
    generateEventGlobal (3);
}

void LacewingMMF::onServerMessage (bool blasted, int subchannel, char * data, int size, int type)
{    
    storeReceived (0, 0, subchannel, data, size, type);
    
    switch (type)
    {
        case 0:
            
            generateEventGlobal (blasted ? 20 : 8);
            break;
            
        case 1:
            
            generateEventGlobal (blasted ? 21 : 15);
            break;
        
        case 2:
            
            generateEventGlobal (blasted ? 34 : 32);
            break;            
    };
    
    unstoreReceived ();
    storeReceived (0, 0, subchannel, data, size, type);
    
    generateEventGlobal (blasted ? 50 : 47);
    
    unstoreReceived ();
}

void LacewingMMF::onChannelMessage (Lacewing::RelayClient::Channel &channel,
                    Lacewing::RelayClient::Channel::Peer &peer,
                    bool blasted, int subchannel, char * data, int size, int type)
{
    storeReceived (&channel, &peer, subchannel, data, size, type);
    
    switch (type)
    {
        case 0:
            
            generateEventGlobal (blasted ? 22 : 9);
            break;
            
        case 1:

            generateEventGlobal (blasted ? 23 : 16);            
            break;
        
        case 2:
            
            generateEventGlobal (blasted ? 35 : 33);
            break;            
    };
    
    unstoreReceived ();
    storeReceived (&channel, &peer, subchannel, data, size, type);
    
    generateEventGlobal (blasted ? 51 : 48);
        
    unstoreReceived ();
}

void LacewingMMF::onServerChannelMessage (Lacewing::RelayClient::Channel &channel,
                    bool blasted, int subchannel, char * data, int size, int type)
{
    storeReceived (&channel, 0, subchannel, data, size, type);
    
    switch (type)
    {
        case 0:
            
            generateEventGlobal (blasted ? 69 : 65);
            break;
            
        case 1:
            
            generateEventGlobal (blasted ? 70 : 66);
            break;
        
        case 2:
            
            generateEventGlobal (blasted ? 71 : 67);
            break;            
    };
    
    unstoreReceived ();
    storeReceived (&channel, 0, subchannel, data, size, type);
    
    generateEventGlobal (blasted ? 72 : 68);
        
    unstoreReceived ();
}

void LacewingMMF::onPeerMessage (Lacewing::RelayClient::Channel &channel,
                    Lacewing::RelayClient::Channel::Peer &peer,
                    bool blasted, int subchannel, char * data, int size, int type)
{
    storeReceived (&channel, &peer, subchannel, data, size, type);
    
    switch (type)
    {
        case 0:
            
            generateEventGlobal (blasted ? 39 : 36);
            break;
            
        case 1:
            
            generateEventGlobal (blasted ? 40 : 37);
            break;
        
        case 2:
            
            generateEventGlobal (blasted ? 41 : 38);
            break;            
    };
    
    unstoreReceived ();
    storeReceived (&channel, &peer, subchannel, data, size, type);
    
    generateEventGlobal (blasted ? 52 : 49);
        
    unstoreReceived ();
}

void LacewingMMF::storeReceived (Lacewing::RelayClient::Channel * channel,
            Lacewing::RelayClient::Channel::Peer * peer, int subchannel,
                    char * data, int size, int type)
{
    received = data;
    receivedSize = size;
    cursor = 0;
    
    this->subchannel = subchannel;
    
    if (type == 1 && size == 4)
        receivedNumber = *(int *) received;
    else
        receivedNumber = 0;

    if (channel)
        selectChannel (channel);
        
    if (peer)
        selectedPeer = peer;
}

void LacewingMMF::unstoreReceived ()
{
    if (freeReceived)
    {
        free (received);
        freeReceived = false;
    }
    
    receivedSize = 0;
    cursor = 0;
}

void LacewingMMF::selectChannel (Lacewing::RelayClient::Channel * channel)
{
    if (selectedChannel)
        selectedChannel->Tag = selectedPeer;
    
    selectedChannel = channel;

    if (selectedChannel)
        selectedPeer = (Lacewing::RelayClient::Channel::Peer *) selectedChannel->Tag;
    else
        selectedPeer = 0;
}

void LacewingMMF::onJoinChannel (Lacewing::RelayClient::Channel &channel)
{
	selectChannel (&channel);	
	generateEventGlobal (4);
}

void LacewingMMF::onJoinDenied (const char * channelName, const char * denyReason)
{
	this->denyReason = denyReason;
	
	generateEventGlobal (5);

	this->denyReason = "";
}

void LacewingMMF::onLeave (Lacewing::RelayClient::Channel &channel)
{
	selectChannel (&channel);	
	generateEventGlobal (43);
    selectChannel (0);
}

void LacewingMMF::onLeaveDenied (Lacewing::RelayClient::Channel &channel, const char * denyReason)
{
	selectChannel (&channel);	
	
    this->denyReason = denyReason;
    generateEventGlobal (44);
    this->denyReason = "";
}

void LacewingMMF::generateEventLocal  (int ID)
{
    generateEvent (curExt, ID);
}

void LacewingMMF::generateEventGlobal (int ID)
{
    for (int i = 0; i < extCount; ++ i)
    {
        /* TODO: What if a Lacewing object is destroyed or created by the generated event? */
        
        generateEvent (exts [i], ID);
    }
}

void LacewingMMF::onNameSet ()
{
    generateEventGlobal (6);
}

void LacewingMMF::onNameChanged (const char * oldName)
{
    previousName = oldName;
    generateEventGlobal (53);
    previousName = "";
}

void LacewingMMF::onNameDenied (const char * name, const char * denyReason)
{
    this->denyReason = denyReason;

    generateEventGlobal (7);
    
    this->denyReason = "";
}

void LacewingMMF::onPeerConnect (Lacewing::RelayClient::Channel &channel, 
                                 Lacewing::RelayClient::Channel::Peer &peer)
{
    selectChannel (&channel);
    selectedPeer = &peer;

    generateEventGlobal (10);
}

void LacewingMMF::onPeerDisconnect (Lacewing::RelayClient::Channel &channel, 
                                 Lacewing::RelayClient::Channel::Peer &peer)
{
    selectChannel (&channel);
    selectedPeer = &peer;

    generateEventGlobal (11);
}

void LacewingMMF::onPeerChangeName (Lacewing::RelayClient::Channel &channel, 
                                    Lacewing::RelayClient::Channel::Peer &peer,
                                    const char * oldName)
{
    selectChannel (&channel);
    selectedPeer = &peer;

    previousName = oldName;
    generateEventGlobal (45);
    previousName = "";
}

void LacewingMMF::onChannelListReceived ()
{
    generateEventGlobal (26);
}

void LacewingMMF::onError (Lacewing::Error &error)
{
    lastError = &error;
    generateEventGlobal (0);
    lastError = 0;
}

