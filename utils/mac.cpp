/*
Code by Anders Riggelsen. Included with permission.
Licensed under the zlib license.
*/

#include "mac.h"

///////////////////////////////////////////////////////////////////////////////
//ICON FRAGMENT////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
MacIconFragment::MacIconFragment(int size, unsigned char* buffer)
{
	iconType[0] = 'i';
	iconType[1] = '-';
	iconType[2] = '3';
	iconType[3] = '2';

	alphaType[0] = '-';
	alphaType[1] = '8';
	alphaType[2] = 'm';
	alphaType[3] = 'k';

	iconSize = size;

	switch(size)
	{
	case 16:
		iconType[1] = alphaType[0] = 's';
		alphaBufferLength = 256;
		break;
	case 32:
		iconType[1] = alphaType[0] = 'l';
		alphaBufferLength = 1024;
		break;
	case 48:
		iconType[1] = alphaType[0] = 'h';
		alphaBufferLength = 2304;
		break;
	case 128:
		iconType[1] = alphaType[0] = 't';
		alphaBufferLength = 16384;
		break;
	default:
		//throw "Cannot create icon!";
		break;
	}
	alphaLength = alphaBufferLength+8;

	int redSize=0, greenSize=0, blueSize=0;
	char *packedRed, *packedGreen, *packedBlue;
	int channelSize = size*size*4;

	RLECompressChannel(buffer,		channelSize, packedRed,		redSize);
	RLECompressChannel(buffer+1,	channelSize, packedGreen,	greenSize);
	RLECompressChannel(buffer+2,	channelSize, packedBlue,	blueSize);

	bufferLength = redSize+greenSize+blueSize;
	iconLength = bufferLength+8;
	iconData = new unsigned char[bufferLength];

	if(size == 128)
		iconLength += 4;

	//Extract alpha channel
	alphaData = new unsigned char[alphaBufferLength];
	for(unsigned int i=0; i<alphaBufferLength; ++i)
		alphaData[i] = buffer[i*4+3];

	memcpy(iconData,							packedRed,		redSize);
	memcpy(iconData+redSize,					packedGreen,	greenSize);
	memcpy(iconData+redSize+greenSize,			packedBlue,		blueSize);

	delete [] packedRed;
	delete [] packedGreen;
	delete [] packedBlue;
}

MacIconFragment::~MacIconFragment()
{
	delete [] iconData;
	delete [] alphaData;
}
int MacIconFragment::GetTotalSize()
{
	return iconLength + alphaLength;
}


void MacIconFragment::RLECompressChannel(unsigned char* channelStart, int bufferSize, char* &outputBuffer, int &bufferLength)
{
	int adjustedSize = bufferSize*2;
	outputBuffer = new char[adjustedSize];
	memset(outputBuffer, 0, adjustedSize);
	int outputPointer = 0;

	for(int i=0; i<bufferSize; i+=4)
	{
		int current = channelStart[i];
		
		int rleCount = 0;
		int rle=0;

		//Find how many concurrent pixels are present from the current index.
		while(rle<127 && (i+rle*4)<bufferSize && channelStart[i+rle*4] == current)
			++rle;
		
		if(rle>=3)		//Worth doing an RLE block of data
		{
			//printf("Storing RLE block of size %i\n", rle);
			outputBuffer[outputPointer++] = -128 + rle - 3;
			outputBuffer[outputPointer++] = current;
			i += rle*4-4;
		}
		else			//Do a binary blob instead
		{
			int headerPointer = outputPointer++;
			int x;
			for(x=0; x<127 && (i+x*4)<bufferSize; ++x)	// Look ahead for how long the buffer of random pixels should be.
			{
				int index = i+x*4;
				int a = channelStart[index];
				if ( index+8 < bufferSize && channelStart[index+4] == a && channelStart[index+8] == a )
					break;
				else
					outputBuffer[outputPointer++] = channelStart[index];
			}
			outputBuffer[headerPointer] = -1+x;
			//printf("Storing binary block of size %i\n", x);
			i += x*4-4;
		}
	}
	bufferLength = outputPointer;
}


///////////////////////////////////////////////////////////////////////////////
//MAC ICON CONVERTER///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MacIconConvert::WriteMacIcon(char* filename, unsigned char* size16, unsigned char* size32, unsigned char* size48, unsigned char* size128)
{
	FILE * file;
	int error = 0;
	file = fopen(filename, "wb");
	if ( file == NULL )
		error = -1;
	if(error)
	{
		return;
	}

	MacIconFragment i16 = MacIconFragment(16, size16);
	MacIconFragment i32 = MacIconFragment(32, size32);
	MacIconFragment i48 = MacIconFragment(48, size48);
	MacIconFragment i128 = MacIconFragment(128, size128);
 
	fwrite("icns",4,1,file);

	int icon16size = i16.GetTotalSize();
	int icon32size = i32.GetTotalSize();
	int icon48size = i48.GetTotalSize();
	int icon128size = i128.GetTotalSize();

	int totalSize = 8 + 12 + icon16size + icon32size + icon48size + icon128size;
	WriteInt(totalSize, file);
	
	WriteRGBFragment(i16, file);
	WriteAlphaFragment(i16, file);
	
	WriteRGBFragment(i32, file);
	WriteAlphaFragment(i32, file);
	
	WriteRGBFragment(i48, file);
	WriteAlphaFragment(i48, file);

	WriteRGBFragment(i128, file);
	WriteAlphaFragment(i128, file);

	Write_icnV_chunk(file);

	fclose(file);
}

void MacIconConvert::WriteRGBFragment(MacIconFragment &fragment, FILE* file)
{
	//Write RGB chunk
	fwrite(&fragment.iconType, 1, 4, file);
	WriteInt(fragment.iconLength, file);

	if(fragment.iconSize == 128)
		WriteInt(0, file);

	fwrite(fragment.iconData, fragment.bufferLength, 1, file);
}

void MacIconConvert::WriteAlphaFragment(MacIconFragment &fragment, FILE* file)
{
	//Write alpha chunk
	fwrite(&fragment.alphaType, 1, 4, file);
	WriteInt(fragment.alphaLength, file);
	fwrite(fragment.alphaData, fragment.alphaBufferLength, 1, file);
}

void MacIconConvert::WriteInt(int i, FILE* f)
{
	unsigned char* buff = (unsigned char*)&i;
	unsigned char a = buff[0];
	unsigned char b = buff[1];
	unsigned char c = buff[2];
	unsigned char d = buff[3];
	fwrite(&d, 1, 1, f);
	fwrite(&c, 1, 1, f);
	fwrite(&b, 1, 1, f);
	fwrite(&a, 1, 1, f);
}

void MacIconConvert::Write_icnV_chunk(FILE* f)
{
	unsigned char chunk[] = {0x69, 0x63, 0x6E, 0x56, 0x00, 0x00, 0x00, 0x0C, 0x42, 0xFE, 0x66, 0x66};
	fwrite(&chunk, 12, 1, f);
}

// C interface

void make_icon(char* filename, unsigned char* size16, unsigned char* size32, unsigned char* size48, unsigned char* size128)
{
    MacIconConvert::WriteMacIcon(filename, size16, size32, size48, size128);
}