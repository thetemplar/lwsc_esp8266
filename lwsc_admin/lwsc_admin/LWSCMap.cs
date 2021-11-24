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

        public Action<int> LocationUpdate { get; internal set; }
        public Action<uint> Blink { get; internal set; }
        public Action<uint, uint> Fire { get; internal set; }

        public LWSCMap()
        {
            InitializeComponent();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            var g = e.Graphics;

            for (int i = 0; i < Form1.machines.Count; i++)
            {
                Form1.MachineData m = Form1.machines[i];
                g.FillRectangle(mouseMapped == i ? Brushes.LightGray : Brushes.Gray, new RectangleF(m.symbolX, m.symbolY, 200, 16 + 16 * m.FunctionCount()));

                g.DrawString("[O] [B] " + m.GetName(), this.Font, Brushes.Black, new Point((int)m.symbolX, (int)m.symbolY));

                int i1 = 0;
                foreach (var f in m.functions)
                {
                    if (f.relaisBitmask != 0)
                    {
                        i1++;
                        g.DrawString("[X] " + f.name, this.Font, Brushes.Black, new Point((int)m.symbolX, (int)m.symbolY + 16 * i1));                    }
                }
            }
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            if (mouseMapped == -1)
            {
                for (int i = 0; i < Form1.machines.Count; i++)
                {
                    var m = Form1.machines[i];
                    if (e.Location.X > m.symbolX && e.Location.X < m.symbolX + 16 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                    {
                        mouseMapped = i;
                        Invalidate();
                        return;
                    }
                    if (e.Location.X > m.symbolX + 20 && e.Location.X < m.symbolX + 16 + 20 && e.Location.Y > m.symbolY && e.Location.Y < m.symbolY + 16)
                    {
                        Blink(m.id);
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
                            }
                        }
                    }
                }
            } else
            {
                Form1.machines[mouseMapped].symbolX = (uint)e.Location.X;
                Form1.machines[mouseMapped].symbolY = (uint)e.Location.Y;
                LocationUpdate(mouseMapped);
                mouseMapped = -1;
                Invalidate();
            }
        }
    }
}
