#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <utility>
#include <format>
#include "ThemeEntry.hpp"
#include "ImageEntry.hpp"
#include "../../SwitchThemesCommon/MyTypes.h"
#include "../../SwitchThemesCommon/Bntx/ImageConversion.hpp"
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
		{ "Hekate boot image", "__boot"},
	};
}

ImageEntry::ImageEntry(const std::string& fileName, std::vector<u8>&& RawData)
{
	FileName = fileName;
	lblFname = fs::GetFileName(fileName);
	lblLine1 = fileName;
	imageData = std::move(RawData);
	Icon = Icons::Type::Image;
	canInstallInternal = true;
}

void ImageEntry::PerformConversion()
{
	if (CannotInstallReason.size() || conversionDone)
		return;

	// Regardless of the result don't try again
	conversionDone = true;

	if (imageData.empty())
	{
		MakeError("The image file or format conversion failed");
		return;
	}

	// We want to allow installing images both to hekate and qlaunch
	// Previously we converted images to dds here directly but that would reduce the quality for the bootloader
	// Instead now we load the image and check the size, if the size is correct we continue with the raw image from the SD, otherwise we convert to JPG at an acceptable quality
	// We do need this conversion step to avoid trying to preview huge images which opengl might not like
	auto loaded = ImageConversion::LoadBitmap(imageData, CannotInstallReason);
	if (!loaded)
	{
		MakeError("Error loading image");
		imageData.clear();
		return;
	}

	// Exact resolution, do nothing
	if (loaded->Width() == 1280 && loaded->Height() == 720)
		return;
	
	auto converted = ImageConversion::ToJPG(std::move(loaded), 1280, 720, true);
	if (converted.ErrorMessage.size())
	{
		MakeError("Error processing file: "+ converted.ErrorMessage);
		imageData.clear();
	}
	else if (converted.Data.size() == 0)
	{
		MakeError("Image conversion failed");
		imageData.clear();
	}
	else
	{
		imageData = std::move(converted.Data);
		resizeWarning = converted.resized;
	}
}

ImageRef ImageEntry::GetConvertedImage()
{
	PerformConversion();

	if (previewImage)
		return previewImage;

	auto res = std::make_shared<RenderImage>(imageData);
	if (!res || !res->IsValid())
	{
		MakeError("Failed to load the image after conversion");
		imageData.clear();
	}

	// Cache previews only when not in applet mode
	if (!UseLowMemory)
		previewImage = res;

	return res;
}

bool ImageEntry::DoInstall(bool ShowDialogs)
{
	PerformConversion();

	if (!CanInstall() || imageData.empty())
		return false;

	auto preview = GetConvertedImage();
	if (!preview || !preview->IsValid())
		return false;

	bool result;
	PushPageBlocking(new InstallImageDialog(preview, imageData, resizeWarning, ShowDialogs, &result));

	return result;
}

InstallImageDialog::InstallImageDialog(ImageRef preview, const std::vector<u8>& imageBytes, bool resizeWarning, bool showInstallDialogs, bool* outSuccess) :
	previewImage(preview), imageBytes(imageBytes), resizeWarning(resizeWarning), showInstallDialogs(showInstallDialogs), 
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
	if (previewLoadFailure || part == "__boot")
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

void InstallImageDialog::ApplyToBootloader()
{
	if (!fs::DirectoryExists(fs::path::BootloaderDir))
	{
		Dialog("Bootloader directory not found. Make sure hekate is installed and try again.");
		return;
	}

	DisplayLoading("Installing...");

	try {
		auto image = ImageConversion::ToBootloaderBMP(imageBytes);
		fs::WriteFile(fs::path::BootlogoPath, image.Data);
		Dialog("Image installed to the bootloader successfully. Reboot to see the changes.");
	}
	catch (const std::exception& ex)
	{
		Dialog("Failed to install the image to the bootloader: " + std::string(ex.what()));
		return;
	}
}

void InstallImageDialog::ApplyToPart(const std::string& part)
{
	DisplayLoading("Installing...");
		
	// Hacky impl: build an nxtheme in memory and start the installation process
	FileContainer files =
	{
		{"info.json", ThemeFileManifest::ForInternalUse(part) },
	};

	// But only convert if needed
	if (ImageConversion::IsDDS(imageBytes))
		files["image.dds"] = FileData(imageBytes.begin(), imageBytes.end());
	else
	{
		auto conversion = ImageConversion::ToDDS(imageBytes, false, 1280, 720, true);
		if (!conversion.IsSuccess())
		{
			Dialog("Failed to convert the image to dds: " + conversion.ErrorMessage);
			return;
		}

		files["image.dds"] = std::move(conversion.Data);
	}

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
	bool first = true;
	for (const auto& [label, part] : TargetInstallParts)
	{
		ImGui::SetCursorPosX(x);

		auto id = ImGui::GetID(label.c_str());
		if (Selectable(label.c_str()))
		{
			PushFunction([this, part]()
				{
					if (part == "__boot")
						ApplyToBootloader();
					else
						ApplyToPart(part);
				});
		}

		if (first)
		{
			FirstItemHere();
			first = false;
		}

		if (ImGui::GetFocusID() == id)
			currentPart = &part;
	}

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