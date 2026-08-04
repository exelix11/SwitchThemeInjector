#ifndef SWITCHTHEMESCOMMON_TESTS
#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../MyTypes.h"

namespace ImageConversion
{	
	struct ConversionResult 
	{
		std::vector<u8> Data;
		std::string ErrorMessage;
		bool resized;

		bool IsSuccess() const { return ErrorMessage.empty(); }

		static ConversionResult Success(std::vector<u8> data, bool resized) 
		{
			return { std::move(data), "", resized };
		}

		static ConversionResult Fail(std::string error) 
		{
			return { {}, error, false };
		}
	};

	struct ImageBitmap;
	using BitmapRef = std::unique_ptr<ImageBitmap>;

	struct ImageBitmap
	{
		virtual ~ImageBitmap() = default;

		virtual int Width() = 0;
		virtual int Height() = 0;
		virtual int Channels() = 0;
	};

	bool IsDDS(std::span<const u8> imgData);

	BitmapRef LoadBitmap(std::span<const u8> imgData, std::string& error);

	ConversionResult ToDDS(std::span<const u8> imgData,
		bool DXT5 = false,
		int Width = 1280,
		int Height = 720,
		bool ResizeIfNeeded = false);

	ConversionResult ToJPG(BitmapRef imgData,
		int Width = 1280,
		int Height = 720,
		bool ResizeIfNeeded = false);

	ConversionResult ToBootloaderBMP(std::span<const u8> imgData);
}
#endif