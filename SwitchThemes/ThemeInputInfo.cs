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
		public Tuple<string, string> result = null;

		public ThemeInputInfo()
		{
			InitializeComponent();
		}

		public static Tuple<string, string> Ask()
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
			result = new Tuple<string, string>(tbThemeName.Text, tbAuthorName.Text);
			this.Close();
		}

		private void button2_Click(object sender, EventArgs e)
		{
			result = null;
			this.Close();
		}
	}
}
