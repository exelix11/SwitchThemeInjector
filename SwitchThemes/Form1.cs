using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
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

		bool Advanced = false;

		Dictionary<string,LayoutPatch> Layouts = new Dictionary<string, LayoutPatch>();

		public static Dictionary<string, string> HomeMenuParts = new Dictionary<string, string>()
		{
			{"Home menu", "home"},
			{"Lock screen", "lock"},
			{"User page", "user"},
			{"All apps menu (All applets on 5.X)", "apps"},
			{"Settings applet (All applets on 5.X)", "set"},
			{"News applet (All applets on 5.X)", "news"},
			//{"Options menu (Pressing + on a game)", "opt"},
			{"Player select", "psl"},
		};

		public string BgImage 
		{
			get => tbImageFile.Text;
			set => tbImageFile.Text = tbImageFile2.Text = value;
		}

		public Form1()
		{
			MaterialSkin.MaterialSkinManager.Instance.Theme = MaterialSkin.MaterialSkinManager.Themes.DARK;
			InitializeComponent();

			Advanced = Properties.Settings.Default.Adv;
			if (Advanced) EnableAdvanced();
			else DisableAdvanced();

			//LayoutPatch.CreateTestTemplates();
			//PatchTemplate.BuildTemplateFile();
			if (Directory.Exists("Layouts"))
			{
				foreach (var f in Directory.GetFiles("Layouts").Where(x => x.EndsWith(".json")))
					Layouts.Add(f,LayoutPatch.Load(File.ReadAllText(f)));
			}

			HomeMenuPartBox.Items.AddRange(HomeMenuParts.Keys.ToArray());
			HomeMenuPartBox.SelectedIndex = 0;
			HomeMenuPartBox_SelectedIndexChanged(null, null);

			HomeAppletIcoButtonsInit();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			Text += " Ver. " + Info.CoreVer;
			materialLabel10.Text = $"Switch Theme Injector Ver {Info.CoreVer} by exelix";
#if CIRelease
			materialLabel10.Text += $" (Github build)";
#endif
#if DEBUG
			lblDebug.Visible = true;
#endif
			var extra = PatchTemplate.LoadExtraTemplates();
			if (extra.Exception != null)
                MessageBox.Show("Loading extra templates failed, this is caused by the ExtraTemplates.json file. If you don't know what this means, delete the file and try again.\n\nFull error:" + extra.Exception.ToString());
            else if (extra.Result != null)
                DefaultTemplates.ExtraTemplates = extra.Result;
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
			if (!materialTabControl1.TabPages.Contains(InjectPage))
				materialTabControl1.TabPages.Add(InjectPage);
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
			materialTabControl1.TabPages.Remove(InjectPage);
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
				MessageBox.Show("This SZS doesn't contain the btnx file");
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
			var szs2 = SARCExt.SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
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
				MessageBox.Show("Open the modded file via SZS PATCHING>OPEN SZS first");
				return;
			}
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "Open the original SZS to diff",
				Filter = "SZS file|*.szs"
			};
			if (opn.ShowDialog() != DialogResult.OK) return;
			var originalSzs = SARCExt.SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			LayoutPatch res = null;
#if !DEBUG
			try
			{
#endif
				var diff = new LayoutDiff(originalSzs, CommonSzs, null);
				res = diff.ComputeDiff();
				if (!string.IsNullOrWhiteSpace(diff.OutputLog))
					MessageBox.Show(diff.OutputLog);
#if !DEBUG
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message);
				return;
			}
