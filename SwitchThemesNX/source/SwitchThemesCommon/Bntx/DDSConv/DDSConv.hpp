#pragma once
#ifndef SWITCHTHEMESCOMMON_TESTS
//This is not properly part of SwitchThemesCommon but it's needed for installing since support for png background instead of dds
//The image must be converted to DDS so it can go through the already existing lib

#include <vector>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../../../UI/glad.h"
#include "../../MyTypes.h"

namespace DDSConv
{	
	std::vector<u8> ImageToDDS(const std::vector<u8>& imgData, bool DXT5 = false, int ExpectedW = 1280, int ExpectedH = 720);
	const std::string& GetError();
}
#endif