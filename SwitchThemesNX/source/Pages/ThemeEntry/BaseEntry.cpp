#include <filesystem>
#include <stdexcept>
#include <exception>
#include <vector>
#include <utility>

#include "ThemeEntry.hpp"
#include "../ImagePreview.hpp"
#include "../../ViewFunctions.hpp"
#include "../../fs.hpp"
#include "../../Platform/Platform.hpp"
#include "../../UI/UI.hpp"
#include "../../SwitchThemesCommon/MyTypes.h"

using namespace std;
using namespace SwitchThemesCommon;

void ThemeEntry::DisplayInstallDialog(const std::string& path)
{
	DisplayLoading({ "Installing " + path + " ...", "CFW folder: " + fs::path::CfwFolder() });
}

class DummyEntry : public ThemeEntry 
{
public:
	bool Folder = false;

	DummyEntry(const string& fname, const string& _lblFname, const string& description, const string& rightlabel)
	{
		FileName = fname;
		lblFname = _lblFname;
		lblLine1 = description;
		lblLine2 = rightlabel;
	}

	bool IsFolder() override { return Folder; }
	bool CanInstall() override { return false; }
protected:
	bool DoInstall(bool ShowDialogs = true) override { return false; }
};

bool ThemeEntry::Install(bool ShowDialogs)
{
	if (!CanInstall())
	{
		// Sometimes ::Install is called automatically eg RemoteInstall/Detail.cpp, it would fail silently if CanInstall() == false
		DialogBlocking(CannotInstallReason == "" ? "This theme can't be installed." : CannotInstallReason);
		return false;
	}

	InstallLog.clear();

	try 
	{
		if (!DoInstall(ShowDialogs))
			return false;
	}
	catch (const exception & ex)
	{
		DialogBlocking("Error while installing this theme: " + string(ex.what()) + "\n\n" + InstallLog);
		return false;
	}

	if (ShowDialogs) 
	{
		if (InstallLog.empty())
			DialogBlocking("Done, restart the console to apply the changes");
		else
			DialogBlocking("Done, restart the console to apply the changes.\n\nThe following warnings were generated:\n" + InstallLog);

		// If !ShowDialogs keep the install log so it can be displayed by the caller
		InstallLog.clear();
	}

	return true;
}

ThemeEntry::~ThemeEntry()
{

}

unique_ptr<ThemeEntry> ThemeEntry::FromFile(const std::string& fileName)
{
	try {
		if (filesystem::is_directory(fileName))
		{
			// By convention all folder entries should end with a /
			auto fullPath = fileName;
			if (!StrEndsWith(fullPath, "/"))
				fullPath += "/";

			auto&& e = make_unique<DummyEntry>(fullPath, fs::GetFileName(fileName), fileName, "folder");
			e->Folder = true;
			return move(e);
		}

		auto data = fs::OpenFile(fileName);

		if (data.size() == 0)
			return make_unique<DummyEntry>(fileName, "Couldn't open this file", fileName, "ERROR");

		if (StrEndsWith(fileName, ".ttf"))
			return make_unique<FontEntry>(fileName, move(data));
		if (StrEndsWith(fileName, ".szs"))
			return make_unique<LegacyEntry>(fileName, move(data));
		if (StrEndsWith(fileName, ".nxtheme") || StrEndsWith(fileName, ".zip"))
			return make_unique<NxEntry>(fileName, move(data));
		if (StrEndsWith(fileName, ".jpg") || StrEndsWith(fileName, ".jpeg") || StrEndsWith(fileName, ".png"))
			return make_unique<ImageEntry>(fileName, move(data));
	}
	catch (std::exception &ex)
	{
		auto err = make_unique<DummyEntry>(fileName, "Error - Open for details", fileName, "ERROR");
		err->CannotInstallReason = ex.what();
		return err;
	}
	catch (...)
	{
		return make_unique<DummyEntry>(fileName, "Unknown exception while opening this file", fileName, "ERROR");
	}

	return make_unique<DummyEntry>(fileName, "Unknown file type", fileName, "ERROR");
}

unique_ptr<ThemeEntry> ThemeEntry::FromMemory(const std::vector<u8>& binary)
{
	std::string error = "Unknown file format";

	// new nxtheme format
	if (zip::IsZip(binary))
	{
		auto data = zip::Extract(binary);
		if (std::holds_alternative<std::string>(data))
		{
			error = std::get<std::string>(data);
			goto hande_error;
		}

		return make_unique<NxEntry>("", move(std::get<FileContainer>(data)));
	}

	if (Yaz0::IsYaz0(binary))
	{
		// Could also be a raw szs qlaunch file, try to load it and check if it's nxtheme.
		auto szs = szs::Extract(binary);
		if (std::holds_alternative<std::string>(szs))
		{
			error = std::get<std::string>(szs);
			goto hande_error;
		}

		auto& data = std::get<FileContainer>(szs);
		if (data.count("info.json"))
			return make_unique<NxEntry>("", move(data));
		else
		{
			std::vector<u8> copy = binary;
			return make_unique<LegacyEntry>("", move(copy));
		}
	}	

hande_error:
	return make_unique<DummyEntry>("Error", "Failed to load", error, "");
}

using namespace ImGui;
bool ThemeEntry::IsHighlighted() 
{
	return GImGui->NavId == GetCurrentWindow()->GetID(FileName.c_str());
}

ThemeEntry::UserAction ThemeEntry::Render(bool OverrideColor)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return UserAction::None;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(FileName.c_str());

	ImGui::PushFont(font30);
	const ImVec2 name_size = CalcTextSize(lblFname.c_str(), NULL, false);
	ImGui::PopFont();
	ImGui::PushFont(font25);
	const ImVec2 line1_size = CalcTextSize(lblLine1.c_str(), NULL, false, EntryW - 5);
	const ImVec2 line2_size = CalcTextSize(lblLine2.c_str(), NULL, false);
	ImGui::PopFont();

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 sz = { EntryW, 5 + name_size.y + line1_size.y };

	const ImRect bb(pos, pos + sz);
	ItemSize(sz, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return UserAction::None;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);
	if (pressed)
		MarkItemEdited(id);

	// Render
	const ImU32 col = GetColorU32((held && hovered && !OverrideColor) ? ImGuiCol_ButtonActive : hovered && !OverrideColor ? ImGuiCol_ButtonHovered : ImGuiCol_WindowBg);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	
	if (HasOptions() && (hovered || held) && KeyPressed(GLFW_GAMEPAD_BUTTON_X))
	{
		OpenOptions();
		return UserAction::Options;
	}

	ImGui::PushFont(font30);
	RenderText({ pos.x + 2, pos.y + 2 }, lblFname.c_str(), 0, false);
	ImGui::PopFont();
	ImGui::PushFont(font25);
	RenderText({ pos.x + EntryW - line2_size.x - 2, pos.y + 2 }, lblLine2.c_str(), 0, false);
	RenderTextWrapped({ pos.x + 2, pos.y + name_size.y + 2 }, lblLine1.c_str(), 0, EntryW - 5);
	ImGui::PopFont();
	
	window->DrawList->AddRectFilled({ bb.Min.x + 20, bb.Max.y }, { bb.Max.x - 20, bb.Max.y + 1}, 0xff4e4e4e);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed && Utils::ItemNotDragging() ? UserAction::Enter : UserAction::None;
}

const std::vector<u8> ThemeEntry::_emtptyVec = {};