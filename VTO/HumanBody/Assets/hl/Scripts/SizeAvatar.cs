using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum HumanSize:int
{
    Height = 1,     //身高
    Waist_Height,   //腰高
    Crotch_Height,  //跨高
    Knee_Height,    //膝盖高
    Arm_Length,     //上臂长
    Forearm_Length, //前臂长
    Shoulder_Width, //肩宽
    Bust_Grith,     //胸围
    Waist_Grith,    //腰围
    Upperarm_Grith, //上臂围
    Forearm_Grith,  //前臂围
    Hip_Grith,      //臀围
    Thigh_Grith,    //大腿围
    Knee_Grith,     //膝盖围
    Calf_Grith,     //小腿围
    Ankle_Grith,    //踝围
    Undefined       //未定义
}

public class SizeAvatar
{
    private HumanBody templateBody;

    private Mesh sizedMesh;
    private Mesh templateMesh;
    private Mesh templateLenMesh;
    private Transform[] templateBone;

    private Transform mRoot;
    private Vector3[] mLocalPosInBone0;
    private Vector3[] mLocalPosInBone1;
    private Vector3[] mLocalPosInBone2;
    private Vector3[] mLocalPosInBone3;
    private Vector3 templateShoulderLPos;
    private Vector3 templateShoulderRPos;
    // Grith: measurement is the grith radius, then grith = Pi * radius * radius
    // Height or Length: measurement itself
    private Dictionary<HumanSize, float> measurement;
    private float[] initMeasurements;
    // Control planes
    private ControlPlane planes;

    public ArrayList testList;
    // Constructor Function
    public SizeAvatar (Mesh sized, Mesh templateLen, HumanBody template)
    {
        testList = new ArrayList ();
        templateBody = template;
        sizedMesh = sized;
        templateMesh = template.getMesh ();
        templateBone = template.getBones ();
        mRoot = template.getTransform ();
        templateLenMesh = templateLen;
        measurement = new Dictionary<HumanSize, float> ();
        initMeasurements = new float[(int)HumanSize.Undefined];
        planes = new ControlPlane ();
        mLocalPosInBone0 = new Vector3[templateMesh.vertexCount];
        mLocalPosInBone1 = new Vector3[templateMesh.vertexCount];
        mLocalPosInBone2 = new Vector3[templateMesh.vertexCount];
        mLocalPosInBone3 = new Vector3[templateMesh.vertexCount];
        templateShoulderLPos = templateBody.boneInfo [HumanBone.L_Clavicle].localPosition;
        templateShoulderRPos = templateBody.boneInfo [HumanBone.R_Clavicle].localPosition;
        updateLocalPosInBones ();
        initMearsurement ();
    }

    private void updateLocalPosInBones ()
    {
        BoneWeight[] boneWeights = templateMesh.boneWeights;
        for (int i = 0; i < templateMesh.vertexCount; ++i) {
            //initial local position of bone
            mLocalPosInBone0 [i] = templateMesh.bindposes [boneWeights [i].boneIndex0].MultiplyPoint (sizedMesh.vertices [i]);
            mLocalPosInBone1 [i] = templateMesh.bindposes [boneWeights [i].boneIndex1].MultiplyPoint (sizedMesh.vertices [i]);
            mLocalPosInBone2 [i] = templateMesh.bindposes [boneWeights [i].boneIndex2].MultiplyPoint (sizedMesh.vertices [i]);
            mLocalPosInBone3 [i] = templateMesh.bindposes [boneWeights [i].boneIndex3].MultiplyPoint (sizedMesh.vertices [i]);
        }
    }
    private void initMearsurement ()
    {
        /* 胸围初始化 */     
        initGrith ("PlaneBust", HumanSize.Bust_Grith, HumanBone.Spine1);    
        /* 腰围初始化 */     
        initGrith ("PlaneWaist", HumanSize.Waist_Grith, HumanBone.Spine);
        /* 臀围初始化 */
        initGrith ("PlaneHip", HumanSize.Hip_Grith, HumanBone.Pelvis);       
        /* 大腿围初始化 */
        initGrith ("PlaneThigh", HumanSize.Thigh_Grith, HumanBone.L_Thigh);       
        /* 小腿围初始化 */
        initGrith ("PlaneCalf", HumanSize.Calf_Grith, HumanBone.L_Calf);

        initLength ("PlaneBase", "PlaneHeight", HumanSize.Height);
        initLength ("PlaneShoulderL", "PlaneShoulderR", HumanSize.Shoulder_Width);
        initLength ("PlaneBase", "PlaneCrotchHeight", HumanSize.Crotch_Height);     
        initLength ("PlaneBase", "PlaneKneeHeight", HumanSize.Knee_Height);
        initLength ("PlaneArmBase", "PlaneForearmLen", HumanSize.Forearm_Length);
        initLength ("PlaneArmBase", "PlaneArmLen", HumanSize.Arm_Length);
    }

