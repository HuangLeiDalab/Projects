using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;

public class ClothSimulator : MonoBehaviour
{
    public bool SaveIndex = false;
    public int MaxIndex = 17;

    private Mesh bodyMesh = null;
    private Mesh clothMesh = null;
    private Vector3[] bodyVertices = null;
    private Vector3[] clothVertices = null;
    private int[] clothOuterTriangles = null;
    private bool start = false;
    private int featureCnt = 0;
    private static int simulateTime = 12;

    private ControlPlane planes;
    private CreateHumanCloth tmpClothCreater;
    private int[] featureTriIndex;
    private int[] featureTri;

    /// <summary>
    /// to delete
    /// </summary>
    public int testIndex = 0;

    [DllImport ("ClothSimulator")]    
    private static extern bool initHashMesh (int vertexCount, Vector3[] vertices, Vector3[] normals, 
                                             int triangleCount, int[] triangles,
                                             float scale);

    [DllImport ("ClothSimulator")] 
    private static extern void updateMeshSpatial (Vector3[] vertices, Vector3[] normals, int[] triangles);

    [DllImport ("ClothSimulator")] 
    private static extern void clearMesh ();

    [DllImport ("ClothSimulator")] 
    private static extern bool initCloth (int vertexCount, Vector3[] vertices, int triangleCount, int[] triangles,
                                float edgeKs, float edgeKd, float bendKs, float bendKd);

    [DllImport ("ClothSimulator")] 
    private static extern void simulateCloth (float deltaT);

    [DllImport ("ClothSimulator")] 
    private static extern void getOuterTriangles (int[] dstTriangles);

    [DllImport ("ClothSimulator")] 
    private static extern void clearCloth ();

    // Use this for initialization
    void Start ()
    {
        planes = new ControlPlane ();
        // Register Global Message 1
        GlobalSignal.GlobalMsg1 = MessageManager.CreatMessage ();
        MessageManager.RegisterHander (GlobalSignal.GlobalMsg1, this.simulateAndSave);
        // Register Global Message 2
        GlobalSignal.GlobalMsg2 = MessageManager.CreatMessage ();
        MessageManager.RegisterHander (GlobalSignal.GlobalMsg2, this.restartSimulation);

        //GlobalSignal.LoadDataToTable ("/Results/iso", "txt", 0);
        //GlobalSignal.SaveTableToFile ("testCorse.txt", 0);
    }
	
