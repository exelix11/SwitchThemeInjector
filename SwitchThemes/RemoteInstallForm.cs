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
		public RemoteInstallForm()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if (textBox1.Text.Trim() == "")
			{
				MessageBox.Show("Enter a valid address");
				return;
			}
			OpenFileDialog opn = new OpenFileDialog() { Filter = "theme files (*.nxtheme,*.szs)|*.nxtheme;*.szs"};
			if (opn.ShowDialog() != DialogResult.OK) return;
			byte[] theme = System.IO.File.ReadAllBytes(opn.FileName);
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
				sock.SendBufferSize = arr.Length;

				sock.Connect(textBox1.Text, 5000);

				if (sock.Connected)
				{
					sock.Send(arr, SocketFlags.None);

					byte[] by = new byte[2];
					sock.Receive(by, SocketFlags.None);

					sock.Close();

				}
				else
					MessageBox.Show("Socket didn't connect");
			}
			catch (Exception ex)
			{
				MessageBox.Show("There was an error: " + ex.ToString());
			}
		}
	}
}