    private void initGrith (string planeName, HumanSize grithKey, HumanBone planeInBone)
    {
        float[] plane = {0.0f, 0.0f, 0.0f, 0.0f};
        planes.getControlPlane (planeName, plane);
        ArrayList grithList = new ArrayList ();
        ArrayList edgeList = templateBody.edgeIndexsPerBone [planeInBone];
        for (int i = 0; i < edgeList.Count; i++) {
            Edge e = edgeList [i] as Edge;
            if (AuxMath.isAbovePlane (templateMesh.vertices [e.e1], plane) != 
                AuxMath.isAbovePlane (templateMesh.vertices [e.e2], plane)) {
                Vector3 intersectPoint = AuxMath.intersectPlane (templateMesh.vertices [e.e1],
                                                                 templateMesh.vertices [e.e2],
                                                                 plane);
                if (intersectPoint != AuxMath.nullVector) {
                    if (HumanSize.Thigh_Grith == grithKey) {
                        if (intersectPoint.x < 0.0f) {
                            grithList.Add (intersectPoint);
                        }
                    } else {
                        grithList.Add (intersectPoint);
                    }
                }
            }
        }
        if (grithList.Count > 0) {
            ArrayList grithIndex = new ArrayList ();
            AuxMath.convexInPlane (new Vector3 (plane [0], plane [1], plane [2]),
                                   grithList, grithIndex);
            initMeasurements [(int)grithKey - 1] = AuxMath.getLength (grithIndex, grithList);
            measurement [grithKey] = initMeasurements [(int)grithKey - 1] / (2.0f * Mathf.PI);
        }
    }

    private void initLength (string plane1, string plane2, HumanSize lenKey)
    {
        initMeasurements [(int)lenKey - 1] = planes.getControlPlanesDist (plane1, plane2);
        measurement [lenKey] = initMeasurements [(int)lenKey - 1];
    }

    public void resetAvatar (HumanBone bone)
    {
        Vector3[] vertices = sizedMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        if (bone == HumanBone.Undefined) {
            for (int i = 0; i < vertices.Length; ++i) {
                vertices [i].x = templateMesh.vertices [i].x;
                vertices [i].y = templateMesh.vertices [i].y;
                vertices [i].z = templateMesh.vertices [i].z;
                tmpLen [i].x = templateMesh.vertices [i].x;
                tmpLen [i].y = templateMesh.vertices [i].y;
                tmpLen [i].z = templateMesh.vertices [i].z;
            }
            sizedMesh.vertices = vertices;
            templateLenMesh.vertices = tmpLen;
            return;
        }
        ArrayList boneVertexlist = templateBody.vertexIndexsPerBone [bone];
        for (int i = 0; i < boneVertexlist.Count; ++i) {
            vertices [(int)boneVertexlist [i]].x = templateMesh.vertices [(int)boneVertexlist [i]].x;
            vertices [(int)boneVertexlist [i]].y = templateMesh.vertices [(int)boneVertexlist [i]].y;
            vertices [(int)boneVertexlist [i]].z = templateMesh.vertices [(int)boneVertexlist [i]].z;
            tmpLen [i].x = templateLenMesh.vertices [i].x;
            tmpLen [i].y = templateLenMesh.vertices [i].y;
            tmpLen [i].z = templateLenMesh.vertices [i].z;
        }
        sizedMesh.vertices = vertices;
    }

