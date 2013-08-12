using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// Helper class that represents a parameterized vertex
/// </summary>
public class Vector3Param {
	///bernstein polynomial packing 
	public List<List<float>> bernPolyPack;

	///Point after applying s,t,u to p0, should result in original point
	public Vector3 p = Vector3.zero;

	///Origin
	public Vector3 p0 = Vector3.zero;

	///Distances along S/T/U axes
	public float s,t,u;

	public Vector3Param()
	{
		s = 0.0f;
		t = 0.0f;
		u = 0.0f;
	}

	public Vector3Param(Vector3Param v)
	{
		s = v.s;
		t = v.t;
		u = v.u;
		p = v.p;
		p0 = v.p0;
	}

};

/// <summary>
/// Free form deformation class
/// 
/// Based off of the paper 'Free-Form Deformation of Solid Geometric Models'[1] this class
/// creates a system of control points that can deform a mesh as if that mesh was embedded
/// in a flexible parallelpiped.
/// 
/// Confused?  Yeah, who uses the term parallelpiped.  The idea is to create a uniformly spaced
/// grid of control points around some mesh.  Each control point has some effect on the mesh, like
/// bone weighting in animation.  The effect each control point has is directly proportional to the
/// total number of control points in the entire grid, each exerts some control.
/// 
/// [1] - http://pages.cpsc.ucalgary.ca/~blob/papers/others/ffd.pdf
/// </summary>
public class FreeFormDeformer : MonoBehaviour {

	/// <summary>
	/// Allow FixedUpdate to modify the mesh.
	/// </summary>
	public bool AllowMeshUpdate = false;

	/// <summary>
	/// Target to be morphed
	/// </summary>
	Mesh MorphTarget = null;

	/// <summary>
	/// Target to be filtered (assumed to contain a meshfilter and valid mesh)
	/// </summary>
	public MeshFilter MorphTargetFilter = null;

	/// <summary>
	/// Update frequency in seconds
	/// </summary>
	public float UpdateFrequency = 1.0f;

	/// <summary>
	/// Game object to represent a control point.  Can be anything really, I suggest spheres.
	/// </summary>
	public GameObject ControlPointPrefab;

	/// <summary>
	/// Local coordinate system
	/// </summary>
	Vector3 S, T, U;

	/// <summary>
	/// Number of controls for S, T, & U respectively.  (L,M, and N MUST be >= 1)
	/// </summary>
	public int L=1, M=1, N=1;

	/// <summary>
	/// Time elapsed since last update
	/// </summary>
	float elapsedTime = 0.0f;

	/// <summary>
	/// Grid of controls points. Stored as 3D grid for easier because width,height, and depth can randomly vary. 
	/// </summary>
	GameObject[, ,] controlPoints;

	/// <summary>
	/// Original vertices from MorphTarget
	/// </summary>
	Vector3[] originalVertices;

	/// <summary>
	/// Current updated vertices for MorphTarget
	/// </summary>
	Vector3[] transformedVertices;


	/// <summary>
	/// Vertex parameters
	/// 
	/// Each vertex is given a set of parameters that will define
	/// its final position based on a local coordinate system.
	/// </summary>
	List<Vector3Param> vertexParams = new List<Vector3Param>();

	void Start () {
		MorphTarget = MorphTargetFilter.mesh ;
		originalVertices = MorphTarget.vertices;
		transformedVertices = new Vector3[originalVertices.Length];

		Parameterize();
	}

	/// <summary>
	/// Calculate a binomial coefficient using the multiplicative formula
	/// </summary>
	float binomialCoeff(int n, int k){
		float total = 1.0f;
		for(int i = 1; i <= k; i++){
			total *= (n - (k - i)) / (float)i;
		}
		return total;
	}

	/// <summary>
	/// Calculate a bernstein polynomial
	/// </summary>
	float bernsteinPoly(int n, int v, float x)
	{
		return binomialCoeff(n,v) * Mathf.Pow(x, (float)v) * Mathf.Pow((float)(1.0f - x), (float)(n - v));
	}

	/// <summary>
	/// Calculate local coordinates
	/// </summary>
	void calculateSTU(Vector3 max, Vector3 min){
		S = new Vector3(max.x - min.x, 0.0f, 0.0f);
		T = new Vector3(0.0f, max.y - min.y, 0.0f);
		U = new Vector3(0.0f, 0.0f, max.z - min.z);
	}

