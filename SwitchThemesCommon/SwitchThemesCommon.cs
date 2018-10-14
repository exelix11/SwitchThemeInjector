using SwitchThemes.Common.Bntxx;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace SwitchThemes.Common
{
    static class SwitchThemesCommon
    {
		public const float CoreVer = 3f;
		const string LoadFileText =
			"To create a theme open an szs first, these are the patches available in this version:" +
			"{0} \r\n" +
			"Always read the instructions because they are slightly different for each version";

		public static string GeneratePatchListString(IEnumerable<PatchTemplate> Templates)
		{
			var sortedTemplates = Templates.OrderBy(x => x.FirmName).Reverse();

			string curSection = "";
			string FileList = "";
			foreach (var p in sortedTemplates)
			{
				if (curSection != p.FirmName)
				{
					curSection = p.FirmName;
					FileList += $"\r\nFor {curSection}: \r\n";
				}
				FileList += $"  - {p.TemplateName} : the file is called {p.szsName} from title {p.TitleId}\r\n";
			}
			return string.Format(LoadFileText, FileList);
		}

		public static BflytFile.PatchResult PatchLayouts(SARCExt.SarcData sarc, PatchTemplate template)
		{
			BflytFile BflytFromSzs(string name) => new BflytFile(new MemoryStream(sarc.Files[name]));
			BflytFile MainFile = BflytFromSzs(template.MainLayoutName);
			var res = MainFile.PatchMainLayout(template);
			if (res == BflytFile.PatchResult.OK)
			{
				sarc.Files[template.MainLayoutName] = MainFile.SaveFile();
				foreach (var f in template.SecondaryLayouts)
				{
					BflytFile curTarget = BflytFromSzs(f);
					curTarget.PatchTextureName(template.MaintextureName, template.SecondaryTexReplace);
					sarc.Files[f] = curTarget.SaveFile();
				}
			}
			return res;
		}

		public static BflytFile.PatchResult PatchBntx(SARCExt.SarcData sarc, byte[] DDS, PatchTemplate targetPatch)
		{
			QuickBntx q = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.Fail;
			}
			q.ReplaceTex(targetPatch.MaintextureName, DDS);
			DDS = null;
			sarc.Files[@"timg/__Combined.bntx"] = null;
			sarc.Files[@"timg/__Combined.bntx"] = q.Write();
			return BflytFile.PatchResult.OK;
		}

		public static BflytFile.PatchResult PatchBntx(SARCExt.SarcData sarc, DDSEncoder.DDSLoadResult DDS, PatchTemplate targetPatch)
		{
			QuickBntx q = new QuickBntx(new BinaryDataReader(new MemoryStream(sarc.Files[@"timg/__Combined.bntx"])));
			if (q.Rlt.Length != 0x80)
			{
				return BflytFile.PatchResult.Fail;
			}
			q.ReplaceTex(targetPatch.MaintextureName, DDS);
			DDS = null;
			sarc.Files[@"timg/__Combined.bntx"] = null;
			sarc.Files[@"timg/__Combined.bntx"] = q.Write();
			return BflytFile.PatchResult.OK;
		}

		public static PatchTemplate DetectSarc(SARCExt.SarcData sarc, IEnumerable<PatchTemplate> Templates)
		{
			bool SzsHasKey(string key) => sarc.Files.ContainsKey(key);

			if (!SzsHasKey(@"timg/__Combined.bntx"))
				return null;

			foreach (var p in Templates)
			{
				if (!SzsHasKey(p.MainLayoutName))
					continue;
				bool isTarget = true;
				foreach (string s in p.SecondaryLayouts)
				{
					if (!SzsHasKey(s))
					{
						isTarget = false;
						break;
					}
				}
				if (!isTarget) continue;
				foreach (string s in p.FnameIdentifier)
				{
					if (!SzsHasKey(s))
					{
						isTarget = false;
						break;
					}
				}
				if (!isTarget) continue;
				foreach (string s in p.FnameNotIdentifier)
				{
					if (SzsHasKey(s))
					{
						isTarget = false;
						break;
					}
				}
				if (!isTarget) continue;
				return p;
				break;
			}
			return null;
		}

	}
}