    public void sizeLength (float height,  
                            float shoulder_Width,
                            float crotch_Height, 
                            float knee_Height,  
                            float forearm_Length,
                            float arm_Length)
    {
        //Height
//        float pelvisS = (height - crotch_Height) / 
//            (measurement [HumanSize.Height] - measurement [HumanSize.Crotch_Height]);       
//        float thighS1 = crotch_Height / (pelvisS * measurement [HumanSize.Crotch_Height]);
//        // pelvis * pelvisS
//        // Thigh * thighS1
//        templateBody.boneInfo [HumanBone.Pelvis].localScale = new Vector3 (pelvisS, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.Head].localScale = new Vector3 (1.0f, 1.0f, pelvisS);
//
//        //Thigh
//        float thighS2 = (crotch_Height - knee_Height) / 
//            (crotch_Height - pelvisS * thighS1 * measurement [HumanSize.Knee_Height]);
//        float calfS = knee_Height / (thighS2 * thighS1 * pelvisS * measurement [HumanSize.Knee_Height]);
//        // Thigh * thighS1 * thighS2
//        // Calf * calfS
//        templateBody.boneInfo [HumanBone.L_Thigh].localScale = new Vector3 (thighS1 * thighS2, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.R_Thigh].localScale = new Vector3 (thighS1 * thighS2, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.L_Calf].localScale = new Vector3 (calfS, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.R_Calf].localScale = new Vector3 (calfS, 1.0f, 1.0f);    
//
//        // Arm 
//        float upperarmS = (arm_Length - forearm_Length) /
//            (measurement [HumanSize.Arm_Length] - measurement [HumanSize.Forearm_Length]);
//        float forearmS = forearm_Length / (upperarmS * measurement [HumanSize.Forearm_Length]);
//        templateBody.boneInfo [HumanBone.L_Upperarm].localScale = new Vector3 (upperarmS, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.R_Upperarm].localScale = new Vector3 (upperarmS, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.L_Forearm].localScale = new Vector3 (forearmS, 1.0f, 1.0f);
//        templateBody.boneInfo [HumanBone.R_Forearm].localScale = new Vector3 (forearmS, 1.0f, 1.0f); 
//
//        // Shoulder
//        float shoulderT = (shoulder_Width - measurement [HumanSize.Shoulder_Width]) / 2.0f;
//        Vector3 shoulderL = templateShoulderLPos;
//        shoulderL.z = templateShoulderLPos.z - shoulderT;
//        Vector3 shoulderR = templateShoulderRPos;
//        shoulderR.z = templateShoulderRPos.z + shoulderT;
//        templateBody.boneInfo [HumanBone.L_Clavicle].localPosition = shoulderL;
//        templateBody.boneInfo [HumanBone.R_Clavicle].localPosition = shoulderR;

        //Height
        float pelvisS = (height - crotch_Height) / 
            (measurement [HumanSize.Height] - measurement [HumanSize.Crotch_Height]);       
        float thighS1 = crotch_Height / (pelvisS * measurement [HumanSize.Crotch_Height]);
        // pelvis * pelvisS
        // Thigh * thighS1
        templateBody.boneInfo [HumanBone.Pelvis].localScale = new Vector3 (1.0f, pelvisS, 1.0f);
        templateBody.boneInfo [HumanBone.Head].localScale = new Vector3 (1.0f, 1.0f, pelvisS);
        //Thigh
        float thighS2 = (crotch_Height - knee_Height) / 
            (crotch_Height - pelvisS * thighS1 * measurement [HumanSize.Knee_Height]);
        float calfS = knee_Height / (thighS2 * thighS1 * pelvisS * measurement [HumanSize.Knee_Height]);
        // Thigh * thighS1 * thighS2
        // Calf * calfS
        templateBody.boneInfo [HumanBone.L_Thigh].localScale = new Vector3 (1.0f, thighS1 * thighS2, 1.0f);
        templateBody.boneInfo [HumanBone.R_Thigh].localScale = new Vector3 (1.0f, thighS1 * thighS2, 1.0f);
        templateBody.boneInfo [HumanBone.L_Calf].localScale = new Vector3 (1.0f, calfS, 1.0f);
        templateBody.boneInfo [HumanBone.R_Calf].localScale = new Vector3 (1.0f, calfS, 1.0f); 

        // Arm 
        float upperarmS = (arm_Length - forearm_Length) /
            (measurement [HumanSize.Arm_Length] - measurement [HumanSize.Forearm_Length]);
        float forearmS = forearm_Length / (upperarmS * measurement [HumanSize.Forearm_Length]);
        templateBody.boneInfo [HumanBone.L_Upperarm].localScale = new Vector3 (1.0f, upperarmS, 1.0f);
        templateBody.boneInfo [HumanBone.R_Upperarm].localScale = new Vector3 (1.0f, upperarmS, 1.0f);
        templateBody.boneInfo [HumanBone.L_Forearm].localScale = new Vector3 (1.0f, forearmS, 1.0f);
        templateBody.boneInfo [HumanBone.R_Forearm].localScale = new Vector3 (1.0f, forearmS, 1.0f); 
        // Shoulder
        float shoulderT = (shoulder_Width - measurement [HumanSize.Shoulder_Width]) / 2.0f;
        Vector3 shoulderL = templateShoulderLPos;
        shoulderL.x = templateShoulderLPos.x - shoulderT;
        Vector3 shoulderR = templateShoulderRPos;
        shoulderR.x = templateShoulderRPos.x + shoulderT;
        templateBody.boneInfo [HumanBone.L_Clavicle].localPosition = shoulderL;
        templateBody.boneInfo [HumanBone.R_Clavicle].localPosition = shoulderR;

        Vector3[] vertices = sizedMesh.vertices; 
        BoneWeight[] boneWeights = templateMesh.boneWeights;
        Vector3 tmpPos = Vector3.zero;
        Vector3 tmpV;
        for (int i = 0; i < vertices.Length; i++) {
            tmpPos.x = tmpPos.y = tmpPos.z = 0.0f;               
            if (boneWeights [i].weight0 != 0.0f) {
                tmpV = templateBone [boneWeights [i].boneIndex0].TransformPoint (mLocalPosInBone0 [i]);
                tmpPos += tmpV * boneWeights [i].weight0;
            }
            if (boneWeights [i].weight1 != 0.0f) {
                tmpV = templateBone [boneWeights [i].boneIndex1].TransformPoint (mLocalPosInBone1 [i]);
                tmpPos += tmpV * boneWeights [i].weight1;
            }
            if (boneWeights [i].weight2 != 0.0f) {                  
                tmpV = templateBone [boneWeights [i].boneIndex2].TransformPoint (mLocalPosInBone2 [i]);
                tmpPos += tmpV * boneWeights [i].weight2;
            }
            if (boneWeights [i].weight3 != 0.0f) {
                tmpV = templateBone [boneWeights [i].boneIndex3].TransformPoint (mLocalPosInBone3 [i]);
                tmpPos += tmpV * boneWeights [i].weight3;
            }
            vertices [i] = tmpPos;
            vertices [i] = mRoot.InverseTransformPoint (vertices [i]);
        }
        sizedMesh.vertices = vertices;
        // 此处可优化，待优化
        Vector3[] tmpLen = templateLenMesh.vertices;
        sizedMesh.RecalculateBounds ();
        for (int i = 0; i < vertices.Length; i++) {
            vertices [i].y -= sizedMesh.bounds.min.y;
            tmpLen [i] = vertices [i];
        }
        sizedMesh.vertices = vertices;
        templateLenMesh.vertices = tmpLen;
    }

