using Microsoft.VisualStudio.TestTools.UnitTesting;
using SwitchThemes.Common;
using SwitchThemes.Common.Bflyt;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace SwitchThemesCommonTests
{
	// These tests use data manually generated to test patching

	[TestClass]
	public class SyntheticTests
	{
		HashUtil hash = new HashUtil();

		[TestMethod]
		public void BflanDeserialize() 
		{
			var bflan = SwitchThemes.Common.Serializers.BflanSerializer.FromJson(Util.ReadString("Synthetic/bflan.json"));
			Assert.AreEqual(hash.StringHash(bflan.WriteFile()), "43CE2CDE8B2638E36CA1723328CD571DB350D3BC011B6389944FAD69260BC748");
		}

		[TestMethod]
		public void BgPaneInjection() 
		{
			var bflyt = new BflytFile(Util.ReadData("Synthetic/bginjection.bflyt"));
			var t = DefaultTemplates.Templates.Where(x => x.szsName == "ResidentMenu.szs" && x.targetPanels.Contains("P_Bg_00")).First();
			
			Assert.IsTrue(bflyt.PatchBgLayout(t));
			
			Assert.AreEqual(hash.StringHash(bflyt.SaveFile()), "C4F98DF5F9227E122076DA31BEA351523E2780C2287EC7F604FBC86D59703C21");
		}
	}
}
