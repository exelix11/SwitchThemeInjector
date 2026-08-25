#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <memory>
#include <cctype>
#include <cstdlib>
#include <sstream>
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
	constexpr std::string_view HekateBootTarget = "__boot";

	std::vector<std::tuple<std::string, std::string>> TargetInstallParts = {
		{ "Home menu",		"home"},
		{ "Lock screen",	"lock"},
		{ "All apps menu",	"apps"},
		{ "Settings applet","set"},
		{ "News applet",	"news"},
		{ "User page",		"user"},
		{ "Player selection", "psl"},
		{ "Hekate boot image", std::string(HekateBootTarget)},
	};

	std::string TrimIniValue(std::string_view value)
	{
		size_t start = 0;
		while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
			start++;

		size_t end = value.size();
		while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
			end--;

		return std::string(value.substr(start, end - start));
	}

	bool HekateBootSplashEnabled()
	{
		try
		{
			const auto path = fs::path::BootloaderDir + "hekate_ipl.ini";
			if (!fs::Exists(path))
				return false;

			const auto data = fs::OpenFile(path);
			std::istringstream lines(std::string(data.begin(), data.end()));
			std::string line;
			bool inConfigSection = false;
			while (std::getline(lines, line))
			{
				if (const auto comment = line.find_first_of("#;"); comment != std::string::npos)
					line.resize(comment);
				const auto trimmedLine = TrimIniValue(line);
				if (trimmedLine.size() >= 2 && trimmedLine.front() == '[' && trimmedLine.back() == ']')
				{
					inConfigSection = trimmedLine == "[config]";
					continue;
				}
				if (!inConfigSection)
					continue;

				const auto equals = trimmedLine.find('=');
				if (equals == std::string::npos)
					continue;

				if (TrimIniValue(std::string_view(trimmedLine).substr(0, equals)) != "bootwait")
					continue;

				const auto value = TrimIniValue(std::string_view(trimmedLine).substr(equals + 1));
				return std::atoi(value.c_str()) > 0;
			}

			return false;
		}
		catch (...)
		{
			return false;
		}
	}

	std::string HekateBootSplashWarning()
	{
		if (HekateBootSplashEnabled())
			return {};

		return "Don't forget to enable boot splash in Hekate: set the global bootwait setting to a value greater than 0.";
	}
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
	
	const bool rotatePortrait = loaded->Width() < loaded->Height();
	originalImageData = std::move(imageData);
	auto converted = ImageConversion::ToJPG(std::move(loaded), 1280, 720, true,
		rotatePortrait);
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
	std::string installWarning;
	const auto& bootSource = originalImageData.empty() ? imageData : originalImageData;
	PushPageBlocking(new InstallImageDialog(preview, imageData, resizeWarning, ShowDialogs, &result,
		bootSource, false, &installWarning));
	if (!installWarning.empty())
		AppendInstallMessage(installWarning);

	return result;
}

InstallImageDialog::InstallImageDialog(ImageRef preview,
	const std::vector<u8>& imageBytes,
	bool resizeWarning,
	bool showInstallDialogs,
	bool* outSuccess,
	std::span<const u8> bootImageBytes,
	bool showBootloaderSuccessDialog,
	std::string* outInstallWarning) :
	previewImage(preview), imageBytes(imageBytes), bootImageBytes(bootImageBytes),
	resizeWarning(resizeWarning), showInstallDialogs(showInstallDialogs),
	showBootloaderSuccessDialog(showBootloaderSuccessDialog),
	outSuccess(outSuccess), outInstallWarning(outInstallWarning)
{
	PageName = "InstallImageDialog";
	if (outSuccess) *outSuccess = false;
	if (outInstallWarning) outInstallWarning->clear();
	if (this->bootImageBytes.empty())
		this->bootImageBytes = imageBytes;

	if (!UseLowMemory)
	{
		// Warmup all the overlays in the image cache
		for (const auto& [_, part] : TargetInstallParts)
			LoadOverlayPart(part);
	}
}

ImageRef InstallImageDialog::LoadOverlayPart(const std::string& part)
{
	if (previewLoadFailure || part == HekateBootTarget)
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
		DialogBlocking("Bootloader directory not found. Make sure hekate is installed and try again.");
		return;
	}

	DisplayLoading("Installing...");

	try {
		auto image = ImageConversion::ToBootloaderBMP(bootImageBytes);
		if (!image.IsSuccess() || image.Data.empty())
		{
			DialogBlocking("Failed to convert the image to a bootloader BMP: " +
				(image.ErrorMessage.empty() ? "conversion returned no data" : image.ErrorMessage));
			return;
		}

		fs::WriteFile(fs::path::BootlogoPath, image.Data);
		if (outSuccess) *outSuccess = true;
		const auto warning = HekateBootSplashWarning();
		if (outInstallWarning) *outInstallWarning = warning;
		if (showBootloaderSuccessDialog)
		{
			std::string message = "Image installed to the bootloader successfully. Reboot to see the changes.";
			if (!warning.empty())
				message += "\n\n" + warning;
			Dialog(message);
		}
		PopPage(this);
	}
	catch (const std::exception& ex)
	{
		DialogBlocking("Failed to install the image to the bootloader: " + std::string(ex.what()));
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
	Utils::ImGuiCenterString("Select Target");
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
					if (part == HekateBootTarget)
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
		ImGui::Text("This image was automatically resized. For optimal results use 1280x720 images.");
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
