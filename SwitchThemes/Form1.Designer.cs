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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
			this.materialTabControl1 = new MaterialSkin.Controls.MaterialTabControl();
			this.PatchListPage = new System.Windows.Forms.TabPage();
			this.tbPatches = new System.Windows.Forms.TextBox();
			this.InjectPage = new System.Windows.Forms.TabPage();
			this.LayoutPatchList = new System.Windows.Forms.ComboBox();
			this.materialLabel6 = new MaterialSkin.Controls.MaterialLabel();
			this.materialLabel1 = new MaterialSkin.Controls.MaterialLabel();
			this.materialLabel3 = new MaterialSkin.Controls.MaterialLabel();
			this.materialFlatButton1 = new MaterialSkin.Controls.MaterialFlatButton();
			this.materialLabel2 = new MaterialSkin.Controls.MaterialLabel();
			this.tbBntxFile = new System.Windows.Forms.TextBox();
			this.materialRaisedButton2 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.AdvPanel = new System.Windows.Forms.Panel();
			this.materialRaisedButton6 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialRaisedButton4 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialLabel5 = new MaterialSkin.Controls.MaterialLabel();
			this.SzsFileList = new System.Windows.Forms.ListBox();
			this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.extractToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.replaceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.materialRaisedButton1 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialRaisedButton5 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.checkBox1 = new System.Windows.Forms.CheckBox();
			this.materialLabel4 = new MaterialSkin.Controls.MaterialLabel();
			this.materialRaisedButton3 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.label1 = new System.Windows.Forms.Label();
			this.materialTabSelector1 = new MaterialSkin.Controls.MaterialTabSelector();
			this.lblDetected = new MaterialSkin.Controls.MaterialLabel();
			this.materialDivider1 = new MaterialSkin.Controls.MaterialDivider();
			this.materialRaisedButton7 = new MaterialSkin.Controls.MaterialRaisedButton();
			this.materialTabControl1.SuspendLayout();
			this.PatchListPage.SuspendLayout();
			this.InjectPage.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.AdvPanel.SuspendLayout();
			this.contextMenuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// materialTabControl1
			// 
			this.materialTabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialTabControl1.Controls.Add(this.PatchListPage);
			this.materialTabControl1.Controls.Add(this.InjectPage);
			this.materialTabControl1.Controls.Add(this.tabPage1);
			this.materialTabControl1.Depth = 0;
			this.materialTabControl1.Location = new System.Drawing.Point(-1, 152);
			this.materialTabControl1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialTabControl1.Name = "materialTabControl1";
			this.materialTabControl1.SelectedIndex = 0;
			this.materialTabControl1.Size = new System.Drawing.Size(645, 303);
			this.materialTabControl1.TabIndex = 4;
			// 
			// PatchListPage
			// 
			this.PatchListPage.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.PatchListPage.Controls.Add(this.tbPatches);
			this.PatchListPage.Location = new System.Drawing.Point(4, 22);
			this.PatchListPage.Name = "PatchListPage";
			this.PatchListPage.Size = new System.Drawing.Size(637, 277);
			this.PatchListPage.TabIndex = 2;
			this.PatchListPage.Text = "Patch list";
			// 
			// tbPatches
			// 
			this.tbPatches.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbPatches.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.tbPatches.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.tbPatches.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.tbPatches.ForeColor = System.Drawing.Color.White;
			this.tbPatches.Location = new System.Drawing.Point(9, 0);
			this.tbPatches.Multiline = true;
			this.tbPatches.Name = "tbPatches";
			this.tbPatches.ReadOnly = true;
			this.tbPatches.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.tbPatches.Size = new System.Drawing.Size(628, 249);
			this.tbPatches.TabIndex = 12;
			// 
			// InjectPage
			// 
			this.InjectPage.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.InjectPage.Controls.Add(this.LayoutPatchList);
			this.InjectPage.Controls.Add(this.materialLabel6);
			this.InjectPage.Controls.Add(this.materialLabel1);
			this.InjectPage.Controls.Add(this.materialLabel3);
			this.InjectPage.Controls.Add(this.materialFlatButton1);
			this.InjectPage.Controls.Add(this.materialLabel2);
			this.InjectPage.Controls.Add(this.tbBntxFile);
			this.InjectPage.Controls.Add(this.materialRaisedButton2);
			this.InjectPage.Location = new System.Drawing.Point(4, 22);
			this.InjectPage.Name = "InjectPage";
			this.InjectPage.Padding = new System.Windows.Forms.Padding(3);
			this.InjectPage.Size = new System.Drawing.Size(637, 277);
			this.InjectPage.TabIndex = 1;
			this.InjectPage.Text = "Inject & patch";
			// 
			// LayoutPatchList
			// 
			this.LayoutPatchList.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.LayoutPatchList.BackColor = System.Drawing.Color.White;
			this.LayoutPatchList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.LayoutPatchList.FormattingEnabled = true;
			this.LayoutPatchList.Items.AddRange(new object[] {
            "Don\'t patch"});
			this.LayoutPatchList.Location = new System.Drawing.Point(114, 66);
			this.LayoutPatchList.Name = "LayoutPatchList";
			this.LayoutPatchList.Size = new System.Drawing.Size(485, 21);
			this.LayoutPatchList.TabIndex = 8;
			// 
			// materialLabel6
			// 
			this.materialLabel6.AutoSize = true;
			this.materialLabel6.Depth = 0;
			this.materialLabel6.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel6.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel6.Location = new System.Drawing.Point(11, 68);
			this.materialLabel6.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel6.Name = "materialLabel6";
			this.materialLabel6.Size = new System.Drawing.Size(103, 19);
			this.materialLabel6.TabIndex = 7;
			this.materialLabel6.Text = "Layout patch: ";
			// 
			// materialLabel1
			// 
			this.materialLabel1.AutoSize = true;
			this.materialLabel1.Depth = 0;
			this.materialLabel1.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel1.Location = new System.Drawing.Point(20, 9);
			this.materialLabel1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel1.Name = "materialLabel1";
			this.materialLabel1.Size = new System.Drawing.Size(603, 19);
			this.materialLabel1.TabIndex = 6;
			this.materialLabel1.Text = "Since version 3.0 you don\'t need bntx_editor anymore, just click on ... and open " +
    "your dds !";
			// 
			// materialLabel3
			// 
			this.materialLabel3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialLabel3.Depth = 0;
			this.materialLabel3.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel3.Location = new System.Drawing.Point(11, 90);
			this.materialLabel3.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel3.Name = "materialLabel3";
			this.materialLabel3.Size = new System.Drawing.Size(612, 142);
			this.materialLabel3.TabIndex = 5;
			this.materialLabel3.Text = resources.GetString("materialLabel3.Text");
			// 
			// materialFlatButton1
			// 
			this.materialFlatButton1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.materialFlatButton1.AutoSize = true;
			this.materialFlatButton1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialFlatButton1.Depth = 0;
			this.materialFlatButton1.ForeColor = System.Drawing.Color.White;
			this.materialFlatButton1.Icon = null;
			this.materialFlatButton1.Location = new System.Drawing.Point(600, 31);
			this.materialFlatButton1.Margin = new System.Windows.Forms.Padding(4, 6, 4, 6);
			this.materialFlatButton1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialFlatButton1.Name = "materialFlatButton1";
			this.materialFlatButton1.Primary = false;
			this.materialFlatButton1.Size = new System.Drawing.Size(32, 36);
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
			this.materialLabel2.Location = new System.Drawing.Point(10, 39);
			this.materialLabel2.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel2.Name = "materialLabel2";
			this.materialLabel2.Size = new System.Drawing.Size(71, 19);
			this.materialLabel2.TabIndex = 2;
			this.materialLabel2.Text = "DDS file: ";
			// 
			// tbBntxFile
			// 
			this.tbBntxFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbBntxFile.Location = new System.Drawing.Point(85, 38);
			this.tbBntxFile.Name = "tbBntxFile";
			this.tbBntxFile.Size = new System.Drawing.Size(514, 20);
			this.tbBntxFile.TabIndex = 1;
			// 
			// materialRaisedButton2
			// 
			this.materialRaisedButton2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.materialRaisedButton2.AutoSize = true;
			this.materialRaisedButton2.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton2.Depth = 0;
			this.materialRaisedButton2.Icon = null;
			this.materialRaisedButton2.Location = new System.Drawing.Point(495, 235);
			this.materialRaisedButton2.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton2.Name = "materialRaisedButton2";
			this.materialRaisedButton2.Primary = true;
			this.materialRaisedButton2.Size = new System.Drawing.Size(135, 36);
			this.materialRaisedButton2.TabIndex = 0;
			this.materialRaisedButton2.Text = "Patch and save";
			this.materialRaisedButton2.UseVisualStyleBackColor = true;
			this.materialRaisedButton2.Click += new System.EventHandler(this.PatchButtonClick);
			// 
			// tabPage1
			// 
			this.tabPage1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(51)))), ((int)(((byte)(51)))), ((int)(((byte)(51)))));
			this.tabPage1.Controls.Add(this.AdvPanel);
			this.tabPage1.Controls.Add(this.checkBox1);
			this.tabPage1.Controls.Add(this.materialLabel4);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(637, 277);
			this.tabPage1.TabIndex = 3;
			this.tabPage1.Text = "Advanced";
			// 
			// AdvPanel
			// 
			this.AdvPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.AdvPanel.Controls.Add(this.materialRaisedButton7);
			this.AdvPanel.Controls.Add(this.materialRaisedButton6);
			this.AdvPanel.Controls.Add(this.materialRaisedButton4);
			this.AdvPanel.Controls.Add(this.materialLabel5);
			this.AdvPanel.Controls.Add(this.SzsFileList);
			this.AdvPanel.Controls.Add(this.materialRaisedButton1);
			this.AdvPanel.Controls.Add(this.materialRaisedButton5);
			this.AdvPanel.Enabled = false;
			this.AdvPanel.Location = new System.Drawing.Point(9, 52);
			this.AdvPanel.Name = "AdvPanel";
			this.AdvPanel.Size = new System.Drawing.Size(619, 219);
			this.AdvPanel.TabIndex = 7;
			this.AdvPanel.Visible = false;
			// 
			// materialRaisedButton6
			// 
			this.materialRaisedButton6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.materialRaisedButton6.AutoSize = true;
			this.materialRaisedButton6.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton6.Depth = 0;
			this.materialRaisedButton6.Icon = null;
			this.materialRaisedButton6.Location = new System.Drawing.Point(294, 177);
			this.materialRaisedButton6.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton6.Name = "materialRaisedButton6";
			this.materialRaisedButton6.Primary = true;
			this.materialRaisedButton6.Size = new System.Drawing.Size(104, 36);
			this.materialRaisedButton6.TabIndex = 9;
			this.materialRaisedButton6.Text = "Layout diff";
			this.materialRaisedButton6.UseVisualStyleBackColor = true;
			this.materialRaisedButton6.Click += new System.EventHandler(this.materialRaisedButton6_Click);
			// 
			// materialRaisedButton4
			// 
			this.materialRaisedButton4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.materialRaisedButton4.AutoSize = true;
			this.materialRaisedButton4.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton4.Depth = 0;
			this.materialRaisedButton4.Icon = null;
			this.materialRaisedButton4.Location = new System.Drawing.Point(181, 177);
			this.materialRaisedButton4.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton4.Name = "materialRaisedButton4";
			this.materialRaisedButton4.Primary = true;
			this.materialRaisedButton4.Size = new System.Drawing.Size(110, 36);
			this.materialRaisedButton4.TabIndex = 8;
			this.materialRaisedButton4.Text = "Diff file list";
			this.materialRaisedButton4.UseVisualStyleBackColor = true;
			this.materialRaisedButton4.Click += new System.EventHandler(this.materialRaisedButton4_Click);
			// 
			// materialLabel5
			// 
			this.materialLabel5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialLabel5.Depth = 0;
			this.materialLabel5.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel5.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel5.Location = new System.Drawing.Point(3, 0);
			this.materialLabel5.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel5.Name = "materialLabel5";
			this.materialLabel5.Size = new System.Drawing.Size(613, 23);
			this.materialLabel5.TabIndex = 7;
			this.materialLabel5.Text = "Files in the SZS : (left click to replace or extract)";
			// 
			// SzsFileList
			// 
			this.SzsFileList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.SzsFileList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.SzsFileList.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.SzsFileList.ContextMenuStrip = this.contextMenuStrip1;
			this.SzsFileList.ForeColor = System.Drawing.Color.White;
			this.SzsFileList.FormattingEnabled = true;
			this.SzsFileList.Location = new System.Drawing.Point(3, 26);
			this.SzsFileList.Name = "SzsFileList";
			this.SzsFileList.Size = new System.Drawing.Size(613, 143);
			this.SzsFileList.Sorted = true;
			this.SzsFileList.TabIndex = 5;
			// 
			// contextMenuStrip1
			// 
			this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.extractToolStripMenuItem,
            this.replaceToolStripMenuItem});
			this.contextMenuStrip1.Name = "contextMenuStrip1";
			this.contextMenuStrip1.Size = new System.Drawing.Size(116, 48);
			// 
			// extractToolStripMenuItem
			// 
			this.extractToolStripMenuItem.Name = "extractToolStripMenuItem";
			this.extractToolStripMenuItem.Size = new System.Drawing.Size(115, 22);
			this.extractToolStripMenuItem.Text = "Extract";
			this.extractToolStripMenuItem.Click += new System.EventHandler(this.extractToolStripMenuItem_Click);
			// 
			// replaceToolStripMenuItem
			// 
			this.replaceToolStripMenuItem.Name = "replaceToolStripMenuItem";
			this.replaceToolStripMenuItem.Size = new System.Drawing.Size(115, 22);
			this.replaceToolStripMenuItem.Text = "Replace";
			this.replaceToolStripMenuItem.Click += new System.EventHandler(this.replaceToolStripMenuItem_Click);
			// 
			// materialRaisedButton1
			// 
			this.materialRaisedButton1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.materialRaisedButton1.AutoSize = true;
			this.materialRaisedButton1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton1.Depth = 0;
			this.materialRaisedButton1.Icon = null;
			this.materialRaisedButton1.Location = new System.Drawing.Point(484, 177);
			this.materialRaisedButton1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton1.Name = "materialRaisedButton1";
			this.materialRaisedButton1.Primary = true;
			this.materialRaisedButton1.Size = new System.Drawing.Size(132, 36);
			this.materialRaisedButton1.TabIndex = 4;
			this.materialRaisedButton1.Text = "Save edited SZS";
			this.materialRaisedButton1.UseVisualStyleBackColor = true;
			this.materialRaisedButton1.Click += new System.EventHandler(this.materialRaisedButton1_Click);
			// 
			// materialRaisedButton5
			// 
			this.materialRaisedButton5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.materialRaisedButton5.AutoSize = true;
			this.materialRaisedButton5.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton5.Depth = 0;
			this.materialRaisedButton5.Icon = null;
			this.materialRaisedButton5.Location = new System.Drawing.Point(3, 177);
			this.materialRaisedButton5.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton5.Name = "materialRaisedButton5";
			this.materialRaisedButton5.Primary = true;
			this.materialRaisedButton5.Size = new System.Drawing.Size(175, 36);
			this.materialRaisedButton5.TabIndex = 3;
			this.materialRaisedButton5.Text = "Texture usage Count";
			this.materialRaisedButton5.UseVisualStyleBackColor = true;
			this.materialRaisedButton5.Click += new System.EventHandler(this.materialRaisedButton5_Click);
			// 
			// checkBox1
			// 
			this.checkBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.checkBox1.AutoSize = true;
			this.checkBox1.ForeColor = System.Drawing.Color.White;
			this.checkBox1.Location = new System.Drawing.Point(493, 28);
			this.checkBox1.Name = "checkBox1";
			this.checkBox1.Size = new System.Drawing.Size(135, 17);
			this.checkBox1.TabIndex = 0;
			this.checkBox1.Text = "Enable advanced tools";
			this.checkBox1.UseVisualStyleBackColor = true;
			this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
			// 
			// materialLabel4
			// 
			this.materialLabel4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialLabel4.Depth = 0;
			this.materialLabel4.Font = new System.Drawing.Font("Roboto", 11F);
			this.materialLabel4.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.materialLabel4.Location = new System.Drawing.Point(0, 3);
			this.materialLabel4.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialLabel4.Name = "materialLabel4";
			this.materialLabel4.Size = new System.Drawing.Size(637, 46);
			this.materialLabel4.TabIndex = 6;
			this.materialLabel4.Text = "Advanced tools allow you to manually edit the szs and more to create custom patch" +
    "es, enable them only if you know what you\'re doing";
			// 
			// materialRaisedButton3
			// 
			this.materialRaisedButton3.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.materialRaisedButton3.AutoSize = true;
			this.materialRaisedButton3.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton3.Depth = 0;
			this.materialRaisedButton3.Icon = null;
			this.materialRaisedButton3.Location = new System.Drawing.Point(279, 52);
			this.materialRaisedButton3.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton3.Name = "materialRaisedButton3";
			this.materialRaisedButton3.Primary = true;
			this.materialRaisedButton3.Size = new System.Drawing.Size(85, 36);
			this.materialRaisedButton3.TabIndex = 11;
			this.materialRaisedButton3.Text = "Open szs";
			this.materialRaisedButton3.UseVisualStyleBackColor = true;
			this.materialRaisedButton3.Click += new System.EventHandler(this.OpenSzsButton);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(0, 444);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(113, 13);
			this.label1.TabIndex = 12;
			this.label1.Text = "Exelix @ Team Qcean";
			this.label1.Click += new System.EventHandler(this.label1_Click);
			// 
			// materialTabSelector1
			// 
			this.materialTabSelector1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialTabSelector1.BaseTabControl = this.materialTabControl1;
			this.materialTabSelector1.Depth = 0;
			this.materialTabSelector1.Location = new System.Drawing.Point(-1, 116);
			this.materialTabSelector1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialTabSelector1.Name = "materialTabSelector1";
			this.materialTabSelector1.Size = new System.Drawing.Size(646, 30);
			this.materialTabSelector1.TabIndex = 5;
			this.materialTabSelector1.Text = "materialTabSelector1";
			// 
			// lblDetected
			// 
			this.lblDetected.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.lblDetected.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(55)))), ((int)(((byte)(71)))), ((int)(((byte)(79)))));
			this.lblDetected.Depth = 0;
			this.lblDetected.Font = new System.Drawing.Font("Roboto", 11F);
			this.lblDetected.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
			this.lblDetected.Location = new System.Drawing.Point(-1, 91);
			this.lblDetected.MouseState = MaterialSkin.MouseState.HOVER;
			this.lblDetected.Name = "lblDetected";
			this.lblDetected.Size = new System.Drawing.Size(646, 22);
			this.lblDetected.TabIndex = 13;
			this.lblDetected.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.lblDetected.MouseDown += new System.Windows.Forms.MouseEventHandler(this.DragForm_MouseDown);
			// 
			// materialDivider1
			// 
			this.materialDivider1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.materialDivider1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(55)))), ((int)(((byte)(71)))), ((int)(((byte)(78)))));
			this.materialDivider1.Depth = 0;
			this.materialDivider1.Location = new System.Drawing.Point(-1, 63);
			this.materialDivider1.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialDivider1.Name = "materialDivider1";
			this.materialDivider1.Size = new System.Drawing.Size(646, 67);
			this.materialDivider1.TabIndex = 14;
			this.materialDivider1.Text = "materialDivider1";
			this.materialDivider1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.DragForm_MouseDown);
			// 
			// materialRaisedButton7
			// 
			this.materialRaisedButton7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.materialRaisedButton7.AutoSize = true;
			this.materialRaisedButton7.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.materialRaisedButton7.Depth = 0;
			this.materialRaisedButton7.Icon = null;
			this.materialRaisedButton7.Location = new System.Drawing.Point(401, 177);
			this.materialRaisedButton7.MouseState = MaterialSkin.MouseState.HOVER;
			this.materialRaisedButton7.Name = "materialRaisedButton7";
			this.materialRaisedButton7.Primary = true;
			this.materialRaisedButton7.Size = new System.Drawing.Size(79, 36);
			this.materialRaisedButton7.TabIndex = 10;
			this.materialRaisedButton7.Text = "File diff";
			this.materialRaisedButton7.UseVisualStyleBackColor = true;
			this.materialRaisedButton7.Click += new System.EventHandler(this.materialRaisedButton7_Click);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(645, 457);
			this.Controls.Add(this.lblDetected);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.materialRaisedButton3);
			this.Controls.Add(this.materialTabSelector1);
			this.Controls.Add(this.materialTabControl1);
			this.Controls.Add(this.materialDivider1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimumSize = new System.Drawing.Size(645, 457);
			this.Name = "Form1";
			this.Text = "Qcean\'s Switch theme injector";
			this.Load += new System.EventHandler(this.Form1_Load);
			this.materialTabControl1.ResumeLayout(false);
			this.PatchListPage.ResumeLayout(false);
			this.PatchListPage.PerformLayout();
			this.InjectPage.ResumeLayout(false);
			this.InjectPage.PerformLayout();
			this.tabPage1.ResumeLayout(false);
			this.tabPage1.PerformLayout();
			this.AdvPanel.ResumeLayout(false);
			this.AdvPanel.PerformLayout();
			this.contextMenuStrip1.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private MaterialSkin.Controls.MaterialTabControl materialTabControl1;
		private System.Windows.Forms.TabPage InjectPage;
		private MaterialSkin.Controls.MaterialLabel materialLabel3;
		private MaterialSkin.Controls.MaterialFlatButton materialFlatButton1;
		private MaterialSkin.Controls.MaterialLabel materialLabel2;
		private System.Windows.Forms.TextBox tbBntxFile;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton2;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton3;
		private System.Windows.Forms.Label label1;
		private MaterialSkin.Controls.MaterialTabSelector materialTabSelector1;
		private System.Windows.Forms.TabPage PatchListPage;
		private System.Windows.Forms.TextBox tbPatches;
		private MaterialSkin.Controls.MaterialLabel lblDetected;
		private MaterialSkin.Controls.MaterialDivider materialDivider1;
		private MaterialSkin.Controls.MaterialLabel materialLabel1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.CheckBox checkBox1;
		private MaterialSkin.Controls.MaterialLabel materialLabel4;
		private System.Windows.Forms.Panel AdvPanel;
		private MaterialSkin.Controls.MaterialLabel materialLabel5;
		private System.Windows.Forms.ListBox SzsFileList;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton1;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton5;
		private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
		private System.Windows.Forms.ToolStripMenuItem extractToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem replaceToolStripMenuItem;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton4;
		private System.Windows.Forms.ComboBox LayoutPatchList;
		private MaterialSkin.Controls.MaterialLabel materialLabel6;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton6;
		private MaterialSkin.Controls.MaterialRaisedButton materialRaisedButton7;
	}
}

