#include "Detail.hpp"
#include "../../ViewFunctions.hpp"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../ThemeEntry/ImagePreview.hpp"
#include "Worker.hpp"
#include "../ThemeEntry/ThemeEntry.hpp"
#include "../ThemePage.hpp"

RemoteInstall::DetailPage::DetailPage(const RemoteInstall::API::Entry& entry, LoadedImage i) : entry(entry), img(i)
{
	PartName = ThemeTargetToName.count(entry.Target) ? ThemeTargetToName[entry.Target] : "Unknown part name";
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
	if (ImGui::ImageButton((ImTextureID)(uintptr_t)img, ImVec2(SCR_W, SCR_H) / 2))
		PushPage(new ImagePreview(img));

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
	ImGui::Button("Cancel", ImVec2(BtnW, 0));

	ImGui::End();
	ImGui::PopFont();
}

RemoteInstall::DetailPage::~DetailPage()
{
	if (img)
		Image::Free(img);
}

void RemoteInstall::DetailPage::UserDownload(Action action)
{
	PushFunction([this, action]() {
		auto&& theme = DownloadData();
		if (theme.size() == 0) return;
		
		auto entry = ThemeEntry::FromSZS(theme);
		if (!entry->CanInstall())
		{
			DialogBlocking("This theme is not valid");
			return;
		}

		if ((int)action & (int)Action::Download)
		{
			fs::EnsureDownloadsFolderExists();
			std::string name = fs::path::DownloadsFolder + fs::SanitizeName(this->entry.Name) + ".nxtheme";
			if (fs::Exists(name) && !YesNoPage::Ask("A file called " + name + " already exists on the sd card, do you want to replace it ?"))
			{
				if (action == Action::Download) // If the user asked to download the theme don't close the page, otherwise just install it
					return;
			}
			else
			{
				fs::WriteFile(name, theme);
				fs::theme::RequestThemeListRefresh();
				ThemesPage::Instance->SelectElementOnRescan(name);
			}
		}

		if ((int)action & (int)Action::Install)
			entry->Install(true);

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
