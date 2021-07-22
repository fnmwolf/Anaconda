/*
Code by Jamie.
zlib license.
*/

#ifndef LACEWINGMMF_H
#define LACEWINGMMF_H

#define LacewingFunction
#include <Lacewing.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifndef __APPLE__
    #include <malloc.h>
#endif

#include <string.h>
// #include <zlib.h>

#ifdef _WIN32
    #define strcasecmp _stricmp
#endif

struct LacewingMMF
{
	const static int numberOfConditions = 65;

    struct EditData
    {
        char unused1 [5];
        
        bool autoClearBinary;
        
        char unused2 [1];
        
        bool global;    
        char globalID [512];
    };
    
    static void init ();
    static LacewingMMF * create (void *, EditData *);
    
    static LacewingMMF * first;
    LacewingMMF * next;
    
    LacewingMMF (EditData *, void * ext);
    ~ LacewingMMF ();
    
    bool global;
    char globalID [512];
    
    void * exts [8];
    int extCount;
    
    inline void addExt (void * ext)
    {
        exts [extCount ++] = ext;        
    }
    
    inline void removeExt (void * ext)
    {
        for (int i = 0; i < extCount; ++ i)
        {
            if (exts [i] == ext)
            {   
                memcpy (exts + i, exts + i + 1,
                    (extCount * sizeof(void *)) - (i * sizeof (void *)));

                -- extCount;

                break;
            }
        }
    }
    
    void * curExt;
   
    Lacewing::RelayClient client;
    
    Lacewing::RelayClient::Channel * selectedChannel;
    Lacewing::RelayClient::Channel::Peer * selectedPeer;
    
    void selectChannel (Lacewing::RelayClient::Channel *);
    
    char * binary;
    int binarySize, binaryAllocated;
    
    bool autoClearBinary;
    
    inline void binaryPrep (int size)
    {
        if ((binarySize + size) > binaryAllocated)
            binary = (char *) realloc (binary, binaryAllocated = ((binarySize + size) * 3));            
    }
    
    inline void binaryAdd (const char * data, int size)
    {
        binaryPrep (size);
        
        memcpy (binary + binarySize, data, size);
        binarySize += size;
    }
    
    const char * loopName;
    const char * denyReason;
    const char * previousName;
    
    Lacewing::RelayClient::ChannelListing * channelListing;
    
    char * received;
    int receivedSize, cursor, receivedNumber;  
    
    inline bool checkCursor (int size)
    {
        return (cursor + size) <= receivedSize;
    }
    
    bool freeReceived;
    
    int subchannel;
    
    Lacewing::Error * lastError;
    
    void onConnect ();
	void onConnectionDenied (const char *);
    void onDisconnect ();
	void onJoinChannel (Lacewing::RelayClient::Channel &);
	void onJoinDenied (const char *, const char *);
	void onNameSet ();
    void onNameChanged (const char *);
    void onNameDenied (const char *, const char *);
    void onLeave (Lacewing::RelayClient::Channel &);
    void onLeaveDenied (Lacewing::RelayClient::Channel &, const char *);
    void onChannelListReceived ();
    void onPeerConnect (Lacewing::RelayClient::Channel &, Lacewing::RelayClient::Channel::Peer &);
    void onPeerDisconnect (Lacewing::RelayClient::Channel &, Lacewing::RelayClient::Channel::Peer &);
    void onPeerChangeName (Lacewing::RelayClient::Channel &, Lacewing::RelayClient::Channel::Peer &, const char *);
    void onError (Lacewing::Error &);

	void onServerMessage (bool, int, char *, int, int);	
	
	void onChannelMessage (Lacewing::RelayClient::Channel &, Lacewing::RelayClient::Channel::Peer &,
							bool, int, char *, int, int);
							
	void onServerChannelMessage (Lacewing::RelayClient::Channel &, bool, int, char *, int, int);
							
	void onPeerMessage (Lacewing::RelayClient::Channel &, Lacewing::RelayClient::Channel::Peer &,
							bool, int, char *, int, int);
	
    void storeReceived (Lacewing::RelayClient::Channel * channel,
            Lacewing::RelayClient::Channel::Peer * peer, int subchannel,
                    char * data, int size, int type);
                    
    void unstoreReceived ();
                    
    /* Same implementation for all runtimes */
    
      void action     (void * ext, int ID);
      bool condition  (void * ext, int ID);
      void expression (void * ext, int ID);
      
      int handleRunObject (void * ext);
      
      void generateEventLocal  (int ID);
      void generateEventGlobal (int ID);
      
    /* Different implementation for each runtime */
    
      int getActParamInt ();
      const char * getActParamString ();
      float getActParamFloat ();

      int getCndParamInt ();
      const char * getCndParamString ();
    
      int getExpParamInt ();
      const char * getExpParamString ();
      
      void setExpReturnString (const char *);
      void setExpReturnInt (int);
      void setExpReturnFloat (float);
      
      void generateEvent (void * ext, int ID);
};

#endif