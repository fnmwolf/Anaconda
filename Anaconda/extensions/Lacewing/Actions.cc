

#include "LacewingMMF.h"

void LacewingMMF::action (void * ext, int ID)
{
    curExt = ext;
    
    switch (ID)
    {
        case 0: /* Connect to %0 on port %1 (deprecated) */
        {
            const char * host = getActParamString ();
            int port = getActParamInt ();
            
            client.Connect (host, port);
            return;
        }
        
        case 1: /* Disconnect */
        
            client.Disconnect ();
            return;
            
        case 2: /* Set name to %0 */

            client.Name (getActParamString ());
            return;
        
        case 3: /* Join channel %0 (hidden: %1) (deprecated) */
        {
            const char * name = getActParamString ();
            bool hidden = getActParamInt() != 0;
            
            client.Join (name, false, hidden);
            
            return;
        }

        case 4: /* Leave channel */
        
            if (selectedChannel)
                selectedChannel->Leave ();
                
            return;
        
        case 5: /* Send text %1 to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            client.SendServer (subchannel, text, -1, 0);
            
            return;
        }
                
        case 6: /* Send text %1 to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            if (selectedChannel)
                selectedChannel->Send (subchannel, text, -1, 0);
                
            return;
        }
        
        case 7: /* Send text %1 to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            if (selectedPeer)
                selectedPeer->Send (subchannel, text, -1, 0);
                
            return;
        }
        
        case 8: /* Send number %1 to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            client.SendServer (subchannel, (char *) &number, 4, 1);
            
            return;
        }
                
        case 9: /* Send number %1 to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            if (selectedChannel)
                selectedChannel->Send (subchannel, (char *) &number, 4, 1);
                
            return;
        }
        
        case 10: /* Send number %1 to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            if (selectedPeer)
                selectedPeer->Send (subchannel, (char *) &number, 4, 1);
                
            return;
        }
        
        case 11: /* Blast text %1 to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            client.BlastServer (subchannel, text, -1, 0);
            
            return;
        }
                
        case 12: /* Blast text %1 to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            if (selectedChannel)
                selectedChannel->Blast (subchannel, text, -1, 0);
                
            return;
        }
        
        case 13: /* Blast text %1 to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            const char * text = getActParamString ();
            
            if (selectedPeer)
                selectedPeer->Blast (subchannel, text, -1, 0);
                
            return;
        }
        
        case 14: /* Blast number %1 to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            client.BlastServer (subchannel, (char *) &number, 4, 1);
            
            return;
        }
                
        case 15: /* Blast number %1 to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            if (selectedChannel)
                selectedChannel->Blast (subchannel, (char *) &number, 4, 1);
                
            return;
        }
        
        case 16: /* Blast number %1 to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            int number = getActParamInt ();
            
            if (selectedPeer)
                selectedPeer->Blast (subchannel, (char *) &number, 4, 1);
                
            return;
        }
        
        case 17: /* Select channel by name %0 */
        {
            const char * name = getActParamString ();

            for (Lacewing::RelayClient::Channel * channel
                    = client.FirstChannel (); channel; channel = channel->Next())
	        {
                if (!strcasecmp (channel->Name (), name))
                {
                    selectChannel (channel);
                    return;
                }
            }

            selectChannel (0);
            return;
        }
        
        case 19: /* Loop channels */
        {
            Lacewing::RelayClient::Channel * oldSelected = selectedChannel;

            selectChannel (0);

            for (Lacewing::RelayClient::Channel * channel
                    = client.FirstChannel (); channel; channel = channel->Next())
	        {
                selectChannel (channel);
                generateEventLocal (14);
                selectChannel (channel);
            }

            generateEventLocal (18);
            selectChannel (oldSelected);

            return;
        }
        
        case 20: /* Select peer on channel by name %0 */
        {
            const char * name = getActParamString ();

            if (!selectedChannel)
                return;

            for (Lacewing::RelayClient::Channel::Peer * peer
                    = selectedChannel->FirstPeer (); peer; peer = peer->Next())
	        {
                if (!strcasecmp (peer->Name(), name))
                {
                    selectedPeer = peer;
                    return;
                }
            }

            selectedPeer = 0;
            return;
        }
        
        case 21: /* Select peer on channel by ID %0 */
        {
            int ID = getActParamInt ();

            if (!selectedChannel)
                return;

            for (Lacewing::RelayClient::Channel::Peer * peer
                    = selectedChannel->FirstPeer (); peer; peer = peer->Next())
	        {
                if (peer->ID () == ID)
                {
                    selectedPeer = peer;
                    return;
                }
            }

            selectedPeer = 0;
            return;
        }
        
        case 22: /* Loop peers on channel */
        {
            if (!selectedChannel)
                return;

            Lacewing::RelayClient::Channel::Peer * oldSelected = selectedPeer;

            selectedPeer = 0;

            for (Lacewing::RelayClient::Channel::Peer * peer
                    = selectedChannel->FirstPeer (); peer; peer = peer->Next())
	        {
                selectedPeer = peer;
                generateEventLocal (13);
                selectedPeer = peer;
            }

            generateEventLocal (17);
            selectedPeer = oldSelected;

            return;
        }
        
        case 30: /* Request channel list */
        {
            client.ListChannels ();
            return;
        }
        
        case 31: /* Loop listed channels */
        {
            channelListing = client.FirstChannelListing ();
            
            while (channelListing)
            {
                generateEventLocal (27);                
                channelListing = channelListing->Next ();
            }
            
            generateEventLocal (28);
            return;
        }
        
        case 35: /* Send binary to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            client.SendServer (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;

            return;
        }
                
        case 36: /* Send binary to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            if (selectedChannel)
                selectedChannel->Send (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;
                
            return;
        }
        
        case 37: /* Send binary to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            if (selectedPeer)
                selectedPeer->Send (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;
                
            return;
        }
        
        case 38: /* Blast binary to server on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            client.BlastServer (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;

            return;
        }
                
        case 39: /* Blast binary to channel on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            if (selectedChannel)
                selectedChannel->Blast (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;
                
            return;
        }
        
        case 40: /* Blast binary to peer on subchannel %0 */
        {
            int subchannel = getActParamInt ();
            
            if (selectedPeer)
                selectedPeer->Blast (subchannel, binary, binarySize, 2);
            
            if (autoClearBinary)
                binarySize = 0;
                
            return;
        }
        
        case 41: /* Add byte %0 (as string) */
        {
            binaryAdd (getActParamString (), 1);
            return;
        }
        
        case 42: /* Add byte %0 (as number) */
        {
            char value = (char) getActParamInt ();
            binaryAdd ((char *) &value, 1);
            
            return;
        }
        
        case 43: /* Add short %0 */
        {
            short value = (short) getActParamInt ();
            binaryAdd ((char *) &value, 2);
            
            return;
        }
        
        case 44: /* Add integer %0 */
        {
            int value = getActParamInt ();
            binaryAdd ((char *) &value, 4);
            
            return;
        }
        
        case 45: /* Add float %0 */
        {
            float value = getActParamFloat ();
            binaryAdd ((char *) &value, 4);
            
            return;
        }
        
        case 46: /* Add string %0 without null terminator */
        {
            const char * string = getActParamString ();            
            binaryAdd (string, strlen (string));
            
            return;
        }
        
        case 47: /* Add string %0 with null terminator */
        {
            const char * string = getActParamString ();            
            binaryAdd (string, strlen (string) + 1);
            
            return;
        }
        
        case 48: /* Add binary at address %0 with size %1 */
        {
            int address = getActParamInt ();
            int size = getActParamInt ();
            
            binaryAdd ((char *) address, size);

            return;
        }
        
        case 49: /* Clear binary */
        {
            binarySize = 0;
            return;
        }
        
        case 50: /* Save received binary ... */
        {
            int position = getActParamInt ();
            int size = getActParamInt ();

            if (size <= 0 || position < 0)
                return;

            if (position + size >= receivedSize)
                size = receivedSize - position;

            FILE * file = fopen (getActParamString (), "wb");

            if (!file)
                return;

            fwrite (received + position, 1, size, file);
            fclose (file);

            return;
        }

        case 51: /* Append received binary ... */
        {
            int position = getActParamInt ();
            int size = getActParamInt ();

            if (size <= 0 || position < 0)
                return;

            if (position + size >= receivedSize)
                size = receivedSize - position;

            FILE * file = fopen (getActParamString (), "ab");

            if (!file)
                return;

            fwrite (received + position, 1, size, file);
            fclose (file);

            return;
        }
        
        case 52: /* Add file %0 onto the binary to send */
        {
            FILE * file = fopen (getActParamString (), "rb");
         
            if (!file)
                return;
                
            fseek (file, 0, SEEK_END);
            int size = ftell (file);
            fseek (file, 0, SEEK_SET);
            
            binaryPrep (size);
            
            fread (binary + binarySize, 1, size, file);
            binarySize += size;
            
            fclose (file);
            
            return;
        }
        
        case 64: /* Select the channel master */

            if (!selectedChannel)
                return;

            for (Lacewing::RelayClient::Channel::Peer * peer
                    = selectedChannel->FirstPeer (); peer; peer = peer->Next())
	        {
                if (peer->IsChannelMaster ())
                {
                    selectedPeer = peer;
                    return;
                }
            }

            selectedPeer = 0;
            return;
            
        case 65: /* Join channel %0 (hide from list: %1, close automatically: %2) */
        {
            const char * name = getActParamString ();
            
            bool hidden = getActParamInt () != 0;
            bool autoClose = getActParamInt () != 0;
            
            client.Join (name, hidden, autoClose);
            
            return;
        }

        case 66: /* Compress send binary (ZLIB) */
        {
            char * temp = (char *) malloc (binarySize);
            memcpy(temp, binary, binarySize);

            binaryPrep (binarySize + binarySize * 0.1 + 13 + 4);
            *(unsigned int *) binary = binarySize;

            unsigned int destSize = binaryAllocated - 4;

            if (!compress2((unsigned char *) (binary + 4), (unsigned long *) &destSize,
                        (unsigned char *) temp, binarySize))
            {
                free (temp);
                return;
            }

            binarySize = 4 + destSize;
            free(temp);
            
            return;
        }
        
        case 67: /* Uncompress received binary (ZLIB */
        {
            if(receivedSize < 4 || !received)
                return;

            unsigned int uncompressedSize = *(unsigned int *) received;

            char * buffer = (char *) malloc(uncompressedSize);

            if (!uncompress((unsigned char *) buffer, (unsigned long *) &uncompressedSize,
                                        (unsigned char *) (received + 4), (receivedSize - 4)))
            {
                free (buffer);
                return;
            }
        
            received = buffer;
            receivedSize = uncompressedSize;
            
            freeReceived = true;
            
            return;
        }
        
        case 68: /* Move received binary cursor to %0 */
        {
            int position = getActParamInt ();
            
            if (position < 0)
                position = 0;
                
            if (position >= receivedSize)
                position = receivedSize - 1;
                
            cursor = position;            
            
            return;
        }
        
        case 69: /* Loop listed channels (loop name: %0) */
        {
            const char * loopName = getActParamString ();
            
            channelListing = client.FirstChannelListing ();
            
            while (channelListing)
            {
                this->loopName = loopName;
                
                generateEventLocal (59);   
                channelListing = channelListing->Next ();
            }
            
            this->loopName = loopName;
            generateEventLocal (60);
            this->loopName = "";
            
            return;
        }
        
        case 70: /* Loop channels (loop name: %0) */
        {
            const char * loopName = getActParamString ();
            
            Lacewing::RelayClient::Channel * oldSelected = selectedChannel;

            selectChannel (0);

            for (Lacewing::RelayClient::Channel * channel
                    = client.FirstChannel (); channel; channel = channel->Next())
	        {
                selectChannel (channel);

                this->loopName = loopName;
                generateEventLocal (63);

                selectChannel (channel);
            }

            this->loopName = loopName;
            generateEventLocal (64);

            selectChannel (oldSelected);
            
            return;
        }
        
        case 71: /* Loop peers on channel (loop name: %0) */
        {
            const char * loopName = getActParamString ();

            if (!selectedChannel)
                return;

            Lacewing::RelayClient::Channel::Peer * oldSelected = selectedPeer;

            selectedPeer = 0;

            for (Lacewing::RelayClient::Channel::Peer * peer
                    = selectedChannel->FirstPeer (); peer; peer = peer->Next())
	        {
                selectedPeer = peer;

                this->loopName = loopName;
                generateEventLocal (61);
                
                selectedPeer = peer;
            }

            this->loopName = loopName;
            generateEventLocal (62);
            this->loopName = "";

            selectedPeer = oldSelected;

            return;
        }
        
        case 73: /* Connect to %0 */
        {
            client.Connect (getActParamString ());
            return;
        }
        
        case 74: /* Resize binary to send to %0 bytes */
        {
            int newSize = getActParamInt ();
            
            if (newSize > binarySize)
                binaryPrep ((newSize - binarySize) + 1);
                
            binarySize = newSize;
            
            return;
        }
    };
}
