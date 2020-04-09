using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SwitchThemes
{
	public partial class ThemeInputInfo : Form
	{
		public (string, string) result { get; set; } = (null, null);

		public ThemeInputInfo()
		{
			InitializeComponent();
		}

		public static (string, string) Ask()
		{
			var f = new ThemeInputInfo();
			f.ShowDialog();
			return f.result;
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if (tbThemeName.Text.Trim() == "")
			{
				MessageBox.Show("Insert a valid theme name to continue");
				return;
			}
			result = (tbThemeName.Text, tbAuthorName.Text);
			this.Close();
		}

		private void button2_Click(object sender, EventArgs e)
		{
			result = (null, null);
			this.Close();
		}

		private void ThemeInputInfo_Load(object sender, EventArgs e)
		{

		}
	}
}
