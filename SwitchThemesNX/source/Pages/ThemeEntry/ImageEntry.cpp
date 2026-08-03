#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <utility>
#include <format>
#include "ThemeEntry.hpp"
#include "ImageEntry.hpp"
#include "../../SwitchThemesCommon/MyTypes.h"
#include "../../SwitchThemesCommon/Bntx/DDS_conversion.hpp"
#include "../../SwitchThemesCommon/Common.hpp"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../../UI/UI.hpp"
#include "../../fs.hpp"
#include "../../ViewFunctions.hpp"

namespace 
{
	std::vector<std::tuple<std::string, std::string>> TargetInstallParts = {
		{ "Home menu",		"home"},
		{ "Lock screen",	"lock"},
		{ "All apps menu",	"apps"},
		{ "Settings applet","set"},
		{ "News applet",	"news"},
		{ "User page",		"user"},
		{ "Player selection", "psl"},
	};
}

ImageEntry::ImageEntry(const std::string& fileName, std::vector<u8>&& RawData)
{
	FileName = fileName;
	lblFname = fs::GetFileName(fileName);
	lblLine1 = fileName;
	lblLine2 = "Image file";
	_originalData = std::move(RawData);
}

void ImageEntry::PerformConversion()
{
	if (CannotInstallReason.size() || _convertedDds.size())
		return;

	if (_originalData.empty())
	{
		CannotInstallReason = "No data to convert";
		lblLine1 = "Error loading file";
		return;
	}

	auto dds = DDSConv::ConvertImage(_originalData, false, 1280, 720, true);
	_originalData.clear();

	if (dds.ErrorMessage.size())
	{
		CannotInstallReason = dds.ErrorMessage;
		lblLine1 = "Error loading file";
	}
	else
	{
		_convertedDds = std::move(dds.Data);
		_resizeWarning = dds.resized;
	}
}

ImageRef ImageEntry::GetConvertedImage()
{
	PerformConversion();

	if (_previewImage)
		return _previewImage;

	auto res = std::make_shared<RenderImage>(_convertedDds);
	if (!res || !res->IsValid())
	{
		CannotInstallReason = "Failed to load the image after conversion";
		lblLine1 = "Error loading file";
		_convertedDds.clear();
	}

	// Cache previews only when not in applet mode
	if (!UseLowMemory)
		_previewImage = res;

	return res;
}

bool ImageEntry::DoInstall(bool ShowDialogs)
{
	PerformConversion();

	if (!CanInstall() || _convertedDds.empty())
		return false;

	auto preview = GetConvertedImage();
	if (!preview || !preview->IsValid())
		return false;

	bool result;
	PushPageBlocking(new InstallImageDialog(preview, _convertedDds, _resizeWarning, ShowDialogs, &result));

	return result;
}

InstallImageDialog::InstallImageDialog(ImageRef preview, const std::vector<u8>& ddsImage, bool resizeWarning, bool showInstallDialogs, bool* outSuccess) :
	previewImage(preview), ddsImage(ddsImage), resizeWarning(resizeWarning), showInstallDialogs(showInstallDialogs), 
	outSuccess(outSuccess)
{
	PageName = "InstallImageDialog";
	if (outSuccess) *outSuccess = false;

	if (!UseLowMemory)
	{
		// Warmup all the overlays in the image cache
		for (const auto& [_, part] : TargetInstallParts)
			LoadOverlayPart(part);
	}
}

ImageRef InstallImageDialog::LoadOverlayPart(const std::string& part)
{
	if (previewLoadFailure)
		return nullptr;

	std::string cacheKey = "preview_overlay://";
	cacheKey.append(part);

	ImageRef res = ImageCache::Get(cacheKey);
	if (res) return res;

	auto path = ASSET("preview/") + part + ".png";
	try 
	{
		auto image = fs::OpenFile(path);
		res = ImageCache::Load(image, cacheKey);
	}
	catch(const std::exception& ex)
	{
		LOGf("%s", ex.what());
		previewError = ex.what();
	}

	if (!res || !res->IsValid())
	{
		previewLoadFailure = true;
		return nullptr;
	}

	return res;
}

void InstallImageDialog::ApplyToPart(const std::string& part)
{
	DisplayLoading("Installing...");

	// Hacky impl: build an nxtheme in memory and start the installation process
	FileContainer files =
	{
		{"info.json", ThemeFileManifest::ForInternalUse(part) },
		{"image.dds", FileData(ddsImage.begin(), ddsImage.end()) },
	};

	auto entry = NxEntry("theme", std::move(files));
	if (!entry.CanInstall())
	{
		Dialog("Failed to build the theme file. Open an issue on github.\n" + entry.CannotInstallReason);
		return;
	}

	auto res = entry.Install(showInstallDialogs);
	
	if (outSuccess) *outSuccess = res;
	PopPage(this);
}

void InstallImageDialog::RenderTop() 
{
	ImGui::PushFont(font40);
	Utils::ImGuiCenterString("Set theme wallpaper");
	ImGui::PopFont();
	Utils::ImGuiCenterString("Select where you want to apply this image");
	PaddingLine();
}

void InstallImageDialog::RenderLeftPanel(float allowedWidth) 
{
	auto cursor = ImGui::GetCursorPos();

	auto previewRatio = (float)previewImage->Height / previewImage->Width;
	auto previewHeight = allowedWidth * previewRatio;
	ImGui::Image(previewImage->TextureId, { allowedWidth, previewHeight });

	if (previewOverlay && previewOverlay->IsValid())
	{
		ImGui::SetCursorPos(cursor);
		ImGui::Image(previewOverlay->TextureId, { allowedWidth, previewHeight });
	}
}

void InstallImageDialog::RenderRightPanel(float x, float allowedWidth, float endY) 
{
	const std::string* currentPart = nullptr;
	for (const auto& [label, part] : TargetInstallParts)
	{
		ImGui::SetCursorPosX(x);

		auto id = ImGui::GetID(label.c_str());
		if (Selectable(label.c_str()))
		{
			PushFunction([this, part]()
				{
					ApplyToPart(part);
				});
		}

		if (ImGui::GetFocusID() == id)
			currentPart = &part;
	}

	if (ImGui::GetFocusID() == 0)
		ImGui::SetFocusID(ImGui::GetID("Home menu"), ImGui::GetCurrentWindow());

	if (currentPart && *currentPart != currentPreviewOverlay)
	{
		currentPreviewOverlay = *currentPart;
		previewOverlay = LoadOverlayPart(*currentPart);
	}
}

void InstallImageDialog::RenderBottom() 
{	
	if (resizeWarning)
	{
		auto x = ImGui::GetCursorPosX();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
		ImGui::Text("This image was automatically resized. For optimal resuls use 1280x720 images.");
		ImGui::PopStyleColor();

		ImGui::SetCursorPosX(x);
	}

	if (previewLoadFailure)
	{
		auto x = ImGui::GetCursorPosX();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
		if (UseLowMemory)
			ImGui::Text("Failed to load previews. You are running in applet mode, this is not supported. Relaunch with title takeover");
		else
			ImGui::Text("Failed to load previews.");

		if (!previewError.empty())
		{
			ImGui::SetCursorPosX(x);
			ImGui::PushTextWrapPos(SCR_W - PaddingSizeX);
			ImGui::TextWrapped("%s", previewError.c_str());
			ImGui::PopTextWrapPos();
		}

		ImGui::PopStyleColor();
	}
}