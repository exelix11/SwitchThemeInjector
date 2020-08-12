#include "UI.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include <SOIL/SOIL.h>
#include <algorithm>

#include "../Platform/Platform.hpp"

#include "imgui/imgui_internal.h"
#include "../ViewFunctions.hpp"

static_assert(std::is_same<GLuint, LoadedImage>::value);
static_assert(sizeof(LoadedImage) <= sizeof(ImTextureID)); //We must not lose data when passing the image to ImGui

//moved here from ViewFunctions as it needs static variables
void Utils::ImGuiDragWithLastElement()
{
	static float scrollY = 0;
	static ImGuiID PrevItem = 0;
	const auto ScrollItem = GImGui->CurrentWindow->DC.LastItemId;
	if (ImGui::IsItemActive()) // Is the scrolling item active ?
	{
		if (!PrevItem) //If we're not scrolling, begin.
		{
			PrevItem = ScrollItem;
			scrollY = ImGui::GetScrollY();
		}
		if (PrevItem == ScrollItem) //Calculate the scrolling
		{			
			ImVec2 drag = ImGui::GetMouseDragDelta(0);
			ImGui::SetScrollY(scrollY - drag.y);
		}
	}	
	else if (PrevItem == ScrollItem) //we were scrolling but now we stopped
	{
		scrollY = 0;
		PrevItem = 0;
	}
}

static size_t ImageCount = 0;

void Image::Free(LoadedImage img)
{
	if (img)
	{
		ImageCount--;
		glDeleteTextures(1, &img);
	}
}

LoadedImage Image::Load(const std::vector<u8>& data)
{
	auto img = SOIL_load_OGL_texture_from_memory
	(
		data.data(),
		data.size(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		0
	);
	if (img) ImageCount++;
	return img;
}

void Image::Internal::AssertOnLeaks()
{
	ImageCache::Clear();
	if (ImageCount)
		throw std::runtime_error("Leaking images !");
}

using namespace std;

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
	LOGf("Pushing %s size %lu\n", str.c_str(), ImagePool.size());

	const u32 MaxCachedImages = UseLowMemory ? 2 : 7;
	if (ImagePool.size() > MaxCachedImages)
		PopFirst();
}

void ImageCache::Clear()
{
	for (const auto& i : ImagePool)
		Image::Free(i.second);

	ImagePool.clear();
}

void ImageCache::FreeImage(const string &img)
{
	auto res = HasString(img);
	if (res == ImagePool.end()) return;
	Image::Free(res->second);
	ImagePool.erase(res);
}

LoadedImage ImageCache::LoadDDS(const vector<u8> &data, const string &name)
{	
	auto res = HasString(name);
	if (res != ImagePool.end())
		return res->second;

	GLuint tex = Image::Load(data);

	if (tex)
		AddValue(name, tex);
	
	return tex;
}

IPage::~IPage(){}
IUIControlObj::~IUIControlObj(){}