    public void sizeGrith (float bust_Grith,
                           float waist_Grith,
                           float hip_Grith,
                           float thigh_Grith,    
                           float calf_Grith)
    {
        float radius = bust_Grith / (2.0f * Mathf.PI);
        scaleBustGrith (radius / measurement [HumanSize.Bust_Grith]);
        radius = waist_Grith / (2.0f * Mathf.PI);
        scaleWaistGrith (radius / measurement [HumanSize.Waist_Grith]);
        radius = hip_Grith / (2.0f * Mathf.PI);
        scaleHipGrith (radius / measurement [HumanSize.Hip_Grith]);
        radius = thigh_Grith / (2.0f * Mathf.PI);
        scaleThighGrith (radius / measurement [HumanSize.Thigh_Grith]);       
        radius = calf_Grith / (2.0f * Mathf.PI);
        scaleCalfGrith (radius / measurement [HumanSize.Calf_Grith]);       
    }

    public float getInitMeasurement (HumanSize sizeName)
    {
        return initMeasurements [(int)sizeName - 1];
    }
    // 胸围
    private void scaleBustGrith (float scale)
    {
        Vector3[] old = sizedMesh.vertices;
        Vector3[] tmp = templateMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        ArrayList vertex = templateBody.vertexIndexsPerBone [HumanBone.Spine2];
        float[] top = planes.getControlPlane ("PlaneBustTop");
        float[] topL = planes.getControlPlane ("PlaneBustTopLeft");
        float[] topR = planes.getControlPlane ("PlaneBustTopRight");
        float[] bust = planes.getControlPlane ("PlaneBust");
        float[] bottom = planes.getControlPlane ("PlaneBustBottom");
        Vector3 busthNormal = new Vector3 (bust [0], bust [1], bust [2]);

        // Scale the left thigh
        for (int i = 0; i < vertex.Count; ++i) {
            if (AuxMath.isAbovePlane (tmp [(int)vertex [i]], top) !=
                AuxMath.isAbovePlane (tmp [(int)vertex [i]], bottom)) {
                if (!AuxMath.isAbovePlane (tmp [(int)vertex [i]], topL) && 
                    !AuxMath.isAbovePlane (tmp [(int)vertex [i]], topR)) {
                    Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], 
                                                                busthNormal);
                    float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.forward);
                    float k = (scale - 1.0f) * cosAngel + 
                        Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                    old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                        (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Bust_Grith]);
                }
            }
        }
        sizedMesh.vertices = old;
    }

    // 腰围
    private void scaleWaistGrith (float scale)
    {
        Vector3[] old = sizedMesh.vertices;
        Vector3[] tmp = templateMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        ArrayList vertex = templateBody.vertexIndexsPerBone [HumanBone.Pelvis];
        float top = planes.getPlaneHeight ("PlaneWaistTop");
        float waist = planes.getPlaneHeight ("PlaneWaist");
        float bottom = planes.getPlaneHeight ("PlaneWaistBottom");
        float height = top - bottom;
        float std = ControlPoints.wasitBezier.getBezierPoint ((waist - bottom) / height).z;
        for (int i = 0; i < vertex.Count; ++i) {
            if (tmp [(int)vertex [i]].y < top && tmp [(int)vertex [i]].y > bottom) {
                float ratio = (tmp [(int)vertex [i]].y - bottom) / height;
                float p = (ControlPoints.wasitBezier.getBezierPoint (ratio).z - std) / 5.0f + 1.0f;
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], Vector3.up);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.forward);
                //float sinAngel_square = 1.0f - cosAngel * cosAngel;
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Waist_Grith] * p);
            }
        }
        sizedMesh.vertices = old;
    }

    // 臀围
    private void scaleHipGrith (float scale)
    {
        Vector3[] old = sizedMesh.vertices;
        Vector3[] tmp = templateMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        ArrayList vertex = templateBody.vertexIndexsPerBone [HumanBone.Pelvis];
        float top = planes.getPlaneHeight ("PlaneHipTop");
        float[] topPlane = planes.getControlPlane ("PlaneHipTop");
        float hip = planes.getPlaneHeight ("PlaneHip");
        float bottom = planes.getPlaneHeight ("PlaneHipBottom");
        float height = top - bottom;
        float std = ControlPoints.hipBezier.getBezierPoint ((hip - bottom) / height).z;
        for (int i = 0; i < vertex.Count; ++i) {
            if (!AuxMath.isAbovePlane (tmp [(int)vertex [i]], topPlane) && tmp [(int)vertex [i]].y > bottom) {
                float ratio = (tmp [(int)vertex [i]].y - bottom) / height;
                float p = (ControlPoints.hipBezier.getBezierPoint (ratio).z - std) / 5.0f + 1.0f;
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], Vector3.up);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.back);
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Hip_Grith] * p);
            }        
        }
        sizedMesh.vertices = old;
    }

    // 大腿围
    private void scaleThighGrith (float scale)
    {
        Vector3[] old = sizedMesh.vertices;
        Vector3[] tmp = templateMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        ArrayList vertex = templateBody.vertexIndexsPerBone [HumanBone.L_Thigh];
        float[] top = planes.getControlPlane ("PlaneThighTopLeft");
        float[] thigh = planes.getControlPlane ("PlaneThigh");
        float[] bottom = planes.getControlPlane ("PlaneThighBottom");
        Vector3 thighNormal = new Vector3 (thigh [0], thigh [1], thigh [2]);
        // Scale the left thigh
        for (int i = 0; i < vertex.Count; ++i) {
            if (AuxMath.isAbovePlane (tmp [(int)vertex [i]], top) !=
                AuxMath.isAbovePlane (tmp [(int)vertex [i]], bottom)) {               
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], 
                                                                thighNormal);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.back);
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Thigh_Grith]);
            }
        }
        // Scale the Right Thigh
        vertex = templateBody.vertexIndexsPerBone [HumanBone.R_Thigh];
        top = planes.getControlPlane ("PlaneThighTopRight");
        for (int i = 0; i < vertex.Count; ++i) {
            if (AuxMath.isAbovePlane (tmp [(int)vertex [i]], top) !=
                AuxMath.isAbovePlane (tmp [(int)vertex [i]], bottom)) {
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], 
                                                                thighNormal);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.back);
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Thigh_Grith]);
            }
        }
        sizedMesh.vertices = old;
    }
    // 小腿围
    private void scaleCalfGrith (float scale)
    {
        Vector3[] old = sizedMesh.vertices;
        Vector3[] tmp = templateMesh.vertices;
        Vector3[] tmpLen = templateLenMesh.vertices;
        ArrayList vertex = templateBody.vertexIndexsPerBone [HumanBone.L_Calf];
        float[] top = planes.getControlPlane ("PlaneCalfTop");
        float[] calf = planes.getControlPlane ("PlaneCalf");
        float[] bottom = planes.getControlPlane ("PlaneCalfBottom");
        Vector3 calfNormal = new Vector3 (calf [0], calf [1], calf [2]);
        // Scale the left calf
        for (int i = 0; i < vertex.Count; ++i) {
            if (AuxMath.isAbovePlane (tmp [(int)vertex [i]], top) !=
                AuxMath.isAbovePlane (tmp [(int)vertex [i]], bottom)) {
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], 
                                                                calfNormal);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.back);
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Calf_Grith]);
            }
        }
        // Scale the Right calf
        vertex = templateBody.vertexIndexsPerBone [HumanBone.R_Calf];
        for (int i = 0; i < vertex.Count; ++i) {
            if (AuxMath.isAbovePlane (tmp [(int)vertex [i]], top) !=
                AuxMath.isAbovePlane (tmp [(int)vertex [i]], bottom)) {
                Vector3 projectNormal = AuxMath.projectOnPlane (templateMesh.normals [(int)vertex [i]], 
                                                                calfNormal);
                float cosAngel = Vector3.Dot (projectNormal.normalized, Vector3.back);
                float k = (scale - 1.0f) * cosAngel + 
                    Mathf.Sqrt ((scale - 1.0f) * (scale - 1.0f) * cosAngel * cosAngel + 2 * scale - 1);
                old [(int)vertex [i]] = tmpLen [(int)vertex [i]] + 
                    (templateMesh.normals [(int)vertex [i]] * (k - 1.0f) * measurement [HumanSize.Calf_Grith]);
            }
        }
        sizedMesh.vertices = old;
    }
}

