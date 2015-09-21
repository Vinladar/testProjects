using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Excel = Microsoft.Office.Interop.Excel;
using System.IO;
using Microsoft.Win32;

namespace CDAAutoFiller
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static Excel.Application mainApp = new Excel.Application();
        
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Multiselect = true;
            dlg.Filter = "CSV files (*.csv)|*.csv";
            dlg.InitialDirectory = @"C:\Users\Bryan\Desktop\Dell Testing";
            dlg.ShowDialog();

            foreach (string filename in dlg.FileNames)
            {
                listBox.Items.Add(filename);
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            char delimeter = '_';
            string[] tempString;
            DataEntry tempDataEntry = new DataEntry();
            Excel.Workbook TemplateWB;
            Excel.Worksheet TemplateWS;
            Excel.Range destrng;

            foreach (string filename in listBox.Items)
            {
                TemplateWB = mainApp.Workbooks.Open(@"C:\Users\Bryan\Desktop\Dell Testing\CDA_Tool_Dell 0820 (2).xlsx");
                TemplateWS = TemplateWB.Sheets[2];
                Excel.Worksheet calculatorWS = TemplateWB.Sheets["Calculator"];
                destrng = TemplateWS.get_Range("A1:F80");
            
                tempDataEntry.gamma = "2.2";
                string destPath = @"C:\Users\Bryan\Desktop\Dell Testing\temp\update\";
                string destPathAndFile = string.Concat(destPath, (string.Concat(System.IO.Path.GetFileNameWithoutExtension(filename), ".xlsx")));
                tempString = System.IO.Path.GetFileNameWithoutExtension(filename).Split(delimeter);
                tempDataEntry.fileName = filename;
                tempDataEntry.colorSpace = tempString[2];
                tempDataEntry.whitePoint = tempString[3];
                if (tempDataEntry.colorSpace.Equals("DCIP3"))
                {
                    tempDataEntry.gamma = "2.6";
                }
                else if (tempDataEntry.colorSpace.Equals("REC709"))
                {
                    tempDataEntry.gamma = "2.4";
                }
                Excel.Workbook srcWB = mainApp.Workbooks.Open(filename);
                Excel.Worksheet srcWS = srcWB.Sheets[1];
                Excel.Range srcrng = srcWS.get_Range("A1:F80");
                srcrng.Copy(destrng);
                TemplateWS = TemplateWB.Sheets[3];
                destrng = TemplateWS.get_Range("Q7");
                destrng.Value2 = tempDataEntry.gamma;
                destrng = TemplateWS.get_Range("Q9");
                if (tempDataEntry.colorSpace.Equals("AdRGB"))
                    destrng.Value2 = "Adobe RGB";
                else if (tempDataEntry.colorSpace.Equals("DCIP3"))
                    destrng.Value2 = "DCI-P3";
                else if (tempDataEntry.colorSpace.Equals("REC709"))
                    destrng.Value2 = "Rec. 709/HDTV";
                else if (tempDataEntry.colorSpace.Equals("REC601"))
                    destrng.Value2 = "Rec. 601/SMPTE-C";
                else if (tempDataEntry.colorSpace.Equals("FULL"))
                    destrng.Value2 = "xNative";
                else if (tempDataEntry.colorSpace.Equals("SRGB"))
                    destrng.Value2 = "sRGB";
                
                destrng = TemplateWS.get_Range("Q10");
                if (tempDataEntry.whitePoint.Equals("5000"))
                    destrng.Value2 = "D50 ";
                else if (tempDataEntry.whitePoint.Equals("6500"))
                    destrng.Value2 = "D65 ";
                else if (tempDataEntry.whitePoint.Equals("8200"))
                    destrng.Value2 = "8200K";
                else if (tempDataEntry.whitePoint.Equals("9300"))
                    destrng.Value2 = "9300K";
                else if (tempDataEntry.whitePoint.Equals("10000"))
                    destrng.Value2 = "10000K";

                
                srcWB.Close(false);
                TemplateWB.SaveAs(destPathAndFile);
                TemplateWB.Close(false);
            }
            
        }

        public class DataEntry
        {
            public string fileName { get; set; }
            public string colorSpace { get; set; }
            public string whitePoint { get; set; }
            public string gamma;
        }
    }
}
