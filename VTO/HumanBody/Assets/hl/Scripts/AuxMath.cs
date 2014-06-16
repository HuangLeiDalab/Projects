using UnityEngine;
using System.Collections;

public class AuxMath
{
    // /*向量v 在法向量为 n 的平面上的投影向量*/
    public static Vector3 projectOnPlane (Vector3 v, Vector3 n)
    {
        Vector3 norm = n.normalized;
        float dot = Vector3.Dot (v, norm);
        if (dot < 0.0f) {
            norm = -norm;
            dot = Mathf.Abs (dot);
        }
        return (v - norm * dot);
    }
    // /*向量v 在平面 Ax + By + Cz + D = 0 上的投影向量*/
    public static Vector3 projectOnPlane (Vector3 v, float A, float B, float C)
    {
        Vector3 norm = new Vector3 (A, B, C);
        return projectOnPlane (v, norm);
    }
    // /*顶点p 在平面Ax + By + Cz + D = 0的正面（true）or 反面（false）*/
    public static bool isAbovePlane (Vector3 p, float A, float B, float C, float D)
    {
        return A * p.x + B * p.y + C * p.z + D >= 0;
    }
    // /*顶点p 在平面Ax + By + Cz + D = 0的正面（true）or 反面（false）,plane[4] = [A, B, C, D]*/
    public static bool isAbovePlane (Vector3 p, float[] plane)
    {
        return plane [0] * p.x + plane [1] * p.y + plane [2] * p.z + plane [3] >= 0;
    }
    // /*射线origin + dir * t与平面Ax + By + Cz + D = 0的相交测试 */
    public static Vector3 nullVector = new Vector3 (-99.0f, -99.0f, -99.0f);
    public static Vector3 hitPlane (Vector3 origin, Vector3 dir, float A, float B, float C, float D)
    {
        Vector3 norm = new Vector3 (A, B, C);
        float temp1 = Vector3.Dot (dir, norm.normalized);
        float temp2 = Vector3.Dot (origin, norm.normalized);
        if (temp1 == 0.0f) {
            return nullVector;
        }
        float t = -(temp2 + D) / temp1;
        if (t < 0.0f) {
            return nullVector;
        }
        return (origin + dir * t);
    }
    // /*线段ab与平面Ax + By + Cz + D = 0的相交测试*/
    public static Vector3 intersectPlane (Vector3 a, Vector3 b, float A, float B, float C, float D)
    {
        Vector3 dir = (b - a).normalized;
        if (dir == Vector3.zero) {
            return nullVector;
        }
        Vector3 hit = hitPlane (a, dir, A, B, C, D);
        if (hit == nullVector) {
            return nullVector;
        }
        float t = Vector3.Distance (a, hit) / Vector3.Distance (a, b);
        if (t > 1.0f) {
            return nullVector;
        }
        return hit;
    }
    // /*线段ab与平面Ax + By + Cz + D = 0的相交测试, plane[4] = [A, B, C, D]*/
    public static Vector3 intersectPlane (Vector3 a, Vector3 b, float[] plane)
    {
        return intersectPlane (a, b, plane [0], plane [1], plane [2], plane [3]);
    }
    // /*顶点连线的长度和*/
    public static float getLength (int[] index, Vector3[] vertices)
    {
        float len = 0.0f;
        for (int i = 0; i < index.Length - 1; ++i) {
            len += Vector3.Distance (vertices [index [i]], vertices [index [i + 1]]);
        }
        return len;
    }
    // /*顶点连线的长度和*/
    public static float getLength (ArrayList indexList, ArrayList verticesList)
    {
        int[] index = (int[])indexList.ToArray (typeof(int));
        Vector3[] vertices = (Vector3[])verticesList.ToArray (typeof(Vector3));
        return getLength (index, vertices);
    }
    // /*顶点连线的长度和*/
    public static float getLength (Vector3[] vertices)
    {
        float len = 0.0f;
        for (int i = 0; i < vertices.Length - 1; ++i) {
            len += Vector3.Distance (vertices [i], vertices [i + 1]);
        }
        return len;
    }
    // /*顶点连线的长度和*/
    public static float getLength (ArrayList list)
    {
        Vector3[] vertices = (Vector3[])list.ToArray (typeof(Vector3));
        return getLength (vertices);
    }
    // /*处于同一个平面（法向为 n )的点集合 vertices 的凸包点索引集合 convex*/
    public static void convexInPlane (Vector3 n, Vector3[] vertices, ArrayList convex)
    {
        if (vertices.Length < 3)
            return;
        //将点所在的平面转到 xz 平面
        Quaternion q = Quaternion.FromToRotation (n, Vector3.up);
        Vector3[] tmpVertices = new Vector3[vertices.Length];
        int[] index = new int[vertices.Length];
        for (int i = 0; i < vertices.Length; i++) {
            tmpVertices [i] = q * vertices [i];
            index [i] = i;
        }
        //对点集合按照 x 坐标插入排序
        for (int j = 1; j < index.Length; j++) {
            float tmp = tmpVertices [index [j]].x;
            int tmpIndex = index [j];
            int i;
            for (i = j - 1; i >= 0; i--) {
                if (tmp < tmpVertices [index [i]].x)
                    index [i + 1] = index [i];
                else
                    break;
            }
            index [i + 1] = tmpIndex;
        }
        //存储下半部的凸集点索引
        ArrayList convexDown = new ArrayList ();
        convexDown.Add (index [0]);
        //baseVec: 上下的分界线
        Vector2 baseVec = new Vector2 (tmpVertices [index [index.Length - 1]].x - tmpVertices [index [0]].x,
		                               tmpVertices [index [index.Length - 1]].z - tmpVertices [index [0]].z);
        Vector2 checkVec = Vector2.zero;
        Vector2 currPos0 = new Vector2 (tmpVertices [index [0]].x, tmpVertices [index [0]].z);
        Vector2 currPos1 = new Vector2 (tmpVertices [index [0]].x, tmpVertices [index [0]].z);
        Vector2 currVec = Vector2.zero;
        Vector2 tmpVec = Vector2.zero;
        //计算上半部的凸集点索引
        convex.Add (index [0]);
        for (int i = 1; i < index.Length - 1; i++) {
            checkVec.Set (tmpVertices [index [i]].x - tmpVertices [index [0]].x,
                          tmpVertices [index [i]].z - tmpVertices [index [0]].z);         
            //检查当前点在base的上部还是下部
            //根据baseVec和currVec的叉积正负
            if (baseVec.x * checkVec.y - checkVec.x * baseVec.y >= 0) {
                tmpVec.Set (tmpVertices [index [i]].x - currPos0.x,
                            tmpVertices [index [i]].z - currPos0.y);              
                int top = convex.Count - 1;
                while (tmpVec.x * currVec.y - tmpVec.y * currVec.x < 0) {
                    convex.RemoveAt (top);
                    top--;
                    if (top == 0) {
                        currPos1.Set (tmpVertices [(int)convex [top]].x, tmpVertices [(int)convex [top]].z);
                        break;
                    }
                    currPos0.Set (tmpVertices [(int)convex [top - 1]].x, tmpVertices [(int)convex [top - 1]].z);
                    currPos1.Set (tmpVertices [(int)convex [top]].x, tmpVertices [(int)convex [top]].z);
                    currVec = currPos1 - currPos0;
                    tmpVec.Set (tmpVertices [index [i]].x - currPos0.x,
                                tmpVertices [index [i]].z - currPos0.y);                
                }
                if (currPos1.x != tmpVertices [index [i]].x || 
                    currPos1.y != tmpVertices [index [i]].z) {
                    currPos0.Set (currPos1.x, currPos1.y);
                    currPos1.Set (tmpVertices [index [i]].x, tmpVertices [index [i]].z);
                    currVec = currPos1 - currPos0;
                    convex.Add (index [i]);
                }
            } else {
                //如果在base的下部暂且存储待处理
                convexDown.Add (index [i]);
            }
        }
        convex.Add (index [index.Length - 1]);
        //计算下半部的凸集点索引
        currPos0.Set (tmpVertices [index [index.Length - 1]].x, 
                      tmpVertices [index [index.Length - 1]].z);
        currPos1.Set (currPos0.x, currPos0.y);
        currVec = Vector2.zero;
        for (int i = convexDown.Count - 1; i >= 0; i--) {
            tmpVec.Set (tmpVertices [(int)convexDown [i]].x - currPos0.x,
                        tmpVertices [(int)convexDown [i]].z - currPos0.y);
            int top = convex.Count - 1;
            while (tmpVec.x * currVec.y - tmpVec.y * currVec.x < 0) {
                convex.RemoveAt (top);
                top--;
                if (top == 0) {
                    currPos1.Set (tmpVertices [(int)convex [top]].x, tmpVertices [(int)convex [top]].z);
                    break;
                }
                currPos0.Set (tmpVertices [(int)convex [top - 1]].x, tmpVertices [(int)convex [top - 1]].z);
                currPos1.Set (tmpVertices [(int)convex [top]].x, tmpVertices [(int)convex [top]].z);
                currVec = currPos1 - currPos0;
                tmpVec.Set (tmpVertices [(int)convexDown [i]].x - currPos0.x,
                            tmpVertices [(int)convexDown [i]].z - currPos0.y);
            }
            if (currPos1.x != tmpVertices [(int)convexDown [i]].x || 
                currPos1.y != tmpVertices [(int)convexDown [i]].z) {
                currPos0.Set (currPos1.x, currPos1.y);          
                currPos1.Set (tmpVertices [(int)convexDown [i]].x, tmpVertices [(int)convexDown [i]].z);
                currVec = currPos1 - currPos0;
                convex.Add (convexDown [i]);
            }
        }
    }
    // /*处于同一个平面（法向为 n )的点集合 vertices 的凸包点索引集合 convex*/
    public static void convexInPlane (Vector3 n, ArrayList vertices, ArrayList convex)
    {
        Vector3[] ver = (Vector3[])vertices.ToArray (typeof(Vector3));
        convexInPlane (n, ver, convex);
    }
}