#endif

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
			var originalSzs = SARCExt.SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
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
				Filter = "SZS file|*.szs",
			};
			if (sav.ShowDialog() != DialogResult.OK) return;
			var sarc = SARC.Pack(CommonSzs);
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

		private void BgImageSelectBtn_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "open a picture",
				Filter = "Supported files (DDS,JPG)|*.dds;*.jpg;*.jpeg|all files|*.*",
			};

			BgImage = opn.ShowDialog() != DialogResult.OK ? "" : opn.FileName;
		}

		private void OpenSzsButton(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog()
			{
				Title = "Open SZS",
				Filter = "SZS file|*.szs|all files|*.*",
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
			
			CommonSzs = SARCExt.SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(opn.FileName)));
			targetPatch = DefaultTemplates.GetFor(CommonSzs);

			if (targetPatch == null)
			{
				if (Advanced)
				{
					AdvancedUpdate();
					lblDetected.Text = "Unknown SZS file";
					return;
				}

				MessageBox.Show("This is not a valid theme file, if it's from a newer firmware it's not compatible with this tool yet");
				CommonSzs = null;
				targetPatch = null;
				lblDetected.Text = "";
				return;
			}

			AdvancedUpdate();
			lblDetected.Text = "Detected " + targetPatch.TemplateName + " " + targetPatch.FirmName;

			foreach (var l in Layouts.Values)
				if (l.IsCompatible(CommonSzs))
					LayoutPatchList.Items.Add(l);
			LayoutPatchList.Items.Add("Open from file...");
			LayoutPatchList.SelectedIndex = 0;
		}

		private void HomeMenuPartBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			grpHomeExtra.Visible = HomeMenuParts[HomeMenuPartBox.Text] == "home";
			grpLockExtra.Visible = HomeMenuParts[HomeMenuPartBox.Text] == "lock";

			AllLayoutsBox.Items.Clear();
			AllLayoutsBox.Items.Add("Don't patch");
			foreach (var l in Layouts.Values)
			{
				if (l.TargetName == null || l.TargetName.Contains(Info.PartToFileName[HomeMenuParts[HomeMenuPartBox.Text]]))
					AllLayoutsBox.Items.Add(l);
			}
			AllLayoutsBox.Items.Add("Open from file...");
			AllLayoutsBox.SelectedIndex = 0;
		}

		private void PatchButtonClick(object sender, EventArgs e)
		{
			if (CommonSzs == null || targetPatch == null)
			{
				MessageBox.Show("Open a valid SZS first");
				return;
			}

			bool HasImage = BgImage.Trim() != "";

			if (!HasImage)
			{
				if (LayoutPatchList.SelectedIndex <= 0)
				{
					MessageBox.Show("There is nothing to patch");
					return;
				}

				if (MessageBox.Show("Are you sure you want to continue without selecting a background image ?", "", MessageBoxButtons.YesNo) == DialogResult.No)
					return;
			}
			else if (!File.Exists(BgImage))
			{
				MessageBox.Show($"{BgImage} not found!");
				return;
			}
			else if (!BgImage.ToLower().EndsWith("dds"))
			{
				MessageBox.Show($"For szs patching only dds images are supported");
				return;
			}
			
			if (HasImage)
			{
				var dds = Common.Images.Util.ParseDds(File.ReadAllBytes(BgImage));
				if (dds.Encoding != "DXT1") MessageBox.Show("WARNING: the encoding of the selected DDS is not DXT1, it may crash on the switch");
				if (dds.Size.Width != 1280 || dds.Size.Height != 720) MessageBox.Show("WARNING: the selected image is not 720p (1280x720), it may crash on the swtich");
			}

			SaveFileDialog sav = new SaveFileDialog()
			{
				Filter = "SZS file|*.szs",
				FileName = targetPatch.szsName
			};
			if (sav.ShowDialog() != DialogResult.OK) return;

			SzsPatcher Patcher = new SzsPatcher(CommonSzs);

			var res = true;
			if (HasImage)
			{
				res = Patcher.PatchMainBG(File.ReadAllBytes(BgImage));
				if (!res)
				{
					MessageBox.Show("Couldn't patch this file, it might have been already modified or it's from an unsupported system version.");
					return;
				}					
			}

			if (targetPatch.NXThemeName == "home")
			{
				foreach (var n in TextureReplacement.NxNameToList[targetPatch.NXThemeName])
				{
					if (HomeAppletIcons[n.NxThemeName] == null) continue;
					string path = HomeAppletIcons[n.NxThemeName].ToLower();
					if (!path.EndsWith(".dds"))
					{
						MessageBox.Show($"For szs patching only dds images are supported");
						return;
					}
					HomeAppletIcons[n.NxThemeName] = path;
					if (!Patcher.PatchAppletIcon(File.ReadAllBytes(path), n.NxThemeName))
					{
						MessageBox.Show($"Failed applet icon patch for {n.NxThemeName}");
						return;
					}
				}
			}
			else if (targetPatch.NXThemeName == "lock" && LockCustomIcon != null)
			{
				if (!LockCustomIcon.ToLower().EndsWith(".dds"))
				{
					MessageBox.Show($"For szs patching only dds images are supported");
					return;
				}
				Patcher.PatchAppletIcon(File.ReadAllBytes(LockCustomIcon), TextureReplacement.Entrance[0].NxThemeName);
			}

			if (LayoutPatchList.SelectedIndex != 0)
			{
				var layoutres = Patcher.PatchLayouts(LayoutPatchList.SelectedItem as LayoutPatch);
				if (!layoutres)
				{
					MessageBox.Show("One of the target files for the selected layout patch is missing in the SZS, you are probably using an already patched SZS");
					return;
				}
			}

			CommonSzs = Patcher.GetFinalSarc();
			var sarc = SARC.Pack(CommonSzs);
			
			File.WriteAllBytes(sav.FileName, ManagedYaz0.Compress(sarc.Item2, 3, (int)sarc.Item1));
			GC.Collect();

			if (Patcher.PatchTemplate.RequiresCodePatch)
				MessageBox.Show("The file has been patched successfully but due to memory limitations this szs requires an extra code patch to be applied to the home menu, if you use NXThemesInstaller to install this it will be done automatically, otherwise you need to manually copy the patches from https://github.com/exelix11/SwitchThemeInjector/tree/master/SwitchThemesNX/romfs to the exefs patches directory of your cfw");
			else
				MessageBox.Show("Done");
		}

		LayoutPatch ExtraCommonLyt = null;
		Dictionary<string, string> HomeAppletIcons = new Dictionary<string, string>();
		string LockCustomIcon = null;
		private void NnBuilderBuild_Click(object sender, EventArgs e)
		{
			if (BgImage.Trim() == "")
			{
				if (AllLayoutsBox.SelectedIndex == 0)
				{
					MessageBox.Show("You need at least a custom image or layout to make a theme.");
					return;
				}

				if (MessageBox.Show("This will create a theme without any background image, the console default one will be used. Do you want to continue?", "", MessageBoxButtons.YesNo) == DialogResult.No)
					return;
			}

			var (name, author) = ThemeInputInfo.Ask();
			if (name == null)
				return;

			string target = HomeMenuParts[HomeMenuPartBox.Text];

			LayoutPatch layout = null;
			if (AllLayoutsBox.SelectedIndex != 0)
				layout = AllLayoutsBox.SelectedItem as LayoutPatch;
			try
			{
				var builder = new NXThemeBuilder(target, name, author);

				if (layout != null)
					builder.AddMainLayout(layout);

				if (BgImage != "")
					builder.AddMainBg(File.ReadAllBytes(BgImage));

				if (ExtraCommonLyt != null)
					builder.AddCommonLayout(ExtraCommonLyt);

				if (target == "home")
				{
					foreach (var ico in HomeAppletIcons)
						if (ico.Value != null)
							builder.AddAppletIcon(ico.Key, File.ReadAllBytes(ico.Value));
				}
				else if (target == "lock" && LockCustomIcon != null)
					builder.AddAppletIcon("lock", File.ReadAllBytes(LockCustomIcon));

				SaveFileDialog sav = new SaveFileDialog() { Filter = "Theme pack (*.nxtheme)|*.nxtheme" };
				if (sav.ShowDialog() != DialogResult.OK)
					return;
				File.WriteAllBytes(sav.FileName, builder.GetNxtheme());
				MessageBox.Show("Done");
			}
			catch (Exception ex)
			{
				MessageBox.Show("ERROR: " + ex.Message);
			}
		}

		void LayoutPreview(LayoutPatch patch)
		{
			if (patch == null)
				MessageBox.Show("Select a layout first");
			else
			{
				string imagePath = Layouts.FirstOrDefault(x => x.Value == patch).Key;
				if (imagePath == null)
				{
					MessageBox.Show("This theme doesn't have a preview");
					return;
				}
				imagePath = imagePath.Substring(0, imagePath.Length - 5) + ".jpg";
				if (!File.Exists(imagePath))
					MessageBox.Show("This theme doesn't have a preview");
				else
					System.Diagnostics.Process.Start(imagePath);
			}
		}

		private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) => LayoutPreview(LayoutPatchList.SelectedItem as LayoutPatch);
		private void linkLabel2_LinkClicked_1(object sender, LinkLabelLinkClickedEventArgs e) => LayoutPreview(AllLayoutsBox.SelectedItem as LayoutPatch);

		private void linkLabel5_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			MessageBox.Show(".nxtheme files are a new file format for custom themes, they work pretty much like SZS files but they are legal to share and work on every firmware. To install .nxtheme files you need to download the NXThemes Installer on your console");
		}

		private void LayoutPatchList_OpenFile(string path, ComboBox comboBox)
		{
			comboBox.Items.Insert(1, LayoutPatch.Load(File.ReadAllText(path)));
			comboBox.SelectedIndex = 1;
		}

		private void LayoutPatchList_SelectedIndexChanged(object sender, EventArgs e)
		{
			ComboBox comboBox = (ComboBox)sender;
			if (comboBox.SelectedIndex == comboBox.Items.Count - 1)
			{
				OpenFileDialog opn = new OpenFileDialog() { Title = "Select a layout", Filter = "Json files|*.json" };
				if (opn.ShowDialog() != DialogResult.OK)
				{
					comboBox.SelectedIndex = 0;
					return;
				}
				LayoutPatchList_OpenFile(opn.FileName, comboBox);
			}
		}

		private void materialRaisedButton9_Click(object sender, EventArgs e)
		{
			new RemoteInstallForm().ShowDialog();
		}

		private void btnOpenCustomLayout_Click(object sender, EventArgs e)
		{
			if (ExtraCommonLyt != null)
			{
				btnOpenCustomLayout.Text = "...";
				lblCustomCommonLyt.Text = "Custom common layout: Not set";
				ExtraCommonLyt = null;
				return;
			}
			OpenFileDialog opn = new OpenFileDialog() { Filter = "json layout|*.json" };
			if (opn.ShowDialog() != DialogResult.OK) return;
			ExtraCommonLyt = LayoutPatch.Load(File.ReadAllText(opn.FileName));
			lblCustomCommonLyt.Text = $"Custom common layout: {ExtraCommonLyt.ToString()}";
			btnOpenCustomLayout.Text = "X";
		}

		private void HomeAppletIcoButtonsInit()
		{
			var btns = new List<Button> { btnApplet1, btnApplet2, btnApplet3, btnApplet4, btnApplet5, btnApplet6, btnApplet7, btnApplet8, btnApplet9  };
			int i = 0;
			foreach (var p in TextureReplacement.ResidentMenu)
			{
				HomeAppletIcons.Add(p.NxThemeName, null);
				btns[i].Tag = btns[i].Text = p.NxThemeName;
					btns[i].Click += delegate (object sender, EventArgs e) 
					{
						AppletIcoButtonBehavior((Button)sender, (string)((Button)sender).Tag);
					};
				i++;
			}
		}

		private void AppletIcoButtonBehavior(Button sender, string Name)
		{
			if (HomeAppletIcons[Name] != null)
			{
				sender.Text = Name;
				HomeAppletIcons[Name] = null;
				return;
			}
			OpenFileDialog opn = new OpenFileDialog() { Filter = "PNG or DDS|*.png;*.dds" };
			if (opn.ShowDialog() != DialogResult.OK) return;
			HomeAppletIcons[Name] = opn.FileName;
			sender.Text = "X";
		}

		private void BtnCustomLock_Click(object sender, EventArgs e)
		{
			if (LockCustomIcon != null)
			{
				btnCustomLock.Text = "...";
				lblCustomLock.Text = "Custom home icon: not set";
				LockCustomIcon = null;
				return;
			}
			OpenFileDialog opn = new OpenFileDialog() { Filter = "PNG or DDS|*.png;*.dds" };
			if (opn.ShowDialog() != DialogResult.OK) return;
			LockCustomIcon = opn.FileName;
			btnCustomLock.Text = "X";
			lblCustomLock.Text = "Custom home icon: " + LockCustomIcon;
		}

		private void btnAlbumIcoHelp_Click(object sender, EventArgs e) =>
			MessageBox.Show("These images will replace the applet icons in the home menu. Use only 64x56 PNG images, colors are not allowed: they should be white on a transparent background.\r\nIf you know what you're doing: DDS is supported as well.");

		private void button1_Click(object sender, EventArgs e) =>
			MessageBox.Show("This is a custom layout that is applied to the common.szs file, if unsure leave it empty. This is not the main layout");

		private void Button7_Click(object sender, EventArgs e) =>
			MessageBox.Show("This image will replace the home icon on the lock screen. Use only 184x168 PNG images, colors are supported.\r\nIf you know what you're doing: DDS is supported as well.");

		private void GithubLinkLbl_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start("https://github.com/exelix11/SwitchThemeInjector/releases");

		private void DiscordLinkLbl_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start("https://discord.gg/rqU5Tf8");

		private void SupportLinkLbl_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start("https://ko-fi.com/exelix11");

		private void SubredditLinkLbl_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start("https://www.reddit.com/r/NXThemes/");

		private void QceanLinkLbl_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start("https://discord.gg/CUnHJgb");

		int eggCounter = 0;
		private void materialLabel10_Click(object sender, EventArgs e)
		{
			if (eggCounter++ == 4)
				MessageBox.Show("---ALL YOUR THEMES ARE BELONG TO US---");
		}

		public static void DoExtractNxTheme(string FilePath, string OutDir)
		{
			var data = SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(FilePath)));
			foreach (var f in data.Files.Where(x => x.Key != "info.json"))
				File.WriteAllBytes(Path.Combine(OutDir, f.Key), f.Value);
		}

		private void ExtractNxTheme(string FilePath = null, string OutDir = null) 
		{
			try
			{
				if (FilePath == null)
				{
					OpenFileDialog opn = new OpenFileDialog() { Filter = "nxtheme files|*.nxtheme" };
					if (opn.ShowDialog() != DialogResult.OK) return;
					FilePath = opn.FileName;	
				}

				if (OutDir == null)
				{
					FolderBrowserDialog fld = new FolderBrowserDialog() { Description = "Extract the theme to.." };
					if (fld.ShowDialog() != DialogResult.OK) return;
					OutDir = fld.SelectedPath;
				}

				DoExtractNxTheme(FilePath, OutDir);

				MessageBox.Show("Done");
			}
			catch (Exception ex)
			{
				MessageBox.Show($"Error while extracting the file:\r\n{ex}");
			}
		}

		private void ExtractNxTheme_Click(object sender, EventArgs e)
		{
			ExtractNxTheme();
		}

		private void ExtractNxthemeBtn_DragDrop(object sender, DragEventArgs e)
		{
			ExtractNxTheme(e.GetFiles().FirstOrDefault());
		}

		private void Shared_FileDragEnter(object sender, DragEventArgs e)
		{
			if (e.GetFiles()?.Length == 1)
				e.Effect = DragDropEffects.Copy;
		}

		private void BgImage_DragDrop(object sender, DragEventArgs e) =>
			BgImage = e.GetFiles().FirstOrDefault();

		private void LayoutPatchList_DragDrop(object sender, DragEventArgs e)
		{
			string file = e.GetFiles().FirstOrDefault();
			if (string.IsNullOrWhiteSpace(file)) return;

			ComboBox comboBox = (ComboBox)sender;
			LayoutPatchList_OpenFile(file, comboBox);
		}

		private void RemoteInstal_DragDrop(object sender, DragEventArgs e) =>
			new RemoteInstallForm(e.GetFiles().FirstOrDefault()).ShowDialog();
	}

	internal static class Exten 
	{
		public static string[] GetFiles(this DragEventArgs e)
		{
			if (!e.Data.GetDataPresent(DataFormats.FileDrop))
				return null;

			return (string[])e.Data.GetData(DataFormats.FileDrop);
		}
	}
}