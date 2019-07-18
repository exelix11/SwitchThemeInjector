using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using SwitchThemes.Common;

namespace SwitchThemesOnline.Pages
{
	public class StateHolder
	{
		private string _Target = "home";
		public string Target { get => _Target; set { _Target = value; TargetHasChanged?.Invoke(); } } 
		public byte[] MainBG;
		public LayoutPatch MainLayout;
		public string Name;
		public string Author;

		public delegate void SimpleHandler();
		public event SimpleHandler TargetHasChanged;
	}

	public static class Constants
	{
		public static Dictionary<string, string> NXPartToName = new Dictionary<string, string>() {
			{"home","Home menu"},
			{"lock","Lockscreen"},
			{"user","User page"},
			{"apps","All apps page"},
			{"set","Settings applet"},
			{"news","News applet"},
			{ "psl","Player select applet" },
		};
	}
}
