using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using static SwitchThemes.Common.FirmwareDetection;

namespace SwitchThemes.Common
{
	public static class NewFirmFixes
	{
		const string NoOnlineButton = "{\"PatchName\":\"NoOnline11\",\"Anims\":[{\"FileName\":\"anim/RdtBase_SystemAppletPos.bflan\",\"AnimJson\":\"{\\\"LittleEndian\\\":true,\\\"Version\\\":150994944,\\\"pat1\\\":{\\\"AnimationOrder\\\":6,\\\"Name\\\":\\\"SystemAppletPos\\\",\\\"ChildBinding\\\":190,\\\"Groups\\\":[\\\"G_System\\\"],\\\"Unk_StartOfFile\\\":0,\\\"Unk_EndOfFile\\\":0,\\\"Unk_EndOfHeader\\\":\\\"AL8AAAAAAA==\\\"},\\\"pai1\\\":{\\\"FrameSize\\\":1,\\\"Flags\\\":0,\\\"Textures\\\":[],\\\"Entries\\\":[]}}\"}]}";

		/*
			The RdtBase_SystemAppletPos animation on 11.0 will change the position and size of the applet buttons root element, to make layout developers' lives easier
			this is automatically patched away. It's possible to override this by including that file in the layout json
		 */
		const string NoMoveApplets11 = "{\"PatchName\":\"G_SystemNoMove11.0\",\"Anims\":[{\"FileName\":\"anim/RdtBase_SystemAppletPos.bflan\",\"AnimJson\":\"{\\\"LittleEndian\\\":true,\\\"Version\\\":150994944,\\\"pat1\\\":{\\\"AnimationOrder\\\":6,\\\"Name\\\":\\\"SystemAppletPos\\\",\\\"ChildBinding\\\":190,\\\"Groups\\\":[\\\"G_System\\\"],\\\"Unk_StartOfFile\\\":0,\\\"Unk_EndOfFile\\\":0,\\\"Unk_EndOfHeader\\\":\\\"AL8AAAAAAA==\\\"},\\\"pai1\\\":{\\\"FrameSize\\\":1,\\\"Flags\\\":0,\\\"Textures\\\":[],\\\"Entries\\\":[{\\\"Name\\\":\\\"L_BtnLR\\\",\\\"Target\\\":0,\\\"Tags\\\":[{\\\"Unknown\\\":0,\\\"TagType\\\":\\\"FLVI\\\",\\\"Entries\\\":[{\\\"Index\\\":0,\\\"AnimationTarget\\\":0,\\\"DataType\\\":1,\\\"KeyFrames\\\":[{\\\"Frame\\\":0.0,\\\"Value\\\":0.0,\\\"Blend\\\":0.0},{\\\"Frame\\\":1.0,\\\"Value\\\":1.0,\\\"Blend\\\":0.0}],\\\"FLEUUnknownInt\\\":0,\\\"FLEUEntryName\\\":\\\"\\\"}]}],\\\"UnkwnownData\\\":\\\"\\\"}]}}\"}]}";

