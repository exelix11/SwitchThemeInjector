using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using EveryFileExplorer;
using SARCExt;
using Syroot.BinaryData;

namespace SwitchThemes
{
	public partial class Form1 : MaterialSkin.Controls.MaterialForm
	{
		PatchTemplate targetPatch;
		SarcData CommonSzs = null;
		
		readonly string ExtractLabelText = "";
		readonly string PatchLabelText = "";
		readonly string LoadFileText = 
			"To create a theme open an szs first, these are the patches available in this version:" +
			"{0} \r\n" +
			"Always read the instructions because they are slightly different for each version";

		List<PatchTemplate> Templates = new List<PatchTemplate>();

		public Form1()
		{
			InitializeComponent();
			ExtractLabelText = materialLabel1.Text;
			PatchLabelText = materialLabel3.Text;

			//PatchTemplate.BuildTemplateFile();
			Templates.AddRange(DefaultTemplates.templates);
			if (File.Exists("ExtraTemplates.json"))
				Templates.AddRange(PatchTemplate.LoadTemplates());

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
			LoadFileText = string.Format(LoadFileText, FileList);
			tbPatches.Text = LoadFileText;

			materialTabSelector1.Enabled = false;
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			MaterialSkin.MaterialSkinManager.Instance.Theme = MaterialSkin.MaterialSkinManager.Themes.DARK;
			materialLabel1.ForeColor = Color.White;
			lblDetected.ForeColor = Color.White;
		}

		private void ExtractBntxButton(object sender, EventArgs e)
		{
			if (!materialTabSelector1.Enabled || CommonSzs == null)
			{
				MessageBox.Show("Open a theme file first");
				return;
			}
			if (targetPatch == null)
			{
				MessageBox.Show("Version unsupported");
				return;
			}
			SaveFileDialog sav = new SaveFileDialog()
			{
				Filter = "bntx file|*.bntx",
				Title = "Save theme resources",
				FileName = "__Combined.bntx"
			};
			if (sav.ShowDialog() != DialogResult.OK)
				return;
			File.WriteAllBytes(sav.FileName, CommonSzs.Files[@"timg/__Combined.bntx"]);
			MessageBox.Show("Done");
		}
		
		private void DragForm_MouseDown(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				ReleaseCapture();
				SendMessage(Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
			}
		}

		bool SzsHasKey(string key) =>
			CommonSzs.Files.ContainsKey(key);

		private void OpenSzsButton(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open szs",
				Filter = "szs file|*.szs|all files|*.*",
			};
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			if (!File.Exists(opn.FileName))
			{
				MessageBox.Show("Could not open file");
				return;
			}

			targetPatch = null;
			CommonSzs = SARCExt.SARC.UnpackRamN(YAZ0.Decompress(File.ReadAllBytes(opn.FileName)));

#if DEBUG
			#region Check snippets
			//diff file lists :
			//if (opn.ShowDialog() != DialogResult.OK)
			//	return;
			//var szs2 = SARCExt.SARC.UnpackRamN(YAZ0.Decompress(File.ReadAllBytes(opn.FileName)));
			//foreach (var f in szs2.Files.Keys)
			//{
			//	if (CommonSzs.Files.ContainsKey(f))
			//		CommonSzs.Files.Remove(f);
			//	else
			//		Console.WriteLine($"{f} missing");
			//}
			//if (CommonSzs.Files.Count != 0)
			//{
			//	Console.WriteLine($"Files left : \r\n {string.Join("\r\n", CommonSzs.Files.Keys.ToArray())}");
			//}
			//find a string or a panel by name
			//foreach (string k in CommonSzs.Files.Keys)
			//{
			//	if (UTF8Encoding.Default.GetString(CommonSzs.Files[k]).Contains("NavBg_03^d"))
			//	{
			//		Console.WriteLine(k);
			//	}
			//if (k.EndsWith(".bflyt"))
			//{
			//	var l = BflytFromSzs(k);
			//	foreach (var p in l.Panels)
			//	{
			//		string pName = BflytFile.TryGetPanelName(p);
			//		if (pName == null) continue;
			//		if (pName.ToLower().Contains("bg")) Console.WriteLine($"{k} : {p.name} {pName}");
			//	}
			//}
			//}
			#endregion
#endif

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
				targetPatch = p;
				break;
			}			