	/// <summary>
	/// Calculate the trivariate bernstein polynomial as described by [1]
	/// 
	/// My method adapts [1] slightly by precalculating the BP coefficients and storing
	/// them in Vector3Param.  When it comes time to extract a world coordinate, 
	/// it's just a matter of summing up multiplications through each polynomial from eq (2).
	/// </summary>
	/// <links>
	///  [1] - Method based on: http://pages.cpsc.ucalgary.ca/~blob/papers/others/ffd.pdf
	/// </links>
	/// <param name="p0">Origin of our coordinate system (where STU meet)</param>
	void calculateTrivariateBernsteinPolynomial(Vector3 p0){
		Vector3 TcU = Vector3.Cross(T, U);
		Vector3 ScU = Vector3.Cross(S, U);
		Vector3 ScT = Vector3.Cross(S, T);

		float TcUdS = Vector3.Dot(TcU, S);
		float ScUdT = Vector3.Dot(ScU, T);
		float ScTdU = Vector3.Dot(ScT, U);

		for (int v = 0; v < originalVertices.Length; v++)
		{
			Vector3 diff = originalVertices[v] - p0;

			Vector3Param tmp = new Vector3Param();
			tmp.s = Vector3.Dot(TcU, diff / TcUdS);
			tmp.t = Vector3.Dot(ScU, diff / ScUdT);
			tmp.u = Vector3.Dot(ScT, diff / ScTdU);
			tmp.p = p0 + (tmp.s * S) + (tmp.t * T) + (tmp.u * U);
			tmp.p0 = p0;
			tmp.bernPolyPack = new List<List<float>>();

			{	// Reserve room for each bernstein polynomial pack.
				tmp.bernPolyPack.Add(new List<float>(L));	//outer bernstein poly
				tmp.bernPolyPack.Add(new List<float>(M));	//middle bernstein poly
				tmp.bernPolyPack.Add(new List<float>(N));	//inner bernstein poly
			}

			{	// Pre-calculate bernstein polynomial expansion.  It only needs to be done once per parameterization
				for (int i = 0; i <= L; i++)
				{
					for (int j = 0; j <= M; j++)
					{
						for (int k = 0; k <= N; k++)
						{
							tmp.bernPolyPack[2].Add(bernsteinPoly(N, k, tmp.u));
						}
						tmp.bernPolyPack[1].Add(bernsteinPoly(M, j, tmp.t));
					}
					tmp.bernPolyPack[0].Add(bernsteinPoly(L, i, tmp.s));
				}
			}
			vertexParams.Add(tmp);
			if (Vector3.Distance(tmp.p, originalVertices[v]) > 0.001f)
			{
				//Debug.Log("Warning, mismatched parameterization");
			}
		}
	}

	/// <summary>
	/// Parameterize MorphTarget's vertices
	/// </summary>
	void Parameterize(){
		Vector3 min = new Vector3(Mathf.Infinity,Mathf.Infinity,Mathf.Infinity);
		Vector3 max = new Vector3(-Mathf.Infinity,-Mathf.Infinity,-Mathf.Infinity);
		foreach(Vector3 v in originalVertices){
			max = Vector3.Max(v,max);
			min = Vector3.Min(v,min);
		}
		calculateSTU(max, min);
		calculateTrivariateBernsteinPolynomial(min);
		createControlPoints(min);
	}


	/// <summary>
	/// Create grid of control points.
	/// </summary>
	void createControlPoints(Vector3 origin){
		controlPoints = new GameObject[L + 1, M + 1, N + 1];
		for(int i = 0; i <= L; i++){
			for(int j = 0; j <= M; j++){
				for(int k = 0; k <= N; k++){
					controlPoints[i, j, k] = createControlPoint(origin, i, j, k);
				}
			}
		}
	}

	/// <summary>
	/// Create a single control point.
	/// </summary>
	GameObject createControlPoint(Vector3 p0, int i, int j, int k)
	{
		Vector3 position = p0 + (i / (float)L * S) + (j / (float)M * T) + (k / (float)N * U);
		return (GameObject)Instantiate(ControlPointPrefab, position, Quaternion.identity);
	}

	/// <summary>
	/// Convert parameterized vertex in to a world coordinate
	/// </summary>
	Vector3 getWorldVector3(Vector3Param r){
		int l = L;
		int m = M;
		int n = N;

		Vector3 tS = Vector3.zero;
		for(int i = 0; i <= l; i++){
			
			Vector3 tM = Vector3.zero;
			for(int j = 0; j <= m; j++){

				Vector3 tK = Vector3.zero;
				for(int k = 0; k <= n; k++){
					tK += r.bernPolyPack[2][k] * controlPoints[i,j,k].transform.position;
				}
				tM += r.bernPolyPack[1][j] * tK;
			}
			tS += r.bernPolyPack[0][i] * tM;
		}
		return tS;
	}


	void UpdateMesh(){
		elapsedTime = 0.0f;

		int idx = 0;
		foreach(Vector3Param vp in vertexParams){
			Vector3 p = getWorldVector3(vp);
			transformedVertices[idx++] = p;
		}

		MorphTarget.vertices = transformedVertices;
		MorphTarget.RecalculateBounds();
		MorphTarget.RecalculateNormals();
		MorphTarget.Optimize();
	}

	void FixedUpdate()
	{
		elapsedTime += Time.fixedDeltaTime;
		if (AllowMeshUpdate)
		{
			if (elapsedTime >= UpdateFrequency) UpdateMesh();
		}
	}
	// Update is called once per frame
	void Update () {
	
	}
}
