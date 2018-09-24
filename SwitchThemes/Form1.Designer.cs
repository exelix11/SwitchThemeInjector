namespace SwitchThemes
{
	partial class Form1
	{
		/// <summary>
		/// Variabile di progettazione necessaria.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Pulire le risorse in uso.
		/// </summary>
		/// <param name="disposing">ha valore true se le risorse gestite devono essere eliminate, false in caso contrario.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Codice generato da Progettazione Windows Form

		/// <summary>
		/// Metodo necessario per il supporto della finestra di progettazione. Non modificare
		/// il contenuto del metodo con l'editor di codice.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
			this.materialTabSelector1 = new MaterialSkin.Controls.MaterialTabSelector();
			this.materialTabControl1 = new MaterialSkin.Controls.MaterialTabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.materialRaisedButton1 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialLabel1 = new MaterialSkin.Controls.MaterialLabel();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.materialLabel3 = new MaterialSkin.Controls.MaterialLabel();
			this.materialFlatButton1 = new MaterialSkin.Controls.MaterialFlatButton();
			this.materialLabel2 = new MaterialSkin.Controls.MaterialLabel();
			this.tbBntxFile = new System.Windows.Forms.TextBox();
			this.materialRaisedButton2 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialDivider1 = new MaterialSkin.Controls.MaterialDivider();
			this.materialRaisedButton3 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.label1 = new System.Windows.Forms.Label();
			this.materialTabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.SuspendLayout();
			// 
			// materialTabSelector1
			// 
			this.materialTabSelector1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialTabSelector1.BaseTabControl = this.materialTabControl1;
			this.materialTabSelector1.Depth = 0;
			this.materialTabSelector1.Enabled = false;
			this.materialTabSelector1.Location = new System.Drawing.Point(-1, 103);
			this.materialTabSelector1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialTabSelector1.Name = "materialTabSelector1";
			this.materialTabSelector1.Size = new System.Drawing.Size(665, 52);
			this.materialTabSelector1.TabIndex = 5;
			this.materialTabSelector1.Text = "materialTabSelector1";
			// 
			// materialTabControl1
			// 
			this.materialTabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialTabControl1.Controls.Add(this.tabPage1);
			this.materialTabControl1.Controls.Add(this.tabPage2);
			this.materialTabControl1.Depth = 0;
			this.materialTabControl1.Location = new System.Drawing.Point(-1, 151);
			this.materialTabControl1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialTabControl1.Name = "materialTabControl1";
			this.materialTabControl1.SelectedIndex = 0;
			this.materialTabControl1.Size = new System.Drawing.Size(645, 269);
			this.materialTabControl1.TabIndex = 4;
			// 
			// tabPage1
			// 
			this.tabPage1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.tabPage1.Controls.Add(this.materialRaisedButton1);
			this.tabPage1.Controls.Add(this.materialLabel1);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(637, 243);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "Extract resources";
			// 
			// materialRaisedButton1
			// 
			this.materialRaisedButton1.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.materialRaisedButton1.Depth = 0;
			this.materialRaisedButton1.Location = new System.Drawing.Point(248, 164);
			this.materialRaisedButton1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton1.Name = "materialRaisedButton1";
			this.materialRaisedButton1.Primary = true;
			this.materialRaisedButton1.Size = new System.Drawing.Size(142, 36);
			this.materialRaisedButton1.TabIndex = 10;
			this.materialRaisedButton1.Text = "Export BNTX";
			this.materialRaisedButton1.UseVisualStyleBackColor = true;
			this.materialRaisedButton1.Click += new System.EventHandler(this.materialRaisedButton1_Click);
			// 
			// materialLabel1
			// 
			this.materialLabel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialLabel1.Depth = 0;
			this.materialLabel1.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel1.Location = new System.Drawing.Point(9, 13);
			this.materialLabel1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel1.Name = "materialLabel1";
			this.materialLabel1.Size = new System.Drawing.Size(618, 148);
			this.materialLabel1.TabIndex = 0;
			this.materialLabel1.Text = resources.GetString("materialLabel1.Text");
			// 
			// tabPage2
			// 
			this.tabPage2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.tabPage2.Controls.Add(this.materialLabel3);
			this.tabPage2.Controls.Add(this.materialFlatButton1);
			this.tabPage2.Controls.Add(this.materialLabel2);
			this.tabPage2.Controls.Add(this.tbBntxFile);
			this.tabPage2.Controls.Add(this.materialRaisedButton2);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(637, 243);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Inject & patch";
			// 
			// materialLabel3
			// 
			this.materialLabel3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialLabel3.Depth = 0;
			this.materialLabel3.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel3.Location = new System.Drawing.Point(11, 50);
			this.materialLabel3.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel3.Name = "materialLabel3";
			this.materialLabel3.Size = new System.Drawing.Size(652, 148);
			this.materialLabel3.TabIndex = 5;
			this.materialLabel3.Text = resources.GetString("materialLabel3.Text");
			// 
			// materialFlatButton1
			// 
			this.materialFlatButton1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.materialFlatButton1.AutoSize = true;
			this.materialFlatButton1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialFlatButton1.Depth = 0;
			this.materialFlatButton1.Location = new System.Drawing.Point(607, 10);
			this.materialFlatButton1.Margin = new System.Windows.Forms.Padding(4, 6, 4, 6);
			this.materialFlatButton1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialFlatButton1.Name = "materialFlatButton1";
			this.materialFlatButton1.Primary = false;
			this.materialFlatButton1.Size = new System.Drawing.Size(23, 36);
			this.materialFlatButton1.TabIndex = 3;
			this.materialFlatButton1.Text = "...";
			this.materialFlatButton1.UseVisualStyleBackColor = true;
			this.materialFlatButton1.Click += new System.EventHandler(this.materialFlatButton1_Click);
			// 
			// materialLabel2
			// 
			this.materialLabel2.AutoSize = true;
			this.materialLabel2.Depth = 0;
			this.materialLabel2.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel2.Location = new System.Drawing.Point(11, 18);
			this.materialLabel2.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel2.Name = "materialLabel2";
			this.materialLabel2.Size = new System.Drawing.Size(71, 19);
			this.materialLabel2.TabIndex = 2;
			this.materialLabel2.Text = "Bntx file: ";
			// 
			// tbBntxFile
			// 
			this.tbBntxFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbBntxFile.Location = new System.Drawing.Point(86, 17);
			this.tbBntxFile.Name = "tbBntxFile";
			this.tbBntxFile.Size = new System.Drawing.Size(514, 20);
			this.tbBntxFile.TabIndex = 1;
			// 
			// materialRaisedButton2
			// 
			this.materialRaisedButton2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.materialRaisedButton2.Depth = 0;
			this.materialRaisedButton2.Location = new System.Drawing.Point(457, 192);
			this.materialRaisedButton2.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton2.Name = "materialRaisedButton2";
			this.materialRaisedButton2.Primary = true;
			this.materialRaisedButton2.Size = new System.Drawing.Size(173, 39);
			this.materialRaisedButton2.TabIndex = 0;
			this.materialRaisedButton2.Text = "Patch and save";
			this.materialRaisedButton2.UseVisualStyleBackColor = true;
			this.materialRaisedButton2.Click += new System.EventHandler(this.materialRaisedButton2_Click);
			// 
			// materialDivider1
			// 
			this.materialDivider1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialDivider1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(55)))), ((int)(((byte)(71)))), ((int)(((byte)(79)))));
			this.materialDivider1.Depth = 0;
			this.materialDivider1.Location = new System.Drawing.Point(-1, 63);
			this.materialDivider1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialDivider1.Name = "materialDivider1";
			this.materialDivider1.Size = new System.Drawing.Size(646, 52);
			this.materialDivider1.TabIndex = 6;
			this.materialDivider1.Text = "materialDivider1";
			this.materialDivider1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.materialDivider1_MouseDown);
			// 
			// materialRaisedButton3
			// 
			this.materialRaisedButton3.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.materialRaisedButton3.Depth = 0;
			this.materialRaisedButton3.Location = new System.Drawing.Point(252, 70);
			this.materialRaisedButton3.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton3.Name = "materialRaisedButton3";
			this.materialRaisedButton3.Primary = true;
			this.materialRaisedButton3.Size = new System.Drawing.Size(142, 36);
			this.materialRaisedButton3.TabIndex = 11;
			this.materialRaisedButton3.Text = "Open common.szs";
			this.materialRaisedButton3.UseVisualStyleBackColor = true;
			this.materialRaisedButton3.Click += new System.EventHandler(this.materialRaisedButton3_Click);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(0, 409);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(113, 13);
			this.label1.TabIndex = 12;
			this.label1.Text = "Exelix @ Team Qcean";
			this.label1.Click += new System.EventHandler(this.label1_Click);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(645, 422);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.materialRaisedButton3);
			this.Controls.Add(this.materialTabSelector1);
			this.Controls.Add(this.materialDivider1);
			this.Controls.Add(this.materialTabControl1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimumSize = new System.Drawing.Size(645, 422);
			this.Name = "Form1";
			this.Text = "Qcean\'s Switch theme injector - only for 5.1.0";
			this.Load += new System.EventHandler(this.Form1_Load);
			this.materialTabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.tabPage2.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private MaterialSkin.Controls.MaterialTabSelector materialTabSelector1;
		private MaterialSkin.Controls.MaterialTabControl materialTabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private MaterialSkin.Controls.MaterialDivider materialDivider1;
		private MaterialSkin.Controls.MaterialLabel materialLabel1;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton1;
		private MaterialSkin.Controls.MaterialLabel materialLabel3;
		private MaterialSkin.Controls.MaterialFlatButton materialFlatButton1;
		private MaterialSkin.Controls.MaterialLabel materialLabel2;
		private System.Windows.Forms.TextBox tbBntxFile;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton2;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton3;
		private System.Windows.Forms.Label label1;
	}
}

