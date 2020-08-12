#include "List.hpp"
#include "../../ViewFunctions.hpp"
#include "../ThemeEntry/NxEntry.hpp"
#include "../ThemeEntry/ImagePreview.hpp"
#include "Worker.hpp"
#include "../ThemePage.hpp"

#include <sstream>

const ImVec2 ImageSize = { 398, 224 };

RemoteInstall::ListPage::ListPage(API::APIResponse&& res, Worker::ImageFetch::Result&& img)
	: response(res), images(img)
{
	Selection.resize(response.Entries.size());
	ApplySelection(true);
}

void RemoteInstall::ListPage::Update()
{

}

void RemoteInstall::ListPage::Render(int X, int Y)
{
	ImGui::PushFont(font25);

	Utils::ImGuiNextFullScreen();
	ImGui::Begin("InstallList", nullptr, DefaultWinFlags);
	ImGui::SetCursorPosY(20);
	Utils::ImGuiCenterString("This link contains multiple themes, select which ones you want to download.");
	
	ImGui::Separator();

	ImGui::SetCursorPosX(22);
	ImGui::BeginChild("child", { -20 , -ImGui::GetFrameHeightWithSpacing() * 2 });
	int lineIndex = 0;
	for (size_t i = 0; i < response.Entries.size(); i++)
	{
		auto res = RenderWidget(IsSelected(i), response.Entries[i].Name, images.List[i]);
		if (res == Result::Clicked)
			ToggleSelected(i);
		else if (res == Result::Preview)
			PushPage(new ImagePreview(images.List[i]));
		else 
			Utils::ImGuiDragWithLastElement();

		if (lineIndex++ < 2)
			ImGui::SameLine();
		else
			lineIndex = 0;
	}
	Utils::ImGuiSetWindowScrollable();
	ImGui::EndChild();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::SetCursorPosX(15);
	if (ImGui::Button("Select none"))
		ApplySelection(false);
	ImGui::SameLine();
	if (ImGui::Button("Select all"))
		ApplySelection(true);

	//From https://github.com/ocornut/imgui/issues/934#issuecomment-340231002
	ImGui::PushFont(font30);
	const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;
	static float DownloadButtonWidth = 100.0f; //The 100.0f is just a guess size for the first frame.
	float pos = DownloadButtonWidth + ItemSpacing;
	ImGui::SameLine(ImGui::GetWindowWidth() - pos);
	if (ImGui::Button(DownloadBtnText.c_str()))
		DownloadClicked();
	DownloadButtonWidth = ImGui::GetItemRectSize().x; //Get the actual width for next frame.
	ImGui::PopFont();

	Utils::ImGuiSelectItemOnce();

	ImGui::SetCursorPos({ 20, 15 });
	if (ImGui::Button(" X "))
		PopPage(this);

	ImGui::End();
	ImGui::PopFont();
}

RemoteInstall::ListPage::~ListPage()
{
	for (LoadedImage i : images.List)
		Image::Free(i);
}

int RemoteInstall::ListPage::SelectedCount() const
{
	int count = 0;
	for (auto b : Selection)
		if (b) count++;
	return count;
}

void RemoteInstall::ListPage::ApplySelection(bool all)
{
	for (size_t i = 0; i < Selection.size(); i++)
		Selection[i] = all;
	SelectionChanged();
}

void RemoteInstall::ListPage::ToggleSelected(size_t i)
{
	Selection[i] = !Selection[i];
	SelectionChanged();
}

bool RemoteInstall::ListPage::IsSelected(size_t i)
{
	return Selection[i];
}

std::vector<std::string> RemoteInstall::ListPage::GetSelectedUrls()
{
	std::vector<std::string> Urls;
	for (size_t i = 0; i < response.Entries.size(); i++)
		if (IsSelected(i)) Urls.push_back(response.Entries[i].Url);
	return Urls;
}

void RemoteInstall::ListPage::SelectionChanged()
{
	std::stringstream ss;

	if (SelectedCount() == response.Entries.size())
		ss << "Download all";
	else if (SelectedCount() == 0)
		ss << "Cancel";
	else
		ss << "Download ("  << SelectedCount() << ")";

	ss << "###Download";
	DownloadBtnText = ss.str();
}

void RemoteInstall::ListPage::DownloadClicked()
{
	if (!SelectedCount())
		PopPage(this);
	else
	{
		PushFunction([this]() {
			fs::EnsureDownloadsFolderExists();

			std::string folderName = fs::path::DownloadsFolder + fs::SanitizeName(response.GroupName);

			if (fs::Exists(fs::path::DownloadsFolder + folderName)) {
				if (!YesNoPage::Ask("The themes will be downloaded to " + folderName + " but this folder already exists, existing files will be overwritten.\nDo you want to continue ?"))
					return;
			}
			else
			{
				if (!std::filesystem::is_directory(folderName))
					fs::Delete(folderName);

				fs::CreateDirectory(folderName);
			}

			folderName += '/';

			auto urls = GetSelectedUrls();

			size_t numFailed;			
			std::string OutFirstFilaName = "";			
			auto worker = new Worker::ActionOnItemFinish(urls, numFailed, [&folderName, &OutFirstFilaName](std::vector<u8>&& vec, uintptr_t index) {
				std::string name = folderName + std::to_string(index) + ".nxtheme";

				try {
					fs::WriteFile(name, vec);

					if (OutFirstFilaName == "")
						OutFirstFilaName = name;
				}
				catch (...)
				{
					// Unlikely but an exception here will probably leak CURL handles
					Dialog("Failed saving theme " + name);
				}
			});
			
			PushPageBlocking(worker);

			fs::theme::RequestThemeListRefresh();

			if (OutFirstFilaName != "")
				ThemesPage::Instance->SelectElementOnRescan(OutFirstFilaName);

			PopPage(this);
		});
	}
}

RemoteInstall::ListPage::Result RemoteInstall::ListPage::RenderWidget(bool selected, const std::string& Name, LoadedImage img)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return Result::None;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(Name.c_str()) + img;

	const ImVec2 name_size = ImGui::CalcTextSize(Name.c_str(), NULL, false, ImageSize.x - 6);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 sz = { ImageSize.x, ImageSize.y + 6 + name_size.y };

	const ImRect imageBox(pos, pos + ImageSize);

	const ImRect bb(pos, pos + sz);
	ImGui::ItemSize(sz, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return Result::None;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0) && !ImGui::GetMouseDragDelta(0).y;
	if (pressed)
		ImGui::MarkItemEdited(id);
	else if (hovered && KeyPressed(GLFW_GAMEPAD_BUTTON_X) && img)
		return Result::Preview;

	// Render
	const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::RenderNavHighlight(bb, id);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

	window->DrawList->AddImage((ImTextureID)img, imageBox.Min, imageBox.Max);

	const float Checkboxsz = 35;
	window->DrawList->AddRectFilled(pos, pos + ImVec2(Checkboxsz, Checkboxsz), ImGui::GetColorU32(ImGuiCol_FrameBg));
	if (selected)
		ImGui::RenderCheckMark(pos, ImGui::GetColorU32(ImGuiCol_CheckMark), Checkboxsz);

	ImGui::PushFont(font25);
	ImGui::RenderTextWrapped({ pos.x + 3, pos.y + ImageSize.y + 3 }, Name.c_str(), 0, ImageSize.x - 6);
	ImGui::PopFont();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);

	return pressed ? Result::Clicked : Result::None;
}