		const string DogeLayoutFix = "{\"PatchName\":\"DogeLayout 8.x fix\",\"AuthorName\":\"autoDiff\",\"Files\":[{\"FileName\":\"blyt/HudTime.bflyt\",\"Patches\":[{\"PaneName\":\"N_AMPM\",\"Position\":{\"X\":30,\"Y\":-1,\"Z\":0},\"Scale\":{\"X\":0.9,\"Y\":0.9}}]},{\"FileName\":\"blyt/RdtBtnFullLauncher.bflyt\",\"Patches\":[{\"PaneName\":\"N_Tip\",\"Scale\":{\"X\":1.1,\"Y\":1.1}},{\"PaneName\":\"B_Hit\",\"Size\":{\"X\":80,\"Y\":80}}]},{\"FileName\":\"blyt/Cursor3.bflyt\",\"Patches\":[{\"PaneName\":\"P_Main\",\"UsdPatches\":[{\"PropName\":\"S_BorderSize\",\"PropValues\":[\"7\"],\"type\":2}]},{\"PaneName\":\"P_Grow\",\"UsdPatches\":[{\"PropName\":\"S_BorderSize\",\"PropValues\":[\"7\"],\"type\":2}]}]},{\"FileName\":\"blyt/RdtBtnMyPage.bflyt\",\"Patches\":[{\"PaneName\":\"N_Tip\",\"Position\":{\"X\":125,\"Y\":0,\"Z\":0}},{\"PaneName\":\"B_Hit\",\"Scale\":{\"X\":1.428571,\"Y\":1.428571},\"Size\":{\"X\":40,\"Y\":40}}]},{\"FileName\":\"blyt/RdtBtnIconGame.bflyt\",\"Patches\":[{\"PaneName\":\"RootPane\",\"Scale\":{\"X\":0.5,\"Y\":0.5}},{\"PaneName\":\"P_InnerCursor\",\"Visible\":false},{\"PaneName\":\"N_BtnFocusKey\",\"Size\":{\"X\":259,\"Y\":259}},{\"PaneName\":\"N_Tip\",\"Scale\":{\"X\":1.1,\"Y\":1.1}},{\"PaneName\":\"B_Hit\",\"Scale\":{\"X\":2,\"Y\":2},\"Size\":{\"X\":132,\"Y\":132}}]},{\"FileName\":\"blyt/RdtBase.bflyt\",\"Patches\":[{\"PaneName\":\"N_ScrollArea\",\"Position\":{\"X\":0,\"Y\":-218,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":0.5},\"Size\":{\"X\":1300,\"Y\":322}},{\"PaneName\":\"N_ScrollWindow\",\"Position\":{\"X\":0,\"Y\":-218,\"Z\":0},\"Size\":{\"X\":100000,\"Y\":322}},{\"PaneName\":\"T_Blank\",\"Position\":{\"X\":0,\"Y\":197,\"Z\":0}},{\"PaneName\":\"N_GameRoot\",\"Position\":{\"X\":-530,\"Y\":-218,\"Z\":0},\"Scale\":{\"X\":0.00001,\"Y\":1}},{\"PaneName\":\"N_Game\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":100000,\"Y\":1}},{\"PaneName\":\"N_Icon_01\",\"Position\":{\"X\":135,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_02\",\"Position\":{\"X\":270,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_03\",\"Position\":{\"X\":405,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_04\",\"Position\":{\"X\":540,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_05\",\"Position\":{\"X\":675,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_06\",\"Position\":{\"X\":810,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_07\",\"Position\":{\"X\":945,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_08\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_09\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_10\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_11\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_12\",\"Position\":{\"X\":1080,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":1}},{\"PaneName\":\"L_BtnFlc\",\"Scale\":{\"X\":0.5,\"Y\":0.5}}]},{\"FileName\":\"blyt/Hud.bflyt\",\"Patches\":[{\"PaneName\":\"N_Time\",\"Size\":{\"X\":12,\"Y\":30}},{\"PaneName\":\"L_Time\",\"Position\":{\"X\":-18,\"Y\":0,\"Z\":0}}]}]}";
		const string DiamondFix = "{\"PatchName\":\"Diamond 8 fix\",\"AuthorName\":\"akai\",\"Files\":[{\"FileName\":\"blyt/RdtBtnFullLauncher.bflyt\",\"Patches\":[{\"PaneName\":\"N_Root\",\"Rotation\":{\"X\":0,\"Y\":0,\"Z\":45}}]},{\"FileName\":\"blyt/RdtBtnIconGame.bflyt\",\"Patches\":[{\"PaneName\":\"RootPane\",\"Scale\":{\"X\":0.37,\"Y\":0.37}},{\"PaneName\":\"B_Hit\",\"Scale\":{\"X\":3,\"Y\":3},\"Size\":{\"X\":97.68,\"Y\":97.68}}]},{\"FileName\":\"blyt/RdtBase.bflyt\",\"Patches\":[{\"PaneName\":\"N_ScrollArea\",\"Position\":{\"X\":-30,\"Y\":-200,\"Z\":0},\"Size\":{\"X\":1300,\"Y\":322}},{\"PaneName\":\"N_ScrollWindow\",\"Position\":{\"X\":-30,\"Y\":-200,\"Z\":0},\"Size\":{\"X\":1080,\"Y\":322}},{\"PaneName\":\"N_GameRoot\",\"Position\":{\"X\":200,\"Y\":-195,\"Z\":0},\"Scale\":{\"X\":0.00001,\"Y\":1}},{\"PaneName\":\"N_Game\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":100000,\"Y\":1}},{\"PaneName\":\"N_Icon_01\",\"Position\":{\"X\":140,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_02\",\"Position\":{\"X\":280,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_03\",\"Position\":{\"X\":68,\"Y\":73,\"Z\":0}},{\"PaneName\":\"N_Icon_04\",\"Position\":{\"X\":208,\"Y\":73,\"Z\":0}},{\"PaneName\":\"N_Icon_05\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_06\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_07\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_08\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_09\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_10\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_11\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0}},{\"PaneName\":\"N_Icon_12\",\"Position\":{\"X\":348,\"Y\":73,\"Z\":0}},{\"PaneName\":\"L_BtnFlc\",\"Scale\":{\"X\":0.37,\"Y\":0.37}},{\"PaneName\":\"N_System\",\"Position\":{\"X\":-600,\"Y\":-250,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_00\",\"Position\":{\"X\":-247,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_01\",\"Position\":{\"X\":-175,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_02\",\"Position\":{\"X\":-103,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_03\",\"Position\":{\"X\":-31,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_04\",\"Position\":{\"X\":41,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_05\",\"Position\":{\"X\":113,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_06\",\"Position\":{\"X\":185,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_Hud\",\"Position\":{\"X\":-300,\"Y\":-325,\"Z\":0}}]},{\"FileName\":\"blyt/Hud.bflyt\",\"Patches\":[{\"PaneName\":\"N_Time\",\"Position\":{\"X\":10,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_Time\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0}}]}]}";
		const string CompactFix = "{\"PatchName\":\"Compact 8 fix\",\"AuthorName\":\"akai\",\"Files\":[{\"FileName\":\"blyt/RdtBtnFullLauncher.bflyt\",\"Patches\":[{\"PaneName\":\"N_Root\",\"Rotation\":{\"X\":0,\"Y\":0,\"Z\":45}}]},{\"FileName\":\"blyt/RdtBtnIconGame.bflyt\",\"Patches\":[{\"PaneName\":\"RootPane\",\"Scale\":{\"X\":0.37,\"Y\":0.37}},{\"PaneName\":\"B_Hit\",\"Scale\":{\"X\":3,\"Y\":3},\"Size\":{\"X\":97.68,\"Y\":97.68}}]},{\"FileName\":\"blyt/RdtBase.bflyt\",\"Patches\":[{\"PaneName\":\"N_ScrollArea\",\"Position\":{\"X\":0,\"Y\":-280,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":0.5},\"Size\":{\"X\":1300,\"Y\":322}},{\"PaneName\":\"N_ScrollWindow\",\"Position\":{\"X\":0,\"Y\":-280,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":0.5},\"Size\":{\"X\":1080,\"Y\":322}},{\"PaneName\":\"N_GameRoot\",\"Position\":{\"X\":-50,\"Y\":-230,\"Z\":0},\"Scale\":{\"X\":0.000001,\"Y\":1}},{\"PaneName\":\"N_Game\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":1000000,\"Y\":1}},{\"PaneName\":\"N_Icon_01\",\"Position\":{\"X\":100,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_02\",\"Position\":{\"X\":200,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_03\",\"Position\":{\"X\":300,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_04\",\"Position\":{\"X\":400,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_05\",\"Position\":{\"X\":500,\"Y\":0,\"Z\":0},\"Visible\":true},{\"PaneName\":\"N_Icon_06\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_07\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_08\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_09\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_10\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_11\",\"Position\":{\"X\":1,\"Y\":-9999999,\"Z\":0}},{\"PaneName\":\"N_Icon_12\",\"Position\":{\"X\":600,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnFlc\",\"Scale\":{\"X\":0.37,\"Y\":0.37}},{\"PaneName\":\"L_BtnAccount_00\",\"Position\":{\"X\":-247,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_01\",\"Position\":{\"X\":-175,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_02\",\"Position\":{\"X\":-103,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_03\",\"Position\":{\"X\":-31,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_04\",\"Position\":{\"X\":41,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_05\",\"Position\":{\"X\":113,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_BtnAccount_06\",\"Position\":{\"X\":185,\"Y\":0,\"Z\":0}},{\"PaneName\":\"L_Hud\",\"Position\":{\"X\":-290,\"Y\":-325,\"Z\":0}}]},{\"FileName\":\"blyt/Hud.bflyt\",\"Patches\":[{\"PaneName\":\"N_Time\",\"Position\":{\"X\":10,\"Y\":0,\"Z\":0}}]}]}";

		const string ClearLock9Fix = "{\"PatchName\":\"clearlayout 9.x fix\",\"AuthorName\":\"exelix\",\"Ready8X\":true,\"Files\":[{\"FileName\":\"blyt/EntMain.bflyt\",\"Patches\":[{\"PaneName\":\"L_BtnResume\",\"Position\":{\"X\":-180,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_CntHud\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0}}]}]}";

		public static LayoutFilePatch[] GetFixLegacy(string LayoutName, Firmware fw, string nxName)
		{
			// Check PatchRevision definitions in PatchTemplte.cs for firmware version
			if (fw >= Firmware.Fw9_0 && nxName == "lock")
			{
				if (LayoutName.ToLower().Contains("clear lockscreen"))
					return JsonConvert.DeserializeObject<LayoutPatch>(ClearLock9Fix).Files;
			}

			// These are have all been updated in the builtins as of 4.4
			if (fw >= Firmware.Fw8_0 && nxName == "home") // >= 8.0 home menu
			{
				if (LayoutName.ToLower().Contains("dogelayout") || LayoutName.ToLower().Contains("clearlayout"))
					return JsonConvert.DeserializeObject<LayoutPatch>(DogeLayoutFix).Files;
				else if (LayoutName.ToLower().Contains("diamond layout"))
					return JsonConvert.DeserializeObject<LayoutPatch>(DiamondFix).Files;
				else if (LayoutName.ToLower().Contains("small compact"))
					return JsonConvert.DeserializeObject<LayoutPatch>(CompactFix).Files;
			}
			return null;
		}

		public static LayoutFilePatch[] GetFix(string NxPart, string LayoutID, Firmware fw)
		{
			// As of 4.5 this still hasn't been fixed in the builtin layouts but it has been given an ID
			if (fw >= Firmware.Fw9_0 && LayoutID == "builtin_ClearLock" && NxPart == "lock") /* Do we need NxPart ? */
				return JsonConvert.DeserializeObject<LayoutPatch>(ClearLock9Fix).Files;

			return null;
		}

		public static AnimFilePatch[] GetNoOnlineButtonFix(Firmware fw)
		{
			if (fw >= Firmware.Fw11_0)
				return JsonConvert.DeserializeObject<LayoutPatch>(NoOnlineButton).Anims;

			return null;
		}

		// Apply only if the layout is not overriding the target file
		public static bool ShouldApplyAppletPositionFix(IEnumerable<AnimFilePatch> anims) =>
			!anims.Any(x => x.FileName == "anim/RdtBase_SystemAppletPos.bflan");

		public static AnimFilePatch[] GetAppletsPositionFix(Firmware fw)
		{
			if (fw >= Firmware.Fw11_0)
				return JsonConvert.DeserializeObject<LayoutPatch>(NoMoveApplets11).Anims;

			return null;
		}
	}
}
