#include "Detail.hpp"
#include "Worker.hpp"
#include "RemoteTarget.hpp"
#include "../../fs.hpp"
#include "../../ViewFunctions.hpp"
#include "../ThemeEntry/ThemeEntry.hpp"
#include "../ThemeEntry/ImageEntry.hpp"
#include "../ImagePreview.hpp"
#include "../ThemePage.hpp"
#include "../../SwitchThemesCommon/Bntx/ImageConversion.hpp"

#include <utility>

RemoteInstall::DetailPage::DetailPage(const RemoteInstall::API::Entry& entry, ImageRef i) : entry(entry), img(i)
{
	PartName = RemoteInstall::TargetLabel(entry.Target);
}

void RemoteInstall::DetailPage::Update() {}

void RemoteInstall::DetailPage::Render(int X, int Y)
{
	ImGui::PushFont(font25);

	Utils::ImGuiNextFullScreen();
	ImGui::Begin("InstallDetail", nullptr, DefaultWinFlags);
	ImGui::SetCursorPosY(20);
	Utils::ImGuiCenterString(entry.Name);
	Utils::ImGuiCenterString(PartName);

	ImGui::SetCursorPosX(SCR_W / 4.0f);
	if (ImGui::ImageButton(img->TextureId, ImVec2(SCR_W, SCR_H) / 2))
		PushPage(new ImagePreview(img, entry.Name));

	const float BtnW = SCR_W / 3.0f;

	ImGui::SetCursorPosX(SCR_W / 2 - BtnW / 2);
	if (ImGui::Button("Install", ImVec2(BtnW, 0)))
		UserDownload(Action::DownloadInstall);
	Utils::ImGuiSelectItemOnce();
	
	ImGui::SetCursorPosX(SCR_W / 2 - BtnW / 2);
	if (ImGui::Button("Install but don't save to the SD card", ImVec2(BtnW, 0)))
		UserDownload(Action::Install);
	
	ImGui::SetCursorPosX(SCR_W / 2 - BtnW / 2);
	if (ImGui::Button("Just download", ImVec2(BtnW, 0)))
		UserDownload(Action::Download);
	
	ImGui::NewLine();
	ImGui::SetCursorPosX(SCR_W / 2 - BtnW / 2);
	if (ImGui::Button("Cancel", ImVec2(BtnW, 0)))
		PopPage(this);

	ImGui::End();
	ImGui::PopFont();
}

void RemoteInstall::DetailPage::UserDownload(Action action)
{
	const bool isImage = RemoteInstall::IsImage(entry.Target);
	PushFunction([this, action, isImage]() {
		auto theme = DownloadData();
		if (theme.size() == 0) return;

		std::string imageError;
		const auto imageExtension = isImage ?
			ImageConversion::GetSupportedImageExtension(theme, imageError) : std::string_view{};
		if (isImage && imageExtension.empty())
		{
			DialogBlocking("The downloaded image is not valid: " + imageError);
			return;
		}

		std::unique_ptr<ThemeEntry> themeEntry;
		if (isImage)
		{
			themeEntry = std::make_unique<ImageEntry>(this->entry.Name + std::string(imageExtension), std::move(theme));
		}
		else
			themeEntry = ThemeEntry::FromMemory(theme);

		if (!themeEntry->CanInstall())
		{
			DialogBlocking("This theme is not valid");
			return;
		}

		if ((int)action & (int)Action::Download)
		{
			fs::EnsureDownloadsFolderExists();
			const auto extension = isImage ? imageExtension : std::string_view(".nxtheme");
			std::string name = fs::path::DownloadsFolder + fs::SanitizeName(this->entry.Name) + std::string(extension);
			if (fs::Exists(name) && !YesNoPage::Ask("A file called " + name + " already exists on the sd card, do you want to replace it ?"))
			{
				if (action == Action::Download) // If the user asked to download the theme don't close the page, otherwise just install it
					return;
			}
			else
			{
				fs::WriteFile(name, isImage ? DownloadedTheme : theme);
				fs::theme::RequestThemeListRefresh();
				ThemesPage::Instance->SelectElementOnRescan(name);
			}
		}

		if ((int)action & (int)Action::Install)
		{
			if (!themeEntry->Install(true) && isImage)
			{
				if (!themeEntry->CanInstall() && !themeEntry->CannotInstallReason.empty())
					DialogBlocking(themeEntry->CannotInstallReason);
				return;
			}
		}

		PopPage(this);
	});
}

std::vector<u8> RemoteInstall::DetailPage::DownloadData()
{
	if (DownloadedTheme.size())
		return DownloadedTheme;

	PushPageBlocking(new Worker::DownloadSingle(entry.Url, DownloadedTheme));

	return DownloadedTheme;
}