    // Update is called once per frame
    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.I)) {
            init ();
        }
        if (Input.GetKeyDown (KeyCode.G)) {
            start = true;
        }

        if (start) {
            //clothMesh.vertices = clothVertices;
            //clothMesh.RecalculateNormals ();
        }

        if (SaveIndex) {
            start = false;
            SaveIndex = false;
            //saveTriangleIndexToFile ("index.txt");
            //saveFeatureVertexIndexToFile ("vIndex.txt");
            testIndex = testIndex > MaxIndex ? MaxIndex : testIndex;
            testIndex = testIndex < 0 ? 0 : testIndex;
            string s = "/Results/iso/Feature_s" + testIndex.ToString () + ".txt";
            loadFeatureVertex ("/Samples/vIndex.txt", s);
        }
    }

    void FixedUpdate ()
    {
        if (start) {
            updateMeshSpatial (bodyMesh.vertices, bodyMesh.normals, bodyMesh.triangles);
            simulateCloth (0.2f);
        }
    }

    void onDestroy ()
    {
        clearMesh ();
        clearCloth ();
    }

    private void init ()
    {
        GameObject body = GameObject.Find ("HumanBody");
        if (body != null) {
            MeshFilter bodyMeshFilter = body.GetComponent<MeshFilter> ();
            bodyMesh = bodyMeshFilter.mesh;
            bodyVertices = bodyMesh.vertices;
        }
        GameObject cloth = GameObject.Find ("HumanCloth");
        if (cloth != null) {
            tmpClothCreater = cloth.GetComponent<CreateHumanCloth> ();
            MeshFilter clothMeshFilter = cloth.GetComponent<MeshFilter> ();
            clothMesh = clothMeshFilter.mesh;
            clothVertices = clothMesh.vertices;
            clothOuterTriangles = new int[clothMesh.triangles.Length];
            // Init FeatureTriangleIndex to be saved
            int gap = 0;
            /// times = 1 for corse version, times = 16 for iso version
            int times = 16;
            featureTriIndex = FileOperator.LoadDataFromFile ("/Samples/index.txt", out gap);
            featureTri = new int[featureTriIndex.Length * times * 3];
            int index = 0;
            for (int i = 0; i < featureTri.Length; i += 3 * times, index++) {
                int tmpIndex = featureTriIndex [index] * times * 3;
                for (int j = 0; j < 3 * times; j++) {
                    featureTri [i + j] = clothMesh.triangles [tmpIndex + j];
                }
            }
        }
        initHashMesh (bodyMesh.vertexCount, bodyVertices, bodyMesh.normals,
                      bodyMesh.triangles.Length / 3, bodyMesh.triangles, 2.0f);
        initCloth (clothMesh.vertexCount, clothVertices, 
                   clothMesh.triangles.Length / 3, clothMesh.triangles,
                   0.95f, -0.25f, 0.25f, -0.25f);
        getOuterTriangles (clothOuterTriangles);
    }

    private void simulateAndSave ()
    {
        Invoke ("saveFeature", simulateTime);
    }

    private void saveFeature ()
    {
        start = false;
        //string filename = "corse/Feature_c" + featureCnt.ToString () + ".txt";
        string filename = "iso/Feature_s" + featureCnt.ToString () + ".txt";
        saveFeatureToFile (filename);
        featureCnt++;
        start = true;
        // Notify humanbody to be updated
        MessageManager.SendMessage (GlobalSignal.GlobalMsg0);
    }

    private void restartSimulation ()
    {
        start = false;
        clothMesh = tmpClothCreater.recreateMesh ();
        clothVertices = clothMesh.vertices;

        initHashMesh (bodyMesh.vertexCount, bodyVertices, bodyMesh.normals,
                      bodyMesh.triangles.Length / 3, bodyMesh.triangles, 2.0f);
        initCloth (clothMesh.vertexCount, clothVertices, 
                   clothMesh.triangles.Length / 3, clothMesh.triangles,
                   0.95f, -0.25f, 0.25f, -0.25f);
        getOuterTriangles (clothOuterTriangles);
        start = true;
        Invoke ("saveFeature", simulateTime);
    }

    private void saveFeatureToFile (string fileName)
    {
        int[] tri = featureTri;
        //Vector3[] vertices = clothMesh.vertices;
        Vector3[] vertices = clothVertices;
        string data = "";
        int index = 0;
        int times = 16;
        Dictionary<int, int> d = new Dictionary<int, int> ();
        for (int i = 0; i < tri.Length; i += times * 3, index++) {
            d.Clear ();
            for (int j = 0; j < times * 3; j++) {
                if (d.ContainsKey (tri [i + j])) {
                    continue;
                }
                d.Add (tri [i + j], 0);
                data += vertices [tri [i + j]].x.ToString () + " " +
                    vertices [tri [i + j]].y.ToString () + " " +
                    vertices [tri [i + j]].z.ToString () + " ";
            }
            data += Environment.NewLine;
//            int outerIndex = featureTriIndex [index] * 3;
//            if (clothOuterTriangles [outerIndex] == -1) {
//                data += "0 0 0 ";
//            } else {
//                data += vertices [clothOuterTriangles [outerIndex]].x.ToString () + " " + 
//                    vertices [clothOuterTriangles [outerIndex]].y.ToString () + " " + 
//                    vertices [clothOuterTriangles [outerIndex]].z.ToString () + " ";
//            }
//
//            if (clothOuterTriangles [outerIndex + 1] == -1) {
//                data += "0 0 0 ";
//            } else {
//                data += vertices [clothOuterTriangles [outerIndex + 1]].x.ToString () + " " + 
//                    vertices [clothOuterTriangles [outerIndex + 1]].y.ToString () + " " + 
//                    vertices [clothOuterTriangles [outerIndex + 1]].z.ToString () + " ";
//            }
//
//            if (clothOuterTriangles [outerIndex + 2] == -1) {
//                data += "0 0 0" + Environment.NewLine;
//            } else {
//                data += vertices [clothOuterTriangles [outerIndex + 2]].x.ToString () + " " + 
//                    vertices [clothOuterTriangles [outerIndex + 2]].y.ToString () + " " + 
//                    vertices [clothOuterTriangles [i + 2]].z.ToString () + Environment.NewLine;
//            }
        }
        FileOperator.SaveDataToFile (fileName, data);
    }

    private void saveTriangleIndexToFile (string fileName)
    {
        float[] shoulderL = planes.getControlPlane ("shoulderLeft");
        float[] shoulderR = planes.getControlPlane ("shoulderRight");
        //float[] back = planes.getControlPlane ("back");

        int[] tri = clothMesh.triangles;
        //Vector3[] vertices = clothMesh.vertices;
        Vector3[] vertices = clothVertices;
        string data = "";
        int index = 0;
        for (int i = 0; i < tri.Length; i += 3, index++) {
            for (int j = 0; j < 3; j++) {
                if (!AuxMath.isAbovePlane (vertices [tri [i + j]], shoulderL) && 
                    !AuxMath.isAbovePlane (vertices [tri [i + j]], shoulderR)
                    ) {
                    data += index.ToString () + " ";
                    break;
                }
            }
        }
        FileOperator.SaveDataToFile (fileName, data);
    }

    private void saveFeatureVertexIndexToFile (string fileName)
    {
        int[] tri = featureTri;
        int times = 16;
        string vIndex = "";
        Dictionary<int, int> d = new Dictionary<int, int> ();
        for (int i = 0; i < tri.Length; i += times * 3) {
            d.Clear ();
            for (int j = 0; j < times * 3; j++) {
                if (d.ContainsKey (tri [i + j])) {
                    continue;
                }
                d.Add (tri [i + j], 0);
                vIndex += tri [i + j].ToString () + " ";     
            }
        }
        FileOperator.SaveDataToFile (fileName, vIndex);
    }

    private void loadFeatureVertex (string index, string data)
    {
        int gap = 0;
        int[] vIndex = FileOperator.LoadDataFromFile (index, out gap);
        float[] vData = FileOperator.LoadDataFromFile (data);

        if (vData.Length != vIndex.Length * 3) {
            Debug.LogError ("Wrong index and data paris: data length " + vData.Length.ToString () +
                " index length " + vIndex.Length.ToString ());
        } else {
            Dictionary<int, int> cntDic = new Dictionary<int, int> ();
            Dictionary<int, Vector3> sumDic = new Dictionary<int, Vector3> ();

            Vector3[] vertices = clothMesh.vertices;
            for (int i = 0; i < vIndex.Length; ++i) {
                if (cntDic.ContainsKey (vIndex [i])) {
                    sumDic [vIndex [i]] += new Vector3 (vData [i * 3],
                                                     vData [i * 3 + 1],
                                                     vData [i * 3 + 2]);
                    cntDic [vIndex [i]]++;
                } else {
                    sumDic [vIndex [i]] = new Vector3 (vData [i * 3],
                                                     vData [i * 3 + 1],
                                                     vData [i * 3 + 2]);
                    cntDic [vIndex [i]] = 1;
                }
            }
 
            foreach (KeyValuePair<int, int> kvp in cntDic) {
                vertices [kvp.Key] = sumDic [kvp.Key] / (float)kvp.Value;
            }

            clothMesh.vertices = vertices;
            clothMesh.RecalculateNormals ();
        }
    }


    private int getSubdidedVertexCnt (int subdivided)
    {
        if (subdivided < 0)
            return 0;
        if (subdivided == 0)
            return 1;
        return 3 * subdivided + getSubdidedVertexCnt (subdivided - 3);
    }
}
