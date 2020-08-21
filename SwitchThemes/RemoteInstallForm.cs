using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using Syroot.BinaryData;
using System.Net;
using System.Net.Sockets;

namespace SwitchThemes
{
	public partial class RemoteInstallForm : Form
	{
		public string DefaultFileName;

		public RemoteInstallForm(string DefaultName = "")
		{
			InitializeComponent();
			DefaultFileName = DefaultName;
		}

		public static string DoRemoteInstall(string Ip, byte[] theme)
		{
			var mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem, UTF8Encoding.ASCII);
			bin.Write("theme", BinaryStringFormat.NoPrefixOrTermination);
			bin.Write(new byte[3]);
			bin.Write((Int32)theme.Length);
			bin.Write(theme);
			try
			{
				Socket sock =
					new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

				var arr = mem.ToArray();

				sock.Connect(Ip, 5000);

				if (sock.Connected)
				{
					sock.Send(arr, SocketFlags.None);

					byte[] by = new byte[2];
					if (sock.Receive(by, SocketFlags.None) != 2)
						return "Didn't receive confirmation from switch :(";

					sock.Close();

				}
				else
					return "Socket didn't connect";
			}
			catch (Exception ex)
			{
				return "There was an error: " + ex.ToString();
			}
			return null;
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if (textBox1.Text.Trim() == "")
			{
				MessageBox.Show("Enter a valid address");
				return;
			}
			OpenFileDialog opn = new OpenFileDialog() { Filter = "theme files (*.nxtheme,*.szs)|*.nxtheme;*.szs", FileName = DefaultFileName };
			if (opn.ShowDialog() != DialogResult.OK) return;
			byte[] theme = System.IO.File.ReadAllBytes(opn.FileName);

			string res = DoRemoteInstall(textBox1.Text, theme);
			if (res != null)
				MessageBox.Show(res);
		}

		private void RemoteInstallForm_DragEnter(object sender, DragEventArgs e)
		{
			if (e.GetFiles()?.Length == 1)
				e.Effect = DragDropEffects.Copy;
		}

		private void RemoteInstallForm_DragDrop(object sender, DragEventArgs e)
		{
			byte[] theme = System.IO.File.ReadAllBytes(e.GetFiles().First());
			string res = DoRemoteInstall(textBox1.Text, theme);
			if (res != null)
				MessageBox.Show(res);
		}
	}
}
