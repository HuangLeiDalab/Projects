using UnityEngine;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System;

public class GlobalSignal
{
    static public int GlobalMsg0 = 0;
    static public int GlobalMsg1 = 0;
    static public int GlobalMsg2 = 0;


    static private int tableSize = 2000;
    static public ArrayList[] CorseTable = new ArrayList[tableSize];
    static public ArrayList[] IsoTable = new ArrayList[tableSize];


    static private void initTables ()
    {
        for (int i = 0; i < tableSize; ++i) {
            if (CorseTable [i] == null) {
                CorseTable [i] = new ArrayList ();
            } else {
                CorseTable [i].Clear ();
            }
            if (IsoTable [i] == null) {
                IsoTable [i] = new ArrayList ();
            } else {
                IsoTable [i].Clear ();
            }
        }
    }
    static public void LoadDataToTable (string dirName, string suffix, int type)
    {
        try {
            string absolutePath = Application.dataPath + dirName;
            DirectoryInfo dirInfo = new DirectoryInfo (absolutePath);
            FileInfo[] fileInfo = dirInfo.GetFiles ();
            ArrayList[] dstTable = (type == 0) ? CorseTable : IsoTable;
            initTables ();
            foreach (FileInfo info in fileInfo) {
                string infoSuffix = info.Name.Substring (info.Name.LastIndexOf ('.') + 1);
                if (suffix.CompareTo (infoSuffix) == 0) {
                    int endIndex = 0;
                    string[] dataLine = File.ReadAllLines (absolutePath + "/" + info.Name);
                    foreach (string s in dataLine) {
                        string[] dataSet = Regex.Split (s, @"\s+");
                        float result;
                        bool validData = false;
                        foreach (string data in dataSet) {
                            if (float.TryParse (data, out result)) {
                                dstTable [endIndex].Add (result);
                                validData = true;
                            }
                        }
                        if (validData)
                            endIndex++;
                    }
                }           
            }
        } catch (DirectoryNotFoundException e) {
            Debug.LogError (dirName + " is not Found");
        }
    }

    static public void SaveTableToFile (string fileName, int type)
    {
        string data = "";
        ArrayList[] dstTable = type == 0 ? CorseTable : IsoTable;
        for (int i = 0; i < tableSize; ++i) {
            if (dstTable [i].Count == 0) {
                break;
            }
            for (int j = 0; j < dstTable[i].Count; ++j) {
                data += dstTable [i] [j].ToString () + " ";
            }
            data += Environment.NewLine;
        }
        FileOperator.SaveDataToFile (fileName, data);
    }
}