public class ControlPoints
{
    public static Vector3[] wasitBezierControlPoints = {
		new Vector3 (-20.0f, 60.0f, -10.0f),
		new Vector3 (-20.0f, 65.0f, -11.0f),
		new Vector3 (-20.0f, 70.0f, -3.747f),
		new Vector3 (-20.0f, 75.0f, -10.0f)
	};


    public static Vector3[] hipBezierControlPoints = {
        new Vector3 (-20.0f, 60.0f, -10.0f),
        new Vector3 (-20.0f, 65.0f, -9.34f),
        new Vector3 (-20.0f, 70.0f, -1.127f),
        new Vector3 (-20.0f, 75.0f, -10.0f)
    };
    

    public static Bezier wasitBezier = new Bezier (wasitBezierControlPoints);
    public static Bezier hipBezier = new Bezier (hipBezierControlPoints);
}

public class ControlPlane
{
    //private Dictionary<HumanSize, float> measurement;
    private Dictionary <string, Transform> planes;

    public ControlPlane ()
    {
        planes = new Dictionary<string, Transform> ();
        GameObject[] objs = GameObject.FindGameObjectsWithTag ("Plane");
        for (int i = 0; i < objs.Length; ++i) {
            planes [objs [i].name] = objs [i].transform;
        }
    }
    //type = 0, represents Local, else Global
    public void getControlPlane (string name, float[] plane)
    {
        if (planes.ContainsKey (name)) {
            Vector3 n = planes [name].localRotation * Vector3.up;
            plane [0] = n.x;
            plane [1] = n.y;
            plane [2] = n.z;
            plane [3] = - n.x * planes [name].localPosition.x
                - n.y * planes [name].localPosition.y
                - n.z * planes [name].localPosition.z;

        } else {
            Debug.LogError ("The plane " + name + " is not exist");
        }
    }

