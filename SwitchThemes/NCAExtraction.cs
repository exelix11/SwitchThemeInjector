using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SwitchThemes
{
	class dummy { } //this is to prevent visual studio to show this file as a form

	public partial class Form1 : MaterialSkin.Controls.MaterialForm
	{
		private void linkLabel4_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e) =>
			System.Diagnostics.Process.Start(@"https://github.com/exelix11/SwitchThemeInjector/blob/master/DumpingFiles.md");

		private void MountBtn_Click(object sender, EventArgs e)
		{
			OpenFileDialog opn = new OpenFileDialog() { Filter= "prod.keys file|*.keys|all files|*.*" };
			if (opn.ShowDialog() != DialogResult.OK) return;
			keyFileTb.Text = opn.FileName;
		}

		private void OutputBtn_Click(object sender, EventArgs e)
		{
			FolderBrowserDialog fld = new FolderBrowserDialog();
			if (fld.ShowDialog() != DialogResult.OK) return;
			SdCardTb.Text = fld.SelectedPath;
		}

		string path(string p, string p1) => Path.Combine(p, p1);

		string KeyFile = "";
		string HactoolExe = "";

		bool HactoolExtract(string fname, string target)
		{
			string cmdline = $"-k \"{KeyFile}\" --romfsdir=\"{target}\" \"{fname}\"";
			Console.WriteLine(cmdline);
			var start = new ProcessStartInfo()
			{
				FileName = HactoolExe,
				Arguments = cmdline,
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
				RedirectStandardError = true
			};
			using (var p = Process.Start(start))
			{
				string output = "stdout:\r\n" + p.StandardOutput.ReadToEnd() + "\r\nstderr:\r\n" + p.StandardError.ReadToEnd();
				p.WaitForExit(10000);
				if (!p.HasExited)
				{
					p.Kill();
					System.IO.File.WriteAllText("hactool.log", output);
					MessageBox.Show("The hactool process timed out and has been killed, the log has been saved as hactool.log");
					Console.WriteLine(output);
					return false;
				}
				if (!Directory.Exists(path(target,"lyt")))
				{
					System.IO.File.WriteAllText("hactool.log", output);
					MessageBox.Show("Couldn't find lyt dir, the log has been saved as hactool.log");
					return false;
				}
				return true;
			}

		}

		long fileSize(string name)
		{
			var f = File.Open(name, FileMode.Open);
			long res = f.Length;
			f.Close();
			return res;
		}

		void UpdateTb()
		{
			this.Update();
		}

		private void NCARunBtn_Click(object sender, EventArgs e)
		{
			if (keyFileTb.Text.Trim() == "" || SdCardTb.Text.Trim() == "")
			{
				MessageBox.Show("Select the key and nca paths first");
				return;
			}
			string HomeNcaDir = path(SdCardTb.Text, "home.nca");
			string UserNcaDir = path(SdCardTb.Text, "user.nca");
			if (!File.Exists(HomeNcaDir) || !File.Exists(UserNcaDir))
			{
				MessageBox.Show("Couldn't find the target NCA files, you have to dump them first from the NXThemes Installer app, if you already did, make sure you selected the correct folder");
				return;
			}
			if (!File.Exists("hactool\\hactool.exe"))
			{
				MessageBox.Show("Couldn't find hactool.exe.\r\nYou must extract it in a folder called hactool in the same directory of this program");
				return;
			}

			KeyFile = keyFileTb.Text;
			HactoolExe = Path.GetFullPath("hactool\\hactool.exe");
			string OutDir = Path.GetFullPath(path("hactool\\", "Temp_ncaExtraction"));
			if (Directory.Exists(OutDir))
				Directory.Delete(OutDir, true);
			Directory.CreateDirectory(OutDir);

			if (!HactoolExtract(path(SdCardTb.Text, "home.nca"), OutDir))
				return;
			foreach (var f in Directory.GetFiles(path(OutDir, "lyt")))
			{
				string outFile = path(SdCardTb.Text, Path.GetFileName(f));
				if (File.Exists(outFile)) File.Delete(outFile);
				File.Move(f, outFile);
			}

			Directory.Delete(OutDir, true);
			Directory.CreateDirectory(OutDir);

			if (!HactoolExtract(path(SdCardTb.Text, "user.nca"), OutDir))
				return;
			{
				string outFile = path(SdCardTb.Text, "MyPage.szs");
				if (File.Exists(outFile)) File.Delete(outFile);
				File.Move(path(OutDir, "lyt/MyPage.szs"), outFile);
			}

			Directory.Delete(OutDir, true);

			if (MessageBox.Show("Done, delete the original NCA files ?", "", MessageBoxButtons.YesNo) == DialogResult.Yes)
			{
				File.Delete(HomeNcaDir);
				File.Delete(UserNcaDir);
			}
		}
	}
}
