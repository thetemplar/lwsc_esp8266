
namespace lwsc_admin
{
    partial class Form1
    {
        /// <summary>
        /// Erforderliche Designervariable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Verwendete Ressourcen bereinigen.
        /// </summary>
        /// <param name="disposing">True, wenn verwaltete Ressourcen gelöscht werden sollen; andernfalls False.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Vom Windows Form-Designer generierter Code

        /// <summary>
        /// Erforderliche Methode für die Designerunterstützung.
        /// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
        /// </summary>
        private void InitializeComponent()
        {
            this.tvMachines = new System.Windows.Forms.TreeView();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.dgvFunctions = new System.Windows.Forms.DataGridView();
            this.colName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colDuration = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colRelais1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colRelais2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colMachineId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colMachineName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colFunctionId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.pMachine = new System.Windows.Forms.Panel();
            this.tbMNewId = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.btMReassign = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.btMSave = new System.Windows.Forms.Button();
            this.lbMName = new System.Windows.Forms.Label();
            this.cbMDisabled = new System.Windows.Forms.CheckBox();
            this.tbMShortName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.tbMName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.pFunction = new System.Windows.Forms.Panel();
            this.btFSave = new System.Windows.Forms.Button();
            this.cbFRelais2 = new System.Windows.Forms.CheckBox();
            this.cbFRelais1 = new System.Windows.Forms.CheckBox();
            this.tbFDuration = new System.Windows.Forms.TextBox();
            this.lbFName = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.tbFName = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.btGetFunctions = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label10 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.btGetRSSI = new System.Windows.Forms.Button();
            this.btQueryRSSI = new System.Windows.Forms.Button();
            this.btUploadConfig = new System.Windows.Forms.Button();
            this.btDownloadConfig = new System.Windows.Forms.Button();
            this.btSave = new System.Windows.Forms.Button();
            this.btGetData = new System.Windows.Forms.Button();
            this.lwscMap1 = new lwsc_admin.LWSCMap();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgvFunctions)).BeginInit();
            this.pMachine.SuspendLayout();
            this.pFunction.SuspendLayout();
            this.panel1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tvMachines
            // 
            this.tvMachines.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvMachines.Location = new System.Drawing.Point(3, 3);
            this.tvMachines.Name = "tvMachines";
            this.tvMachines.Size = new System.Drawing.Size(449, 498);
            this.tvMachines.TabIndex = 0;
            this.tvMachines.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.tvMachines_AfterSelect);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.splitContainer2);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.btGetFunctions);
            this.splitContainer1.Panel2.Controls.Add(this.panel1);
            this.splitContainer1.Panel2.Controls.Add(this.btGetRSSI);
            this.splitContainer1.Panel2.Controls.Add(this.btQueryRSSI);
            this.splitContainer1.Panel2.Controls.Add(this.btUploadConfig);
            this.splitContainer1.Panel2.Controls.Add(this.btDownloadConfig);
            this.splitContainer1.Panel2.Controls.Add(this.btSave);
            this.splitContainer1.Panel2.Controls.Add(this.btGetData);
            this.splitContainer1.Panel2.Controls.Add(this.lwscMap1);
            this.splitContainer1.Size = new System.Drawing.Size(1392, 742);
            this.splitContainer1.SplitterDistance = 463;
            this.splitContainer1.TabIndex = 1;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.tabControl1);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.pMachine);
            this.splitContainer2.Panel2.Controls.Add(this.pFunction);
            this.splitContainer2.Size = new System.Drawing.Size(463, 742);
            this.splitContainer2.SplitterDistance = 533;
            this.splitContainer2.TabIndex = 1;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(463, 533);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.tvMachines);
            this.tabPage1.Location = new System.Drawing.Point(4, 25);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(455, 504);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Machines";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.dgvFunctions);
            this.tabPage2.Location = new System.Drawing.Point(4, 25);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(455, 504);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Functions";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // dgvFunctions
            // 
            this.dgvFunctions.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvFunctions.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colName,
            this.colDuration,
            this.colRelais1,
            this.colRelais2,
            this.colMachineId,
            this.colMachineName,
            this.colFunctionId});
            this.dgvFunctions.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dgvFunctions.Location = new System.Drawing.Point(3, 3);
            this.dgvFunctions.Name = "dgvFunctions";
            this.dgvFunctions.RowHeadersWidth = 51;
            this.dgvFunctions.RowTemplate.Height = 24;
            this.dgvFunctions.Size = new System.Drawing.Size(449, 498);
            this.dgvFunctions.TabIndex = 0;
            this.dgvFunctions.DoubleClick += new System.EventHandler(this.dgvFunctions_DoubleClick);
            // 
            // colName
            // 
            this.colName.HeaderText = "Name";
            this.colName.MinimumWidth = 6;
            this.colName.Name = "colName";
            this.colName.ReadOnly = true;
            this.colName.Width = 150;
            // 
            // colDuration
            // 
            this.colDuration.HeaderText = "Duration";
            this.colDuration.MinimumWidth = 6;
            this.colDuration.Name = "colDuration";
            this.colDuration.ReadOnly = true;
            this.colDuration.Width = 80;
            // 
            // colRelais1
            // 
            this.colRelais1.HeaderText = "Relais1";
            this.colRelais1.MinimumWidth = 6;
            this.colRelais1.Name = "colRelais1";
            this.colRelais1.ReadOnly = true;
            this.colRelais1.Width = 60;
            // 
            // colRelais2
            // 
            this.colRelais2.HeaderText = "Relais2";
            this.colRelais2.MinimumWidth = 6;
            this.colRelais2.Name = "colRelais2";
            this.colRelais2.ReadOnly = true;
            this.colRelais2.Width = 60;
            // 
            // colMachineId
            // 
            this.colMachineId.HeaderText = "Machine Id";
            this.colMachineId.MinimumWidth = 6;
            this.colMachineId.Name = "colMachineId";
            this.colMachineId.ReadOnly = true;
            this.colMachineId.Width = 120;
            // 
            // colMachineName
            // 
            this.colMachineName.HeaderText = "Machine Name";
            this.colMachineName.MinimumWidth = 6;
            this.colMachineName.Name = "colMachineName";
            this.colMachineName.Width = 125;
            // 
            // colFunctionId
            // 
            this.colFunctionId.HeaderText = "Function Id";
            this.colFunctionId.MinimumWidth = 6;
            this.colFunctionId.Name = "colFunctionId";
            this.colFunctionId.ReadOnly = true;
            this.colFunctionId.Width = 80;
            // 
            // pMachine
            // 
            this.pMachine.Controls.Add(this.tbMNewId);
            this.pMachine.Controls.Add(this.label6);
            this.pMachine.Controls.Add(this.btMReassign);
            this.pMachine.Controls.Add(this.label4);
            this.pMachine.Controls.Add(this.btMSave);
            this.pMachine.Controls.Add(this.lbMName);
            this.pMachine.Controls.Add(this.cbMDisabled);
            this.pMachine.Controls.Add(this.tbMShortName);
            this.pMachine.Controls.Add(this.label2);
            this.pMachine.Controls.Add(this.tbMName);
            this.pMachine.Controls.Add(this.label1);
            this.pMachine.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pMachine.Location = new System.Drawing.Point(0, 0);
            this.pMachine.Name = "pMachine";
            this.pMachine.Size = new System.Drawing.Size(463, 205);
            this.pMachine.TabIndex = 3;
            this.pMachine.Visible = false;
            // 
            // tbMNewId
            // 
            this.tbMNewId.Location = new System.Drawing.Point(292, 62);
            this.tbMNewId.Name = "tbMNewId";
            this.tbMNewId.Size = new System.Drawing.Size(100, 22);
            this.tbMNewId.TabIndex = 13;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(271, 65);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(22, 17);
            this.label6.TabIndex = 15;
            this.label6.Text = "0x";
            // 
            // btMReassign
            // 
            this.btMReassign.Location = new System.Drawing.Point(292, 90);
            this.btMReassign.Name = "btMReassign";
            this.btMReassign.Size = new System.Drawing.Size(100, 26);
            this.btMReassign.TabIndex = 14;
            this.btMReassign.Text = "Reassign";
            this.btMReassign.UseVisualStyleBackColor = true;
            this.btMReassign.Click += new System.EventHandler(this.btMReassign_Click);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(240, 42);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(146, 17);
            this.label4.TabIndex = 12;
            this.label4.Text = "Reassign ID (Careful!)";
            // 
            // btMSave
            // 
            this.btMSave.Location = new System.Drawing.Point(103, 122);
            this.btMSave.Name = "btMSave";
            this.btMSave.Size = new System.Drawing.Size(85, 26);
            this.btMSave.TabIndex = 11;
            this.btMSave.Text = "Save";
            this.btMSave.UseVisualStyleBackColor = true;
            this.btMSave.Click += new System.EventHandler(this.btMSave_Click);
            // 
            // lbMName
            // 
            this.lbMName.AutoSize = true;
            this.lbMName.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbMName.Location = new System.Drawing.Point(19, 17);
            this.lbMName.Name = "lbMName";
            this.lbMName.Size = new System.Drawing.Size(52, 18);
            this.lbMName.TabIndex = 6;
            this.lbMName.Text = "Name";
            // 
            // cbMDisabled
            // 
            this.cbMDisabled.AutoSize = true;
            this.cbMDisabled.Location = new System.Drawing.Point(103, 95);
            this.cbMDisabled.Name = "cbMDisabled";
            this.cbMDisabled.Size = new System.Drawing.Size(85, 21);
            this.cbMDisabled.TabIndex = 5;
            this.cbMDisabled.Text = "Disabled";
            this.cbMDisabled.UseVisualStyleBackColor = true;
            // 
            // tbMShortName
            // 
            this.tbMShortName.Location = new System.Drawing.Point(104, 67);
            this.tbMShortName.Name = "tbMShortName";
            this.tbMShortName.Size = new System.Drawing.Size(100, 22);
            this.tbMShortName.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(19, 70);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(79, 17);
            this.label2.TabIndex = 2;
            this.label2.Text = "ShortName";
            // 
            // tbMName
            // 
            this.tbMName.Location = new System.Drawing.Point(104, 39);
            this.tbMName.Name = "tbMName";
            this.tbMName.Size = new System.Drawing.Size(100, 22);
            this.tbMName.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(52, 42);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(45, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "Name";
            // 
            // pFunction
            // 
            this.pFunction.Controls.Add(this.btFSave);
            this.pFunction.Controls.Add(this.cbFRelais2);
            this.pFunction.Controls.Add(this.cbFRelais1);
            this.pFunction.Controls.Add(this.tbFDuration);
            this.pFunction.Controls.Add(this.lbFName);
            this.pFunction.Controls.Add(this.label3);
            this.pFunction.Controls.Add(this.tbFName);
            this.pFunction.Controls.Add(this.label5);
            this.pFunction.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pFunction.Location = new System.Drawing.Point(0, 0);
            this.pFunction.Name = "pFunction";
            this.pFunction.Size = new System.Drawing.Size(463, 205);
            this.pFunction.TabIndex = 7;
            this.pFunction.Visible = false;
            // 
            // btFSave
            // 
            this.btFSave.Location = new System.Drawing.Point(104, 153);
            this.btFSave.Name = "btFSave";
            this.btFSave.Size = new System.Drawing.Size(85, 26);
            this.btFSave.TabIndex = 10;
            this.btFSave.Text = "Save";
            this.btFSave.UseVisualStyleBackColor = true;
            this.btFSave.Click += new System.EventHandler(this.btFSave_Click);
            // 
            // cbFRelais2
            // 
            this.cbFRelais2.AutoSize = true;
            this.cbFRelais2.Location = new System.Drawing.Point(104, 126);
            this.cbFRelais2.Name = "cbFRelais2";
            this.cbFRelais2.Size = new System.Drawing.Size(77, 21);
            this.cbFRelais2.TabIndex = 9;
            this.cbFRelais2.Text = "Relais2";
            this.cbFRelais2.UseVisualStyleBackColor = true;
            // 
            // cbFRelais1
            // 
            this.cbFRelais1.AutoSize = true;
            this.cbFRelais1.Location = new System.Drawing.Point(104, 99);
            this.cbFRelais1.Name = "cbFRelais1";
            this.cbFRelais1.Size = new System.Drawing.Size(81, 21);
            this.cbFRelais1.TabIndex = 7;
            this.cbFRelais1.Text = "Relais 1";
            this.cbFRelais1.UseVisualStyleBackColor = true;
            // 
            // tbFDuration
            // 
            this.tbFDuration.Location = new System.Drawing.Point(104, 70);
            this.tbFDuration.Name = "tbFDuration";
            this.tbFDuration.Size = new System.Drawing.Size(100, 22);
            this.tbFDuration.TabIndex = 8;
            // 
            // lbFName
            // 
            this.lbFName.AutoSize = true;
            this.lbFName.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbFName.Location = new System.Drawing.Point(19, 17);
            this.lbFName.Name = "lbFName";
            this.lbFName.Size = new System.Drawing.Size(52, 18);
            this.lbFName.TabIndex = 6;
            this.lbFName.Text = "Name";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 73);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(94, 17);
            this.label3.TabIndex = 7;
            this.label3.Text = "Duration (ms)";
            // 
            // tbFName
            // 
            this.tbFName.Location = new System.Drawing.Point(104, 39);
            this.tbFName.Name = "tbFName";
            this.tbFName.Size = new System.Drawing.Size(100, 22);
            this.tbFName.TabIndex = 1;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(52, 42);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(45, 17);
            this.label5.TabIndex = 0;
            this.label5.Text = "Name";
            // 
            // btGetFunctions
            // 
            this.btGetFunctions.Location = new System.Drawing.Point(736, 12);
            this.btGetFunctions.Name = "btGetFunctions";
            this.btGetFunctions.Size = new System.Drawing.Size(121, 31);
            this.btGetFunctions.TabIndex = 9;
            this.btGetFunctions.Text = "Get Functions";
            this.btGetFunctions.UseVisualStyleBackColor = true;
            this.btGetFunctions.Click += new System.EventHandler(this.btGetFunctions_Click);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.label10);
            this.panel1.Controls.Add(this.label9);
            this.panel1.Controls.Add(this.label8);
            this.panel1.Controls.Add(this.label7);
            this.panel1.Location = new System.Drawing.Point(532, 12);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(71, 76);
            this.panel1.TabIndex = 8;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(1, 54);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(57, 17);
            this.label10.TabIndex = 3;
            this.label10.Text = "X = Fire";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(1, 37);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(65, 17);
            this.label9.TabIndex = 2;
            this.label9.Text = "R = RSSI";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(1, 20);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(63, 17);
            this.label8.TabIndex = 1;
            this.label8.Text = "B = Blink";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(1, 3);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(69, 17);
            this.label7.TabIndex = 0;
            this.label7.Text = "O = Move";
            // 
            // btGetRSSI
            // 
            this.btGetRSSI.Location = new System.Drawing.Point(736, 86);
            this.btGetRSSI.Name = "btGetRSSI";
            this.btGetRSSI.Size = new System.Drawing.Size(121, 31);
            this.btGetRSSI.TabIndex = 7;
            this.btGetRSSI.Text = "Get RSSI";
            this.btGetRSSI.UseVisualStyleBackColor = true;
            this.btGetRSSI.Click += new System.EventHandler(this.btGetRSSI_Click);
            // 
            // btQueryRSSI
            // 
            this.btQueryRSSI.Location = new System.Drawing.Point(609, 86);
            this.btQueryRSSI.Name = "btQueryRSSI";
            this.btQueryRSSI.Size = new System.Drawing.Size(121, 31);
            this.btQueryRSSI.TabIndex = 6;
            this.btQueryRSSI.Text = "Query RSSI";
            this.btQueryRSSI.UseVisualStyleBackColor = true;
            this.btQueryRSSI.Click += new System.EventHandler(this.btQueryRSSI_Click);
            // 
            // btUploadConfig
            // 
            this.btUploadConfig.Location = new System.Drawing.Point(736, 49);
            this.btUploadConfig.Name = "btUploadConfig";
            this.btUploadConfig.Size = new System.Drawing.Size(121, 31);
            this.btUploadConfig.TabIndex = 5;
            this.btUploadConfig.Text = "Upload Cfg";
            this.btUploadConfig.UseVisualStyleBackColor = true;
            this.btUploadConfig.Click += new System.EventHandler(this.btUploadConfig_Click);
            // 
            // btDownloadConfig
            // 
            this.btDownloadConfig.Location = new System.Drawing.Point(609, 49);
            this.btDownloadConfig.Name = "btDownloadConfig";
            this.btDownloadConfig.Size = new System.Drawing.Size(121, 31);
            this.btDownloadConfig.TabIndex = 4;
            this.btDownloadConfig.Text = "Download Cfg";
            this.btDownloadConfig.UseVisualStyleBackColor = true;
            this.btDownloadConfig.Click += new System.EventHandler(this.btDownloadConfig_Click);
            // 
            // btSave
            // 
            this.btSave.Location = new System.Drawing.Point(736, 123);
            this.btSave.Name = "btSave";
            this.btSave.Size = new System.Drawing.Size(121, 31);
            this.btSave.TabIndex = 3;
            this.btSave.Text = "Save Config";
            this.btSave.UseVisualStyleBackColor = true;
            this.btSave.Click += new System.EventHandler(this.btSave_Click);
            // 
            // btGetData
            // 
            this.btGetData.Location = new System.Drawing.Point(609, 12);
            this.btGetData.Name = "btGetData";
            this.btGetData.Size = new System.Drawing.Size(121, 31);
            this.btGetData.TabIndex = 1;
            this.btGetData.Text = "Get Data";
            this.btGetData.UseVisualStyleBackColor = true;
            this.btGetData.Click += new System.EventHandler(this.btGetData_Click);
            // 
            // lwscMap1
            // 
            this.lwscMap1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lwscMap1.Location = new System.Drawing.Point(0, 0);
            this.lwscMap1.Name = "lwscMap1";
            this.lwscMap1.Size = new System.Drawing.Size(925, 742);
            this.lwscMap1.TabIndex = 2;
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel});
            this.statusStrip1.Location = new System.Drawing.Point(0, 720);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1392, 22);
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel
            // 
            this.toolStripStatusLabel.Name = "toolStripStatusLabel";
            this.toolStripStatusLabel.Size = new System.Drawing.Size(0, 16);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1392, 742);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.splitContainer1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dgvFunctions)).EndInit();
            this.pMachine.ResumeLayout(false);
            this.pMachine.PerformLayout();
            this.pFunction.ResumeLayout(false);
            this.pFunction.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TreeView tvMachines;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.Button btGetData;
        private LWSCMap lwscMap1;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.Panel pMachine;
        private System.Windows.Forms.Label lbMName;
        private System.Windows.Forms.CheckBox cbMDisabled;
        private System.Windows.Forms.TextBox tbMShortName;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbMName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Panel pFunction;
        private System.Windows.Forms.Label lbFName;
        private System.Windows.Forms.TextBox tbFName;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox tbFDuration;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox cbFRelais2;
        private System.Windows.Forms.CheckBox cbFRelais1;
        private System.Windows.Forms.Button btFSave;
        private System.Windows.Forms.Button btMSave;
        private System.Windows.Forms.Button btSave;
        private System.Windows.Forms.TextBox tbMNewId;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button btMReassign;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button btDownloadConfig;
        private System.Windows.Forms.Button btUploadConfig;
        private System.Windows.Forms.Button btGetRSSI;
        private System.Windows.Forms.Button btQueryRSSI;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btGetFunctions;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.DataGridView dgvFunctions;
        private System.Windows.Forms.DataGridViewTextBoxColumn colName;
        private System.Windows.Forms.DataGridViewTextBoxColumn colDuration;
        private System.Windows.Forms.DataGridViewTextBoxColumn colRelais1;
        private System.Windows.Forms.DataGridViewTextBoxColumn colRelais2;
        private System.Windows.Forms.DataGridViewTextBoxColumn colMachineId;
        private System.Windows.Forms.DataGridViewTextBoxColumn colMachineName;
        private System.Windows.Forms.DataGridViewTextBoxColumn colFunctionId;
    }
}

