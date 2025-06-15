using System.Linq;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public static partial class NewFirmFixes
	{
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
					return JsonConvert.DeserializeObject<LayoutPatch>(ClearLock9Fix);
			}

			// These are have all been updated in the builtins as of 4.4
			if (fw >= ConsoleFirmware.Fw8_0 && nxName == "home") // >= 8.0 home menu
			{
				if (LayoutName.ToLower().Contains("dogelayout") || LayoutName.ToLower().Contains("clearlayout"))
					return JsonConvert.DeserializeObject<LayoutPatch>(DogeLayoutFix);
				else if (LayoutName.ToLower().Contains("diamond layout"))
					return JsonConvert.DeserializeObject<LayoutPatch>(DiamondFix);
				else if (LayoutName.ToLower().Contains("small compact"))
					return JsonConvert.DeserializeObject<LayoutPatch>(CompactFix);
			}

			return null;
		}

		public static LayoutPatch GetFix(LayoutPatch layout, ConsoleFirmware fw)
		{
			// As of 4.5 this still hasn't been fixed in the builtin layouts but it has been given an ID
			if (fw >= ConsoleFirmware.Fw9_0 && layout.ID == "builtin_ClearLock")
				return JsonConvert.DeserializeObject<LayoutPatch>(ClearLock9Fix);

			var apply20Fix = fw >= ConsoleFirmware.Fw20_0 && layout.TargetFirmwareValue < ConsoleFirmware.Fw20_0;

			if (apply20Fix)
			{
				// Themezer allows genearting variants of layouts, they use a specific format and ID to identify them
				// these are managed by migush
				if (layout.ID == "builtin_FlowLayout" || ThemezerNameCheck(layout.ID, "Themezer:5"))
					return JsonConvert.DeserializeObject<LayoutPatch>(FlowLayout20Fix);

				if (layout.ID == "builtin_CarefulLayout" || ThemezerNameCheck(layout.ID, "Themezer:6"))
					return JsonConvert.DeserializeObject<LayoutPatch>(CarefulLayout20Fix);
			}

            return null;
		}

		public static LayoutPatch GetLegacyAppletButtonsFix(ConsoleFirmware fw)
		{
            if (fw >= ConsoleFirmware.Fw20_0)
                return JsonConvert.DeserializeObject<LayoutPatch>(LegacyAppletButtons20);
            if (fw >= ConsoleFirmware.Fw11_0)
				return JsonConvert.DeserializeObject<LayoutPatch>(NoOnlineButton11);

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
				return JsonConvert.DeserializeObject<LayoutPatch>(Downgrade20To19);

			if (fw >= ConsoleFirmware.Fw11_0)
				return JsonConvert.DeserializeObject<LayoutPatch>(NoMoveApplets11);

			return null;
		}
	}
}
