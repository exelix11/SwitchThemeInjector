#include "../NXTheme.hpp"
#include "Patches.hpp"
#include <algorithm>

using namespace std;

// Import the json fixes from the C# codebase
namespace
{
#include "../../../../SwitchThemesCommon/NewFirmFixesJsons.cs"
}

namespace NewFirmFixes
{
	//https://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
	static bool contains_ignore_case(const std::string& strHaystack, const std::string& strNeedle)
	{
		auto it = std::search(
			strHaystack.begin(), strHaystack.end(),
			strNeedle.begin(), strNeedle.end(),
			[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
		);
		return (it != strHaystack.end());
	}

	static bool themezer_name_check(const std::string& layout_id, const std::string& themezer_name)
	{	
		return layout_id == themezer_name ||
			layout_id.starts_with(themezer_name + "|");
	}

	std::optional<LayoutPatch> GetFixLegacy(const std::string& LayoutName, ConsoleFirmware fw, const std::string& nxName)
	{
		// Check PatchRevision definitions in PatchTemplte.cs for firmware version
		if (fw >= ConsoleFirmware::Fw9_0 && nxName == "lock")
		{
			if (contains_ignore_case(LayoutName, "clear lockscreen"))
				return Patches::LoadLayout(ClearLock9Fix);
		}

		// These are have all been updated in the builtins as of 4.4
		if (fw >= ConsoleFirmware::Fw8_0 && nxName == "home") // >= 8.0 home menu
		{
			if (contains_ignore_case(LayoutName, "dogelayout") || contains_ignore_case(LayoutName, "clearlayout"))
				return Patches::LoadLayout(DogeLayoutFix);
			else if (contains_ignore_case(LayoutName, "diamond layout"))
				return Patches::LoadLayout(DiamondFix);
			else if (contains_ignore_case(LayoutName, "small compact"))
				return Patches::LoadLayout(CompactFix);
		}

		return std::nullopt;
	}

	std::optional<LayoutPatch> GetFix(LayoutPatch& layout, ConsoleFirmware fw)
	{
		// As of 4.5 this still hasn't been fixed in the builtin layouts but it has been given an ID
		if (fw >= ConsoleFirmware::Fw9_0 && layout.ID == "builtin_ClearLock")
			return Patches::LoadLayout(ClearLock9Fix);

		const auto apply20Fix = fw >= ConsoleFirmware::Fw20_0 && layout.TargetFirmware < (int)ConsoleFirmware::Fw20_0;

		if (apply20Fix)
		{
			// Themezer allows genearting variants of layouts, they use a specific format and ID to identify them
			// these are managed by migush
			if (layout.ID == "builtin_FlowLayout" || themezer_name_check(layout.ID, "Themezer:5"))
				return Patches::LoadLayout(FlowLayout20Fix);

			if (layout.ID == "builtin_CarefulLayout" || themezer_name_check(layout.ID, "Themezer:6"))
				return Patches::LoadLayout(CarefulLayout20Fix);

			// Hacky. see my comment on https://github.com/exelix11/SwitchThemeInjector/issues/156#issuecomment-2869845256
			// This layout is broken and will crash on 20.0+
			// As an extreme workaround we remove all the animations that cause the crash
			if (layout.ID == "builtin_JAGLayout" || themezer_name_check(layout.ID, "Themezer:2"))
			{
				std::vector<AnimFilePatch> filteredAnims;
				std::copy_if(layout.Anims.begin(), layout.Anims.end(), std::back_inserter(filteredAnims),
					[](const auto& x) {
						return
							x.FileName == "anim/RdtBtnIconGame_Inactive.bflan" ||
							x.FileName == "anim/RdtBtnIconGame_Active.bflan";
					});

				layout.Anims = filteredAnims;
			}
		}

		return std::nullopt;
	}

	std::optional<LayoutPatch> GetLegacyAppletButtonsFix(ConsoleFirmware fw)
	{
		if (fw >= ConsoleFirmware::Fw20_0)
			return Patches::LoadLayout(LegacyAppletButtons20);
		if (fw >= ConsoleFirmware::Fw11_0)
			return Patches::LoadLayout(NoOnlineButton11);

		return std::nullopt;
	}

	bool ShouldApplyAppletPositionFix(const LayoutPatch& layout, ConsoleFirmware consoleFw)
	{
		// On firmware up to and including 11.0 we must fix the N_System pane position by removing RdtBase_SystemAppletPos
		// Except if the layout is already overriding it
		if (consoleFw <= ConsoleFirmware::Fw11_0)
		{
			auto found = std::find_if(layout.Anims.begin(), layout.Anims.end(),
				[](const auto& x) {
					return x.FileName == "anim/RdtBase_SystemAppletPos.bflan";
				});

			return found == layout.Anims.end();
		}

		if (consoleFw >= ConsoleFirmware::Fw20_0)
		{
			// On 20.0 and later we must always apply our compatibility fix if the layout targets an old firmware
			if (layout.TargetFirmware < (int)ConsoleFirmware::Fw20_0)
				return true;

			// Also if the layout requests the legacy applet buttons.
			// When the option is not specified, we assume it's an old layout and we should apply the fix.
			if (layout.HideOnlineBtn)
				return true;
		}

		return false;
	}

	std::optional<LayoutPatch> GetAppletsPositionFix(ConsoleFirmware fw)
	{
		if (fw >= ConsoleFirmware::Fw20_0)
			return Patches::LoadLayout(Downgrade20To19);

		if (fw >= ConsoleFirmware::Fw11_0)
			return Patches::LoadLayout(NoMoveApplets11);

		return std::nullopt;
	}
}