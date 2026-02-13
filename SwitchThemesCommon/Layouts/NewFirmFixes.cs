using System.Collections.Generic;
using System.IO;
using System.Linq;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public static partial class NewFirmFixes
    {
		const string Fix_11_NoMoveApplets = "Fix_11_NoMoveApplets";
		const string Fix_11_NoOnlineButton = "Fix_11_NoOnlineButton";
		const string Fix_20_CarefulLayout = "Fix_20_CarefulLayout";
		const string Fix_20_DowngradeTo19 = "Fix_20_DowngradeTo19";
		const string Fix_20_FlowLayout = "Fix_20_FlowLayout";
		const string Fix_20_LegacyAppletButtons = "Fix_20_LegacyAppletButtons";
		const string Fix_Legacy_ClearLock = "Fix_Legacy_ClearLock";
		const string Fix_Legacy_Compact = "Fix_Legacy_Compact";
		const string Fix_Legacy_Diamond = "Fix_Legacy_Diamond";
		const string Fix_Legacy_DogeLayout = "Fix_Legacy_DogeLayout";

		public static readonly Dictionary<string, string> FixResources = new() {
			{ Fix_11_NoMoveApplets, "11_NoMoveApplets.json"},
            { Fix_11_NoOnlineButton, "11_NoOnlineButton.json"},
            { Fix_20_CarefulLayout, "20_CarefulLayout.json"},
            { Fix_20_DowngradeTo19, "20_DowngradeTo19.json"},
            { Fix_20_FlowLayout, "20_FlowLayout.json"},
            { Fix_20_LegacyAppletButtons, "20_LegacyAppletButtons.json"},
            { Fix_Legacy_ClearLock, "Legacy_ClearLock.json"},
            { Fix_Legacy_Compact, "Legacy_Compact.json"},
            { Fix_Legacy_Diamond, "Legacy_Diamond.json"},
            { Fix_Legacy_DogeLayout, "Legacy_DogeLayout.json" },
        };

		public static LayoutPatch LoadFromFixName(string name)
		{
			if (FixResources.TryGetValue(name, out var resourceName))
			{
				var path = Path.Combine("Compatibility", resourceName);
				var json = File.ReadAllText(path);
                return LayoutPatch.Load(json);
			}

			return null;
        }

        static bool ThemezerNameCheck(string layoutId, string themezerId)
		{
			return layoutId == themezerId ||
				layoutId.StartsWith(themezerId + "|");
        }

		// Fix for very old layouts. These are themes made before version 9.0. At that time we did not have the LayoutID property
		public static LayoutPatch GetFixLegacy(string LayoutName, ConsoleFirmware fw, string nxName)
		{
			// Check PatchRevision definitions in PatchTemplte.cs for firmware version
			if (fw >= ConsoleFirmware.Fw9_0 && nxName == "lock")
			{
				if (LayoutName.ToLower().Contains("clear lockscreen"))
					return LoadFromFixName(Fix_Legacy_ClearLock);
			}

			// These are have all been updated in the builtins as of 4.4
			if (fw >= ConsoleFirmware.Fw8_0 && nxName == "home") // >= 8.0 home menu
			{
				if (LayoutName.ToLower().Contains("dogelayout") || LayoutName.ToLower().Contains("clearlayout"))
					return LoadFromFixName(Fix_Legacy_DogeLayout);
				else if (LayoutName.ToLower().Contains("diamond layout"))
					return LoadFromFixName(Fix_Legacy_Diamond);
				else if (LayoutName.ToLower().Contains("small compact"))
					return LoadFromFixName(Fix_Legacy_Compact);
			}

			return null;
		}

		public static LayoutPatch GetFix(LayoutPatch layout, ConsoleFirmware fw)
		{
			// As of 4.5 this still hasn't been fixed in the builtin layouts but it has been given an ID
			if (fw >= ConsoleFirmware.Fw9_0 && layout.ID == "builtin_ClearLock")
                return LoadFromFixName(Fix_Legacy_ClearLock);

            var apply20Fix = fw >= ConsoleFirmware.Fw20_0 && layout.TargetFirmwareValue < ConsoleFirmware.Fw20_0;

			if (apply20Fix)
			{
				// Themezer allows genearting variants of layouts, they use a specific format and ID to identify them
				// these are managed by migush
				if (layout.ID == "builtin_FlowLayout" || ThemezerNameCheck(layout.ID, "Themezer:5"))
                    return LoadFromFixName(Fix_20_FlowLayout);

                if (layout.ID == "builtin_CarefulLayout" || ThemezerNameCheck(layout.ID, "Themezer:6"))
                    return LoadFromFixName(Fix_20_CarefulLayout);
            }

            return null;
		}

		public static LayoutPatch GetLegacyAppletButtonsFix(ConsoleFirmware fw)
		{
            if (fw >= ConsoleFirmware.Fw20_0)
                return LoadFromFixName(Fix_20_LegacyAppletButtons);
            if (fw >= ConsoleFirmware.Fw11_0)
                return LoadFromFixName(Fix_11_NoOnlineButton);

            return null;
		}
		
		public static bool ShouldApplyAppletPositionFix(LayoutPatch layout, ConsoleFirmware consoleFw)
		{
            // On firmware up to and including 11.0 we must fix the N_System pane position by removing RdtBase_SystemAppletPos
            // Except if the layout is already overriding it
            if (consoleFw <= ConsoleFirmware.Fw11_0)
                return !layout.Anims.Any(x => x.FileName == "anim/RdtBase_SystemAppletPos.bflan");

			if (consoleFw >= ConsoleFirmware.Fw20_0)
			{
				// On 20.0 and later we must always apply our compatibility fix if the layout targets an old firmware
				if (layout.TargetFirmwareValue < ConsoleFirmware.Fw20_0)
					return true;

                // Also if the layout requests the legacy applet buttons.
                // When the option is not specified, we assume it's an old layout and we should apply the fix.
                if (layout.HideOnlineBtn ?? true)
                    return true;
            }

            return false;
		}

		public static LayoutPatch GetAppletsPositionFix(ConsoleFirmware fw)
		{
			if (fw >= ConsoleFirmware.Fw20_0)
				return LoadFromFixName(Fix_20_DowngradeTo19);

			if (fw >= ConsoleFirmware.Fw11_0)
				return LoadFromFixName(Fix_11_NoMoveApplets);

			return null;
		}
	}
}
