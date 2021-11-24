using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
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

        public class MachineFunction
        {
            public uint machineId;
            public uint functionId;
            public string name;
            public char relaisBitmask;
            public int duration;
            public uint symbolX;
            public uint symbolY;
            public char rotation;
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
                    TreeNode nodeFF = new TreeNode(f.functionId.ToString());
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
                cbFRelais2.Checked = (machines[mSelected].functions[fSelected].relaisBitmask > 0) & (char)0x01 == (char)0x01 ? true : false;
                cbFRelais2.Checked = (machines[mSelected].functions[fSelected].relaisBitmask > 1) & (char)0x02 == (char)0x01 ? true : false;
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
            f.relaisBitmask = (char)0x00;
            if (cbFRelais1.Checked) f.relaisBitmask += (char)0x01;
            if (cbFRelais2.Checked) f.relaisBitmask += (char)0x02;

            string res = "";
            var status = RESTful("/function?id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out res);
            if (status != HttpStatusCode.OK)
                throw new Exception();
            FillTreeView();
        }
    }
}
