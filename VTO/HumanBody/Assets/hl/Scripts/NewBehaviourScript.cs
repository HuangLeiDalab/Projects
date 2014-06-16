using UnityEngine;
using System.Collections;

public class NewBehaviourScript : MonoBehaviour
{
    private HumanBody body = null;
    private HumanBody tmpBody = null;
    private HumanBody tmpBodyLen = null;

    public GameObject obj;
    public bool reset = false;
    public HumanBone regionNumber = HumanBone.Pelvis;

    private Vector3[] localCoords = null;
    public float globalScale;
    public float globalScale2;
    public float globalScale3;
    public Vector3 objPos;
    private LineRenderer lineRender;
    private Bezier bezier;
    private Vector3[] bezierCPS;
	
    private Vector3[] objsPosition = null;

    public SizeAvatar sizeTool = null;
    public float radius1;
    public float radius2;
    private int i = 0;
    // Use this for initialization
    void Start ()
    {
        GameObject templateBody = GameObject.Find ("template");
        if (templateBody != null) {
            SkinnedMeshRenderer skinMesh = templateBody.GetComponentInChildren<SkinnedMeshRenderer> ();
            if (skinMesh != null) {
                tmpBody = new HumanBody (skinMesh.sharedMesh, skinMesh.bones, templateBody.transform);
            }
        }
        GameObject templateBodyLen = GameObject.Find ("templateLen");
        if (templateBodyLen != null) {
            SkinnedMeshRenderer skinMesh = templateBodyLen.GetComponentInChildren<SkinnedMeshRenderer> ();
            if (skinMesh != null) {
                tmpBodyLen = new HumanBody (skinMesh.sharedMesh, skinMesh.bones, templateBodyLen.transform);
            }
        }

        SkinnedMeshRenderer render = GetComponent<SkinnedMeshRenderer> ();
        //body = new HumanBody (render.sharedMesh, render.bones);
        //sizeTool = new SizeAvatar (body.getMesh (), tmpBody, tmpBodyLen);

        Mesh mesh = render.sharedMesh;
        ArrayList calfList = body.vertexIndexsPerBone [HumanBone.R_Thigh];
        localCoords = body.getLocalCoordinate (regionNumber);
        objsPosition = new Vector3[calfList.Count];

        for (int i = 0; i < calfList.Count; i++) {
            objsPosition [i].x = mesh.vertices [(int)calfList [i]].x;
            objsPosition [i].y = mesh.vertices [(int)calfList [i]].y;
            objsPosition [i].z = mesh.vertices [(int)calfList [i]].z;
            GameObject o = (GameObject)Instantiate (obj);
            o.name = calfList [i].ToString ();
            o.transform.position = objsPosition [i];
            o.transform.parent = gameObject.transform;
            o.transform.localScale = gameObject.transform.parent.localScale;
			 
        }
      
        bezierCPS = new Vector3[4];
        bezier = new Bezier (bezierCPS);
        for (int i = 0; i < 4; ++i) {
            GameObject o = (GameObject)Instantiate (obj);
            o.name = "bezier" + i.ToString ();
            // o.transform.parent = gameObject.transform;
            //o.transform.localPosition = ControlPoints.wasitBezierControlPoints [i];
        }
        lineRender = GetComponent<LineRenderer> ();
        lineRender.SetVertexCount (51);
        globalScale = 176.0f;
        globalScale2 = 97.0f;
        globalScale3 = 35.0f;

    }

    // Update is called once per frame
    void Update ()
    {
        if (sizeTool != null) {
            if (reset) {
                sizeTool.resetAvatar (HumanBone.Undefined);
                //sizeTool.resetAvatar(HumanBone.R_Thigh);
                reset = false;
                globalScale = 176.0f;
                globalScale2 = 97.0f;
                globalScale3 = 35.0f;
            } else {
                //sizeTool.scaleAvatar (HumanSize.Knee_Height, globalScale);
                //sizeTool.scaleAvatar (HumanSize.Thigh_Grith, globalScale2);
                //sizeTool.sizeLength (globalScale, 0.0f, globalScale2, globalScale3, 0.0f, 0.0f);
            }
        }

        for (int i = 0; i < 4; ++i) {
            GameObject obj = GameObject.Find ("bezier" + i.ToString ());
            if (obj != null) {
                bezierCPS [i] = obj.transform.localPosition;
                ControlPoints.wasitBezierControlPoints [i] = obj.transform.localPosition;
                //lineRender.SetPosition (i, ControlPoints.wasitBezierControlPoints [i]);
            }
        }

        for (int i = 0; i <= 50; i++) {
            //Vector3 v = bezier.getBezierPoint ((float)i * 0.02f);
            //lineRender.SetPosition (i, v);
        }

	
        if (Input.GetKeyDown (KeyCode.I)) {
            Mesh b = this.body.getMesh ();
            Mesh t = this.tmpBody.getMesh ();
            Vector3[] bV = b.vertices;
            Vector3[] tV = t.vertices;
            for (int i = 0; i < b.vertexCount; i++) {
                if (bV [i].x != tV [i].x)
                    Debug.LogError ("Bu yi yang");
            }
        }
    }
    
    // GUI callback function
    void OnGUI ()
    {

    }
	
}