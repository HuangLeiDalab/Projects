using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Threading;

public class CreateHumanBody: MonoBehaviour
{

    private Transform mRoot;
    private Mesh mMesh = null;
    private bool mIsCreate = false;

    public float height;   
    public float crotchHeight;
    public float kneeHeight;
    public float forearmLength;
    public float armLength;
    public float shoulderWidth;
    public float bustGrith;
    public float waistGrith;
    public float hipGrith;   
    public float thighGrith;   
    public float calfGrith;

    private float preHeight;
    private float preCrotchHeight;
    private float preKneeHeight;
    private float preForearmLength;
    private float preArmLength;
    private float preShoulderWidth;
    private float preBustGrith;
    private float preWaistGrith;
    private float preHipGrith;   
    private float preThighGrith;   
    private float preCalfGrith;

    public bool reset = false;
    public bool SaveObj = false;
    public HumanBone boneType = HumanBone.Undefined;

    private SizeAvatar sizeTool = null;
    private HumanBody tmpBody = null;

    private int[] humanData = null;
    private int humanDataGap;
    private int humanDataIndex = 0;
    // Use this for initialization
    void Start ()
    {
        GameObject templateBody = GameObject.Find ("template");
        if (templateBody != null) {
            mRoot = templateBody.transform;
            SkinnedMeshRenderer skinMesh = templateBody.GetComponentInChildren<SkinnedMeshRenderer> ();
            if (skinMesh != null) {
                mMesh = skinMesh.sharedMesh;
                tmpBody = new HumanBody (skinMesh.sharedMesh, skinMesh.bones, templateBody.transform);
            }
        }     
        if (mMesh != null) {
            createMesh ();
            mIsCreate = true;
        }
        MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
        if (thisMesh == null) {
            Debug.LogError ("No mesh filter");
        }
        GameObject templateBodyLen = GameObject.Find ("templateLen");
        if (templateBodyLen != null) {
            MeshFilter tmpLenMesh = templateBodyLen.GetComponentInChildren<MeshFilter> ();
            if (tmpLenMesh != null) {
                sizeTool = new SizeAvatar (thisMesh.mesh, tmpLenMesh.mesh, tmpBody);
            }
//            SkinnedMeshRenderer tmpLenMesh = templateBodyLen.GetComponentInChildren<SkinnedMeshRenderer> ();
//            if (tmpLenMesh != null) {
//                sizeTool = new SizeAvatar (thisMesh.mesh, tmpLenMesh.sharedMesh, tmpBody);
//            }
        }   
        initMeasurements ();
        reset = true;

        //Register Global Msg0
        GlobalSignal.GlobalMsg0 = MessageManager.CreatMessage ();
        MessageManager.RegisterHander (GlobalSignal.GlobalMsg0, this.updateFromFile);
    }
    
    // Update is called once per frame
    void Update ()
    {
        if (reset) {
            resetAvatar ();
        } else {               
            if (checkAndUpdate ()) {
                sizeAvatar ();
            }
        }
        if (Input.GetKeyDown (KeyCode.Space) && !IsInvoking ("updateFromFile")) {
            //InvokeRepeating ("updateFromFile", 5, 30);
            Invoke ("updateFromFile", 5);
        }
        if (SaveObj) {
            MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
            ObjExporter.MeshToFile (thisMesh, "Human2.obj");
            SaveObj = false;
        }
//        if (tmpBody != null) {
//
//            if (tmpBody.vertexIndexsPerBone.ContainsKey (boneType)) {
//                MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
//                if (thisMesh == null) {
//                    Debug.LogError ("No mesh filter");
//                    return;
//                }
//                Color[] colors = thisMesh.mesh.colors;
//                ArrayList indexList = tmpBody.vertexIndexsPerBone [boneType];
//                for (int i = 0; i < indexList.Count; ++i) {
//                    colors [(int)indexList [i]] = 
//                        Color.Lerp (Color.red, Color.green, 10);//vertices[i].y);
//                }
//                thisMesh.mesh.colors = colors;
//            }
//        }
    }

    void sizeAvatar ()
    {
        if (sizeTool != null) {
            sizeTool.sizeLength (height, shoulderWidth, crotchHeight, kneeHeight, forearmLength, armLength);
            sizeTool.sizeGrith (bustGrith, waistGrith, hipGrith, thighGrith, calfGrith);
            mMesh.RecalculateNormals ();
        }
    }

    void resetAvatar ()
    {
        if (sizeTool != null) {
            sizeTool.resetAvatar (HumanBone.Undefined);
            initMeasurements ();
            reset = false;
        }
    }

