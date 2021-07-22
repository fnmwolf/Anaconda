
#include "LacewingMMF.h"

bool LacewingMMF::condition (void * ext, int ID)
{
    curExt = ext;
    
    switch (ID)
    {
        case  0: /* On error */
        case  1: /* On connect */
        case  2: /* On connection denied */
        case  3: /* On disconnect */
        case  4: /* On join */
        case  5: /* On join denied */
        case  6: /* On name set */
        case  7: /* On name denied */
        case 10: /* On peer connect */
        case 11: /* On peer disconnect */
        case 12: /* On channel join (deprecated) */
        case 13: /* On channel peer loop */
        case 14: /* On joined channel loop */
        case 17: /* On channel peer loop finished */
        case 18: /* On joined channel loop finished */
        case 26: /* On channel list received */
        case 27: /* On channel list loop */
        case 28: /* On channel list loop finished */
        case 43: /* On channel leave */
        case 44: /* On channel leave denied */
        case 45: /* On peer change name */
        case 53: /* On name changed */
        
            return true;

        case  8: /* On text server message (subchannel %0) */
        case  9: /* On text channel message (subchannel %0) */
        case 15: /* On number server message (subchannel %0) */
        case 16: /* On number channel message (subchannel %0) */
        case 20: /* On blasted text message from server (subchannel %0) */
        case 21: /* On blasted number message from server (subchannel %0) */
        case 22: /* On blasted text message from channel (subchannel %0) */
        case 23: /* On blasted number message from channel (subchannel %0) */
        case 32: /* On binary message from server (subchannel %0) */
        case 33: /* On binary message from channel (subchannel %0) */
        case 34: /* On blasted binary message from server (subchannel %0) */
        case 35: /* On blasted binary message from channel (subchannel %0) */
        case 36: /* On text message from peer (subchannel %0) */
        case 37: /* On number message from peer (subchannel %0) */
        case 38: /* On binary message from peer (subchannel %0) */
        case 39: /* On blasted text message from peer (subchannel %0) */
        case 40: /* On blasted number message from peer (subchannel %0) */
        case 41: /* On blasted binary message from peer (subchannel %0) */
        case 47: /* On any message from server (subchannel %0) */
        case 48: /* On any message from channel (subchannel %0) */
        case 49: /* On any message from peer (subchannel %0) */
        case 50: /* On any blasted message from server (subchannel %0) */
        case 51: /* On any blasted message from channel (subchannel %0) */
        case 52: /* On any blasted message from peer (subchannel %0) */
        case 65: /* On text channel message from server (subchannel %0) */
        case 66: /* On number channel message from server (subchannel %0) */
        case 67: /* On binary channel message from server (subchannel %0) */
        case 68: /* On any channel message from server (subchannel %0) */
        case 69: /* On blasted text channel message from server (subchannel %0) */
        case 70: /* On blasted number channel message from server (subchannel %0) */
        case 71: /* On blasted binary channel message from server (subchannel %0) */
        case 72: /* On any blasted channel message from server (subchannel %0) */
        {
            int param = getCndParamInt ();

            return param == -1 || subchannel == param;
        }

        case 42: /* Is connected */
            return client.Connected();
            
        case 54: /* Name is set */
            return *client.Name() != 0;
        
        case 57: /* Peer is the channel master */
            return selectedPeer ? selectedPeer->IsChannelMaster() : false;  
            
        case 58: /* You are the channel master */
            return selectedChannel ? selectedChannel->IsChannelMaster() : false;
            
        case 59: /* On channel list loop %0 */
        case 60: /* On channel list loop %0 finished */
        case 61: /* On channel peer loop %0 */
        case 62: /* On channel peer loop %0 finished */
        case 63: /* On joined channel loop %0 */
        case 64: /* On joined channel loop %0 finished */
        
            return !strcasecmp(loopName, getCndParamString());
    };
    
    return false;
}

