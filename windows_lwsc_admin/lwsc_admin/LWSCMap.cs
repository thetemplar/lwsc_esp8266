using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace lwsc_admin
{
    public partial class LWSCMap : UserControl
    {
        int mouseMapped = -1;
        uint mouseMappedId = 0;

        public Action<int> LocationUpdate { get; internal set; }
        public Action<uint> Blink { get; internal set; }
        public Action<uint> ReqVersion { get; internal set; }
        public Action<uint, uint> Fire { get; internal set; }

        public LWSCMap()
        {
            InitializeComponent();
            this.DoubleBuffered = true;
        }

        public Point Midpoint(Point a, Point b) {
            return new Point((a.X + b.X) / 2, (a.Y + b.Y) / 2);
        }
        static Color GetColorFromRedYellowGreenGradient(double percentage)
        {
            percentage = Math.Min(percentage, 100);
            percentage = Math.Max(0, percentage);
            var red = (percentage > 50 ? 1 - 2 * (percentage - 50) / 100.0 : 1.0) * 255;
            var green = (percentage > 50 ? 1.0 : 2 * percentage / 100.0) * 255;
            var blue = 0.0;
            Color result = Color.FromArgb((int)red, (int)green, (int)blue);
            return result;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            var g = e.Graphics;
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

            g.DrawImage(lwsc_admin.Properties.Resources.lwsc, new Point(0, 0));

            for (int i = 0; i < Form1.machines.Count; i++)
            {
                Form1.MachineData m = Form1.machines[i];
                foreach (var r in m.rssiMap)
                {
                    Form1.MachineData md = Form1.machines.FirstOrDefault(x => x.id == r.Key);
                    if (mouseMappedId == 0 || m.id == mouseMappedId || md.id == mouseMappedId)
                    {
                        int offset = 0;
                        if (md.id < m.id)
                            offset = 1;
                        var a = new Point((int)m.symbolX + 8, (int)m.symbolY + 8);
                        var b = new Point((int)md.symbolX + 8, (int)md.symbolY + 8);
                        var dir = new PointF(a.X - b.X, a.Y - b.Y);
                        var dirL = Math.Sqrt(dir.X * dir.X + dir.Y * dir.Y);
                        dir = new PointF(((float)(dir.X / dirL) * 4) * offset, ((float)(dir.Y / dirL) * 4) * offset);
                        var mp = Midpoint(new Point((int)m.symbolX, (int)m.symbolY), new Point((int)md.symbolX, (int)md.symbolY));
                        g.DrawLine(new Pen(GetColorFromRedYellowGreenGradient(100 - ((r.Value + 70) / (-120.0 + 70) * 100))), new PointF(a.X - dir.Y, a.Y + dir.X), new PointF(b.X - dir.Y, b.Y + dir.X));
                        if (md.id < m.id)
                            g.DrawString(r.Value + "db", this.Font, Brushes.White, new Point(mp.X, mp.Y + 8));
                        else
                            g.DrawString(r.Value + "db", this.Font, Brushes.White, new Point(mp.X, mp.Y - 8));
                    }
                }
            }
            for (int i = 0; i < Form1.machines.Count; i++)
            {
                Form1.MachineData m = Form1.machines[i];
                if (m.minimized)
                {
                    g.FillRectangle(mouseMapped == i ? new SolidBrush(Color.FromArgb(140, Color.LightGray)) : new SolidBrush(Color.FromArgb(220, Color.White)), new RectangleF(mouseMapped == i ? _mouse.X : m.symbolX, mouseMapped == i ? _mouse.Y : m.symbolY, 16, 16));

                    g.DrawString("[+]", this.Font, Brushes.Black, new PointF(mouseMapped == i ? _mouse.X : m.symbolX, mouseMapped == i ? _mouse.Y : m.symbolY));
                    g.DrawString(m.ToString(), this.Font, Brushes.White, new PointF(mouseMapped == i ? _mouse.X + 20: m.symbolX + 20, mouseMapped == i ? _mouse.Y : m.symbolY));
                } 
                else
                {
                    g.FillRectangle(mouseMapped == i ? new SolidBrush(Color.FromArgb(140, Color.LightGray)) : new SolidBrush(Color.FromArgb(220, Color.White)), new RectangleF(mouseMapped == i ? _mouse.X : m.symbolX, mouseMapped == i ? _mouse.Y : m.symbolY, 200, 16 + 16 * m.FunctionCount()));

                    g.DrawString("[O] [B] [V]" + m.ToString(), this.Font, Brushes.Black, new PointF(mouseMapped == i ? _mouse.X : m.symbolX, mouseMapped == i ? _mouse.Y : m.symbolY));
                    g.DrawString("[-]", this.Font, Brushes.Black, new PointF(mouseMapped == i ? _mouse.X + 188 : m.symbolX + 188, mouseMapped == i ? _mouse.Y : m.symbolY));

                    int i1 = 0;
                    foreach (var f in m.functions)
                    {
                        if (f.relaisBitmask != 0)
                        {
                            i1++;
                            g.DrawString("[X] " + f.name, this.Font, Brushes.Black, new PointF(mouseMapped == i ? _mouse.X : m.symbolX, mouseMapped == i ? _mouse.Y + 16 * i1 : m.symbolY + 16 * i1));
                        }
                    }
                }
            }
        }

        PointF _mouse = new PointF();
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (mouseMapped != -1)
                Invalidate();
            _mouse = new PointF(e.X, e.Y);
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            if (mouseMapped != -1)
            {
                Form1.machines[mouseMapped].symbolX = (uint)e.Location.X;
                Form1.machines[mouseMapped].symbolY = (uint)e.Location.Y;
                LocationUpdate(mouseMapped);
                mouseMapped = -1;
                Invalidate();
                return;
            }
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            if(e.Button == MouseButtons.Right)
            {
                mouseMapped = -1;
                mouseMappedId = 0;
                Invalidate();
                return;
            }

            if (mouseMapped == -1)
            {
                for (int i = 0; i < Form1.machines.Count; i++)
                {
                    var m = Form1.machines[i];
                    if (!m.minimized)
                    {
                        if (e.Location.X > m.symbolX && e.Location.X < m.symbolX + 16 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                        {
                            mouseMapped = i;
                            return;
                        }
                        if (e.Location.X > m.symbolX + 18 && e.Location.X < m.symbolX + 16 + 18 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                        {
                            Blink(m.id);
                            return;
                        }
                        if (e.Location.X > m.symbolX + 36 && e.Location.X < m.symbolX + 16 + 36 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                        {
                            ReqVersion(m.id);
                            mouseMappedId = m.id;
                            Invalidate();
                            return;
                        }
                        if (e.Location.X > m.symbolX + 188 && e.Location.X < m.symbolX + 16 + 188 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                        {
                            m.minimized = !m.minimized;
                            Invalidate();
                            return;
                        }
                        int j = 0;
                        foreach (var f in m.functions)
                        {
                            j++;
                            if (f.relaisBitmask != 0)
                            {
                                if (e.Location.X > m.symbolX && e.Location.X < m.symbolX + 16 && e.Location.Y > m.symbolY + j * 16 && e.Location.Y < m.symbolY + 16 + j * 16)
                                {
                                    Fire(f.machineId, f.functionId);
                                    return;
                                }
                            }
                        }
                    } else
                    {
                        if (e.Location.X > m.symbolX && e.Location.X < m.symbolX + 16 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                        {
                            m.minimized = !m.minimized;
                            Invalidate();
                            return;
                        }
                    }
                }
            }

            mouseMapped = -1;
            mouseMappedId = 0;
            Invalidate();
        }
    }
}
