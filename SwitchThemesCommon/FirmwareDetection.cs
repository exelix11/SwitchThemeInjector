using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SwitchThemes.Common
{
	public static class FirmwareDetection
	{
		public enum Firmware : int
		{
			Invariant = 0,
			Fw5_0 = 1,
			Fw6_0 = 2,
			Fw7_0 = 3,
			Fw8_0 = 4,
			Fw9_0 = 5,
			Fw10_0 = 6,
			Fw11_0 = 7
		}

		struct FirmInfo 
		{
			public Firmware Version;
			public string[] MustContain;
			public string[] MustNotContain;
		}

		readonly static IReadOnlyDictionary<string, FirmInfo[]> FirmwareInfo = new Dictionary<string, FirmInfo[]>
		{
			{ "home", new FirmInfo[] {
				new FirmInfo() {
					Version = Firmware.Fw11_0,
					MustContain = new string[] { @"blyt/RdtBtnLR.bflyt" },
				},
				new FirmInfo() {
					Version = Firmware.Fw8_0,
					MustContain = new string[] { @"blyt/IconError.bflyt", @"blyt/RdtIconPromotion.bflyt" },
					MustNotContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" }
				},
				new FirmInfo() {
					Version = Firmware.Fw6_0,
					MustContain = new string[] { @"blyt/IconError.bflyt" },
					MustNotContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" }
				},
				new FirmInfo() {
					Version = Firmware.Fw5_0,
					MustContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" ,@"blyt/IconError.bflyt"}
				} }
			},
			{ "lock", new FirmInfo[] {
				new FirmInfo() {
					Version = Firmware.Fw9_0,
					MustContain = new string[] { @"blyt/PageindicatorAlarm.bflyt", @"blyt/EntBtnResumeSystemApplet.bflyt" },
				} }
			},
		};

		public static Firmware Detect(string nxPartName, SARCExt.SarcData sarc)
		{
			if (FirmwareInfo.ContainsKey(nxPartName))
			{
				var t = FirmwareInfo[nxPartName].Where(x => 
					(x.MustContain?.All(y => sarc.Files.ContainsKey(y)) ?? true) &&
					(x.MustNotContain?.All(y => !sarc.Files.ContainsKey(y)) ?? true)
				);
				if (t.Any())
					return t.Max(x => x.Version);
			}

			return Firmware.Invariant;
		}
	}
}
