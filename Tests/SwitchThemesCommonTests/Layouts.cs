using Microsoft.VisualStudio.TestTools.UnitTesting;
using SwitchThemes.Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace SwitchThemesCommonTests
{
	[TestClass]
	public class Layouts
	{
		[TestMethod]
		public void LoadAndOptimizeAll() 
		{
			// This generates layouts to load for SwitchThemesNXTests::Layouts::LayoutLoading
			foreach (var f in Directory.EnumerateFiles("../../../../../SwitchThemes/layouts").Where(x => x.EndsWith(".json")))
				File.WriteAllText(
					Path.Combine(Util.GetPath("ParsedLayouts"), Path.GetFileName(f)), 
					LayoutPatch.Load(File.ReadAllText(f)).AsJson()
				);
		}
		
	}
}
