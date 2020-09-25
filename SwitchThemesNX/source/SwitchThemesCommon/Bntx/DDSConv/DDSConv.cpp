#include "DDSConv.hpp"
#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"
#include "../../../../Libs/SOIL/SOIL.h"

#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../..//Platform/Platform.hpp"

using namespace std;

static int imin(int x, int y) { return (x < y) ? x : y; }

static void extractBlock(const unsigned char* src, int x, int y,
	int w, int h, unsigned char* block)
{
	int i, j;

	if ((w - x >= 4) && (h - y >= 4))
	{
		// Full Square shortcut
		src += x * 4;
		src += y * w * 4;
		for (i = 0; i < 4; ++i)
		{
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4; src += 4;
			*(unsigned int*)block = *(unsigned int*)src; block += 4;
			src += (w * 4) - 12;
		}
		return;
	}

	int bw = imin(w - x, 4);
	int bh = imin(h - y, 4);
	int bx, by;

	const int rem[] =
	{
	   0, 0, 0, 0,
	   0, 1, 0, 1,
	   0, 1, 2, 0,
	   0, 1, 2, 3
	};

	for (i = 0; i < 4; ++i)
	{
		by = rem[(bh - 1) * 4 + i] + y;
		for (j = 0; j < 4; ++j)
		{
			bx = rem[(bw - 1) * 4 + j] + x;
			block[(i * 4 * 4) + (j * 4) + 0] =
				src[(by * (w * 4)) + (bx * 4) + 0];
			block[(i * 4 * 4) + (j * 4) + 1] =
				src[(by * (w * 4)) + (bx * 4) + 1];
			block[(i * 4 * 4) + (j * 4) + 2] =
				src[(by * (w * 4)) + (bx * 4) + 2];
			block[(i * 4 * 4) + (j * 4) + 3] =
				src[(by * (w * 4)) + (bx * 4) + 3];
		}
	}
}

extern "C" char* stbi_failure_reason();
static string LastError = "";

const std::string& DDSConv::GetError()
{
	return LastError;
}

static void failWithError(const std::string &msg) 
{
	LastError = "Image error: " + msg;
	LOGf("%s", LastError.c_str());
}

vector<u8> DDSConv::ImageToDDS(const vector<u8> &imgData, bool DXT5, int ExpectedW, int ExpectedH)
{
	LastError = "";

	if ((ExpectedW % 4) || (ExpectedH % 4))
	{
		failWithError("Image size must be a multiple of 4");
		return {};
	}

	int w, h, n;
	u8* data = SOIL_load_image_from_memory(imgData.data(), imgData.size(), &w, &h, &n, 4);

	if (!data) 
	{
		failWithError(stbi_failure_reason());
		return {};
	}

	if (w != ExpectedW || h != ExpectedH)
	{
		failWithError("Wrong image size");
		return {};
	}

	const int BytePerBlock = DXT5 ? 16 : 8;

	//Hacky af but works(TM)
	Buffer bin;
	bin.ByteOrder = Endianness::LittleEndian;
	bin.Write("DDS ");
	bin.Write((u32)0x7c);
	bin.Write((u32)0xA1007);
	bin.Write((u32)h);
	bin.Write((u32)w);
	bin.Write((u32)((w * h / 16) * BytePerBlock)); //Linear size
	bin.Write((u32)0);
	bin.Write((u32)0); //Mipmap count (?)
	for (int i = 0; i < 11; i++)
		bin.Write((u32)0);
	bin.Write((u32)0x20);
	bin.Write((u32)0x4);
	bin.Write(DXT5 ? "DXT5" : "DXT1"); //Not sure about the difference between DXT3 and 5
	for (int i = 0; i < 5; i++)
		bin.Write((u32)0);
	bin.Write((u32)0x401008);
	for (int i = 0; i < 4; i++)
		bin.Write((u32)0);

	unsigned char block[64];
	vector<u8> dst(BytePerBlock);
	int x, y;

	for (y = 0; y < h; y += 4)
	{
		for (x = 0; x < w; x += 4)
		{
			extractBlock(data, x, y, w, h, block);
			stb_compress_dxt_block(dst.data(), block, DXT5, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
			bin.Write(dst);
		}
	}

	SOIL_free_image_data(data);

	return bin.getBuffer();
}

