
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
            this.pFunction = new System.Windows.Forms.Panel();
            this.lbFName = new System.Windows.Forms.Label();
            this.tbFName = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.pMachine = new System.Windows.Forms.Panel();
            this.lbMName = new System.Windows.Forms.Label();
            this.cbMDisabled = new System.Windows.Forms.CheckBox();
            this.tbMShortName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.tbMName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.btGetData = new System.Windows.Forms.Button();
            this.lwscMap1 = new lwsc_admin.LWSCMap();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.tbFDuration = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.cbFRelais1 = new System.Windows.Forms.CheckBox();
            this.cbFRelais2 = new System.Windows.Forms.CheckBox();
            this.btFSave = new System.Windows.Forms.Button();
            this.btMSave = new System.Windows.Forms.Button();
            this.btSave = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.pFunction.SuspendLayout();
            this.pMachine.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tvMachines
            // 
            this.tvMachines.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvMachines.Location = new System.Drawing.Point(0, 0);
            this.tvMachines.Name = "tvMachines";
            this.tvMachines.Size = new System.Drawing.Size(463, 470);
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
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.tvMachines);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.pFunction);
            this.splitContainer2.Panel2.Controls.Add(this.pMachine);
            this.splitContainer2.Size = new System.Drawing.Size(463, 742);
            this.splitContainer2.SplitterDistance = 470;
            this.splitContainer2.TabIndex = 1;
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
            this.pFunction.Size = new System.Drawing.Size(463, 268);
            this.pFunction.TabIndex = 7;
            this.pFunction.Visible = false;
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
            // pMachine
            // 
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
            this.pMachine.Size = new System.Drawing.Size(463, 268);
            this.pMachine.TabIndex = 3;
            this.pMachine.Visible = false;
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
            // btGetData
            // 
            this.btGetData.Location = new System.Drawing.Point(355, 12);
            this.btGetData.Name = "btGetData";
            this.btGetData.Size = new System.Drawing.Size(163, 31);
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
            // tbFDuration
            // 
            this.tbFDuration.Location = new System.Drawing.Point(104, 70);
            this.tbFDuration.Name = "tbFDuration";
            this.tbFDuration.Size = new System.Drawing.Size(100, 22);
            this.tbFDuration.TabIndex = 8;
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
            // btSave
            // 
            this.btSave.Location = new System.Drawing.Point(524, 12);
            this.btSave.Name = "btSave";
            this.btSave.Size = new System.Drawing.Size(163, 31);
            this.btSave.TabIndex = 3;
            this.btSave.Text = "Save Config";
            this.btSave.UseVisualStyleBackColor = true;
            this.btSave.Click += new System.EventHandler(this.btSave_Click);
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
            this.pFunction.ResumeLayout(false);
            this.pFunction.PerformLayout();
            this.pMachine.ResumeLayout(false);
            this.pMachine.PerformLayout();
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
    }
}

