using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Bntxx;
using Syroot.BinaryData;

namespace SwitchThemes
{
	public partial class Form1 : MaterialSkin.Controls.MaterialForm
	{
		PatchTemplate targetPatch;
		SarcData CommonSzs = null;
		
		readonly string PatchLabelText = "";
		readonly string LoadFileText = "";

		bool Advanced = false;

		List<PatchTemplate> Templates = new List<PatchTemplate>();
		Dictionary<string,LayoutPatch> Layouts = new Dictionary<string, LayoutPatch>();

		public Form1()
		{
			InitializeComponent();
			PatchLabelText = materialLabel3.Text;

			//LayoutPatch.CreateTestTemplates();
			//PatchTemplate.BuildTemplateFile();
			Templates.AddRange(DefaultTemplates.templates);
			if (File.Exists("ExtraTemplates.json"))
				Templates.AddRange(PatchTemplate.LoadTemplates());
			if (Directory.Exists("Layouts"))
			{
				foreach (var f in Directory.GetFiles("Layouts").Where(x => x.EndsWith(".json")))
					Layouts.Add(f,LayoutPatch.LoadTemplate(File.ReadAllText(f)));
			}

			LoadFileText = SwitchThemesCommon.GeneratePatchListString(Templates);
			tbPatches.Text = LoadFileText;
			
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			MaterialSkin.MaterialSkinManager.Instance.Theme = MaterialSkin.MaterialSkinManager.Themes.DARK;
			materialLabel1.ForeColor = Color.White;
			lblDetected.ForeColor = Color.White;
			if (Properties.Settings.Default.Adv)
			{
				Advanced = true;
				EnableAdvanced();
			}
		}

		#region AdvancedTools
		void EnableAdvanced()
		{
			if (!Advanced)
			{
				Properties.Settings.Default.Adv = true;
				Properties.Settings.Default.Save();
			}
			AdvPanel.Enabled = true;
			AdvPanel.Visible = true;
			Advanced = true;
			checkBox1.Checked = true;
			AdvancedUpdate();
		}

		void DisableAdvanced()
		{
			if (Advanced)
			{
				Properties.Settings.Default.Adv = false;
				Properties.Settings.Default.Save();
			}
			AdvPanel.Enabled = false;
			AdvPanel.Visible = false;
			Advanced = false;
			checkBox1.Checked = false;
		}

		void AdvancedUpdate()
		{
			if (!Advanced)
				return;
			SzsFileList.Items.Clear();
			if (CommonSzs != null)
				SzsFileList.Items.AddRange(CommonSzs.Files.Keys.ToArray());
		}

		private void materialRaisedButton5_Click(object sender, EventArgs e)
		{
			if (CommonSzs == null) return;
			if (!CommonSzs.Files.ContainsKey(@"timg/__Combined.bntx"))
			{
				MessageBox.Show("This szs doesn't contain the btnx file");
				return;
			}
			QuickBntx b = new QuickBntx(new BinaryDataReader(new MemoryStream(CommonSzs.Files["timg/__Combined.bntx"])));
			string[] TexList = b.Textures.Select(x => x.Name).ToArray();
			Dictionary<string, List<string>> UsageCount = new Dictionary<string, List<string>>();
			foreach (string s in TexList)
				UsageCount.Add(s, new List<string>());

			foreach (string k in CommonSzs.Files.Keys)
			{
				string fileText = UTF8Encoding.Default.GetString(CommonSzs.Files[k]);
				foreach (string s in TexList)
				{
					if (fileText.Contains(s))
					{
						UsageCount[s].Add(k);
					}
				}
			}

			string res = "Result:\r\n";
			foreach (string s in TexList)
			{
				res += s + " has " + UsageCount[s].Count + " references :" + string.Join(",", UsageCount[s]) + "\r\n";
			}

			MessageBox.Show(res);
		}
		
		private void materialRaisedButton4_Click(object sender, EventArgs e)
		{
			if (CommonSzs == null)
				return;
			var opn = new OpenFileDialog();
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			var szs2 = SARCExt.SARC.UnpackRamN(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			var filesIn1 = CommonSzs.Files.Keys.ToList();
			var filesIn2 = szs2.Files.Keys.ToList();
			var common = filesIn1.Intersect(filesIn2);
			var filesOnlyIn1 = filesIn1.Except(common);
			var filesOnlyIn2 = filesIn2.Except(common);
			filesIn1 = null;
			filesIn2 = null;
			common = null;
			if (CommonSzs.Files.Count != 0)
			{
				MessageBox.Show($"Files only in 1 : \r\n {string.Join("\r\n", filesOnlyIn1)}\r\n\r\nFiles only in 2: \r\n {string.Join("\r\n", filesOnlyIn2)}");
			}
		}

		private void materialRaisedButton6_Click(object sender, EventArgs e)
		{
			if (CommonSzs == null)
			{
				MessageBox.Show("Open the modded file from Open szs first");
				return;
			}
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open the original szs to diff",
				Filter = "szs file|*.szs"
			};
			if (opn.ShowDialog() != DialogResult.OK) return;
			var originalSzs = SARCExt.SARC.UnpackRamN(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			var res = LayoutDiff.Diff(originalSzs, CommonSzs);
			if (res == null) return;
			SaveFileDialog sav = new SaveFileDialog()
			{
				Title = "save the patch file",
				Filter = "json patch file|*.json"
			};
			if (sav.ShowDialog() != DialogResult.OK) return;
			File.WriteAllText(sav.FileName, res.AsJson());
		}

		private void extractToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (SzsFileList.SelectedItem == null || CommonSzs == null) return;
			SaveFileDialog sav = new SaveFileDialog();
			sav.FileName = SzsFileList.SelectedItem as string;
			if (sav.ShowDialog() == DialogResult.OK)
				File.WriteAllBytes(sav.FileName, CommonSzs.Files[SzsFileList.SelectedItem as string]);
		}

