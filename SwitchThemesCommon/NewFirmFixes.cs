using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public static class NewFirmFixes
	{
		const string DogeLayoutFix = "{\"PatchName\":\"Dogelayout 8 fix\",\"AuthorName\":\"Migush\",\"Files\":[{\"FileName\":\"blyt/RdtBtnFullLauncher.bflyt\",\"Patches\":[{\"PaneName\":\"N_Tip\",\"Scale\":{\"X\":1.1,\"Y\":1.1}},{\"PaneName\":\"B_Hit\",\"Size\":{\"X\":80,\"Y\":80}}]},{\"FileName\":\"blyt/RdtBtnMyPage.bflyt\",\"Patches\":[{\"PaneName\":\"N_Tip\",\"Position\":{\"X\":125,\"Y\":0,\"Z\":0}},{\"PaneName\":\"B_Hit\",\"Scale\":{\"X\":1.428571,\"Y\":1.428571},\"Size\":{\"X\":40,\"Y\":40}}]},{\"FileName\":\"blyt/Hud.bflyt\",\"Patches\":[{\"PaneName\":\"N_Time\",\"Position\":{\"X\":-190,\"Y\":640,\"Z\":0},\"Size\":{\"X\":12,\"Y\":30}},{\"PaneName\":\"L_Time\",\"Position\":{\"X\":-18,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":1}}]},{\"FileName\":\"blyt/HudTime.bflyt\",\"Patches\":[{\"PaneName\":\"N_AMPM\",\"Position\":{\"X\":30,\"Y\":-1,\"Z\":0},\"Scale\":{\"X\":0.9,\"Y\":0.9}}]},{\"FileName\":\"blyt/RdtBtnIconGame.bflyt\",\"Patches\":[{\"PaneName\":\"P_InnerCursor\",\"Visible\":false},{\"PaneName\":\"N_BtnFocusKey\",\"Size\":{\"X\":259,\"Y\":259}},{\"PaneName\":\"N_Tip\",\"Scale\":{\"X\":1.1,\"Y\":1.1}}]},{\"FileName\":\"blyt/RdtBase.bflyt\",\"Patches\":[{\"PaneName\":\"T_Blank\",\"Position\":{\"X\":0,\"Y\":197,\"Z\":0}},{\"PaneName\":\"N_ScrollArea\",\"Position\":{\"X\":0,\"Y\":-218,\"Z\":0},\"Size\":{\"X\":1300,\"Y\":322},\"Scale\":{\"X\":1,\"Y\":0.5}},{\"PaneName\":\"N_ScrollWindow\",\"Position\":{\"X\":0,\"Y\":-218,\"Z\":0},\"Size\":{\"X\":100000,\"Y\":322}},{\"PaneName\":\"N_GameRoot\",\"Position\":{\"X\":-530,\"Y\":-218,\"Z\":0},\"Scale\":{\"X\":0.00001,\"Y\":1}},{\"PaneName\":\"N_Game\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":100000,\"Y\":1}},{\"PaneName\":\"N_Icon_00\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_01\",\"Position\":{\"X\":135,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_02\",\"Position\":{\"X\":270,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_03\",\"Position\":{\"X\":405,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_04\",\"Position\":{\"X\":540,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_05\",\"Position\":{\"X\":675,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_06\",\"Position\":{\"X\":810,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_07\",\"Position\":{\"X\":945,\"Y\":0,\"Z\":0}},{\"PaneName\":\"N_Icon_08\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0},\"Visible\":false},{\"PaneName\":\"N_Icon_09\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0},\"Visible\":false},{\"PaneName\":\"N_Icon_10\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0},\"Visible\":false},{\"PaneName\":\"N_Icon_11\",\"Position\":{\"X\":1,\"Y\":99999,\"Z\":0},\"Visible\":false},{\"PaneName\":\"N_Icon_12\",\"Position\":{\"X\":1080,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":1,\"Y\":1}},{\"PaneName\":\"L_BtnFlc\",\"Position\":{\"X\":0,\"Y\":0,\"Z\":0},\"Scale\":{\"X\":0.5,\"Y\":0.5}}]}]}";
		const string DiamondFix = "";

		public static LayoutFilePatch[] GetFix(string LayoutName)
		{
			if (LayoutName.ToLower().Contains("dogelayout"))
				return JsonConvert.DeserializeObject<LayoutPatch>(DogeLayoutFix).Files;
			return null;
		}
	}
}
