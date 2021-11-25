﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace lwsc_admin
{

    public partial class Form1 : Form
    {
        enum RESTType {
            GET,
            POST
        }

        class SimpleFunction
        {
            public uint functionId;
            public uint machineId;
            public string name;
            public byte relaisBitmask;
            public int duration;
        }

        public class MachineFunction
        {
            public uint machineId;
            public uint functionId;
            public string name;
            public byte relaisBitmask;
            public int duration;
            public uint symbolX;
            public uint symbolY;
            public byte rotation;
        }

        public class MachineData
        {
            public uint id;
            public string name;
            public string shortName;
            public bool disabled;
            public uint symbolX;
            public uint symbolY;
            public uint relais1Counter;
            public uint relais2Counter;
            public List<MachineFunction> functions;

            //filled my rssi query
            public sbyte rssi;
            public Dictionary<uint, sbyte> rssiMap = new Dictionary<uint, sbyte>();

            public int FunctionCount()
            {
                int i = 0;
                foreach (var f in functions)
                    if (f.relaisBitmask != 0)
                        i++;
                return i;
            }

            public string GetName()
            {
                string n = name.ToString();
                if (n.Length == 0 || n == "?")
                    n = shortName.ToString();
                if (n.Length == 0 || n == "?")
                    n = "0x" + id.ToString("X8");
                return n;
            }
        }

        const string baseurl = "http://192.168.4.1";
        static public List<MachineData> machines = new List<MachineData>();
        public Form1()
        {
            InitializeComponent();
            lwscMap1.LocationUpdate += LwscMap1_LocationUpdate;
            lwscMap1.Blink += LwscMap1_Blink;
            lwscMap1.Fire += LwscMap1_Fire;
        }

        private void FillTreeView()
        {
            tvMachines.Nodes.Clear();
            for (int i1 = 0; i1 < machines.Count; i1++)
            {
                MachineData m = machines[i1];
                TreeNode node = new TreeNode(m.GetName());
                node.Tag = i1;
                tvMachines.Nodes.Add(node);
                node.Nodes.Add("Id: " + "0x" + m.id.ToString("X8"));
                node.Nodes.Add("Name: " + m.name);
                node.Nodes.Add("ShortName: " + m.shortName);
                node.Nodes.Add("Disabled: " + m.disabled);
                node.Nodes.Add("SymbolY: " + m.symbolX);
                node.Nodes.Add("SymbolY: " + m.symbolY);
                node.Nodes.Add("Relais1Counter: " + m.relais1Counter);
                node.Nodes.Add("Relais2Counter: " + m.relais2Counter);
                TreeNode nodeF = new TreeNode("Functions");
                node.Nodes.Add(nodeF);

                foreach (MachineFunction f in m.functions)
                {
                    TreeNode nodeFF = new TreeNode(f.functionId.ToString() + (f.name.Length > 0 ? ": " + f.name : ""));
                    nodeFF.Tag = (int)f.functionId;
                    nodeF.Nodes.Add(nodeFF);
                    nodeFF.Nodes.Add("Name: " + f.name);
                    nodeFF.Nodes.Add("Duration: " + f.duration);
                    nodeFF.Nodes.Add("RelaisBitmask: 0b" + Convert.ToString(f.relaisBitmask, 2));
                }
            }

            lwscMap1.Invalidate();
        }

        private void LwscMap1_Blink(uint m_id)
        {
            string res = "";
            var status = RESTful("/blink?id=" + m_id, RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
        }

        private void LwscMap1_Fire(uint m_id, uint f_id)
        {
            string res = "";
            var status = RESTful("/fire?id=" + m_id + "&f_id=" + f_id, RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
        }

        private void LwscMap1_LocationUpdate(int i)
        {
            var m = machines[i];
            string res = "";
            var status = RESTful("/machine?id=" + m.id + "&name=" + m.name + "&shortName=" + m.shortName + "&disabled=" + (m.disabled ? "1" : "0") + "&symbolX=" + m.symbolX + "&symbolY=" + m.symbolY + "", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
        }

        private HttpStatusCode RESTful(string url, RESTType type, out string result)
        {
            result = "";
            var request = (HttpWebRequest)WebRequest.Create(baseurl + url);
            request.Timeout = 2000;

            request.Method = (type == RESTType.GET) ? "GET" : "POST";

            toolStripStatusLabel.Text = request.Method + ": " + url.Replace("&", "&&");

            var content = string.Empty;
            try
            {
                var response = (HttpWebResponse)request.GetResponse();
                using (var stream = response.GetResponseStream())
                {
                    using (var sr = new StreamReader(stream))
                    {
                        content = sr.ReadToEnd();
                    }
                }

                result = content;
            } catch
            {
                return HttpStatusCode.NotFound;
            }
            return HttpStatusCode.OK;
        }

        private void btSave_Click(object sender, EventArgs e)
        {
            string res = "";
            var status = RESTful("/save_config", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
        }

        private void btGetData_Click(object sender, EventArgs e)
        {
            int i = 0;
            machines.Clear();
            while (true)
            {
                string res = "";
                var status = RESTful("/machine?it=" + i + "&force=1", RESTType.GET, out res);
                if (status != HttpStatusCode.OK)
                    break;
                machines.Add(JsonConvert.DeserializeObject<MachineData>(res));
                i++;
            }
            machines = machines.OrderBy(x => x.name).ToList();
            FillTreeView();
        }

        private void btGetFunctions_Click(object sender, EventArgs e)
        {
            dgvFunctions.Rows.Clear();
            string res = "";
            var status = RESTful("/all_functions", RESTType.GET, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();

            var fArray = JsonConvert.DeserializeObject<SimpleFunction[]>(res);
            foreach(var f in fArray)
                dgvFunctions.Rows.Add(f.name, f.duration + " ms", ((f.relaisBitmask & 0x01) == 0x01) ? "x" : "", ((f.relaisBitmask & 0x02) == 0x02) ? "x" : "", "0x" + f.machineId.ToString("X8"), machines.FirstOrDefault(x => x.id == f.machineId)?.GetName(), f.functionId);
        }

        int mSelected = -1;
        int fSelected = -1;
        private void tvMachines_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node.Parent == null)
            {
                pMachine.Visible = true;
                pFunction.Visible = false;
                mSelected = (int)e.Node.Tag;
                fSelected = -1;
            }
            else if (e.Node.Parent.Parent == null)
            {
                pMachine.Visible = true;
                pFunction.Visible = false;
                mSelected = (int)e.Node.Parent.Tag;
                fSelected = -1;
            }
            else if (e.Node.Parent.Parent.Parent == null)
            {
                pMachine.Visible = false;
                pFunction.Visible = true;
                mSelected = (int)e.Node.Parent.Parent.Tag;
                fSelected = (int)e.Node.Tag;
            }
            else if (e.Node.Parent.Parent.Parent.Parent == null)
            {
                pMachine.Visible = false;
                pFunction.Visible = true;
                mSelected = (int)e.Node.Parent.Parent.Parent.Tag;
                fSelected = (int)e.Node.Parent.Tag;
            } else
            {
                mSelected = -1;
                fSelected = -1;
            }

            if (mSelected >= 0)
            {
                tbMNewId.Text = machines[mSelected].id.ToString("X8");
                lbMName.Text = machines[mSelected].GetName();
                tbMName.Text = machines[mSelected].name;
                tbMShortName.Text = machines[mSelected].shortName;
                cbMDisabled.Checked = machines[mSelected].disabled;
            }
            if (fSelected >= 0)
            {
                lbFName.Text = machines[mSelected].GetName() + "[" + fSelected + "]";
                tbFName.Text = machines[mSelected].functions[fSelected].name;
                tbFDuration.Text = machines[mSelected].functions[fSelected].duration.ToString();
                var x = machines[mSelected].functions[fSelected].relaisBitmask & 0x01;
                var y = machines[mSelected].functions[fSelected].relaisBitmask & 0x02;
                cbFRelais1.Checked = x == 0x01;
                cbFRelais2.Checked = y == 0x02;
            }
        }

        private void btMSave_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];

            m.name = tbMName.Text;
            m.shortName = tbMShortName.Text;
            m.disabled = cbMDisabled.Checked;

            string res = "";
            var status = RESTful("/machine?id=" + m.id + "&name=" + m.name + "&shortName=" + m.shortName + "&disabled=" + (m.disabled ? "1" : "0") + "&symbolX=" + m.symbolX + "&symbolY=" + m.symbolY + "", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
            FillTreeView();
        }

        private void btFSave_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[fSelected];

            f.name = tbFName.Text;
            f.duration = int.Parse(tbFDuration.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2.Checked) f.relaisBitmask += 0x02;

            string res = "";
            var status = RESTful("/function?id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
            FillTreeView();
        }

        private void btMReassign_Click(object sender, EventArgs e)
        {
            DialogResult dialogResult = MessageBox.Show("Are you sure?", "Reassign ID", MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;
            var m = machines[mSelected];

            var old_id = m.id;
            m.id = uint.Parse(tbMNewId.Text, System.Globalization.NumberStyles.HexNumber);

            string res = "";
            var status = RESTful("/change_id?id=" + old_id + "&new_id=" + m.id, RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
            btGetData_Click(null, null);
        }

        private void btDownloadConfig_Click(object sender, EventArgs e)
        {
            using (var client = new WebClient())
            {
                client.DownloadFile(baseurl + "/config", "machines.conf");
            }
            FileInfo f = new FileInfo(Application.ExecutablePath);
            Process.Start(f.DirectoryName);
        }

        private void btUploadConfig_Click(object sender, EventArgs e)
        {
            DialogResult dialogResult = MessageBox.Show("Are you sure?", "Override Config", MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;
            using(WebClient client = new WebClient())
            {
                client.UploadFile(baseurl + "/upload", "machines.conf");
            }
        }

        private void btQueryRSSI_Click(object sender, EventArgs e)
        {
            DialogResult dialogResult = MessageBox.Show("WIFI will be disconnected, sure?", "Query RSSI", MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;

            string res = "";
            var status = RESTful("/query_rssi", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
        }

        private void btGetRSSI_Click(object sender, EventArgs e)
        {
            foreach (var m in machines)
            {
                string res = "";
                var status = RESTful("/machine_rssi?id=" + m.id, RESTType.GET, out res);
                if (status != HttpStatusCode.OK)
                    MessageBox.Show("Error @ " + m.id.ToString("X8"));

                dynamic dJson = JsonConvert.DeserializeObject(res);
                m.rssi = dJson["rssi"];

                if (dJson["rssiMap"] == null)
                    continue;

                foreach(var r in dJson["rssiMap"])
                {
                    uint id = r["id"];
                    sbyte rssi  = r["rssi"];
                    m.rssiMap[id] = rssi;
                }
            }
            lwscMap1.Invalidate();
        }

        private void dgvFunctions_DoubleClick(object sender, EventArgs e)
        {
            var row = dgvFunctions.CurrentCell.RowIndex;
            var m_id = uint.Parse(dgvFunctions.Rows[row].Cells[4].Value.ToString().Replace("0x", ""), System.Globalization.NumberStyles.HexNumber);
            var f_id = (uint)dgvFunctions.Rows[row].Cells[6].Value;
            LwscMap1_Fire(m_id, f_id);
        }
    }
}