    bool checkAndUpdate ()
    {
        bool rst = false;
        if (preHeight != height) {
            preHeight = height;
            rst = true;
        }
        if (preShoulderWidth != shoulderWidth) {
            preShoulderWidth = shoulderWidth;
            rst = true;
        }
        if (preCrotchHeight != crotchHeight) {
            preCrotchHeight = crotchHeight;
            rst = true;
        }
        if (preKneeHeight != kneeHeight) {
            preKneeHeight = kneeHeight;
            rst = true;
        }
        if (preForearmLength != forearmLength) {
            preForearmLength = forearmLength;
            rst = true;
        }
        if (preArmLength != armLength) {
            preArmLength = armLength;
            rst = true;
        }
        if (preBustGrith != bustGrith) {
            preBustGrith = bustGrith;
            rst = true;
        }
        if (preWaistGrith != waistGrith) {
            preWaistGrith = waistGrith;
            rst = true;
        }
        if (preHipGrith != hipGrith) {
            preHipGrith = hipGrith;
            rst = true;
        }
        if (preThighGrith != thighGrith) {
            preThighGrith = thighGrith;
            rst = true;
        }
        if (preCalfGrith != calfGrith) {
            preCalfGrith = calfGrith;
            rst = true;
        }
        return rst;
    }

    void createMesh ()
    {
        Mesh m = new Mesh ();
        ///顶点坐标
        Vector3[] vertices = new Vector3[mMesh.vertexCount];
        ///法线坐标
        Vector3[] normals = new Vector3[mMesh.vertexCount];
        ///UV坐标
        Vector2[] uv = new Vector2[mMesh.vertexCount];
        ///Colors
        //Color[] colors = new Color[mMesh.vertexCount];
        for (int i = 0; i < vertices.Length; ++i) {
            uv [i] = mMesh.uv [i];
            normals [i] = mMesh.normals [i];
            vertices [i] = mMesh.vertices [i];
            //colors [i] = Color.red;
        }
        ///三角形索引
        int[] tri = new int[mMesh.triangles.Length];
        for (int i = 0; i < tri.Length; ++i) {
            tri [i] = mMesh.triangles [i];
        }
        //ColorCnt = mMesh.colors.Length;
        m.vertices = vertices;
        m.normals = normals;
        m.uv = uv;
        m.triangles = tri;
        
        MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
        if (thisMesh == null) {
            Debug.LogError ("No mesh filter");
        }
        thisMesh.mesh = m;
    }

    void initMeasurements ()
    {
        if (sizeTool != null) {
            height = sizeTool.getInitMeasurement (HumanSize.Height);
            crotchHeight = sizeTool.getInitMeasurement (HumanSize.Crotch_Height);
            kneeHeight = sizeTool.getInitMeasurement (HumanSize.Knee_Height);
            forearmLength = sizeTool.getInitMeasurement (HumanSize.Forearm_Length);
            armLength = sizeTool.getInitMeasurement (HumanSize.Arm_Length);
            shoulderWidth = sizeTool.getInitMeasurement (HumanSize.Shoulder_Width);
            bustGrith = sizeTool.getInitMeasurement (HumanSize.Bust_Grith);
            waistGrith = sizeTool.getInitMeasurement (HumanSize.Waist_Grith);
            hipGrith = sizeTool.getInitMeasurement (HumanSize.Hip_Grith);
            thighGrith = sizeTool.getInitMeasurement (HumanSize.Thigh_Grith);
            calfGrith = sizeTool.getInitMeasurement (HumanSize.Calf_Grith);

            preHeight = height;
            preCrotchHeight = crotchHeight;
            preKneeHeight = kneeHeight;
            preForearmLength = forearmLength;
            preArmLength = armLength;
            preShoulderWidth = shoulderWidth;
            preBustGrith = bustGrith;
            preWaistGrith = waistGrith;
            preHipGrith = hipGrith;
            preThighGrith = thighGrith;
            preCalfGrith = calfGrith;
        }
    }

    void updateFromFile ()
    {
        if (humanData == null) {          
            humanData = FileOperator.LoadDataFromFile ("/Samples/train.txt", out humanDataGap);
        }
        if (humanDataIndex + humanDataGap <= humanData.Length) {          
            height = humanData [humanDataIndex];
            crotchHeight = humanData [humanDataIndex + 1];
            kneeHeight = humanData [humanDataIndex + 2];
            forearmLength = humanData [humanDataIndex + 4];
            armLength = humanData [humanDataIndex + 3];
            shoulderWidth = humanData [humanDataIndex + 5];
            bustGrith = humanData [humanDataIndex + 6];
            waistGrith = humanData [humanDataIndex + 7];
            hipGrith = humanData [humanDataIndex + 8];
            thighGrith = humanData [humanDataIndex + 9];
            calfGrith = humanData [humanDataIndex + 10];
            humanDataIndex += humanDataGap;

            bool isHeightChanged = (Mathf.Abs (height - preHeight) < 1e-3) ? false : true;
            if (!isHeightChanged) {
                //MessageManager.SendMessage (GlobalSignal.GlobalMsg1);
                MessageManager.SendMessage (GlobalSignal.GlobalMsg2);
            } else {
                MessageManager.SendMessage (GlobalSignal.GlobalMsg2);
            }

        }
    }
}
