#include <variant>
#include <utility>
#include <string>
#include <memory>
#include <unordered_map>
#include "../../ViewFunctions.hpp"
#include "../../SwitchThemesCommon/Common.hpp"
#include "../../UI/UI.hpp"
#include "../ImagePreview.hpp"
#include "ThemeOptions.hpp"
#include "ImageEntry.hpp"
#include "ThemeEntry.hpp"

void ThemeOptionsDialog::RenderTop() 
{
	ImGui::PushFont(font40);
	Utils::ImGuiCenterString("Theme options");
	ImGui::PopFont();
	Utils::ImGuiCenterString(name);
	PaddingLine();
}

void ThemeOptionsDialog::RenderLeftPanel(float allowedWidth) 
{
	if (previewImage)
	{
		auto previewRatio = (float)previewImage->Height / previewImage->Width;
		auto previewHeight = allowedWidth * previewRatio;
		ImGui::Image(previewImage->TextureId, { allowedWidth, previewHeight });
	}
	else
	{
		auto previewRatio = 1280.0f/720.0f;
		auto previewHeight = allowedWidth * previewRatio;

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + allowedWidth);
		ImGui::TextWrapped(previewError.c_str());
		ImGui::PopTextWrapPos();
	}
}

void ThemeOptionsDialog::RenderRightPanel(float x, float allowedWidth, float endY) 
{
	const std::string* currentPart = nullptr;
	
	ImGui::SetCursorPosX(x);
	if (previewImage) 
	{
		if (Selectable("Preview")) 
			Preview();
		ImGui::SetCursorPosX(x);

		if (ImGui::GetFocusID() == 0)
			ImGui::SetFocusID(ImGui::GetItemID(), ImGui::GetCurrentWindow());

		if (Selectable("Install image only"))
			PushFunction([this]() { InstallOnlyImage(); });
		ImGui::SetCursorPosX(x);
	}

	if (theme.HasMainLayout() || theme.HasCommonLayout())
	{
		if (Selectable("Install layout only"))
			PushFunction([this]() { InstallOnlyLayout(); });
		ImGui::SetCursorPosX(x);

		if (ImGui::GetFocusID() == 0)
			ImGui::SetFocusID(ImGui::GetItemID(), ImGui::GetCurrentWindow());
	}
}

void ThemeOptionsDialog::RenderBottom() 
{

}

void ThemeOptionsDialog::Initialize() 
{
	auto img = theme.GetMainImage();
	if (std::holds_alternative<std::string>(img))
	{
		previewError = std::get<std::string>(img);
		return;
	}

	rawImage = std::move(std::get<FileData>(img));
	previewImage = std::make_shared<RenderImage>(rawImage);
}

void ThemeOptionsDialog::Preview() 
{
	if (previewImage)
		PushPage(new ImagePreview(previewImage, name));
}

void ThemeOptionsDialog::InstallOnlyImage() 
{
	if (previewImage)
	{
		bool success = false;
		PushPageBlocking(new InstallImageDialog(previewImage, rawImage, false, true, &success));
		if (success)
			PopPage(this);
	}
}

void ThemeOptionsDialog::InstallOnlyLayout() 
{
	DisplayLoading("Installing...");

	// Hack here too: build an nxtheme in memory
	FileContainer files =
	{
		{"info.json", theme.GetRawManifest() }
	};

	if (theme.HasMainLayout())
		files["layout.json"] = theme.GetRawMainLayout();

	if (theme.HasCommonLayout())
		files["common.json"] = theme.GetRawCommonLayout();

	auto entry = NxEntry("theme", std::move(files));
	if (!entry.CanInstall())
	{
		Dialog("Failed to build the theme file. Open an issue on github.\n" + entry.CannotInstallReason);
		return;
	}

	if (entry.Install(true))
		PopPage(this);
}