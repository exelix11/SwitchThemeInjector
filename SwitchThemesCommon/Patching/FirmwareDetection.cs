using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SwitchThemes.Common
{
	// This enum defines the compatibility level of layouts, it is not meant to map exactly to HOS versions. New versions are only added when there are breaking changes to address via the NewFirmFixes feature
    public enum ConsoleFirmware : int
    {
		// Default value
        Invariant = 0,
        // Firmware versions in the format A.B.C => A_B_C
        // These should be set in a way that makes them chronologically comparable with < and > operators
        Fw5_0 = 5_0_0,
        Fw6_0 = 6_0_0,
        Fw8_0 = 8_0_0,
        Fw9_0 = 9_0_0,
        Fw11_0 = 11_0_0,
        Fw20_0 = 20_0_0,
    }

    public static class FirmwareDetection
	{
		struct FirmInfo 
		{
			public ConsoleFirmware Version;
			public string[] MustContain;
			public string[] MustNotContain;
		}

		readonly static IReadOnlyDictionary<string, FirmInfo[]> FirmwareInfo = new Dictionary<string, FirmInfo[]>
		{
			{ "home", new FirmInfo[] {
                new FirmInfo() {
                    Version = ConsoleFirmware.Fw20_0,
                    MustContain = new string[] { @"blyt/RdtBtnSplay.bflyt" },
					MustNotContain = new string[] { @"anim/RdtBase_SystemAppletPos.bflan" }
                },
                new FirmInfo() {
					Version = ConsoleFirmware.Fw11_0,
					MustContain = new string[] { @"blyt/RdtBtnLR.bflyt" },
				},
				new FirmInfo() {
					Version = ConsoleFirmware.Fw8_0,
					MustContain = new string[] { @"blyt/IconError.bflyt", @"blyt/RdtIconPromotion.bflyt" },
					MustNotContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" }
				},
				new FirmInfo() {
					Version = ConsoleFirmware.Fw6_0,
					MustContain = new string[] { @"blyt/IconError.bflyt" },
					MustNotContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" }
				},
				new FirmInfo() {
					Version = ConsoleFirmware.Fw5_0,
					MustContain = new string[] { @"anim/RdtBtnShop_LimitB.bflan" ,@"blyt/IconError.bflyt"}
				} }
			},
			{ "lock", new FirmInfo[] {
				new FirmInfo() {
					Version = ConsoleFirmware.Fw9_0,
					MustContain = new string[] { @"blyt/PageindicatorAlarm.bflyt", @"blyt/EntBtnResumeSystemApplet.bflyt" },
				} }
			},
		};

		public static ConsoleFirmware Detect(string nxPartName, SARCExt.SarcData sarc)
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

			return ConsoleFirmware.Invariant;
		}
	}
}
