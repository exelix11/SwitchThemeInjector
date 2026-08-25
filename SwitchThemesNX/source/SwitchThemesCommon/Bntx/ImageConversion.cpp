#ifndef SWITCHTHEMESCOMMON_TESTS

#include <string>
#include <vector>
#include <span>
#include <optional>
#include <cstdint>
#include "ImageConversion.hpp"
#include "../MyTypes.h"
#include "../BinaryReadWrite/Buffer.hpp"
#include "../../Platform/Platform.hpp"
#include "../../../Libs/SOIL2/stb_image.h"
#include "../../../Libs/SOIL2/stb_image_write.h"
#include "../../../Libs/stb_image/stb_dxt.h"
#include "../../../Libs/stb_image/stb_image_resize2.h"
#include <utility>

namespace 
{
	struct BootloaderBMPInfo
	{
		int width;
		int height;
	};

	u16 ReadLE16(std::span<const u8> data, size_t offset)
	{
		return static_cast<u16>(data[offset]) |
			(static_cast<u16>(data[offset + 1]) << 8);
	}

	u32 ReadLE32(std::span<const u8> data, size_t offset)
	{
		return static_cast<u32>(data[offset]) |
			(static_cast<u32>(data[offset + 1]) << 8) |
			(static_cast<u32>(data[offset + 2]) << 16) |
			(static_cast<u32>(data[offset + 3]) << 24);
	}

	u32 ReadBE32(std::span<const u8> data, size_t offset)
	{
		return (static_cast<u32>(data[offset]) << 24) |
			(static_cast<u32>(data[offset + 1]) << 16) |
			(static_cast<u32>(data[offset + 2]) << 8) |
			static_cast<u32>(data[offset + 3]);
	}

	bool HasCompletePngContainer(std::span<const u8> data)
	{
		size_t offset = 8;
		while (offset <= data.size() && data.size() - offset >= 12)
		{
			const auto chunkSize = static_cast<size_t>(ReadBE32(data, offset));
			if (chunkSize > data.size() - offset - 12)
				return false;

			const bool isEnd = data[offset + 4] == 'I' && data[offset + 5] == 'E' &&
				data[offset + 6] == 'N' && data[offset + 7] == 'D';
			offset += chunkSize + 12;
			if (isEnd)
				return chunkSize == 0 && offset == data.size();
		}

		return false;
	}

	std::optional<BootloaderBMPInfo> ParseBootloaderBMP(std::span<const u8> data)
	{
		// Validate canonical bootlogo files before preserving them verbatim.
		constexpr size_t FileHeaderSize = 14;
		constexpr size_t InfoHeaderSize = 40;
		constexpr size_t MinimumHeaderSize = FileHeaderSize + InfoHeaderSize;
		if (data.size() < MinimumHeaderSize || data[0] != 'B' || data[1] != 'M')
			return std::nullopt;

		const auto dibSize = static_cast<size_t>(ReadLE32(data, FileHeaderSize));
		if (dibSize < InfoHeaderSize || dibSize > data.size() - FileHeaderSize)
			return std::nullopt;

		const auto pixelOffset = static_cast<size_t>(ReadLE32(data, 10));
		if (pixelOffset < FileHeaderSize + dibSize || pixelOffset > data.size())
			return std::nullopt;

		const auto width = static_cast<std::int32_t>(ReadLE32(data, 18));
		const auto height = static_cast<std::int32_t>(ReadLE32(data, 22));
		if (width <= 0 || height <= 0 || width > 720 || height > 1280)
			return std::nullopt;

		const auto compression = ReadLE32(data, 30);
		if (ReadLE16(data, 26) != 1 || ReadLE16(data, 28) != 32 ||
			(compression != 0 && compression != 3))
			return std::nullopt;

		const auto rowSize = static_cast<size_t>(width) * 4;
		const auto pixelSize = rowSize * static_cast<size_t>(height);
		if (pixelSize > data.size() - pixelOffset)
			return std::nullopt;

		const auto imageSize = static_cast<size_t>(ReadLE32(data, 34));
		if (imageSize != 0 && imageSize != pixelSize)
			return std::nullopt;

		const auto fileSize = static_cast<size_t>(ReadLE32(data, 2));
		const auto requiredFileSize = pixelOffset + pixelSize;
		// Reject a valid BMP header followed by response data.
		if (fileSize != data.size() || fileSize < requiredFileSize)
			return std::nullopt;

		return BootloaderBMPInfo{ width, height };
	}

	int imin(int x, int y) { return (x < y) ? x : y; }

	void extractBlock(const unsigned char* src, int x, int y, int w, int h, unsigned char* block)
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

	struct StbImageHolder : public ImageConversion::ImageBitmap
	{
		u8* data = nullptr;
		int width = 0;
		int height = 0;
		int channels = 0;
		std::string error = "";

		int Width() override { return width;}
		int Height() override { return height;}
		int Channels() override { return channels;}

