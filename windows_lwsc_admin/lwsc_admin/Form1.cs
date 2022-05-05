using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

#pragma warning disable IDE1006 // Benennungsstile

namespace lwsc_admin
{

    public partial class Form1 : Form
    {
        enum RESTType {
            GET,
            POST,
            DELETE
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
            public override string ToString()
            {
                return name;
            }
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

            //local
            internal bool minimized = true;

            public int FunctionCount()
            {
                int i = 0;
                foreach (var f in functions)
                    if (f.relaisBitmask != 0)
                        i++;
                return i;
            }

            public override string ToString()
            {
                string n = name.ToString();
                if (n.Length == 0 || n == "?")
                    n = shortName.ToString();
                if (n.Length == 0 || n == "?")
                    n = "0x" + id.ToString("X8");
                return n;
            }
            
        }

        static public List<MachineData> machines = new List<MachineData>();
        static public List<MachineFunction> functions = new List<MachineFunction>();
        static public Dictionary<string, MachineFunction[,]> mappings = new Dictionary<string, MachineFunction[,]>();
        readonly UdpClient udpClient = new UdpClient();
        bool isEthernet = false;

        sealed class tmpM
        {
            public uint m_id = 0;
            public string name;
            public string cmd;
        };
        public Form1()
        {
            InitializeComponent();
            lwscMap1.LocationUpdate += LwscMap1_LocationUpdate;
            lwscMap1.Blink += LwscMap1_Blink;
            lwscMap1.Fire += LwscMap1_Fire;
            lwscMap1.ReqVersion += LwscMap1_ReqVersion;

            udpClient.Client.Bind(new IPEndPoint(IPAddress.Any, 5556));
            var from = new IPEndPoint(0, 0);


            Task.Run(() =>
            {
                bool waiting = true;
                while (waiting)
                {
                    var recvBuffer = udpClient.Receive(ref from);
                    var val = Encoding.UTF8.GetString(recvBuffer);
                    var m = Regex.Match(val, @"WIFIBRIDGE ((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)) (ETH|WIFI)");
                    if(m.Success)
                    {

                        this.tbIpAddress.Invoke((MethodInvoker)delegate {
                            tbIpAddress.Text = m.Groups[1].Value;
                            AddToLog(val);


                            GetData();
                            GetFunctions();
                            GetMappings();

                            pnRSSIButtons.Enabled = true;
                            waiting = false;
                            AddToLog("Setup done");

                            if(m.Groups[2].Value == "ETH")
                            {
                                btEspClick.Visible = true;
                                btEspHome.Visible = true;
                                btEspUp.Visible = true;
                                btEspDown.Visible = true;
                                isEthernet = true;
                            } else
                            {
                                btEspClick.Visible = false;
                                btEspHome.Visible = false;
                                btEspUp.Visible = false;
                                btEspDown.Visible = false;
                                isEthernet = false;
                            }
                        });

                    }

                }
            });

        }

        private static string FormatJson(string json)
        {
            dynamic parsedJson = JsonConvert.DeserializeObject(json);
            return JsonConvert.SerializeObject(parsedJson, Formatting.Indented);
        }

        private void AddToLog(string txt, bool italic = false)
        {
            this.rtbLog.Invoke((MethodInvoker)delegate
            {
                string str = DateTime.Now.ToString("HH:mm:ss") + ": ";
                rtbLog.Text = str + txt + Environment.NewLine + Environment.NewLine + rtbLog.Text;
            });
        }

