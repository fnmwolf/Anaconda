/*
Code by Anders Riggelsen. Included with permission.
Licensed under the zlib license.
*/

#pragma once
#include <fstream>

class MacIconFragment
{
public:
	MacIconFragment(int size, unsigned char* buffer);
	~MacIconFragment();
	
	char			iconType[4];
	unsigned int	iconLength;
	unsigned int	bufferLength;
	unsigned char*	iconData;

	int				iconSize;
	char			alphaType[4];
	unsigned int	alphaLength;
	unsigned int	alphaBufferLength;
	unsigned char*	alphaData;

	int GetTotalSize();
private:
	void RLECompressChannel(unsigned char* channelStart, int maxSize, char* &outputBuffer, int &bufferLength);
};

class MacIconConvert
{
public:
	static void WriteMacIcon(char* filename, unsigned char* size16, unsigned char* size32, unsigned char* size48, unsigned char* size128);
private:
	static void WriteInt(int i, FILE* f);
	static void WriteRGBFragment(MacIconFragment &fragment, FILE* file);
	static void WriteAlphaFragment(MacIconFragment &fragment, FILE* file);
	static void Write_icnV_chunk(FILE* f);
};
