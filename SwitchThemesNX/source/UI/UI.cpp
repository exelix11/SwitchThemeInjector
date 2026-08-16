#include "UI.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include <algorithm>
#include <unordered_map>

#include "imgui/imgui_internal.h"

#include "../ViewFunctions.hpp"
#include "../fs.hpp"
#include "../Platform/Platform.hpp"
#include "../../Libs/SOIL2/SOIL2.h"

static_assert(sizeof(GLuint) <= sizeof(ImTextureID)); //We must not lose data when passing the image to ImGui

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

size_t RenderImage::LeakCount = 0;

RenderImage::RenderImage(const std::vector<u8>& data) : 
	Width(0), Height(0), TextureId(0)
{
	int orig_channels = 0;
	auto img = SOIL_load_image_from_memory(
		data.data(), data.size(),
		&Width, &Height, &orig_channels,
		4);
	
	if (!img)
	{
		LOGf("Failed to load image, SOIL error: %s\n", SOIL_last_result());
		Invalidate();
		return;
	}

	int w = Width;
	int h = Height;

	auto tex_id = SOIL_create_OGL_texture(img, &w, &h, 4, 0, 0);		
	SOIL_free_image_data(img);

	if (tex_id == 0)
	{
		LOGf("Failed to load image, SOIL error: %s\n", SOIL_last_result());
		Invalidate();
		return;
	}

	if (w != Width || h != Height)
		LOGf("Warning: SOIL resized the image from %dx%d to %dx%d\n", Width, Height, w, h); 

	TextureId = (ImTextureID)(intptr_t)tex_id;

	LeakCount++;
}

RenderImage::RenderImage(RenderImage&& other) 
{
	Width = other.Width;
	Height = other.Height;
	TextureId = other.TextureId;

	other.Invalidate();
}

RenderImage& RenderImage::operator=(RenderImage&& other)
{
	Width = other.Width;
	Height = other.Height;
	TextureId = other.TextureId;

	other.Invalidate();
	return *this;
}

void RenderImage::Release()
{
	if (TextureId)
	{
		GLuint id = (GLuint)(uintptr_t)TextureId;
		glDeleteTextures(1, &id);
		--LeakCount;
	}

	Invalidate();
}

RenderImage::~RenderImage()
{
	Release();
}

size_t RenderImage::DebugLoadedImages() 
{
	return LeakCount;
}

void RenderImage::DebugAssertLeaks()
{
	ImageCache::Clear();
	if (LeakCount)
		throw std::runtime_error("Leaking images !");	
}

using CacheEntry = std::pair<std::string, ImageRef>;

namespace 
{
	std::unordered_map<Icons::Type, ImageRef> IconCache;
	std::vector<CacheEntry> ImagePool;
	size_t CurrentCacheSize = 0;

	const size_t CacheSizeLowMem = 10 * 1024 * 1024; //10mb
	const size_t CacheSize = 150 * 1024 * 1024; 

	size_t EstimateSize(const ImageRef& image)
	{
		return image->Width * image->Height * 4; //RGBA8
	}

	auto HasString(const std::string& str)
	{
		auto res = std::find_if(ImagePool.begin(), ImagePool.end(),
			[&str](const CacheEntry& pair) { return pair.first == str; });
		return res;
	}

	static size_t MaxCacheSize() {
		return UseLowMemory ? CacheSizeLowMem : CacheSize;
	}

	static void AddValue(const std::string& str, ImageRef img)
	{
		ImagePool.emplace_back(str, img);
		LOGf("Pushing %s size %lu\n", str.c_str(), ImagePool.size());

		CurrentCacheSize += EstimateSize(img);

		while (ImagePool.size() > 1 && CurrentCacheSize > MaxCacheSize())
			ImageCache::PopOne();
	}
}

void ImageCache::PopOne()
{
	LOGf("popping %s\n", ImagePool[0].first.c_str());
	ImageCache::FreeImage(ImagePool[0].first);
}

void ImageCache::Clear()
{
	CurrentCacheSize = 0;
	ImagePool.clear();
}

void ImageCache::FreeImage(const std::string &img)
{
	auto res = HasString(img);
	if (res == ImagePool.end()) return;
	CurrentCacheSize -= EstimateSize(res->second);
	ImagePool.erase(res);
}

void ImageCache::DebugInformation(size_t& out_size, int& out_count, float& out_percent)
{
	auto max = (float)MaxCacheSize();
	out_size = CurrentCacheSize;
	out_count = ImagePool.size();
	out_percent = max == 0 ? 0 : (CurrentCacheSize / max) * 100.0f;
}

ImageRef ImageCache::Get(const std::string& name)
{
	auto res = HasString(name);
	if (res != ImagePool.end())
		return res->second;

	return nullptr;
}

ImageRef ImageCache::Load(const std::vector<u8> &data, const std::string &name)
{	
	auto res = HasString(name);
	if (res != ImagePool.end())
		return res->second;

	auto image = std::make_shared<RenderImage>(data);

	// Especially in applet mode, sometimes loading images fails due to memory fragmentation, even if we have enough free memory.
	// Try to free some cache and try again a couple of times
	int attempt = 0;
	while (!image->IsValid() && ImagePool.size() > 1)
	{
		LOGf("Loading image %s failed, trying to free some cache and retry (attempt %d)\n", name.c_str(), attempt);
	
		PopOne();
		image = std::make_shared<RenderImage>(data);

		if (++attempt >= 3)
		{
			LOGf("Failed to load image %s after multiple attempts, giving up\n", name.c_str());
			break;
		}
	}

	LOGf("Loaded image %s, size %dx%d, valid: %d\n", name.c_str(), image->Width, image->Height, image->IsValid());
	if (image->IsValid())
		AddValue(name, image);
	
	return image;
}

ImageRef Icons::GetIcon(Icons::Type type)
{
	if (IconCache.count(type))
		return IconCache[type];

	auto name = ASSET("icons/xmark.png");
	switch (type)
	{
		case Type::Folder: name = ASSET("icons/folder-open.png"); break;
		case Type::File: name = ASSET("icons/file.png"); break;
		case Type::Theme: name = ASSET("icons/palette.png"); break;
		case Type::Font: name = ASSET("icons/font.png"); break;
		case Type::Image: name = ASSET("icons/image.png"); break;
		case Type::Question: name = ASSET("icons/question.png"); break;
	}

	auto image = std::make_shared<RenderImage>(fs::OpenFile(name));
	IconCache[type] = image;

	return image;
}

void Icons::DropCache()
{
	IconCache.clear();
}

void Icons::RenderRaw(Icons::Type type, float x, float y, float width, float height)
{
	auto icon = GetIcon(type);
	if (!icon->IsValid())
		return;

	auto window = ImGui::GetCurrentWindow();
	window->DrawList->AddImage(icon->TextureId, ImVec2(x, y), ImVec2(x, y) + ImVec2(width, height));
}

void Icons::Render(Icons::Type type, float x, float y)
{
	auto icon = GetIcon(type);
	if (!icon->IsValid())
		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(x, y));
	else 
		ImGui::Image(icon->TextureId, { x, y });

	ImGui::SameLine();
}