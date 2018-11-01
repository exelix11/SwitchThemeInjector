using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
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
			FolderBrowserDialog fld = new FolderBrowserDialog();
			if (fld.ShowDialog() != DialogResult.OK) return;
			mountPathTb.Text = fld.SelectedPath;
		}

		private void OutputBtn_Click(object sender, EventArgs e)
		{
			FolderBrowserDialog fld = new FolderBrowserDialog();
			if (fld.ShowDialog() != DialogResult.OK) return;
			OutputPathTb.Text = fld.SelectedPath;
		}

		string path(string p, string p1) => Path.Combine(p, p1);

		string KeyFile = "";
		string HactoolExe = "";

		bool HactoolExtract(string fname, string target)
		{
			NCALogTb.AppendText( "Extracting file " + fname + " target " + target + "\r\n");
			string cmdline = $"/C \"\"{HactoolExe}\" -k \"{KeyFile}\" --romfsdir=\"{OutputPathTb.Text}\\{target}RomFS\" \"{fname}\"\"";
			Console.WriteLine(cmdline);
			var start = new ProcessStartInfo()
			{
				FileName = "cmd.exe",
				Arguments = cmdline,
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
			};
			using (var p = Process.Start(start))
			{
				string output = p.StandardOutput.ReadToEnd();
				p.WaitForExit(10000);
				if (!p.HasExited)
				{
					p.Kill();
					MessageBox.Show("The hactool process timed out and has been killed");
					Console.WriteLine(output);
					NCALogTb.AppendText( "PROCESS TIMED OUT.\r\n" + output);
					return false;
				}
				NCALogTb.AppendText( output + "\r\n\r\n");
				if (!Directory.Exists(path(OutputPathTb.Text, target + "RomFS\\lyt")))
				{
					MessageBox.Show("Couldn't find lyt dir, check the log");
					return false;
				}
				return true;
			}

		}

		string HactoolInfo(string fname)
		{
			NCALogTb.AppendText( "Checking file " + fname + "\r\n");
			string cmdline = $"/C \"\"{HactoolExe}\" -k \"{KeyFile}\" \"{fname}\"\"";
			Console.WriteLine(cmdline);
			var start = new ProcessStartInfo()
			{
				FileName = "cmd.exe",
				Arguments = cmdline,
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
				
			};
			using (var p = Process.Start(start))
			{
				string output = p.StandardOutput.ReadToEnd();
				p.WaitForExit(5000);		
				if (!p.HasExited)
				{
					p.Kill();
					MessageBox.Show("The hactool process timed out and has been killed");
					Console.WriteLine(output);
					NCALogTb.AppendText("PROCESS TIMED OUT.\r\n" + output);
					return null;
				}
				NCALogTb.AppendText( output + "\r\n\r\n");
				return output;
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
			if (mountPathTb.Text.Trim() == "" || OutputPathTb.Text.Trim() == "")
			{
				MessageBox.Show("Select the mount and output paths first");
				return;
			}
			string NcasDir = path(mountPathTb.Text, "Contents\\registered");
			if (!Directory.Exists(NcasDir))
			{
				MessageBox.Show("Couldn't find the registered directory, make sure you mounted the NAND correctly");
				return;
			}
			NCALogTb.Text = "Hactool LOG\r\n";
			NCALogTb.Visible = true;

			bool homeFound = false;
			bool myFound = false;

			KeyFile = Path.GetFullPath("hactool\\keys.dat");
			HactoolExe = Path.GetFullPath("hactool\\hactool.exe");

			void FindLoop(string f)
			{
				string o = HactoolInfo(f);
				UpdateTb();
				if (o == null)
					return;
				if (!homeFound && o.Contains("0100000000001000") && fileSize(f) > 1000000) //home  > 1MB
				{
					Console.WriteLine("extracting home");
					HactoolExtract(f, "Home");
					homeFound = true;
				}
				else if (!myFound && o.Contains("0100000000001013") && fileSize(f) > 1000000) //my page > 1MB
				{
					Console.WriteLine("extracting user");
					HactoolExtract(f, "UserSet");
					myFound = true;
				}
			}

			foreach (string dir in Directory.GetDirectories(NcasDir).Reverse())
			{
				FindLoop(path(dir, "00"));
				if (homeFound && myFound) break;
			}
			foreach (string file in Directory.GetFiles(NcasDir))
			{
				FindLoop(file);
				if (homeFound && myFound) break;
			}

			if (!homeFound)
				MessageBox.Show("Couldn't find home menu :(");
			if (!myFound)
				MessageBox.Show("Couldn't find user settings :(");
			MessageBox.Show("Done");
		}
	}
}