        private void FillTreeView()
        {
            this.tvMachines.Invoke((MethodInvoker)delegate
            {
                tvMachines.Nodes.Clear();
                for (int i1 = 0; i1 < machines.Count; i1++)
                {
                    MachineData m = machines[i1];
                    TreeNode node = new TreeNode(m.ToString())
                    {
                        Tag = i1
                    };
                    node.Nodes.Add("Id: " + "0x" + m.id.ToString("X8"));
                    node.Nodes.Add("Name: " + m.name);
                    node.Nodes.Add("ShortName: " + m.shortName);
                    node.Nodes.Add("Disabled: " + m.disabled);
                    node.Nodes.Add("SymbolY: " + m.symbolX);
                    node.Nodes.Add("SymbolY: " + m.symbolY);
                    node.Nodes.Add("Relais1Counter: " + m.relais1Counter);
                    node.Nodes.Add("Relais2Counter: " + m.relais2Counter);
                    tvMachines.Nodes.Add(node);
                    TreeNode nodeF = new TreeNode("Functions");
                    node.Nodes.Add(nodeF);

                    foreach (MachineFunction f in m.functions)
                    {
                        TreeNode nodeFF = new TreeNode(f.functionId.ToString() + (f.name.Length > 0 ? ": " + f.name : ""))
                        {
                            Tag = (int)f.functionId
                        };
                        nodeF.Nodes.Add(nodeFF);
                        nodeFF.Nodes.Add("Name: " + f.name);
                        nodeFF.Nodes.Add("Duration: " + f.duration);
                        nodeFF.Nodes.Add("RelaisBitmask: 0b" + Convert.ToString(f.relaisBitmask, 2));
                    }
                }
            });
            lwscMap1.Invalidate();
        }

