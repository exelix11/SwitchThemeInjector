#include "UI.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include <SOIL/SOIL.h>
#include <algorithm>

#include "../Platform/Platform.hpp"

using namespace std;

const int MaxCachedImages = 8;
vector<pair<string, LoadedImage>> ImagePool;

static auto HasString(const string& str) 
{
	auto res = std::find_if(ImagePool.begin(), ImagePool.end(), [&str](pair<string, LoadedImage>& pair) { return pair.first == str; });
	return res;
}

static void PopFirst()
{
	LOGf("Pool full, popping %s\n", ImagePool[0].first.c_str());
	ImageCache::FreeImage(ImagePool[0].first);
}

static void AddValue(const string& str, LoadedImage img)
{
	ImagePool.emplace_back(str, img);
	LOGf("Pushing %s size %d\n", str.c_str(), ImagePool.size());
	if (ImagePool.size() > MaxCachedImages)
		PopFirst();
}

void ImageCache::FreeImage(const string &img)
{
	auto res = HasString(img);
	if (res == ImagePool.end()) return;
	glDeleteTextures(1, (GLuint*)&res->second);
	ImagePool.erase(res);
}

LoadedImage ImageCache::LoadDDS(const vector<u8> &data, const string &name)
{	
	auto res = HasString(name);
	if (res != ImagePool.end())
		return res->second;

	GLuint tex = SOIL_load_OGL_texture_from_memory
	(
		data.data(),
		data.size(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		0
	);

	if (tex)
		AddValue(name, tex);
	
	return tex;
}

IPage::~IPage(){}
IUIControlObj::~IUIControlObj(){}