    public float[] getControlPlane (string name)
    {
        if (planes.ContainsKey (name)) {
            float[] plane = {0.0f, 0.0f, 0.0f, 0.0f};
            Vector3 n = planes [name].localRotation * Vector3.up;
            plane [0] = n.x;
            plane [1] = n.y;
            plane [2] = n.z;
            plane [3] = - n.x * planes [name].localPosition.x
                - n.y * planes [name].localPosition.y
                - n.z * planes [name].localPosition.z;           
            return plane;
        } else {
            Debug.LogError ("The plane " + name + " is not exist");
            return null;
        }
    }

    public void getControlPlaneNormal (string name, Vector3 n)
    {
        if (planes.ContainsKey (name)) {
            if (n == null) {
                n = Vector3.zero;
            }
            n = planes [name].localRotation * Vector3.up;
        } else {
            Debug.LogError ("The plane " + name + " is not exist");
        }
    }

    public float getControlPlanesDist (string p1, string p2)
    {
        if (planes.ContainsKey (p1) && planes.ContainsKey (p2)) {
            Vector3 n1 = planes [p1].localRotation * Vector3.up;
            Vector3 n2 = planes [p2].localRotation * Vector3.up;
            float d1 = - n1.x * planes [p1].localPosition.x -
                n1.y * planes [p1].localPosition.y -
                n1.z * planes [p1].localPosition.z;
            float d2 = - n2.x * planes [p2].localPosition.x -
                n2.y * planes [p2].localPosition.y -
                n2.z * planes [p2].localPosition.z;
            return Mathf.Abs (d1 - d2) / Mathf.Sqrt (Vector3.Distance (n1, Vector3.zero));

        } else {
            Debug.LogError ("The plane is not exist");
            return 0.0f;
        }
    }

    public float getPlanesHeight (string name1, string name2)
    {
        return Mathf.Abs (planes [name1].localPosition.y - planes [name2].localPosition.y);
    }

    public float getPlaneHeight (string name)
    {
        return planes [name].position.y;
    }

    public float getPlanePos (string name, string xyz)
    {
        float rst = 0.0f;
        switch (xyz) {
        case "x":
        case "X":
            rst = planes [name].localPosition.x;
            break;
        case "y":
        case "Y":
            rst = planes [name].localPosition.y;
            break;
        case "z":
        case "Z":
            rst = planes [name].localPosition.z;
            break;
        default:
            break;
        }
        return rst;
    }
}
