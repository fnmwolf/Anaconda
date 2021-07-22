
#include "LacewingMMF.h"

void LacewingMMF::expression (void * ext, int ID)
{
    curExt = ext;
    
    switch (ID)
    {
         case 0: /* Error$ */
        
            setExpReturnString (lastError ? lastError->ToString() : "");
            return;
        
        case 2: /* Self_Name$ */
        
            setExpReturnString (client.Name());
            return;
            
        case 3: /* Self_ChannelCount */
        
            setExpReturnInt (client.ChannelCount());
            return;
            
        case 4: /* Peer_Name$ */
        
            setExpReturnString (selectedPeer ? selectedPeer->Name () : "");
            return;
            
        case 5: /* Received$ */
        
            setExpReturnString (received);
            return;
            
        case 6: /* Received */
            
            setExpReturnInt (receivedNumber);
            return;
            
        case 7: /* Subchannel */
        
            setExpReturnInt (subchannel);
            return;
            
        case 8: /* Peer_ID */
        
            setExpReturnInt (selectedPeer ? selectedPeer->ID() : -1);
            return;
        
        case 9: /* Channel_Name$ */
        
            setExpReturnString (selectedChannel ? selectedChannel->Name() : "");
            return;
        
        case 10: /* Channel_PeerCount */
        
            setExpReturnInt (selectedChannel ? selectedChannel->PeerCount() : -1);
            return;
            
        case 12: /* ChannelListing_Name$ */
        
            setExpReturnString (channelListing ? channelListing->Name : "");
            return;
            
        case 13: /* ChannelListing_PeerCount */
        
            setExpReturnInt (channelListing ? channelListing->PeerCount : -1);
            return;
        
        case 14: /* Self_ID */
        
            setExpReturnInt (client.ID());
            return;
            
        case 20: /* Byte$ */
        {
            int index = getExpParamInt ();
            
            if (index >= receivedSize || index < 0)
            {
                setExpReturnString ("");
                return;
            }
            
            char result [] = { received [index], 0 };
            
            setExpReturnString (result);
            return;
        }
        
        case 21: /* Byte */
        {
            int index = getExpParamInt ();
            
            if (index >= receivedSize || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt ((unsigned char) received [index]);
            return;
        }
        
        case 22: /* SignedByte */
        {
            int index = getExpParamInt ();
            
            if (index >= receivedSize || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt ((char) received [index]);
            return;
        }
        
        case 23: /* Short */
        {
            int index = getExpParamInt ();
            
            if (index > (receivedSize - 2) || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(unsigned short *) (received + index));
            return;
        }
        
        case 24: /* SignedShort */
        {
            int index = getExpParamInt ();
            
            if (index > (receivedSize - 2) || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(short *) (received + index));
            return;
        }
        
        case 25: /* Integer */
        {
            int index = getExpParamInt ();
            
            if (index > (receivedSize - 4) || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(unsigned int *) (received + index));
            return;
        }
        
        case 26: /* SignedInteger */
        {
            int index = getExpParamInt ();
            
            if (index > (receivedSize - 4) || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(int *) (received + index));
            return;
        }
        
        case 27: /* Float */
        {
            int index = getExpParamInt ();
            
            if (index > (receivedSize - 4) || index < 0)
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnFloat (*(float *) (received + index));
            return;
        }
        
        case 28: /* StringWithSize$ */
        {
            int index = getExpParamInt ();
            int size = getExpParamInt ();

            int end = index + size;
            
            if (index < 0 || end >= receivedSize)
            {
                setExpReturnString ("");
                return;
            }
            
            char * result = (char *) alloca (size + 1);
            
            memcpy (result, received + index, size);
            result [size] = 0;
            
            setExpReturnString (result);
            return;
        }
        
        case 29: /* String$ */
        {
            int index = getExpParamInt ();

            if (index < 0 || index >= receivedSize)
            {
                setExpReturnString ("");
                return;
            }
            
            setExpReturnString (received + index);
            return;
        }
        
        case 30: /* ReceivedBinarySize */
        
            setExpReturnInt (receivedSize);
            return;
        
        case 31: /* Version$ */
        
            setExpReturnString (Lacewing::Version ());
            return;
            
        case 32: /* SendBinarySize */
        
            setExpReturnInt (binarySize);
            return;
            
        case 33: /* Self_PreviousName$ */
        
            setExpReturnString (previousName);
            return;
            
        case 34: /* Peer_PreviousName$ */
        
            setExpReturnString (previousName);
            return;
            
        case 37: /* DenyReason$ */
            
            setExpReturnString (denyReason);
            return;
            
        case 38: /* HostIP$ */
        
            setExpReturnString (client.ServerAddress().ToString());
            return;
            
        case 39: /* HostPort */
        
            setExpReturnInt (client.ServerAddress().Port());
            return;
        
        case 41: /* WelcomeMessage$ */
        
            setExpReturnString (client.WelcomeMessage ());
            return;
            
        case 42: /* ReceivedBinaryAddress */
        
            setExpReturnInt ((int) received);
            return;
            
        case 43: /* CursorByte$ */
        {
            if (!checkCursor (1))
            {
                setExpReturnString ("");
                return;
            }
        
            char result [] = { received [cursor ++], 0 };
            
            setExpReturnString (result);
            return;
        }

        case 44: /* CursorByte */
        {
            if (!checkCursor (1))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt ((unsigned char) received [cursor ++]);
            return;
        }

        case 45: /* CursorSignedByte */
        {
            if (!checkCursor (1))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (received [cursor ++]);
            return;
        }
        
        case 46: /* CursorShort */
        {
            if (!checkCursor (2))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(unsigned short *) (received + cursor));
            cursor += 2;
            
            return;
        }

        case 47: /* CursorSignedShort */
        {
            if (!checkCursor (2))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(short *) (received + cursor));
            cursor += 2;
            
            return;
        }
        
        case 48: /* CursorInteger */
        {
            if (!checkCursor (4))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(unsigned int *) (received + cursor));
            cursor += 4;
            
            return;
        }

        case 49: /* CursorSignedInteger */
        {
            if (!checkCursor (4))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnInt (*(int *) (received + cursor));
            cursor += 4;
            
            return;
        }
        
        case 50: /* CursorFloat */
        {
            if (!checkCursor (4))
            {
                setExpReturnInt (0);
                return;
            }
            
            setExpReturnFloat (*(float *) (received + cursor));
            cursor += 4;
            
            return;
        }
        
        case 51: /* CursorStringWithSize$ */
        {
            int size = getExpParamInt ();
            int end = cursor + size;
            
            if (end >= receivedSize)
            {
                setExpReturnString ("");
                return;
            }
            
            char * result = (char *) alloca (size + 1);
            
            memcpy (result, received + cursor, size);
            result [size] = 0;
            
            cursor += size;
            
            setExpReturnString (result);
            return;
        }
        
        case 52: /* CursorString$ */
        {
            if (cursor >= receivedSize)
            {
                setExpReturnString ("");
                return;
            }

            setExpReturnString (received + cursor);            
            cursor += strlen (received + cursor) + 1;
            
            return;
        }
        
        case 54: /* SendBinaryAddress */
        {
            setExpReturnInt ((int) binary);
            return;
        }
        
        case  1: /* Deprecated (string) */
        case 11:
        case 15:
        case 16:
        case 36:
        case 40:
        case 53:
        
            setExpReturnString ("");
            return;
            
        case 17: /* Deprecated (int) */
        case 18:
        case 19:
        case 35:
        
            setExpReturnInt (-1);
            return;
    };   
}

