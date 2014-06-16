using UnityEngine;
using System.Collections;

public class Bezier
{
    // The control points set of Bezier Curve
    private Vector3[] controlPoints;
    // n th Power Bezier Curve
    private int nPower;
    // Combination Number
    private int[] combNumbers;
    public Bezier (Vector3[] c_p)
    {
        nPower = -1;
        setControlPoints (c_p);
    }
	
    public void setControlPoints (Vector3[] newCP)
    {
        controlPoints = newCP;
        if (newCP.Length - 1 == nPower)
            return;
        nPower = newCP.Length - 1;
        combNumbers = new int[nPower / 2 + 1];
        for (int i = 0; i < combNumbers.Length; i++) {
            combNumbers [i] = -1;
        }
    }
	
    public Vector3 getBezierPoint (float u)
    {
        Vector3 r = Vector3.zero;
        for (int i = 0; i < controlPoints.Length; i++) {
            r += basicFunc (i, u) * controlPoints [i];
        }
        return r;
    }
	
    private float basicFunc (int index, float u)
    {
        return combination (nPower, index) * 
            Mathf.Pow (u, index) * 
            Mathf.Pow (1 - u, nPower - index);
    }
	
    private int combination (int down, int up)
    {
        if (up > down / 2)
            up = down - up;
        if (up > down) {
            return -1;
        } else if (up == down || up == 0) {
            return 1;
        } else {
            if (combNumbers [up] != -1) {
                return combNumbers [up];
            }
        }
        combNumbers [up] = combination (down - 1, up) + combination (down - 1, up - 1);
        return combNumbers [up];
    }
}