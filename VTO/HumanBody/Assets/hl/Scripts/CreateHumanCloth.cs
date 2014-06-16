using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class CreateHumanCloth : MonoBehaviour
{

    private Mesh mMesh = null;
    private Transform mTmpTransform;
    private Quaternion mRotation;
    private Vector3 mScale;
    private Vector3 mTranslation;

    [DllImport ("ClothSimulator")]    
    private static extern void subdivideMesh (int vertexCount, Vector3[] vertices, 
                                              int triangleCount, int[] triangles, 
                                              int subCount);
    [DllImport ("ClothSimulator")]
    private static extern int getSubdivisionVertexCnt ();

    [DllImport ("ClothSimulator")]
    private static extern int getSubdivisionTriangleCnt ();

    [DllImport ("ClothSimulator")]
    private static extern void getSubdivisionMesh (Vector3[] vertices, int[] triangles);

    [DllImport ("ClothSimulator")]
    private static extern void clearSubdivisionMesh ();

    public float translateStep = 0.1f;
    public bool saveObj = false;

    private bool isSubdivided = false;

    // Use this for initialization
    void Start ()
    {
        GameObject templateCloth = GameObject.Find ("tshirt");
        if (templateCloth != null) {
            mTmpTransform = templateCloth.transform;
            mRotation = mTmpTransform.localRotation;
            mScale = mTmpTransform.localScale;
            mTranslation = mTmpTransform.localPosition;
            MeshFilter tmpMesh = templateCloth.GetComponentInChildren<MeshFilter> ();
            if (tmpMesh != null) {
                mMesh = tmpMesh.mesh;             
                //createMesh ();
                subdivideMesh (4);
                updateMesh ();
            }
        }         
    }
	
    // Update is called once per frame
    void Update ()
    {
        if (mTmpTransform.localRotation != mRotation ||
            mTmpTransform.localScale != mScale) {
            mRotation = mTmpTransform.localRotation;
            mScale = mTmpTransform.localScale;
            updateMesh ();
        }
        
        bool isKeyDown = false;
        Vector3 tmpTrans = Vector3.zero;
        if (Input.GetKeyDown (KeyCode.A)) {
            tmpTrans.x += translateStep;
            mTranslation.x += translateStep;
            isKeyDown = true;
        }
        if (Input.GetKeyDown (KeyCode.D)) {
            tmpTrans.x -= translateStep;
            mTranslation.x -= translateStep;
            isKeyDown = true;
        }
        if (Input.GetKeyDown (KeyCode.W)) {
            tmpTrans.y += translateStep;
            mTranslation.y += translateStep;
            isKeyDown = true;
        }
        if (Input.GetKeyDown (KeyCode.S)) {
            tmpTrans.y -= translateStep;
            mTranslation.y -= translateStep;
            isKeyDown = true;
        }
        if (Input.GetKeyDown (KeyCode.Q)) {
            tmpTrans.z += translateStep;
            mTranslation.z += translateStep;
            isKeyDown = true;
        }
        if (Input.GetKeyDown (KeyCode.E)) {
            tmpTrans.z -= translateStep;
            mTranslation.z -= translateStep;
            isKeyDown = true;
        }
        if (isKeyDown) {
            MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
            if (thisMesh == null) {
                Debug.LogError ("No mesh filter");
            }
            Vector3[] vertices = thisMesh.mesh.vertices;
            for (int i = 0; i < vertices.Length; i++) {
                vertices [i] = vertices [i] + tmpTrans;              
            }
            thisMesh.mesh.vertices = vertices;
        }

        if (saveObj) {
            MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
            if (thisMesh != null) {
                ObjExporter.MeshToFile (thisMesh, "Cloth.obj");
            }
            saveObj = false;
        }
    }

    void onDestroy ()
    {
        if (isSubdivided) {
            clearSubdivisionMesh ();
        }
    }

    void createMesh ()
    {
        Mesh m = new Mesh ();
        ///顶点坐标
        Vector3[] vertices = new Vector3[mMesh.vertexCount];
        ///法线坐标
        Vector3[] normals = new Vector3[mMesh.vertexCount];
        ///UV坐标
        Vector2[] uv = new Vector2[mMesh.uv.Length];
        BoneWeight[] boneWeights = mMesh.boneWeights;
        for (int i = 0; i < vertices.Length; ++i) {
            //uv [i] = mMesh.uv [i];
            normals [i] = mMesh.normals [i];
            vertices [i] = mMesh.vertices [i];
        }
        ///三角形索引
//        int gap = 0;
//        int[] triIndex = FileOperator.LoadDataFromFile ("/Samples/index.txt", out gap);
//        int[] tri = new int[triIndex.Length * 3];
//        int index = 0;
//        for (int i = 0; i < tri.Length; i += 3, index++) {
//            int tmpIndex = triIndex [index] * 3;
//            for (int j = 0; j < 3; j++) {
//                tri [i + j] = mMesh.triangles [tmpIndex + j];
//            }
//        }
        int[] tri = new int[mMesh.triangles.Length];
        for (int i = 0; i < tri.Length; ++i) {
            tri [i] = mMesh.triangles [i];
        }
        
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

    void subdivideMesh (int subTime)
    {
        Mesh m = new Mesh ();
        subdivideMesh (mMesh.vertexCount, mMesh.vertices,
                       mMesh.triangles.Length / 3, mMesh.triangles,
                       subTime);
        ///顶点坐标
        Vector3[] vertices = new Vector3[getSubdivisionVertexCnt ()];
        ///法线坐标
        Vector3[] normals = new Vector3[getSubdivisionVertexCnt ()];
        ///三角形索引
        int[] tri = new int[getSubdivisionTriangleCnt () * 3];

        getSubdivisionMesh (vertices, tri);

//        int gap = 0;
//        int[] triIndex = FileOperator.LoadDataFromFile ("/Samples/index.txt", out gap);
//        int[] newtri = new int[triIndex.Length * 16 * 3];
//        int index = 0;
//        for (int i = 0; i < newtri.Length; i += 16 * 3, index++) {
//            int tmpIndex = triIndex [index] * 16 * 3;
//            for (int j = 0; j < 16 * 3; j++) {
//                newtri [i + j] = tri [tmpIndex + j];
//            }
//        }
//        m.triangles = newtri;

        m.vertices = vertices;
        m.normals = normals;
        m.triangles = tri;
        
        MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
        if (thisMesh == null) {
            Debug.LogError ("No mesh filter");
        }
        thisMesh.mesh = m;
        isSubdivided = true;
    }

    void updateMesh ()
    {
        MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
        if (thisMesh == null) {
            Debug.LogError ("No mesh filter");
        }
        Vector3[] vertices = thisMesh.mesh.vertices;
        for (int i = 0; i < vertices.Length; i++) {
            vertices [i] = mRotation * vertices [i];
            vertices [i].x = mScale.x * vertices [i].x + mTranslation.x;
            vertices [i].y = mScale.y * vertices [i].y + mTranslation.y;
            vertices [i].z = mScale.z * vertices [i].z + mTranslation.z;
        }
        thisMesh.mesh.vertices = vertices;
        thisMesh.mesh.RecalculateNormals ();
    }

    public Mesh recreateMesh ()
    {
        MeshFilter thisMesh = GetComponent<MeshFilter> () as MeshFilter;
        if (thisMesh == null) {
            Debug.LogError ("No mesh filter");
        }
        Vector3[] vertices = thisMesh.mesh.vertices;

        if (isSubdivided) {
            int[] tri = new int[getSubdivisionTriangleCnt () * 3];
            getSubdivisionMesh (vertices, tri);
            thisMesh.mesh.triangles = tri;
        } else {
            for (int i = 0; i < vertices.Length; ++i) {
                vertices [i] = mMesh.vertices [i];
            }
        }

        thisMesh.mesh.vertices = vertices;
        //move 
        //mTranslation.y -= 2;

        updateMesh ();
        return thisMesh.mesh;
    }
}
