#ifndef SWITCHTHEMESCOMMON_TESTS
#include "DDS_conversion.hpp"

#include "../BinaryReadWrite/Buffer.hpp"
#include "../../Platform/Platform.hpp"

#include "../../../Libs/SOIL2/stb_image.h"
#include "../../../Libs/stb_image/stb_dxt.h"

#include <string>

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

DDSConv::ConversionResult DDSConv::ConvertImage(const std::vector<u8>& imgData, bool DXT5, int Width, int Height, bool ResizeIfNeeded)
{
	if ((Width % 4) || (Height % 4))
		return DDSConv::ConversionResult::Fail("Width and height must be multiples of 4");

	int w, h, n;
	u8* data = stbi_load_from_memory(imgData.data(), imgData.size(), &w, &h, &n, 4);

	if (!data)
		return DDSConv::ConversionResult::Fail("Failed to load the source image: "s + stbi_failure_reason());

	if (w != Width || h != Height)
	{
		if (!ResizeIfNeeded)
			return DDSConv::ConversionResult::Fail("Image dimensions don't match the required ones.");

		return DDSConv::ConversionResult::Fail("Image resize is not implemented yet.");
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

	stbi_image_free(data);

	return DDSConv::ConversionResult::Success(bin.getBuffer());
}
#endif