		StbImageHolder(const StbImageHolder&) = delete;		
		StbImageHolder& operator=(const StbImageHolder&) = delete;
		
		StbImageHolder& operator=(StbImageHolder&& other) noexcept
		{
			data = other.data;
			width = other.width;
			height = other.height;
			error = other.error;
			isStb = other.isStb;
			channels = other.channels;

			other.data = nullptr;
			return *this;
		}

		StbImageHolder(StbImageHolder&& other) : 
			data(other.data), width(other.width), height(other.height), 
			channels(other.channels), error(other.error), isStb(other.isStb)
		{
			other.data = nullptr;
		}

		StbImageHolder(int width, int height, int channels) : 
			width(width), height(height), channels(channels)
		{
			data = new u8[width * height * channels];
			isStb = false;
		}

		StbImageHolder(std::span<const u8> image)
		{
			int original_channels;
			data = stbi_load_from_memory(image.data(), image.size(), &width, &height, &original_channels, 4);
			isStb = true;
			channels = 4;

			if (!data)
				error = stbi_failure_reason();
		}

		StbImageHolder(std::string error) : error(error) {}

		~StbImageHolder() override
		{
			if (data)
			{
				if (isStb) stbi_image_free(data);
				else delete[] data;
			}

			data = nullptr;
		}

		StbImageHolder Resize(int nextWidth, int nextHeight)
		{
			if (channels != 4)
				return StbImageHolder("Only ARGB images are supported for resizing");

			StbImageHolder result(nextWidth, nextHeight, channels);

			stbir_resize_uint8_linear(
				data, width, height, width * channels, 
				result.data, result.width, result.height, result.width * result.channels, 
				STBIR_RGBA);

			return result;
		}

		StbImageHolder Rotate90DegreesCounterclockwise() 
		{
			if (channels != 4)
				return StbImageHolder("Only ARGB images are supported for rotation");

			StbImageHolder result(height, width, channels);

			u32* source = (u32*)data;
			u32* dest = (u32*)result.data;

			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					dest[(width - 1 - w) * height + h] = source[h * width + w];
				}
			}

			return result;
		}

		StbImageHolder Rotate90DegreesClockwise()
		{
			if (channels != 4)
				return StbImageHolder("Only ARGB images are supported for rotation");

			StbImageHolder result(height, width, channels);

			u32* source = (u32*)data;
			u32* dest = (u32*)result.data;

			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					dest[w * height + (height - 1 - h)] = source[h * width + w];
				}
			}

			return result;
		}

	private:
		bool isStb = false;
	};

	StbImageHolder Resize(StbImageHolder image, int width, int height, bool& resize)
	{
		if (image.error.size())
			return StbImageHolder("Failed to load the source image: " + image.error);

		if (image.width != width || image.height != height)
		{
			if (!resize)
				return StbImageHolder("Image dimensions don't match the required ones.");

			image = image.Resize(width, height);
			if (image.error.size())
				return StbImageHolder("Failed to resize the source image: " + image.error);

			resize = true;
		}

		return image;
	}

	StbImageHolder LoadAndResize(std::span<const u8> imgData, int width, int height, bool& resize)
	{
		StbImageHolder image{ imgData };

		if (image.error.size())
			return StbImageHolder("Failed to load the source image: " + image.error);

		return Resize(std::move(image), width, height, resize);
	}

	void StbiWrite(void* context, void* data, int size)
	{
		std::vector<u8>& target = *(std::vector<u8>*)context;
		target.insert(target.end(), (u8*)data, (u8*)data + size);
	}
}

ImageConversion::BitmapRef ImageConversion::LoadBitmap(std::span<const u8> imgData, std::string& error)
{
	auto res = std::make_unique<StbImageHolder>(imgData);
	if (res->error.size())
	{
		error = res->error;
		return nullptr;
	}

	return res;
}

std::string_view ImageConversion::GetSupportedImageExtension(std::span<const u8> imgData, std::string& error)
{
	error.clear();

	std::string_view extension;
	if (imgData.size() >= 2 && imgData[0] == 'B' && imgData[1] == 'M')
	{
		if (imgData.size() < 14 || ReadLE32(imgData, 2) != imgData.size())
		{
			error = "The BMP file size is invalid";
			return {};
		}
		extension = ".bmp";
	}
	else if (imgData.size() >= 8 &&
		imgData[0] == 0x89 && imgData[1] == 'P' && imgData[2] == 'N' && imgData[3] == 'G' &&
		imgData[4] == 0x0D && imgData[5] == 0x0A && imgData[6] == 0x1A && imgData[7] == 0x0A)
	{
		if (!HasCompletePngContainer(imgData))
		{
			error = "The PNG container is incomplete or contains trailing data";
			return {};
		}
		extension = ".png";
	}
	else if (imgData.size() >= 3 && imgData[0] == 0xFF && imgData[1] == 0xD8 && imgData[2] == 0xFF)
	{
		if (imgData.size() < 4 || imgData[imgData.size() - 2] != 0xFF || imgData.back() != 0xD9)
		{
			error = "The JPEG container is incomplete or contains trailing data";
			return {};
		}
		extension = ".jpg";
	}
	else
	{
		error = "Only BMP, PNG, and JPEG images are supported";
		return {};
	}

	auto image = LoadBitmap(imgData, error);
	if (!image)
	{
		if (error.empty())
			error = "The image could not be decoded";
		return {};
	}

	return extension;
}

