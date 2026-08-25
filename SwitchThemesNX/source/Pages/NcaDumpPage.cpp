#include <string>

#include "NcaDumpPage.hpp"
#include "../ViewFunctions.hpp"

#include "../SwitchTools/hactool.hpp"
#include "../SwitchTools/RomfsCache.hpp"
#include "../fs.hpp"
#include "../SwitchThemesCommon/Common.hpp"

static void WriteExtracted(const std::string& basePath, const FileContainer& files)
{
	for (const auto& [name, data] : files)
	{
		auto finalPath = fs::path::SystemDataFolder + fs::JoinPath(basePath, name);
		auto parent = fs::GetParentDir(finalPath);
		fs::CreateDirectory(parent);
		fs::WriteFile(finalPath, data);
	}
}

static void WriteExtractedVersion()
{
	const auto version = std::to_string(hos::Version.major) + "." +
		std::to_string(hos::Version.minor) + "." +
		std::to_string(hos::Version.micro);
	const FileData data(version.begin(), version.end());

	fs::WriteFile(fs::path::SystemDataFolder + "extracted/ver.cfg", data);
}

NcaDumpPage::NcaDumpPage()
{
	Name = "Extract home menu";
}

void NcaDumpPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);
	ImGui::PushFont(font30);

	ImGui::TextWrapped(
		"This page allows extracting the RomFS of the home menu and related titles.\n\n"
		"This option allows extracting the common szs files (for example, ResidentMenu.szs) to your SD card so you can create custom layouts. For simply installing themes you do not need this feature, it is done automaticaly done during installation."
	);

	if (ImGui::Button("Extract szs layout files"))
	{
		PushFunction([]() 
		{
			try 
			{
				if (fs::Exists(fs::path::SystemDataFolder + "extracted/"))
					fs::DeleteDirectory(fs::path::SystemDataFolder + "extracted/");

				DisplayLoading("Extracting qlaunch...");
				WriteExtracted("extracted/qlaunch", RomfsCache::GetContent(ThemeTargetInfo::QlaunchID));

				DisplayLoading("Extracting playerselect...");
				WriteExtracted("extracted/playerselect", RomfsCache::GetContent(ThemeTargetInfo::PslID));

				DisplayLoading("Extracting mypage...");
				WriteExtracted("extracted/mypage", RomfsCache::GetContent(ThemeTargetInfo::UserPageID));

				WriteExtractedVersion();

				Dialog("The files have been extracted to the themes/systemData/extracted/ folder on your SD card.");
			}
			catch (const std::exception& ex)
			{
				DialogBlocking("Error while extracting the layout files: " + std::string(ex.what()));
			}
		});
	}

	ImGui::NewLine();

	ImGui::TextWrapped(
		"If you need the complete set of files of the home menu you can extract the raw NCA. To extract this you will need hactool or similar software on your computer."
	);
	if (ImGui::Button("Extract raw NCA files"))
	{
		PushFunction([]()
			{
				try
				{
					if (fs::Exists(fs::path::SystemDataFolder + "nca/"))
						fs::DeleteDirectory(fs::path::SystemDataFolder + "nca/");

					fs::CreateDirectory(fs::path::SystemDataFolder + "nca/");

					auto nca = hactool::GetNca(ThemeTargetInfo::QlaunchID);
					fs::WriteFile(fs::path::SystemDataFolder + "nca/" + ThemeTargetInfo::TitleIdToString(ThemeTargetInfo::QlaunchID) + ".nca", nca);
					
					nca = hactool::GetNca(ThemeTargetInfo::PslID);
					fs::WriteFile(fs::path::SystemDataFolder + "nca/" + ThemeTargetInfo::TitleIdToString(ThemeTargetInfo::PslID) + ".nca", nca);

					nca = hactool::GetNca(ThemeTargetInfo::UserPageID);
					fs::WriteFile(fs::path::SystemDataFolder + "nca/" + ThemeTargetInfo::TitleIdToString(ThemeTargetInfo::UserPageID) + ".nca", nca);

					Dialog("The files have been extracted to the themes/systemData/nca folder on your SD card.");
				}
				catch (const std::exception& ex)
				{
					DialogBlocking("Error while extracting the layout files: " + std::string(ex.what()));
				}
			});
	}
		
	ImGui::PopFont();
	Utils::ImGuiCloseWin();
}

void NcaDumpPage::Update()
{	
	if (Utils::PageLeaveFocusInput()){
		Parent->PageLeaveFocus(this);
	}
}