			if (targetPatch == null || !SzsHasKey(@"timg/__Combined.bntx"))
			{
				MessageBox.Show("This is not a valid theme file, if it's from a newer firmware it's not compatible with this tool yet");
				CommonSzs = null;
				lblDetected.Text = "";
				btnExportBntx.Visible = false;
				materialLabel1.Text = LoadFileText;
				return;
			}

			materialLabel1.Text = string.Format(ExtractLabelText, targetPatch.MaintextureName);
			materialLabel3.Text = string.Format(PatchLabelText, targetPatch.szsName, targetPatch.TitleId);
			lblDetected.Text = "Detected " + targetPatch.TemplateName + " " + targetPatch.FirmName;

			btnExportBntx.Visible = true;
			materialTabSelector1.Enabled = true;
		}

		private void materialFlatButton1_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open resources file",
				Filter = "*.bntx|*.bntx|all files|*.*",
			};
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			if (opn.FileName != "")
				tbBntxFile.Text = opn.FileName;
		}
		
		BflytFile BflytFromSzs(string name) =>
			new BflytFile(new MemoryStream(CommonSzs.Files[name]));

		private void PatchButtonClick(object sender, EventArgs e)
		{
			if (tbBntxFile.Text.Trim() == "")
			{
				if (MessageBox.Show("Are you sure you want to continue without selecting a bntx ? Unless the szs already contains a custom one the theme will most likely crash", "", MessageBoxButtons.YesNo) == DialogResult.No)
					return;
			}
			else if (!File.Exists(tbBntxFile.Text))
			{
				MessageBox.Show($"{tbBntxFile.Text} not found !");
				return;
			}

			SaveFileDialog sav = new SaveFileDialog()
			{
				Filter = "szs file|*.szs",
				FileName = targetPatch.szsName
			};
			if (sav.ShowDialog() != DialogResult.OK) return;

			if (tbBntxFile.Text.Trim() != "")
			{
				if (PatchBntx(File.ReadAllBytes(tbBntxFile.Text)) == BflytFile.PatchResult.Fail)
					return;
			}
			
			BflytFile MainFile = BflytFromSzs(targetPatch.MainLayoutName);
			BflytFile.PatchResult res = BflytFile.PatchResult.Fail;

			//Main layout patch
			res = MainFile.PatchMainLayout(targetPatch);

			if (res == BflytFile.PatchResult.Fail)
			{
				MessageBox.Show("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
				return;
			}
			else if (res == BflytFile.PatchResult.CorruptedFile)
			{
				MessageBox.Show("This file has been already patched with another tool and is not compatible, you should get an unmodified layout.");
				return;
			}
			else //Additional patches (usually to free the texture we're replacing)
			{
				CommonSzs.Files[targetPatch.MainLayoutName] = MainFile.SaveFile();
				foreach (var f in targetPatch.SecondaryLayouts)
				{
					BflytFile curTarget = BflytFromSzs(f);
					curTarget.PatchTextureName(targetPatch.SecondaryTexReplace.Item1, targetPatch.SecondaryTexReplace.Item2);
					CommonSzs.Files[f] = curTarget.SaveFile();
				}
			}

			var sarc = SARC.PackN(CommonSzs);
			File.WriteAllBytes(sav.FileName, YAZ0.Compress(sarc.Item2, 3, (uint)sarc.Item1));

			if (res == BflytFile.PatchResult.AlreadyPatched)
				MessageBox.Show("Done, This file has already been patched, only the bntx was replaced.\r\nIf you have issues try with an unmodified file");
			else
				MessageBox.Show("Done");
		}

		int eggCounter = 0;
		private void label1_Click(object sender, EventArgs e)
		{
			if (eggCounter++ == 5)
				MessageBox.Show("---ALL YOUR THEMES ARE BELONG TO US---");
			else
				MessageBox.Show("Switch theme injector V 2.1\r\nby exelix\r\n\r\nTeam Qcean:\r\nCreatable, einso, GRAnimated, Traiver, Cellenseres, Vorphixx, SimonMKWii, Exelix\r\n\r\nDiscord invite code : GrKPJZt");
		}

		BflytFile.PatchResult PatchBntx(byte[] Bntx)
		{
			var ImportBntxReader = new BinaryDataReader(new MemoryStream(Bntx));
			ImportBntxReader.ByteOrder = ByteOrder.LittleEndian;
			ImportBntxReader.BaseStream.Position = 0x18;
			ImportBntxReader.BaseStream.Position = ImportBntxReader.ReadUInt32();
			int fileRltLen = (int)(ImportBntxReader.BaseStream.Length - ImportBntxReader.BaseStream.Position);
			if (fileRltLen == 0x80) //this file has an original rlt
			{
				ImportBntxReader.Dispose();
				CommonSzs.Files[@"timg/__Combined.bntx"] = Bntx;
				return BflytFile.PatchResult.OK;
			}
			else //the rlt has to be fixed
			{
				var reader = new BinaryDataReader(new MemoryStream(CommonSzs.Files[@"timg/__Combined.bntx"]));
				reader.ByteOrder = ByteOrder.LittleEndian;
				reader.BaseStream.Position = 0x18;
				reader.BaseStream.Position = reader.ReadUInt32();
				if (reader.BaseStream.Length - reader.BaseStream.Position > 0x80) //the rlt in the theme is corrupted
				{
					MessageBox.Show(
							"Can't build this theme: the szs you opened doesn't contain some information needed to patch the bntx," +
							"without this information it is not possible to rebuild the bntx." +
							"You should use an original or at least working szs", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return BflytFile.PatchResult.Fail;
				}
				reader.BaseStream.Position += 8;
				var OriginalRlt = reader.ReadBytes(0x80 - 8);
				reader.Dispose();
				MemoryStream mem = new MemoryStream();
				var writer = new BinaryDataWriter(mem);
				writer.ByteOrder = ByteOrder.LittleEndian;
				ImportBntxReader.BaseStream.Position = 0;
				writer.Write(ImportBntxReader.ReadBytes(0x18));
				int rltOffset = ImportBntxReader.ReadInt32();
				int fileLen = rltOffset + OriginalRlt.Length + 8;
				writer.Write(rltOffset);
				writer.Write(fileLen);
				ImportBntxReader.ReadInt32(); //skip file size
				writer.Write(ImportBntxReader.ReadBytes(rltOffset - 0x20));
				writer.Write("_RLT", BinaryStringFormat.NoPrefixOrTermination);
				writer.Write(rltOffset);
				writer.Write(OriginalRlt);
				CommonSzs.Files[@"timg/__Combined.bntx"] = mem.ToArray();
				writer.Dispose();
				mem.Dispose();
				return BflytFile.PatchResult.OK;
			}
		}
	}

	class BflytFile
	{
		public class BasePanel
		{
			public override string ToString()
			{
				return $"Panel {name} len: 0x{length.ToString("X")}";
			}

			public string name;
			public Int32 length;
			public byte[] data;

			public BasePanel(string _name, int len)
			{
				name = _name;
				length = len;
				data = new byte[length - 8];
			}

			public BasePanel(string _name, BinaryDataReader bin)
			{
				name = _name;
				length = bin.ReadInt32();
				data = bin.ReadBytes(length - 8);
			}

			public virtual void WritePanel(BinaryDataWriter bin)
			{
				bin.Write(name, BinaryStringFormat.NoPrefixOrTermination);
				length = data.Length + 8;
				bin.Write(length);
				bin.Write(data);
			}
		}

		public class TextureSection : BasePanel
		{
			public List<string> Textures = new List<string>();
			public TextureSection(BinaryDataReader bin) : base("txl1", bin)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int texCount = dataReader.ReadInt32();
				uint BaseOff = (uint)dataReader.Position;
				var Offsets = dataReader.ReadInt32s(texCount);
				foreach (var off in Offsets)
				{
					dataReader.Position = BaseOff + off;
					Textures.Add(dataReader.ReadString(BinaryStringFormat.ZeroTerminated));
				}
			}

			public TextureSection() : base("txl1", 8) { }

			public override void WritePanel(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write(Textures.Count);
				dataWriter.Write(new int[Textures.Count]);
				for (int i = 0; i < Textures.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Textures[i], BinaryStringFormat.ZeroTerminated);
					while (dataWriter.BaseStream.Position % 4 != 0)
						dataWriter.Write((byte)0);
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off - 4);
					dataWriter.Position = endPos;
				}
				data = newData.ToArray();
				base.WritePanel(bin);
			}
		}

		public class MaterialsSection : BasePanel
		{
			public List<byte[]> Materials = new List<byte[]>();
			public MaterialsSection(BinaryDataReader bin) : base("mat1", bin)
			{
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				int matCount = dataReader.ReadInt32();
				var Offsets = dataReader.ReadInt32s(matCount).Select(x => x - 8).ToArray(); // offsets relative to the stream
				for (int i = 0; i < matCount; i++)
				{
					int matLen = (i == matCount - 1 ? (int)dataReader.BaseStream.Length : Offsets[i + 1]) - (int)dataReader.Position;
					Materials.Add(dataReader.ReadBytes(matLen));
				}
			}

			public MaterialsSection() : base("mat1", 8) { }

			public override void WritePanel(BinaryDataWriter bin)
			{
				var newData = new MemoryStream();
				BinaryDataWriter dataWriter = new BinaryDataWriter(newData);
				dataWriter.ByteOrder = bin.ByteOrder;
				dataWriter.Write(Materials.Count);
				dataWriter.Write(new int[Materials.Count]);
				for (int i = 0; i < Materials.Count; i++)
				{
					uint off = (uint)dataWriter.Position;
					dataWriter.Write(Materials[i]);
					uint endPos = (uint)dataWriter.Position;
					dataWriter.Position = 4 + i * 4;
					dataWriter.Write(off + 8);
					dataWriter.Position = endPos;
				}
				data = newData.ToArray();
				base.WritePanel(bin);
			}
		}

		public class PicturePanel : BasePanel
		{
			public string PanelName;
			public PicturePanel(BinaryDataReader bin) : base("pic1", bin)
			{
				PanelName = TryGetPanelName(this);
			}

			public override string ToString()
			{
				return $"Picture {PanelName}";
			}
		}

		public static string TryGetPanelName(BasePanel p)
		{
			if (p.data.Length < 0x18 + 4) return null;
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(p.data), Encoding.ASCII, false);
			dataReader.ByteOrder = ByteOrder.LittleEndian;
			dataReader.ReadInt32(); //Unknown
			string PanelName = "";
			for (int i = 0; i < 0x18; i++)
			{
				var c = dataReader.ReadChar();
				if (c == 0) break;
				PanelName += c;
			}
			return PanelName;
		}

		public List<BasePanel> Panels = new List<BasePanel>();

		UInt32 version;

		public byte[] SaveFile()
		{
			var res = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(res);
			bin.ByteOrder = ByteOrder.LittleEndian;
			bin.Write("FLYT", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write((byte)0xFF);
			bin.Write((byte)0xFE); //Little endian
			bin.Write((UInt16)0x14); //Header size
			bin.Write(version);
			bin.Write((Int32)0);
			bin.Write((UInt16)Panels.Count);
			bin.Write((UInt16)0); //padding
			foreach (var p in Panels)
				p.WritePanel(bin);
			while (bin.BaseStream.Position % 4 != 0)
				bin.Write((byte)0);
			bin.BaseStream.Position = 0xC;
			bin.Write((uint)bin.BaseStream.Length);
			bin.BaseStream.Position = bin.BaseStream.Length;
			return res.ToArray();
		}

		public enum PatchResult
		{
			AlreadyPatched,
			Fail,
			CorruptedFile,
			OK
		}

		public void PatchTextureName(string original, string _new)
		{
			var texSection = GetTex;
			for (int i = 0; i < texSection.Textures.Count; i++)
			{
				if (texSection.Textures[i] == original)
				{
					texSection.Textures[i] = _new;
				}
			}
		}

		PatchResult AddBgPanel(int index,string TexName, string Pic1Name)
		{
			#region add picture
			if (Pic1Name.Length > 0x18)
				throw new Exception("Pic1Name should not be longer than 24 chars");
			var BgPanel = new BasePanel("pic1", 0x8);
			Panels.Insert(index, BgPanel);
			var MatSect = GetMat;
			var strm = new MemoryStream();
			using (BinaryDataWriter bin = new BinaryDataWriter(strm))
			{
				bin.ByteOrder = ByteOrder.LittleEndian;
				bin.Write((byte)0x01);
				bin.Write((byte)0x00);
				bin.Write((byte)0xFF);
				bin.Write((byte)0x04);
				bin.Write(Pic1Name, BinaryStringFormat.NoPrefixOrTermination);
				int zerCount = Pic1Name.Length;
				while (zerCount++ < 0x38)
					bin.Write((byte)0x00);
				bin.Write(1f);
				bin.Write(1f);
				bin.Write(1280f);
				bin.Write(720f);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt32)0xFFFFFFFF);
				bin.Write((UInt16)MatSect.Materials.Count);
				bin.Write((UInt16)1);
				bin.Write((UInt32)0);
				bin.Write((UInt32)0);
				bin.Write(1f);
				bin.Write((UInt32)0);
				bin.Write((UInt32)0);
				bin.Write(1f);
				bin.Write(1f);
				bin.Write(1f);
				BgPanel.data = strm.ToArray();
			}
			#endregion
			#region AddTextures
			var texSection = GetTex;
			if (!texSection.Textures.Contains(TexName))
				texSection.Textures.Add(TexName);
			int texIndex = texSection.Textures.IndexOf(TexName);
			#endregion
			#region Add material
			{
				MemoryStream mem = new MemoryStream();
				using (BinaryDataWriter bin = new BinaryDataWriter(mem))
				{
					bin.ByteOrder = ByteOrder.LittleEndian;
					bin.Write("P_Custm", BinaryStringFormat.ZeroTerminated);
					bin.Write(new byte[0x14]);
					bin.Write((Int32)0x15);
					bin.Write((Int32)0x8040200);
					bin.Write((Int32)0);
					bin.Write((UInt32)0xFFFFFFFF);
					bin.Write((UInt16)texIndex);
					bin.Write((UInt16)0x0);
					bin.Write(new byte[0xC]);
					bin.Write(1f);
					bin.Write(1f);
					bin.Write(new byte[0x10]);
					MatSect.Materials.Add(mem.ToArray());
				}
			}
			#endregion
			return PatchResult.OK;
		}

		public PatchResult PatchMainLayout(PatchTemplate patch)
		{
			#region DetectPatch
			for (int i = 0; i < Panels.Count; i++)
			{
				if (!(Panels[i] is PicturePanel)) continue;
				var p = Panels[i] as PicturePanel;
				if (p.PanelName == patch.PatchIdentifier) return PatchResult.AlreadyPatched;
				if (p.PanelName == "3x3lxBG") //Fix old layout
				{
					Panels.Remove(p);
					GetTex.Textures[0] = "White1x1^r";
					GetMat.Materials.RemoveAt(1);
				}
			}
			#endregion
			#region FindAndRemoveTargetBgPanels
			int target = int.MaxValue;
			for (int i = 0; i < Panels.Count - 1; i++)
			{
				string name = TryGetPanelName(Panels[i]);
				if (name != null && patch.targetPanels.Contains(name))
				{
					if (i < target) target = i;
					if (!patch.NoRemovePanel)
					{
						using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(Panels[i].data)))
						{
							bin.ByteOrder = ByteOrder.LittleEndian;
							bin.BaseStream.Position = 0x24;
							bin.Write(5000f);
							bin.Write(60000f);
							Panels[target].data = ((MemoryStream)bin.BaseStream).ToArray();
						}
					}
				}
			}
			if (target == int.MaxValue) return PatchResult.Fail;
			#endregion
			return AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
		}

		public TextureSection GetTex
		{
			get
			{
				var res = (TextureSection)Panels.Find(x => x is TextureSection); ;
				if (res == null)
				{
					res = new TextureSection();
					Panels.Insert(2, res);
				}
				return res;
			}
		}
		
		public MaterialsSection GetMat
		{
			get
			{
				var res = (MaterialsSection)Panels.Find(x => x is MaterialsSection);
				if (res == null)
				{
					res = new MaterialsSection();
					Panels.Insert(3,res);
				}
				return res;
			}
		}


		public BflytFile(Stream file)
		{
			BinaryDataReader bin = new BinaryDataReader(file);
			bin.ByteOrder = ByteOrder.LittleEndian;
			if (bin.ReadString(4) != "FLYT") throw new Exception("Wrong signature");
			bin.ReadUInt16(); //BOM
			bin.ReadUInt16(); //HeaderSize
			version = bin.ReadUInt32();
			bin.ReadUInt32(); //File size
			var sectionCount = bin.ReadUInt16();
			bin.ReadUInt16(); //padding
			for (int i = 0; i < sectionCount; i++)
			{
				string name = bin.ReadString(4);
				switch (name)
				{
					case "txl1":
						Panels.Add(new TextureSection(bin));
						break;
					case "mat1":
						Panels.Add(new MaterialsSection(bin));
						break;
					case "pic1":
						Panels.Add(new PicturePanel(bin));
						break;
					default:
						Panels.Add(new BasePanel(name, bin));
						break;
				}
				if (i == sectionCount - 1 && bin.BaseStream.Position != bin.BaseStream.Length) //load sections missing in the section count (my old bflyt patch)
				{
					while (bin.PeekChar() == 0 && bin.BaseStream.Position < bin.BaseStream.Length) bin.ReadChar();
					if (bin.BaseStream.Length - bin.BaseStream.Position >= 8) //min section size
					{
						sectionCount++;
					}
				}
			}
		}
	}
}