ImageConversion::ConversionResult ImageConversion::ToJPG(BitmapRef imgData, int Width, int Height, bool ResizeIfNeeded, bool RotatePortrait)
{
	auto casted = dynamic_cast<StbImageHolder*>(imgData.get());
	auto image = std::move(*casted);
	if (RotatePortrait && image.width < image.height)
		image = image.Rotate90DegreesClockwise();

	const bool imageResized = image.width != Width || image.height != Height;
	auto resizeAllowed = ResizeIfNeeded;
	image = Resize(std::move(image), Width, Height, resizeAllowed);

	if (image.error.size())
		return ImageConversion::ConversionResult::Fail(image.error);

	std::vector<u8> result = {};
	stbi_write_jpg_to_func(StbiWrite, &result, image.width, image.height, image.channels, image.data, 95);
	if (result.empty())
		return ImageConversion::ConversionResult::Fail("Failed to encode image as JPG");

	return ImageConversion::ConversionResult::Success(std::move(result), imageResized);
}

ImageConversion::ConversionResult ImageConversion::ToBootloaderBMP(std::span<const u8> imgData)
{
	if (auto bmp = ParseBootloaderBMP(imgData); bmp && bmp->width == 720 && bmp->height == 1280)
		return ImageConversion::ConversionResult::Success(std::vector<u8>(imgData.begin(), imgData.end()), false);

	StbImageHolder image{ imgData };
	if (image.error.size())
		return ImageConversion::ConversionResult::Fail("Failed to load image: " + image.error);

	auto needsRotate = image.width == 1280 && image.height == 720;
	auto correctResolution = image.width == 720 && image.height == 1280;

	if (!correctResolution && !needsRotate)
	{
		// Resize based on what is the biggest dimension
		if (image.width > image.height)
			image = image.Resize(1280, 720);
		else
			image = image.Resize(720, 1280);

		if (image.error.size())
			return ImageConversion::ConversionResult::Fail("Failed to resize image: " + image.error);
	}

	if (image.channels != 4)
		return ImageConversion::ConversionResult::Fail("The image does not have the right amount of color channels");

	if (image.width == 1280 && image.height == 720)
	{
		image = image.Rotate90DegreesCounterclockwise();
		if (image.error.size())
			return ImageConversion::ConversionResult::Fail("Failed to rotate image: " + image.error);
	}

	std::vector<u8> result = {};
	stbi_write_bmp_to_func(StbiWrite, &result, image.width, image.height, image.channels, image.data);
	if (result.empty())
		return ImageConversion::ConversionResult::Fail("Failed to encode image as BMP");

	return ImageConversion::ConversionResult::Success(std::move(result), false /*don't care*/);
}

bool ImageConversion::IsDDS(std::span<const u8> imgData)
{
	return imgData.size() >= 4 && imgData[0] == 'D' && imgData[1] == 'D' && imgData[2] == 'S' && imgData[3] == ' ';
}

ImageConversion::ConversionResult ImageConversion::ToDDS(std::span<const u8> imgData, bool DXT5, int Width, int Height, bool ResizeIfNeeded)
{
	if ((Width % 4) || (Height % 4))
		return ImageConversion::ConversionResult::Fail("Width and height must be multiples of 4");

	auto imageResized = ResizeIfNeeded;
	auto image = LoadAndResize(imgData, Width, Height, imageResized);
	if (image.error.size())
		return ImageConversion::ConversionResult::Fail(image.error);

	if (image.width != Width || image.height != Height)
		return ImageConversion::ConversionResult::Fail("Resize failure");

	const int BytePerBlock = DXT5 ? 16 : 8;

	//Hacky af but works(TM)
	Buffer bin;
	bin.ByteOrder = Endianness::LittleEndian;
	bin.Write("DDS ");
	bin.Write((u32)0x7c);
	bin.Write((u32)0xA1007);
	bin.Write((u32)image.height);
	bin.Write((u32)image.width);
	bin.Write((u32)((image.width * image.height / 16) * BytePerBlock)); //Linear size
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
	int x, y;

	for (y = 0; y < image.height; y += 4)
	{
		for (x = 0; x < image.width; x += 4)
		{
			extractBlock(image.data, x, y, image.width, image.height, block);
			stb_compress_dxt_block(bin.getExpandedSlice(BytePerBlock).data(), block, DXT5, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
		}
	}

	std::vector<u8> result = {};
	bin.moveOutBuffer(result);
	return ImageConversion::ConversionResult::Success(std::move(result), imageResized);
}
#endif
