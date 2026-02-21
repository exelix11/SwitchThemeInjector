#pragma once
#ifndef SWITCHTHEMESCOMMON_TESTS
//This is not properly part of SwitchThemesCommon but it's needed for installing since support for png background instead of dds
//The image must be converted to DDS so it can go through the already existing lib

#include <vector>
#include <string>

#include "../MyTypes.h"

namespace DDSConv
{	
	struct ConversionResult 
	{
		std::vector<u8> Data;
		std::string ErrorMessage;

		bool IsSuccess() const { return ErrorMessage.empty(); }

		static ConversionResult Success(std::vector<u8> data) 
		{
			return { data, "" };
		}

		static ConversionResult Fail(std::string error) 
		{
			return { {}, error };
		}
	};

	ConversionResult ConvertImage(const std::vector<u8>& imgData, 
		bool DXT5 = false, 
		int Width = 1280, 
		int Height = 720,
		bool ResizeIfNeeded = false);
}
#endif