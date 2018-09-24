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
		SarcData CommonSzs = null;
		int detectedVer = -1;

		public Form1()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			MaterialSkin.MaterialSkinManager.Instance.Theme = MaterialSkin.MaterialSkinManager.Themes.DARK;
			materialLabel1.ForeColor = Color.White;
		}

		private void materialRaisedButton1_Click(object sender, EventArgs e)
		{
			if (!materialTabSelector1.Enabled || CommonSzs == null)
			{
				MessageBox.Show("Open a common.szs first");
				return;
			}
			if (detectedVer == -1)
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
		
		private void materialDivider1_MouseDown(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				ReleaseCapture();
				SendMessage(Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
			}
		}

		private void materialRaisedButton3_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open common.szs",
				Filter = "common.szs|*.szs|all files|*.*",
			};
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			if (!File.Exists(opn.FileName))
			{
				MessageBox.Show("Could not open file");
				return;
			}
			CommonSzs = SARCExt.SARC.UnpackRamN(YAZ0.Decompress(File.ReadAllBytes(opn.FileName)));

			foreach (string k in CommonSzs.Files.Keys)
			{
				if (UTF8Encoding.Default.GetString(CommonSzs.Files[k]).Contains("White1x1A128^s"))
				{
					Console.WriteLine(k);
				}
			}


			if (CommonSzs.Files.ContainsKey(@"blyt/SystemAppletFader.bflyt")) detectedVer = 5;
			else if (CommonSzs.Files.ContainsKey(@"blyt/IconError.bflyt")) detectedVer = 6;

			if (detectedVer != 5 ||
				!CommonSzs.Files.ContainsKey(@"timg/__Combined.bntx") ||
				!CommonSzs.Files.ContainsKey(@"blyt/BgNml.bflyt"))
			{
				MessageBox.Show("This is not a valid common.szs file, if it's from a newer firmware it's not compatible with this tool yet");
				CommonSzs = null;
				return;
			}

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

		private void materialRaisedButton2_Click(object sender, EventArgs e)
		{
			if (tbBntxFile.Text.Trim() == "")
			{
				if (MessageBox.Show("Are you sure you want to continue without a bntx ? The theme will most likely crash on the console", "", MessageBoxButtons.YesNo) == DialogResult.No)
					return;
			}
			else if (!File.Exists(tbBntxFile.Text))
			{
				MessageBox.Show($"{tbBntxFile.Text} not found !");
				return;
			}


			SaveFileDialog sav = new SaveFileDialog()
			{
				FileName = "common.szs",
				Filter = "szs file|*.szs"
			};
			if (sav.ShowDialog() != DialogResult.OK) return;

			if (tbBntxFile.Text.Trim() != "")
				CommonSzs.Files[@"timg/__Combined.bntx"] = File.ReadAllBytes(tbBntxFile.Text);
			BflytFile f = new BflytFile(new MemoryStream(CommonSzs.Files[@"blyt/BgNml.bflyt"]));
			BflytFile.PatchResult res;
			if (detectedVer == 6)
				res = f.PatchMainLayout6x();
			else
				res = f.PatchMainLayout5x();

			if (res == BflytFile.PatchResult.AlreadyPatched)
				MessageBox.Show("This file has already been patched, only the bntx will be replaced.\r\nIf you have issues try with an unmodified file");
			else if (res == BflytFile.PatchResult.Fail)
			{
				MessageBox.Show("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
				return;
			}
			else if (res == BflytFile.PatchResult.CorruptedFile)
			{
				MessageBox.Show("This file has been already patched with another tool and is not compatible, you should get an unpatched layout (either one from your nand or one that has just color hacks).");
				return;
			}
			else
			{
				CommonSzs.Files[@"blyt/BgNml.bflyt"] = f.SaveFile();
				if (detectedVer == 6)
				{
					f = new BflytFile(new MemoryStream(CommonSzs.Files[@"blyt/IconError.bflyt"]));
					f.PatchIconError6x();
					CommonSzs.Files[@"blyt/IconError.bflyt"] = f.SaveFile();
				}
				else
				{
					f = new BflytFile(new MemoryStream(CommonSzs.Files[@"blyt/SystemAppletFader.bflyt"]));
					f.PatchFaderLayout5x();
					CommonSzs.Files[@"blyt/SystemAppletFader.bflyt"] = f.SaveFile();
				}
			}

			var sarc = SARC.PackN(CommonSzs);
			File.WriteAllBytes(sav.FileName, YAZ0.Compress(sarc.Item2, 3, (uint)sarc.Item1));

			MessageBox.Show("Done");
		}

		private void label1_Click(object sender, EventArgs e)
		{
			MessageBox.Show("Switch theme injector V 1.0\r\nby exelix\r\n\r\nTeam Qcean:\r\nCreatable, einso, GRAnimated, Traiver, Cellenseres, Vorphixx, SimonMKWii, Exelix\r\n\r\nDiscord invite code : GrKPJZt");
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
				BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
				dataReader.ByteOrder = bin.ByteOrder;
				dataReader.ReadInt32(); //Unknown
				PanelName = "";
				for (int i = 0; i < 0x18; i++)
				{
					var c = dataReader.ReadChar();
					if (c == 0) break;
					PanelName += c;
				}
			}

			public override string ToString()
			{
				return $"Picture {PanelName}";
			}
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

		void PatchTextureName(string original, string _new)
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

		public void PatchFaderLayout5x() =>
			PatchTextureName("White1x1_180^r", "White1x1^r");

		// blyt/IconError.bflyt
		public void PatchIconError6x() =>
			PatchTextureName("White1x1A128^s", "White1x1A64^t");


		public PatchResult PatchMainLayout6x()
			=> PatchMainLayout5x("White1x1A128^s");

		public PatchResult PatchMainLayout5x(string TexName = "White1x1_180^r")
		{
			#region add picture
			for (int i = 0; i < Panels.Count; i++)
			{
				if (!(Panels[i] is PicturePanel)) continue;
				var p = Panels[i] as PicturePanel;
				if (p.PanelName == "exelixBG") return PatchResult.AlreadyPatched;
				if (p.PanelName == "3x3lxBG") //Fix old layout
				{
					Panels.Remove(p);
					GetTex.Textures[0] = "White1x1^r";
					GetMat.Materials.RemoveAt(1);
				}
			}
			int target = -1;
			int targetSkip = 1;
			for (int i = 0; i < Panels.Count -1; i++)
			{
				if (Panels[i] is PicturePanel && ((PicturePanel)Panels[i]).PanelName == "P_Bg_00")
				{
					target = i;
					targetSkip = Panels[i + 1].name == "usd1" ? 2 : 1; //use latest image panel
				}
			}
			if (target == -1) return PatchResult.Fail;
			using (BinaryDataWriter bin = new BinaryDataWriter(new MemoryStream(Panels[target].data)))
			{
				bin.ByteOrder = ByteOrder.LittleEndian;
				bin.BaseStream.Position = 0x24;
				bin.Write(5000f);
				bin.Write(60000f);
				bin.BaseStream.Position = 0x3C;
				bin.Write(100f);
				bin.Write(100f);
				bin.Write(1200f);
				bin.Write(700f);
				Panels[target].data = ((MemoryStream)bin.BaseStream).ToArray();
			}
			Panels.Insert(target + targetSkip, new BasePanel("pic1", 0x8));
			var MatSect = GetMat;
			var strm = new MemoryStream();
			using (BinaryDataWriter bin = new BinaryDataWriter(strm))
			{
				bin.ByteOrder = ByteOrder.LittleEndian;
				bin.Write((byte)0x01);
				bin.Write((byte)0x00);
				bin.Write((byte)0xFF);
				bin.Write((byte)0x04);
				bin.Write("exelixBG", BinaryStringFormat.NoPrefixOrTermination);
				bin.Write(new byte[0x30]);
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
				Panels[target + 2].data = strm.ToArray();
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

		public TextureSection GetTex => (TextureSection)Panels.Find(x => x is TextureSection);
		public MaterialsSection GetMat => (MaterialsSection)Panels.Find(x => x is MaterialsSection);

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
				if (i == sectionCount - 1 && bin.BaseStream.Position != bin.BaseStream.Length) //load sections not counted in the section count (my old bflyt patch)
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