        private void LwscMap1_Blink(uint m_id)
        {
            var status = RESTful("/blink?password=lwsc-remote&username=Admin&id=" + m_id, RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
        }

        private void LwscMap1_Fire(uint m_id, uint f_id)
        {
            var status = RESTful("/fire?password=lwsc-remote&username=Admin&id=" + m_id + "&f_id=" + f_id, RESTType.POST, out string res);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            else
            {
                if(res.Contains("rssi"))
                {
                    //dynamic dJson = JsonConvert.DeserializeObject(res);
                    var data = (JObject)JsonConvert.DeserializeObject(res);
                    int a = data["rssi"].Value<int>();

                    MachineData m = machines.First(x => x.id == m_id);
                    m.rssiMap[0] = (sbyte)a;
                }
                MessageBox.Show(res);
            }
        }

        private void LwscMap1_ReqVersion(uint m_id)
        {
            MachineData m = machines.First(x => x.id == m_id);
            if (m == null || m.id == 0)
            {
                MessageBox.Show("Error: Machine not found");
                return;
            }

            var status = RESTful("/version?id=" + m.id, RESTType.POST, out string res);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            lwscMap1.Invalidate();
        }

        private void LwscMap1_LocationUpdate(int i)
        {
            var m = machines[i];
            var status = RESTful("/machine?password=lwsc-remote&username=Admin&id=" + m.id + "&name=" + m.name + "&shortName=" + m.shortName + "&disabled=" + (m.disabled ? "1" : "0") + "&symbolX=" + m.symbolX + "&symbolY=" + m.symbolY + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
                MessageBox.Show("Error: " + status);
        }

        private HttpStatusCode RESTful(string url, RESTType type, out string result, string post_content = "")
        {
            result = "";
            if (tbIpAddress.Text.Contains("searching"))
            {
                MessageBox.Show("Searching for Gateway..... Please wait!");
                AddToLog("Searching for Gateway..... Please wait!");
                return HttpStatusCode.BadGateway;
            }

            var request = (HttpWebRequest)WebRequest.Create("http://" + tbIpAddress.Text + url);
            request.Timeout = 3000;

            switch(type)
            {
                case RESTType.GET:
                    request.Method = "GET";
                    break;
                case RESTType.POST:
                    request.Method = "POST";
                    break;
                case RESTType.DELETE:
                    request.Method = "DELETE";
                    break;
                default:
                    throw new Exception();
            }

            toolStripStatusLabel.Text = request.Method + ": " + url.Replace("&", "&&");

            var content = string.Empty;
            AddToLog(type.ToString() + ": " + url);
            HttpWebResponse response;
            try
            {
                if (type == RESTType.POST && post_content.Length > 0)
                {
                    using (var streamWriter = new StreamWriter(request.GetRequestStream()))
                    {
                        streamWriter.Write(post_content);
                    }
                }

                response = (HttpWebResponse)request.GetResponse();
                using (var stream = response.GetResponseStream())
                {
                    using (var sr = new StreamReader(stream))
                    {
                        content = sr.ReadToEnd();
                    }
                }

                result = content;
            } catch (WebException e)
            {
                if (e.Status == WebExceptionStatus.ProtocolError)
                {
                    response = (HttpWebResponse)e.Response;
                    AddToLog(" << " + "Error: " + response.StatusCode);
                    return response.StatusCode;
                }
                else if (e.Status == WebExceptionStatus.Timeout)
                {
                    AddToLog(" << " + "Error: " + e.Status + Environment.NewLine + e.Message);
                    return HttpStatusCode.GatewayTimeout;
                }
                else
                {
                    AddToLog(" << " + "Error: " + e.Status + Environment.NewLine + e.Message);
                    return HttpStatusCode.Conflict;
                }
            }
            catch (Exception ex)
            {
                AddToLog(" << " + ex.Message, true);
                return HttpStatusCode.GatewayTimeout;
            }
            AddToLog(" << " + result, true);
            return HttpStatusCode.OK;
        }

        private void btSave_Click(object sender, EventArgs e)
        {
            var status = RESTful("/save_config", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
        }

        private void GetData()
        {
            machines.Clear();

            var status = RESTful("/machine_count?password=lwsc-remote&username=Admin", RESTType.GET, out string res);
            if (status != HttpStatusCode.OK)
                MessageBox.Show("Error: " + status);

            var o = JObject.Parse(res);
            int c = (int)o["count"];

            for(int i = 0; i < c; i++)
            {
                status = RESTful("/machine?password=lwsc-remote&username=Admin&id=" + i + "&force=1", RESTType.GET, out res);
                if (status != HttpStatusCode.OK)
                    break;
                machines.Add(JsonConvert.DeserializeObject<MachineData>(res));
            }
            machines = machines.OrderBy(x => x.name).ToList();
            FillTreeView();
        }

        private void GetFunctions()
        {
            dgvFunctions.Rows.Clear();
            var status = RESTful("/all_functions?password=lwsc-remote&username=Admin", RESTType.GET, out string res);
            if (status != HttpStatusCode.OK)
                MessageBox.Show("Error: " + status);

            if (res == null || res == "null")
                return;
            var fArray = JsonConvert.DeserializeObject<MachineFunction[]>(res);
            functions.Clear();
            functions.AddRange(fArray);
            foreach(var fRaw in fArray)
            {
                var m = machines.First(i => i.id == fRaw.machineId);
                var f = m.functions.First(i => i.functionId == fRaw.functionId);
                if (f.name != fRaw.name)
                    MessageBox.Show("Error: " + f.name + "!=" + fRaw.name);

                var mach = machines.FirstOrDefault(x => x.id == f.machineId);
                var func = mach?.functions.FirstOrDefault(x => x.functionId == f.functionId);

                this.dgvFunctions.Invoke((MethodInvoker)delegate
                {
                    dgvFunctions.Rows.Add(f.name, func?.duration + " ms", ((func?.relaisBitmask & 0x01) == 0x01) ? "x" : "", ((func?.relaisBitmask & 0x02) == 0x02) ? "x" : "", "0x" + f.machineId.ToString("X8"), mach?.ToString(), f.functionId);
                });
            }
        }
        private void GetMappings()
        {
            this.lbMapPad.Invoke((MethodInvoker)delegate
            {
                lbMapPad.Items.Clear();
                lbMapSelect.Items.Clear();

                using (var client = new WebClient())
                {
                    client.DownloadFile("http://" + tbIpAddress.Text + "/file?password=lwsc-remote&username=Admin&filename=mappings.json", "mappings.json");
                }
                string res = File.ReadAllText("mappings.json");
                if (res == "")
                    return;

                var fArray = JsonConvert.DeserializeObject<Dictionary<string, MachineFunction[,]>>(res);

                if (mappings == null)
                    return;
                mappings = fArray;

                foreach (var m in mappings)
                {
                    if (m.Key.StartsWith("map_"))
                        lbMapSelect.Items.Add(m.Key.ToString().Substring(4));
                    else if (m.Key.StartsWith("pad_"))
                        lbMapPad.Items.Add(m.Key.ToString().Substring(4));
                }
            });
        }

        int mSelected = -1;
        private void tvMachines_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node.Parent == null)
            {
                mSelected = (int)e.Node.Tag;
            }
            else if (e.Node.Parent.Parent == null)
            {
                mSelected = (int)e.Node.Parent.Tag;
            }
            else if (e.Node.Parent.Parent.Parent == null)
            {
                mSelected = (int)e.Node.Parent.Parent.Tag;
            }
            else if (e.Node.Parent.Parent.Parent.Parent == null)
            {
                mSelected = (int)e.Node.Parent.Parent.Parent.Tag;
            } else
            {
                mSelected = -1;
            }

            if (mSelected >= 0)
            {
                tbMNewId.Text = machines[mSelected].id.ToString("X8");
                lbMName.Text = machines[mSelected].ToString();
                tbMName.Text = machines[mSelected].name;
                tbMShortName.Text = machines[mSelected].shortName;
                tbMachineX.Text = machines[mSelected].symbolX.ToString();
                tbMachineY.Text = machines[mSelected].symbolY.ToString();
                cbMDisabled.Checked = machines[mSelected].disabled;

                tbFName1.Text = machines[mSelected].functions[0].name;
                tbFDuration1.Text = machines[mSelected].functions[0].duration.ToString();
                var x1 = machines[mSelected].functions[0].relaisBitmask & 0x01;
                var y1 = machines[mSelected].functions[0].relaisBitmask & 0x02;
                cbFRelais1_1.Checked = x1 == 0x01;
                cbFRelais2_1.Checked = y1 == 0x02;

                tbFName2.Text = machines[mSelected].functions[1].name;
                tbFDuration2.Text = machines[mSelected].functions[1].duration.ToString();
                var x2 = machines[mSelected].functions[1].relaisBitmask & 0x01;
                var y2 = machines[mSelected].functions[1].relaisBitmask & 0x02;
                cbFRelais1_2.Checked = x2 == 0x01;
                cbFRelais2_2.Checked = y2 == 0x02;

                tbFName3.Text = machines[mSelected].functions[2].name;
                tbFDuration3.Text = machines[mSelected].functions[2].duration.ToString();
                var x3 = machines[mSelected].functions[2].relaisBitmask & 0x01;
                var y3 = machines[mSelected].functions[2].relaisBitmask & 0x02;
                cbFRelais1_3.Checked = x3 == 0x01;
                cbFRelais2_3.Checked = y3 == 0x02;

                tbFName4.Text = machines[mSelected].functions[3].name;
                tbFDuration4.Text = machines[mSelected].functions[3].duration.ToString();
                var x4 = machines[mSelected].functions[3].relaisBitmask & 0x01;
                var y4 = machines[mSelected].functions[3].relaisBitmask & 0x02;
                cbFRelais1_4.Checked = x4 == 0x01;
                cbFRelais2_4.Checked = y4 == 0x02;

                tbFName5.Text = machines[mSelected].functions[4].name;
                tbFDuration5.Text = machines[mSelected].functions[4].duration.ToString();
                var x5 = machines[mSelected].functions[4].relaisBitmask & 0x01;
                var y5 = machines[mSelected].functions[4].relaisBitmask & 0x02;
                cbFRelais1_5.Checked = x5 == 0x01;
                cbFRelais2_5.Checked = y5 == 0x02;
            }
        }

        private void btMSave_Click(object sender, EventArgs e)
        {
            var m = new MachineData();
            if (mSelected >= 0)
                m = machines[mSelected];
            else
                m.id = uint.Parse(tbMNewId.Text, System.Globalization.NumberStyles.HexNumber);

            m.name = tbMName.Text;
            m.shortName = tbMShortName.Text;
            m.disabled = cbMDisabled.Checked;
            m.symbolX = uint.Parse(tbMachineX.Text);
            m.symbolY = uint.Parse(tbMachineY.Text);

            var status = RESTful("/machine?password=lwsc-remote&username=Admin&id=" + m.id + "&name=" + m.name + "&shortName=" + m.shortName + "&disabled=" + (m.disabled ? "1" : "0") + "&symbolX=" + m.symbolX + "&symbolY=" + m.symbolY + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
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

            var status = RESTful("/change_id?password=lwsc-remote&username=Admin&id=" + old_id + "&new_id=" + m.id, RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            GetData();
        }

        private void btDownloadConfig_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog
            {
                CheckPathExists = true,
                DefaultExt = "conf",
                Filter = "conf files (*.conf)|*.conf|All files (*.*)|*.*",
                InitialDirectory = Application.StartupPath
            };
            DialogResult dialogResult = sfd.ShowDialog();
            if (dialogResult != DialogResult.OK)
                return;
            using (var client = new WebClient())
            {
                client.DownloadFile("http://" + tbIpAddress.Text + "/file?password=lwsc-remote&username=Admin&filename=machines.conf", sfd.FileName);
            }
            FileInfo f = new FileInfo(Application.ExecutablePath);
            Process.Start(f.DirectoryName);
        }

        private void btUploadConfig_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog
            {
                CheckPathExists = true,
                DefaultExt = "conf",
                Filter = "conf files (*.conf)|*.conf|All files (*.*)|*.*",
                InitialDirectory = Application.StartupPath
            };
            DialogResult dialogResult = sfd.ShowDialog();
            if (dialogResult != DialogResult.OK)
                return;

            if (!File.Exists(sfd.FileName))
            {
                MessageBox.Show("Not found.");
                return;
            }
            dialogResult = MessageBox.Show("Are you sure?", "Override Config", MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;

            if (!Directory.Exists("tmp"))
                Directory.CreateDirectory("tmp");

            if (File.Exists("tmp/machines.conf"))
                File.Delete("tmp/machines.conf");

            File.Copy(sfd.FileName, "tmp/machines.conf");

            using (WebClient client = new WebClient())
            {
                client.UploadFile("http://" + tbIpAddress.Text + "/upload?password=lwsc-remote&username=Admin", "tmp/machines.conf");
            }


            var status = RESTful("/reboot?password=lwsc-remote&username=Admin&id=0", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }

            MessageBox.Show("Restarting Gateway....", "Restart Gateway", MessageBoxButtons.OK);

            GetData();
            GetFunctions();
        }

        private void btQueryRSSI_Click(object sender, EventArgs e)
        {
            if (!isEthernet)
            {
                DialogResult dialogResult = MessageBox.Show("WIFI will be disconnected for ~10secs, sure? (Maybe you need to manually reconnect to the wifi in Windows)", "Query RSSI", MessageBoxButtons.YesNo);
                if (dialogResult != DialogResult.Yes)
                    return;
            }
            var status = RESTful("/query_rssi?password=lwsc-remote&username=Admin", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
        }

        private void btGetRSSI_Click(object sender, EventArgs e)
        {
            foreach (var m in machines)
            {
                var status = RESTful("/machine_rssi?password=lwsc-remote&username=Admin&id=" + m.id, RESTType.GET, out string res);
                if (status != HttpStatusCode.OK)
                {
                    MessageBox.Show("Error: " + status);
                    return;
                }

                dynamic dJson = JsonConvert.DeserializeObject(res);
                m.rssi = (sbyte)dJson["rssi"];

                if (dJson["rssiMap"] == null)
                    continue;

                foreach(var r in dJson["rssiMap"])
                {
                    uint id = (uint)r["id"];
                    sbyte rssi  = (sbyte)r["rssi"];
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

        private void FillNewMapping(int x, int y, string loadMapping = "")
        {
            for (int i = 0; i < y + 1; i++)
            {
                if (i < y)
                {
                    for (int j = 0; j < x; j++)
                    {
                        var cb = new ComboBox
                        {
                            Location = new Point(pnMappings.Width / x * j, pnMappings.Height / (y + 1) * i),
                            Size = new Size(pnMappings.Width / x - 2, 20),
                            Tag = new int[] { j, i }
                        };
                        cb.Items.Add("");
                        cb.Items.AddRange(functions.Select(f => f.name).ToArray());
                        if(loadMapping != "" && i < mappings[loadMapping].GetLength(0) && j < mappings[loadMapping].GetLength(1))
                        {
                            var m = mappings[loadMapping];
                            if (m[i, j] != null)
                            {
                                var t = m[i, j].name;
                                cb.Text = t;
                            }
                        }
                        pnMappings.Controls.Add(cb);
                    }
                }
                else
                {
                    var lb = new Label
                    {
                        Text = "Name: ",
                        Size = new Size(40, 20),
                        Location = new Point(0, pnMappings.Height / (y + 1) * i + 4)
                    };
                    pnMappings.Controls.Add(lb);

                    var tb = new TextBox
                    {
                        Location = new Point(40, pnMappings.Height / (y + 1) * i),
                        Size = new Size(pnMappings.Width - 140, 20)
                    };
                    if (loadMapping != "")
                        tb.Text = loadMapping.Substring(4);
                    pnMappings.Controls.Add(tb);

                    var bt = new Button
                    {
                        Text = "Save",
                        Location = new Point(pnMappings.Width - 90, pnMappings.Height / (y + 1) * i),
                        Size = new Size(90, 20)
                    };
                    bt.Click += btFillNewMapping_Add_Click;
                    pnMappings.Controls.Add(bt);
                }
            }
        }

        private void btFillNewMapping_Add_Click(object sender, EventArgs e)
        {
            List<List<MachineFunction>> map = new List<List<MachineFunction>>();
            string name = "";

            foreach (var cb_ in pnMappings.Controls)
            {
                if (cb_.GetType() == typeof(TextBox))
                {
                    name = ((TextBox)cb_).Text;
                    if (name.Length == 0)
                    {
                        MessageBox.Show("Please name the mapping!");
                        return;
                    }
                }
                if (cb_.GetType() != typeof(ComboBox))
                    continue;

                ComboBox cb = (ComboBox)cb_;

                if (cb.Text == "")
                    continue;

                int[] tags = (int[])cb.Tag;
                int x = tags[0];
                int y = tags[1];

                while (y > map.Count() - 1)
                    map.Add(new List<MachineFunction>());
                while (x > map[y].Count() - 1)
                    map[y].Add(new MachineFunction());

                map[y][x] = functions.FirstOrDefault(fc => fc.name == cb.Text);
            }

            MachineFunction[,] resMap = new MachineFunction[map.Count(),map.Max(x => x.Count)];
            for (int y = 0; y < map.Count; y++)
            {
                for (int x = 0; x < map[y].Count; x++)
                {
                    if (map[y][x] != null && map[y][x].name != null && map[y][x].name != "")
                        resMap[y, x] = map[y][x];
                    else
                        resMap[y, x] = null;
                }
            }

            name = pnMappings.Tag.ToString() + "_" + name;

            if (mappings.ContainsKey(name))
            {
                DialogResult dialogResult = MessageBox.Show("Map-Name already exists. Override?", "Already exist", MessageBoxButtons.YesNo);
                if (dialogResult != DialogResult.Yes)
                    return;
            }

            mappings[name] = resMap;
            File.WriteAllText("mappings.json", JsonConvert.SerializeObject(mappings));

            using (WebClient client = new WebClient())
            {
                client.UploadFile("http://" + tbIpAddress.Text + "/upload?password=lwsc-remote&username=Admin", "mappings.json");
            }

            GetMappings();

        }

        private void btMapSelectNew_Click(object sender, EventArgs e)
        {
            pnMappings.Controls.Clear();
            pnMappings.Tag = "map";
            FillNewMapping(1, 10);
        }

        private void btMapPadNew_Click(object sender, EventArgs e)
        {
            pnMappings.Controls.Clear();
            pnMappings.Tag = "pad";
            FillNewMapping(10, 10);
        }

        private void btEspHome_Click(object sender, EventArgs e)
        {
        }

        private void btEspUp_Click(object sender, EventArgs e)
        {
        }

        private void btEspDown_Click(object sender, EventArgs e)
        {
        }

        private void btEspClick_Click(object sender, EventArgs e)
        {
        }

        private void btResetCounter_Click(object sender, EventArgs e)
        {
            foreach (var m in machines)
            {
                var status = RESTful("/set_relaiscounter?password=lwsc-remote&username=Admin&id=" + m.id + "&relais1Counter=0&relais2Counter=0", RESTType.POST, out _);
                if (status != HttpStatusCode.OK)
                {
                    MessageBox.Show("Error: " + status);
                    return;
                }
            }
        }

        private void btReboot_Click(object sender, EventArgs e)
        {
            foreach (var m in machines)
            {
                var status = RESTful("/reboot?password=lwsc-remote&username=Admin&id=" + m.id, RESTType.POST, out _);
                if (status != HttpStatusCode.OK)
                {
                    MessageBox.Show("Error: " + status);
                    return;
                }
            }
        }


        bool automatic = false;
        private void lbMapSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (automatic)
                return;
            automatic = true;
            lbMapPad.ClearSelected();
            automatic = false;
            if (lbMapSelect.SelectedItem == null)
                return;
            pnMappings.Controls.Clear();
            pnMappings.Tag = "map";
            FillNewMapping(1, 10, "map_" + lbMapSelect.SelectedItem.ToString());
        }

        private void lbMapPad_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (automatic)
                return;
            automatic = true;
            lbMapSelect.ClearSelected();
            automatic = false;
            if (lbMapPad.SelectedItem == null)
                return;
            pnMappings.Controls.Clear();
            pnMappings.Tag = "pad";
            FillNewMapping(10, 10, "pad_" + lbMapPad.SelectedItem.ToString());
        }

        private void btMapPadDelete_Click(object sender, EventArgs e)
        {
            if (lbMapPad.SelectedItem == null)
                return;
            DialogResult dialogResult = MessageBox.Show("Are you sure?", "Delete " + lbMapPad.SelectedItem.ToString(), MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;

            mappings.Remove("pad_" + lbMapPad.SelectedItem.ToString());

            File.WriteAllText("mappings.json", JsonConvert.SerializeObject(mappings));

            using (WebClient client = new WebClient())
            {
                client.UploadFile("http://" + tbIpAddress.Text + "/upload?password=lwsc-remote&username=Admin", "mappings.json");
            }

            GetMappings();
        }

        private void btMapSelectDelete_Click(object sender, EventArgs e)
        {
            if (lbMapSelect.SelectedItem == null)
                return;
            DialogResult dialogResult = MessageBox.Show("Are you sure?", "Delete " + lbMapSelect.SelectedItem.ToString(), MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;

            mappings.Remove("map_" + lbMapSelect.SelectedItem.ToString());

            File.WriteAllText("mappings.json", JsonConvert.SerializeObject(mappings));

            using (WebClient client = new WebClient())
            {
                client.UploadFile("http://" + tbIpAddress.Text + "/upload?password=lwsc-remote&username=Admin", "mappings.json");
            }

            GetMappings();
        }

        private void btResetCounterMachine_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var status = RESTful("/set_relaiscounter?password=lwsc-remote&username=Admin&id=" + m.id + "&relais1Counter=0&relais2Counter=0", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }

            GetData();
        }

        private void btDebug_Click(object sender, EventArgs e)
        {
            machines.Clear();
            mappings.Clear();
            functions.Clear();

            string all = File.ReadAllText(@"C:\Users\Graul\Desktop\tmp.json");

            tmpM[] old = JsonConvert.DeserializeObject<tmpM[]>(all);


            foreach (tmpM o in old)
            {
                if (machines.Count(x => x.id == o.m_id) == 0)
                    machines.Add(new MachineData() { id = o.m_id, name = o.name, shortName = o.name, symbolY = (uint)(machines.Count * 20 + 30), functions = new List<MachineFunction>() });

                var m = machines.FirstOrDefault(x => x.id == o.m_id);

                for (uint i = 0; i < 5; i++)
                {
                    if (m.functions.Count - 1 < i)
                        m.functions.Add(new MachineFunction());

                    var f = m.functions[(int)i];
                    if (f == null || f.name == null)
                    {
                        f = new MachineFunction()
                        {
                            name = o.name,
                            machineId = o.m_id,
                            functionId = i
                        };
                        switch (o.cmd)
                        {
                            case "0x01":
                            case "0x11":
                            case "0x21":
                                f.duration = 1000;
                                break;
                            case "0x03":
                            case "0x13":
                            case "0x23":
                                f.duration = 3000;
                                break;
                            case "0x04":
                            case "0x14":
                            case "0x24":
                                f.duration = 400;
                                break;
                            case "0x0a":
                            case "0x1a":
                            case "0x2a":
                                f.duration = -1;
                                break;
                            case "0x0b":
                            case "0x1b":
                            case "0x2b":
                                f.duration = 1;
                                break;
                        }
                        switch (o.cmd)
                        {
                            case "0x01":
                            case "0x03":
                            case "0x04":
                            case "0x0a":
                            case "0x0b":
                                f.relaisBitmask = 0x01;
                                break;
                            case "0x11":
                            case "0x13":
                            case "0x14":
                            case "0x1a":
                            case "0x1b":
                                f.relaisBitmask = 0x02;
                                break;
                            case "0x21":
                            case "0x23":
                            case "0x24":
                            case "0x2a":
                            case "0x2b":
                                f.relaisBitmask = 0x03;
                                break;
                        }
                        m.functions[(int)i] = f;
                        break;
                    }
                }
            }

            foreach (var m in machines)
            {
                var status = RESTful("/machine?password=lwsc-remote&username=Admin&id=" + m.id + "&name=" + m.name + "&shortName=" + m.shortName + "&disabled=" + (m.disabled ? "1" : "0") + "&symbolX=" + m.symbolX + "&symbolY=" + m.symbolY + "", RESTType.POST, out _);
                if (status != HttpStatusCode.OK)
                {
                    MessageBox.Show("Error: " + status);
                    return;
                }
                foreach(var f in m.functions)
                {
                    status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
                    if (status != HttpStatusCode.OK)
                    {
                        MessageBox.Show("Error: " + status);
                        return;
                    }
                }
            }


            GetData();
            GetFunctions();
        }

        private void btDelete_Click(object sender, EventArgs e)
        {
            DialogResult dialogResult = MessageBox.Show("Are you sure?", "Delete", MessageBoxButtons.YesNo);
            if (dialogResult != DialogResult.Yes)
                return;

            var m = machines[mSelected];
            var status = RESTful("/machine?password=lwsc-remote&username=Admin&id=" + m.id + "", RESTType.DELETE, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }

            GetData();
            GetFunctions();
        }


        private void btFSave1_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[0];

            f.name = tbFName1.Text;
            f.duration = int.Parse(tbFDuration1.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1_1.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2_1.Checked) f.relaisBitmask += 0x02;

            var status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            FillTreeView();
        }

        private void btFSave2_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[1];

            f.name = tbFName2.Text;
            f.duration = int.Parse(tbFDuration2.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1_2.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2_2.Checked) f.relaisBitmask += 0x02;

            var status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            FillTreeView();
        }

        private void btFSave3_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[2];

            f.name = tbFName3.Text;
            f.duration = int.Parse(tbFDuration3.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1_3.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2_3.Checked) f.relaisBitmask += 0x02;

            var status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            FillTreeView();
        }

        private void btFSave4_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[3];

            f.name = tbFName4.Text;
            f.duration = int.Parse(tbFDuration4.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1_4.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2_4.Checked) f.relaisBitmask += 0x02;

            var status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            FillTreeView();
        }

        private void btFSave5_Click(object sender, EventArgs e)
        {
            var m = machines[mSelected];
            var f = machines[mSelected].functions[4];

            f.name = tbFName5.Text;
            f.duration = int.Parse(tbFDuration5.Text);
            f.relaisBitmask = 0x00;
            if (cbFRelais1_5.Checked) f.relaisBitmask += 0x01;
            if (cbFRelais2_5.Checked) f.relaisBitmask += 0x02;

            var status = RESTful("/function?password=lwsc-remote&username=Admin&id=" + m.id + "&f_id=" + f.functionId + "&name=" + f.name + "&duration=" + f.duration + "&relaisBitmask=" + (int)f.relaisBitmask + "&symbolX=" + f.symbolX + "&symbolY=" + f.symbolY + "&rotation=" + (int)f.rotation + "", RESTType.POST, out _);
            if (status != HttpStatusCode.OK)
            {
                MessageBox.Show("Error: " + status);
                return;
            }
            FillTreeView();
        }

        private void btRefresh_Click(object sender, EventArgs e)
        {
            GetData();
            GetFunctions();
            GetMappings();
        }

        private void btDynDns_Click(object sender, EventArgs e)
        {
            tbIpAddress.Text = "lwsc.ddns.net:8280";
            AddToLog("lwsc.ddns.net:8280");

            GetData();
            GetFunctions();
            GetMappings();

            pnRSSIButtons.Enabled = true;
            AddToLog("Setup done");

            btEspClick.Visible = true;
            btEspHome.Visible = true;
            btEspUp.Visible = true;
            btEspDown.Visible = true;
            isEthernet = true;

        }
    }
}
