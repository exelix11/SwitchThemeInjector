#include "QlaunchPatchPage.hpp"
#include "../ViewFunctions.hpp"
#include "RemoteInstall/Worker.hpp"

class ThemeUpdateDownloader : public RemoteInstall::Worker::BaseWorker {
public:
	struct Result {
		std::string error;
		long httpCode;
		std::vector<u8> data;
	};

	ThemeUpdateDownloader(const std::string& url, Result& r) : BaseWorker({url}, true), OutResult(r) {
		appendUrlToError = false;
		SetLoadingLine("Checking for patch updates...");
	}

protected:
	void OnComplete() {
		const auto& str = Errors.str();
		if (str.length())
			OutResult.error = str;
		else
			OutResult.data = Results.at(0);
	}

	bool OnFinished(uintptr_t index, long httpCode) override {
		OutResult.httpCode = httpCode;
		return true;
	}

	Result& OutResult;
};

QlaunchPatchPage::QlaunchPatchPage() : IPage("Themes patches") { }

void QlaunchPatchPage::Render(int X, int Y)
{
	Utils::ImGuiSetupPage(this, X, Y);

	ImGui::TextWrapped(
		"Since firmware 9.0 some parts of the home menu require to be patched in order to install themes.\n"
		"If you see this screen it means you don't have the patches needed for your firmware installed."
	);	

	if (PatchMng::QlaunchBuildId() != "")
	{
		ImGui::Text("Your home menu version is the following (BuildID) :");
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
		Utils::ImGuiCenterString(PatchMng::QlaunchBuildId());
		ImGui::PopStyleColor();
	}
	else 
	{
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
		ImGui::Text("Error: couldn't detect your home menu version");
		ImGui::PopStyleColor();
	}

	if (patchStatus == PatchMng::InstallResult::MissingIps) 
	{		
		ImGui::TextWrapped("This version is not currently supported, after a new firmware is released it can take a few days for the patches to be updated");
		ImGui::TextWrapped(
			"New patches are now automatically downloaded from the internet whenever you launch this application. "
			"If you want you can also check for updates now."
		);
		
		if (ImGui::Button("Check for updates"))
			PushFunction([this]() { CheckForUpdates(); });

		if (updateMessageString != "")
		{
			ImGui::SameLine();

			if (updateMessageIsError)
				ImGui::PushStyleColor(ImGuiCol_Text, Colors::Red);
			else ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
			
			ImGui::TextWrapped(updateMessageString.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::TextWrapped(
			"If you don't want to connect your console to the internet you can manually download the patches by following the the instructions at:"
		);
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
		ImGui::Text("https://github.com/exelix11/theme-patches");
		ImGui::PopStyleColor();
	}
	else if (patchStatus == PatchMng::InstallResult::SDError)
	{
		ImGui::TextWrapped(
			"There was an error reading or writing files from your SD card, this usually means your SD is corrupted.\n"
			"Please run the archive bit fixer, if that still doesn't work format your SD and set it up from scratch."
		);
	}
	else if (patchStatus == PatchMng::InstallResult::UnsupportedCFW)
	{
		ImGui::TextWrapped(
			"Your CFW doesn't seem to be supported.\n"
			"If your CFW is supported and you're seeing this there's probably something wrong with your SD card, install your CFW again."
		);
	}
	else if (patchStatus == PatchMng::InstallResult::Ok)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::Highlight);
		ImGui::Text("Successfully updated, reboot your console !");
		ImGui::PopStyleColor();
	}

	Utils::ImGuiSetWindowScrollable();
	Utils::ImGuiCloseWin();
}

void QlaunchPatchPage::Update()
{
	if (Utils::PageLeaveFocusInput())
		Parent->PageLeaveFocus(this);
}

void QlaunchPatchPage::CheckForUpdates() {
	ThemeUpdateDownloader::Result res;
	PushPageBlocking(new ThemeUpdateDownloader("https://exelix11.github.io/theme-patches/ips/" + PatchMng::QlaunchBuildId(), res));

	if (res.error != "")
	{
		updateMessageIsError = true;
		updateMessageString = res.error;
	}
	else if (res.httpCode == 404)
	{
		updateMessageIsError = false;
		updateMessageString = "No update found";
	}
	else if (res.httpCode != 200)
	{
		updateMessageIsError = true;
		updateMessageString = "HTTP error: code " + res.httpCode;
	}
	else
	{
		updateMessageIsError = false;
		fs::patches::WritePatchForBuild(PatchMng::QlaunchBuildId(), res.data);
		patchStatus = PatchMng::EnsureInstalled();
		updateMessageString = "Successfully updated, reboot your console !";
	}
}

bool QlaunchPatchPage::ShouldShow()
{
	patchStatus = PatchMng::EnsureInstalled();

	if (patchStatus == PatchMng::InstallResult::Ok)
		return false;

	if (patchStatus == PatchMng::InstallResult::MissingIps)
	{
		CheckForUpdates();
		// Has anything changed ? 
		if (patchStatus == PatchMng::InstallResult::Ok)
			return false;
	}
	
	return true;
}
