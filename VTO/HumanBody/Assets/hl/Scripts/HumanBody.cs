using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;


public enum HumanBone:int
{
    Head = 1,
    L_Calf,
    L_Clavicle,
    L_Foot,
    L_Forearm,
    L_Hand,
    L_Thigh,
    L_Upperarm,
    Pelvis,
    R_Calf,
    R_Clavicle,
    R_Foot,
    R_Forearm,
    R_Hand,
    R_Thigh,
    R_Upperarm,
    Spine,
    Spine1,
    Spine2,
    Spine3,
    Spine4,
    Undefined
}

public class Edge : IComparable
{
    public int e1;
    public int e2;

    public Edge (int edgeId1, int edgeId2)
    {
        if (edgeId1 > edgeId2) {
            e1 = edgeId2;
            e2 = edgeId1;
        } else {
            e1 = edgeId1;
            e2 = edgeId2;
        }
    }
    public int CompareTo (object obj)
    {
        Edge edge = obj as Edge;
        if (e1 == edge.e1 && e2 == edge.e2)
            return 0;
        if (e1 > edge.e1)
            return 1;
        else if (e1 == edge.e1) {
            if (e2 > edge.e2)
                return 1;
            else
                return -1;
        } else {
            return -1;
        }
    }
}

public class HumanBody
{
    //根据骨骼索引查找骨骼名
    private Dictionary <int, HumanBone> boneIndex;
    //根据骨骼名查找受该骨骼影响的所有顶点索引
    public  Dictionary <HumanBone, ArrayList> vertexIndexsPerBone;
    //根据骨骼名查找受该骨骼影响的所有三角面片索引
    public  Dictionary <HumanBone, ArrayList> triIndexsPerBone;
    //根据骨骼名查找受该骨骼影响的所有边的索引
    public  Dictionary <HumanBone, ArrayList> edgeIndexsPerBone;
    //根据骨骼名查找骨骼
    public	Dictionary <HumanBone, Transform> boneInfo;
    //根据骨骼名查找受该骨骼影响的顶点的影响权值
    public Dictionary <HumanBone, ArrayList> vertexBoneWeightsPerBone;
    private Mesh mesh;
    private Transform[] bones;
    private Transform rootTransform;
	
    public HumanBody (Mesh m, Transform[] b, Transform r)
    {
        mesh = m;
        bones = b;
        rootTransform = r;
        boneIndex = new Dictionary<int, HumanBone> ();
        vertexIndexsPerBone = new Dictionary<HumanBone, ArrayList> ();
        triIndexsPerBone = new Dictionary<HumanBone, ArrayList> ();
        edgeIndexsPerBone = new Dictionary<HumanBone, ArrayList> ();
        boneInfo = new Dictionary<HumanBone, Transform> ();
        vertexBoneWeightsPerBone = new Dictionary<HumanBone, ArrayList> ();
        int boneCnt = (int)HumanBone.Undefined;
        for (int i = 1; i < boneCnt; i++) {
            vertexIndexsPerBone [(HumanBone)i] = new ArrayList ();
            triIndexsPerBone [(HumanBone)i] = new ArrayList ();
            edgeIndexsPerBone [(HumanBone)i] = new ArrayList ();
            vertexBoneWeightsPerBone [(HumanBone)i] = new ArrayList ();

        }
        initBoneIndex ();
        initVertexsPerBone ();
        initTriPerBone ();
        initEdgePerBone ();
    }

    public Mesh getMesh ()
    {
        return mesh;
    }

    public Transform[] getBones ()
    {
        return bones;
    }

    public Transform getTransform ()
    {
        return rootTransform;
    }

    public Bounds getBounds (HumanBone bone)
    {
        ArrayList vertices = vertexIndexsPerBone [bone];
        if (vertices != null) {
            return getBoundingBox (vertices);
        }
        return new Bounds ();
    }

