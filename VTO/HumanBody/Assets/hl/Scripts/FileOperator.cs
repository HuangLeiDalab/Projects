using UnityEngine;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;

public class FileOperator
{
    public static int[] LoadDataFromFile (string fileName, out int gap)
    {
        string absolutePath = Application.dataPath + fileName;
        ArrayList list = new ArrayList ();
        string[] info = File.ReadAllLines (absolutePath);
        if (info != null) {
            bool isGapGetted = false;
            gap = 0;
            foreach (string s in info) {
                string[] numbers = Regex.Split (s, @"\s+");
                int result;
                foreach (string num in numbers) {
                    if (int.TryParse (num, out result)) {
                        list.Add (result);
                        if (!isGapGetted) {
                            gap++;
                        }
                    }
                }
                isGapGetted = true;
            }
            return (int[])list.ToArray (typeof(int));
        } else {
            Debug.Log (fileName + "not found");
            gap = -1;
            return null;
        }
    }

    public static float[] LoadDataFromFile (string fileName)
    {
        string absolutePath = Application.dataPath + fileName;
        ArrayList list = new ArrayList ();
        string[] info = File.ReadAllLines (absolutePath);
        if (info != null) {
            foreach (string s in info) {
                string[] dataSet = Regex.Split (s, @"\s+");
                float result;
                foreach (string data in dataSet) {
                    if (float.TryParse (data, out result)) {
                        list.Add (result);
                    }
                }
            }
            return (float[])list.ToArray (typeof(float));
        } else {
            Debug.Log (fileName + "not found");
            return null;
        }
    }

    public static void SaveDataToFile (string fileName, string Data)
    {
        string absolutePath = Application.dataPath + "/Results/" + fileName;
        StreamWriter sw = File.CreateText (absolutePath);
        if (sw != null) {
            sw.Write (Data);
            sw.Close ();
        }
    }
}