		private void replaceToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (SzsFileList.SelectedItem == null || CommonSzs == null) return;
			OpenFileDialog opn = new OpenFileDialog();
			opn.FileName = SzsFileList.SelectedItem as string;
			if (opn.ShowDialog() == DialogResult.OK)
				CommonSzs.Files[SzsFileList.SelectedItem as string] = File.ReadAllBytes(opn.FileName);
		}

		private void materialRaisedButton7_Click(object sender, EventArgs e)
		{
			if (CommonSzs == null) return;
			OpenFileDialog opn = new OpenFileDialog();
			if (opn.ShowDialog() != DialogResult.OK) return;
			var originalSzs = SARCExt.SARC.UnpackRamN(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			List<string> diffFiles = new List<string>();
			foreach (string f in originalSzs.Files.Keys)
				if (!originalSzs.Files[f].SequenceEqual(CommonSzs.Files[f]))
					diffFiles.Add(f);
			MessageBox.Show(string.Join("\r\n", diffFiles.ToArray()));
		}
		#endregion

		private void materialRaisedButton1_Click(object sender, EventArgs e)
		{
			if (CommonSzs == null) return;
			SaveFileDialog sav = new SaveFileDialog()
			{
				Filter = "szs file|*.szs",
			};
			if (sav.ShowDialog() != DialogResult.OK) return;
			var sarc = SARC.PackN(CommonSzs);
			File.WriteAllBytes(sav.FileName, ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1));
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e)
		{
			if (Advanced == checkBox1.Checked)
				return;
			if (Advanced)
				DisableAdvanced();
			else
				EnableAdvanced();
		}

		private void DragForm_MouseDown(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				ReleaseCapture();
				SendMessage(Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
			}
		}