    private bool initVertexsPerBone ()
    {
        if (mesh != null && bones != null) {
            BoneWeight[] bw = mesh.boneWeights;
            for (int vIndex = 0; vIndex < mesh.vertexCount; vIndex++) {
                if (bw [vIndex].weight0 != 0) {
                    HumanBone boneName = boneIndex [bw [vIndex].boneIndex0];
                    if (boneName != HumanBone.Undefined) {
                        vertexIndexsPerBone [boneName].Add (vIndex);
                        vertexBoneWeightsPerBone [boneName].Add (bw [vIndex].weight0);
                    }
                    if (bw [vIndex].weight1 != 0) {
                        boneName = boneIndex [bw [vIndex].boneIndex1];
                        if (boneName != HumanBone.Undefined) {
                            vertexIndexsPerBone [boneName].Add (vIndex);
                            vertexBoneWeightsPerBone [boneName].Add (bw [vIndex].weight1);
                        }
                        if (bw [vIndex].weight2 != 0) {
                            boneName = boneIndex [bw [vIndex].boneIndex2];
                            if (boneName != HumanBone.Undefined) {
                                vertexIndexsPerBone [boneName].Add (vIndex);
                                vertexBoneWeightsPerBone [boneName].Add (bw [vIndex].weight2);
                            }
                            if (bw [vIndex].weight3 != 0) {
                                boneName = boneIndex [bw [vIndex].boneIndex3];
                                if (boneName != HumanBone.Undefined) {
                                    vertexIndexsPerBone [boneName].Add (vIndex);
                                    vertexBoneWeightsPerBone [boneName].Add (bw [vIndex].weight3);
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
        return false;
    }

    private void initTriPerBone ()
    {
        if (mesh != null) {
            int[] tri = mesh.triangles;
            for (int tIndex = 0; tIndex < tri.Length / 3; tIndex++) {
                //for (int i = 1; i < HumanBone.Undefined; i++)
                foreach (HumanBone key in vertexIndexsPerBone.Keys) {
                    for (int i = tIndex * 3; i < (tIndex + 1) * 3; i++) {
                        int vI = vertexIndexsPerBone [key].BinarySearch (tri [i]);
                        if (vI >= 0) {
                            triIndexsPerBone [key].Add (tIndex);
                            break;
                        }
                    }
                }
            }
        }
    }

    private void initEdgePerBone ()
    {
        if (mesh != null) {
            foreach (HumanBone key in triIndexsPerBone.Keys) {
                foreach (int i in triIndexsPerBone[key]) {
                    Edge e1 = new Edge (mesh.triangles [i * 3], mesh.triangles [i * 3 + 1]);
                    Edge e2 = new Edge (mesh.triangles [i * 3 + 1], mesh.triangles [i * 3 + 2]);
                    Edge e3 = new Edge (mesh.triangles [i * 3 + 2], mesh.triangles [i * 3]);

                    int isExist = edgeIndexsPerBone [key].BinarySearch (e1);
                    if (isExist < 0) {
                        edgeIndexsPerBone [key].Add (e1);
                    }
                    isExist = edgeIndexsPerBone [key].BinarySearch (e2);
                    if (isExist < 0) {
                        edgeIndexsPerBone [key].Add (e2);
                    }
                    isExist = edgeIndexsPerBone [key].BinarySearch (e3);
                    if (isExist < 0) {
                        edgeIndexsPerBone [key].Add (e3);
                    }
                }
            }
        }
    }

    private void initBoneIndex ()
    {
        for (int bIndex = 0; bIndex < bones.Length; bIndex++) {
            switch (bones [bIndex].name) {
            case "Head":
                boneIndex [bIndex] = HumanBone.Head;
                boneInfo [HumanBone.Head] = bones [bIndex];
                break;
            case "L_Calf":
                boneIndex [bIndex] = HumanBone.L_Calf;
                boneInfo [HumanBone.L_Calf] = bones [bIndex];
                break;
            case "L_Clavicle":
                boneIndex [bIndex] = HumanBone.L_Clavicle;
                boneInfo [HumanBone.L_Clavicle] = bones [bIndex];
                break;
            case "L_Foot":
                boneIndex [bIndex] = HumanBone.L_Foot;
                boneInfo [HumanBone.L_Foot] = bones [bIndex];
                break;
            case "L_Forearm":
                boneIndex [bIndex] = HumanBone.L_Forearm;
                boneInfo [HumanBone.L_Forearm] = bones [bIndex];
                break;
            case "L_Hand":
                boneIndex [bIndex] = HumanBone.L_Hand;
                boneInfo [HumanBone.L_Hand] = bones [bIndex];
                break;
            case "L_Thigh":
                boneIndex [bIndex] = HumanBone.L_Thigh;
                boneInfo [HumanBone.L_Thigh] = bones [bIndex];
                break;
            case "L_Upperarm":
                boneIndex [bIndex] = HumanBone.L_Upperarm;
                boneInfo [HumanBone.L_Upperarm] = bones [bIndex];
                break;
            case "Pelvis":
                boneIndex [bIndex] = HumanBone.Pelvis;
                boneInfo [HumanBone.Pelvis] = bones [bIndex];
                break;
            case "R_Calf":
                boneIndex [bIndex] = HumanBone.R_Calf;
                boneInfo [HumanBone.R_Calf] = bones [bIndex];
                break;
            case "R_Clavicle":
                boneIndex [bIndex] = HumanBone.R_Clavicle;
                boneInfo [HumanBone.R_Clavicle] = bones [bIndex];
                break;
            case "R_Foot":
                boneIndex [bIndex] = HumanBone.R_Foot;
                boneInfo [HumanBone.R_Foot] = bones [bIndex];
                break;
            case "R_Forearm":
                boneIndex [bIndex] = HumanBone.R_Forearm;
                boneInfo [HumanBone.R_Forearm] = bones [bIndex];
                break;
            case "R_Hand":
                boneIndex [bIndex] = HumanBone.R_Hand;
                boneInfo [HumanBone.R_Hand] = bones [bIndex];
                break;
            case "R_Thigh":
                boneIndex [bIndex] = HumanBone.R_Thigh;
                boneInfo [HumanBone.R_Thigh] = bones [bIndex];
                break;
            case "R_Upperarm":
                boneIndex [bIndex] = HumanBone.R_Upperarm;
                boneInfo [HumanBone.R_Upperarm] = bones [bIndex];
                break;
            case "Spine":
                boneIndex [bIndex] = HumanBone.Spine;
                boneInfo [HumanBone.Spine] = bones [bIndex];
                break;
            case "Spine1":
                boneIndex [bIndex] = HumanBone.Spine1;
                boneInfo [HumanBone.Spine1] = bones [bIndex];
                break;
            case "Spine2":
                boneIndex [bIndex] = HumanBone.Spine2;
                boneInfo [HumanBone.Spine2] = bones [bIndex];
                break;
            case "Spine3":
                boneIndex [bIndex] = HumanBone.Spine3;
                boneInfo [HumanBone.Spine3] = bones [bIndex];
                break;
            case "Spine4":
                boneIndex [bIndex] = HumanBone.Spine4;
                boneInfo [HumanBone.Spine4] = bones [bIndex];
                break;
            default:
                boneIndex [bIndex] = HumanBone.Undefined;
                boneInfo [HumanBone.Undefined] = null;
                break;
            }	
        }
    }
	
    private Bounds getBoundingBox (Vector3[] vertices)
    {
        if (vertices.Length > 0) {
            float minX = vertices [0].x;
            float minY = vertices [0].y;
            float minZ = vertices [0].z;
			
            float maxX = vertices [0].x;
            float maxY = vertices [0].y;
            float maxZ = vertices [0].z;
            for (int i = 1; i < vertices.Length; i++) {
                if (minX > vertices [i].x) {
                    minX = vertices [i].x;
                } else if (maxX < vertices [i].x) {
                    maxX = vertices [i].x;
                }
				
                if (minY > vertices [i].y) {
                    minY = vertices [i].y;
                } else if (maxY < vertices [i].y) {
                    maxY = vertices [i].y;
                }
				
                if (minZ > vertices [i].z) {
                    minZ = vertices [i].z;
                } else if (maxZ < vertices [i].z) {
                    maxZ = vertices [i].z;
                }
            }
            return new Bounds (new Vector3 ((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f),
			                  new Vector3 (maxX - minX, maxY - minY, maxZ - minZ));
        }
        return new Bounds ();
    }
	
    public Bounds getBoundingBox (ArrayList indexs)
    {
        Vector3[] vertices = new Vector3[indexs.Count];
        for (int i = 0; i < indexs.Count; i++) {
            vertices [i] = mesh.vertices [(int)indexs [i]];
        }
        return getBoundingBox (vertices);
    }

    public Vector3[] getLocalCoordinate (HumanBone bone)
    {
        ArrayList vertexList = vertexIndexsPerBone [bone];
        Vector3[] localCoords = null;
        if (vertexList.Count != 0) {
            localCoords = new Vector3[vertexList.Count];
            Vector3[] vertices = new Vector3[vertexList.Count];
            for (int i = 0; i < vertexList.Count; i++) {
                vertices [i] = mesh.vertices [(int)vertexList [i]];
            }
            Bounds boundingBox = getBoundingBox (vertices);
            // Calculate the local coordinates of every vertex of the bone
            for (int i = 0; i < vertexList.Count; i++) {
                localCoords [i].x = (vertices [i].x - boundingBox.min.x) / boundingBox.size.x;
                localCoords [i].y = (vertices [i].y - boundingBox.min.y) / boundingBox.size.y;
                localCoords [i].z = (vertices [i].z - boundingBox.min.z) / boundingBox.size.z;
            }
        }
        return localCoords;
    }
}