		private void materialFlatButton1_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open a picture",
				Filter = "Supported files (dds,jpg,png)|*.dds;*.jpg;*.jpeg;*.png|all files|*.*",
			};
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			if (opn.FileName != "")
				tbBntxFile.Text = opn.FileName;
		}

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
			LayoutPatchList.Items.Clear();
			LayoutPatchList.Items.Add("Don't patch");
			
			CommonSzs = SARCExt.SARC.UnpackRamN(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			targetPatch = SwitchThemesCommon.DetectSarc(CommonSzs, Templates);

			if (targetPatch == null)
			{
				if (Advanced)
				{
					AdvancedUpdate();
					lblDetected.Text = "Unknown szs file";
					return;
				}

				MessageBox.Show("This is not a valid theme file, if it's from a newer firmware it's not compatible with this tool yet");
				CommonSzs = null;
				targetPatch = null;
				lblDetected.Text = "";
				return;
			}

			AdvancedUpdate();
			materialLabel3.Text = string.Format(PatchLabelText, targetPatch.szsName, targetPatch.TitleId);
			lblDetected.Text = "Detected " + targetPatch.TemplateName + " " + targetPatch.FirmName;

			foreach (var l in Layouts.Values)
				if (l.IsCompatible(CommonSzs))
					LayoutPatchList.Items.Add(l);
			LayoutPatchList.SelectedIndex = 0;
		}

		bool ImageToDDS(string fileName, string outPath)
		{
			if (!File.Exists("texconv.exe"))
			{
				MessageBox.Show("texconv.exe is missing, this program is needed to convert images to dds.\r\nYou can download it from https://github.com/Microsoft/DirectXTex/releases");
				return false;
			}
			Process p = new Process();
			string pathName = outPath;
			if (pathName.EndsWith("\\") || pathName.EndsWith("/"))
				pathName = pathName.Substring(0, pathName.Length - 1); //fix wierd bug with quotes of texconv
			p.StartInfo = new ProcessStartInfo()
			{
				FileName = "texconv",
				Arguments = $"-y -f DXT1 -ft dds -o \"{pathName}\" \"{fileName}\"",
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
			};
			p.Start();
			p.WaitForExit(8000);
			if (!p.HasExited)
			{
				p.Kill();
				MessageBox.Show("The texture converter has timed out and the process was killed, it may have generated a corrupted image");
			}
			string target = Path.Combine(pathName, Path.GetFileNameWithoutExtension(fileName) + ".dds");
			if (!File.Exists(target))
			{
				string pOut = p.StandardOutput.ReadToEnd();
				MessageBox.Show("Couldn't convert the image to DDS, output of the converter : \r\n\r\n" + pOut);
				return false;
			}
			return true;
		}

		bool ImageToDDS()
		{
			var res = ImageToDDS(tbBntxFile.Text, Path.GetTempPath());
			if (res)
				tbBntxFile.Text = Path.Combine(Path.GetTempPath(), Path.GetFileNameWithoutExtension(tbBntxFile.Text) + ".dds");
			return res;
		}

		private void PatchButtonClick(object sender, EventArgs e)
		{
			if (CommonSzs == null || targetPatch == null)
			{
				MessageBox.Show("Open a valid theme first !");
				return;
			}
			if (tbBntxFile.Text.Trim() == "")
			{
				if (LayoutPatchList.SelectedIndex <= 0)
				{
					MessageBox.Show("There is nothing to patch");
					return;
				}
				if (MessageBox.Show("Are you sure you want to continue without selecting a bntx ?", "", MessageBoxButtons.YesNo) == DialogResult.No)
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

			if (LayoutPatchList.SelectedIndex != 0)
			{
				var layoutres = SwitchThemesCommon.PatchLayouts(CommonSzs, (LayoutPatchList.SelectedItem as LayoutPatch).Files);
				if (layoutres == BflytFile.PatchResult.Fail)
				{
					MessageBox.Show("One of the target files for the selected layout patch is missing in the SZS, you are probably using an already patched SZS");
					return;
				}
				else if (layoutres == BflytFile.PatchResult.CorruptedFile)
				{
					MessageBox.Show("A layout in this SZS is missing a pane required for the selected layout patch, you are probably using an already patched SZS");
					return;
				}
			}

			var res = BflytFile.PatchResult.OK;
			if (tbBntxFile.Text.Trim() != "")
			{
				if (!tbBntxFile.Text.EndsWith(".dds") && !ImageToDDS())
					return;
				if (SwitchThemesCommon.PatchBntx(CommonSzs, File.ReadAllBytes(tbBntxFile.Text), targetPatch) == BflytFile.PatchResult.Fail)
				{
					MessageBox.Show(
							"Can't build this theme: the szs you opened doesn't contain some information needed to patch the bntx," +
							"without this information it is not possible to rebuild the bntx." +
							"You should use an original or at least working szs", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return;
				}

				res = SwitchThemesCommon.PatchBgLayouts(CommonSzs, targetPatch);

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
			}						

			var sarc = SARC.PackN(CommonSzs);
			
			File.WriteAllBytes(sav.FileName, ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1));
			GC.Collect();

			if (res == BflytFile.PatchResult.AlreadyPatched)
				MessageBox.Show("Done, This file has already been patched before.\r\nIf you have issues try with an unmodified file");
			else
				MessageBox.Show("Done");
		}

		int eggCounter = 0;
		private void label1_Click(object sender, EventArgs e)
		{
			if (eggCounter++ == 5)
				MessageBox.Show("---ALL YOUR THEMES ARE BELONG TO US---");
			else
				MessageBox.Show(
					"Switch theme injector V "+ SwitchThemesCommon.CoreVer + "\r\n" +
					"by exelix\r\n\r\n" +
					"Team Qcean:\r\n" +
					"Creatable, einso, GRAnimated, Traiver, Cellenseres, Vorphixx, SimonMKWii, Exelix\r\n\r\n" +
					"Discord invite code : p27kEST\r\n\r\n" +
					"Thanks to:\r\nSyroot for BinaryData lib\r\nAboodXD for Bntx editor");
		}

		private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			var patch = LayoutPatchList.SelectedItem as LayoutPatch;
			if (patch == null)
				MessageBox.Show("Select a layout first");
			else
			{
				string imagePath = Layouts.FirstOrDefault(x => x.Value == patch).Key;
				imagePath = imagePath.Substring(0, imagePath.Length - 5) + ".jpg";
				if (!File.Exists(imagePath))
					MessageBox.Show("This theme doesn't have a preview");
				else
					System.Diagnostics.Process.Start(imagePath);
			}
		}

		private void ImageToDDSBtn_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Multiselect = true,
				Title = "Select one or more images",
				Filter = "Common image files|*.png;*.jpg;*.jpeg;*.bmp|All files|*.*"
			};
			if (opn.ShowDialog() != DialogResult.OK)
				return;
			foreach (var f in opn.FileNames)
			{
				if (!ImageToDDS(f, Path.GetDirectoryName(f)))
					return;
			}
			MessageBox.Show("Done !");
		}
	}
}