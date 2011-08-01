#include "tyrFreeFormDef.h"
#include <glut.h>

using namespace tdio_library;
using namespace std;

float deltaTime = 0.01;
float totalTime = 0.0f;

tyrFreeFormDef::tyrFreeFormDef(void):l(4),m(4),n(4),bAnimate(false),deltaDir(1),speed(0.9)
{
}

tyrFreeFormDef::~tyrFreeFormDef(void)
{
}
void tyrFreeFormDef::dbgPaths(){
	bAnimate = true;
	

	int tl[2] = {0,4};
	int tm[2] = {0,4};
	int tn[2] = {0,4};

	for(int i = tl[0]; i <= tl[1]; i++){
		for(int j = tm[0]; j <= tm[1]; j++){
			for(int k=tn[0]; k <= tn[1]; k++){
				vector3 dir =controlPoints[i][j][k].getVec();
				vector3 dest;

				dir.y = 0.0f;
				Ray ray(controlPoints[i][j][k],dir);
				
				dest = ray.GetPoint(1.5f);

				controlPoints[i][j][k].bPath = true;
				controlPoints[i][j][k]._path.addFeaturePoint(controlPoints[i][j][k]);
				controlPoints[i][j][k]._path.addFeaturePoint(dest);
				controlPoints[i][j][k]._path.buildPath();
			}
		}
	}
}
bool tyrFreeFormDef::loadPly(const std::string file){
	printf("\n[FreeFormDeformation]\n");

	if(!Reader::ReadPLY(file.c_str(),_ply)){
		printf("  - Could not load %s\n",file.c_str());
		return false;
	}
	Face *faces = _ply.GetFaces();
	int nFace = _ply.GetNumFaces();

	printf("  - Loaded ply.  nVtx: %d  nFace: %d\n",_ply.GetNumVertices(),_ply.GetNumFaces());
	printf("  - Min: %f %f %f\n",_ply.GetMin().x,_ply.GetMin().y,_ply.GetMin().z);
	printf("  - Max: %f %f %f\n",_ply.GetMax().x,_ply.GetMax().y,_ply.GetMax().z);

	reParamVertices(vtxParam,S,T,U);
	printf("  - S[%f %f %f]\n",S.x,S.y,S.z);
	printf("  - T[%f %f %f]\n",T.x,T.y,T.z);
	printf("  - U[%f %f %f]\n",U.x,U.y,U.z);

	return true;
}
void tyrFreeFormDef::transformVertices(){
	vtxTransform.clear();
	for(int i = 0; i < vtxParam.size(); i++){
		vector3 P = getGlobalVertice(vtxParam[i],S,T,U);
		vtxTransform.push_back(P);
	}
}
bool tyrFreeFormDef::saveObj(const std::string file){
	Face *faces = _ply.GetFaces();
	int nFace = _ply.GetNumFaces();
	transformVertices();

	FILE *stream = fopen(file.c_str(),"w");
	for(int i = 0; i < vtxTransform.size(); i++){
		fprintf(stream,"v %f %f %f\n",vtxTransform[i].x,vtxTransform[i].y,vtxTransform[i].z);
	}
	for(int i = 0; i < nFace; i++){
		fprintf(stream,"f %d %d %d\n",faces[i].verts[0]+1,faces[i].verts[1]+1,faces[i].verts[2]+1);
	}
	fclose(stream);
}
double tyrFreeFormDef::binom_coeff4(int k){
	///Manually calculated binomial coefficients
	if(k==0||k==4)return 1.0f;
	else if(k==1||k==3)return 4.0f;
	else if(k==2)return 6.0f;
	else return 0.0f;
}	
double tyrFreeFormDef::bern_poly(int n, int v, double x){
	///bernstein polynomial expansion (currently only supports ranges of 0 to 4)
	return binom_coeff4(v) * pow(x,(double)v) *  pow((1.0f-x),(double)(n-v));
}
vector3 tyrFreeFormDef::createControlPoint(vector3 p0,int i,int j, int k,const vector3 &S,const vector3 &T,const vector3 &U){
	double tl = (double)l;
	double tm = (double)m;
	double tn =	(double)n;
	
	vector3 tmpT = T;
	return p0 + ((double)i/tl * S) + ((double)j/tm * T) + ((double)k/tn * U);
}
vector3 tyrFreeFormDef::getGlobalVertice(RectCoord &r,vector3 &S,vector3 &T,vector3 &U){
	double s = r.s;
	double t = r.t;
	double u = r.u;

	vector3 tS(0.0f,0.0f,0.0f);
	for(int i = 0; i <= l; i++){
		
		vector3 tM(0.0f,0.0f,0.0f);
		for(int j = 0; j <= m; j++){
			
			vector3 tK(0.0f,0.0f,0.0f);
			for(int k = 0; k <= n; k++){
				//tK += bern_poly(n,k,u) * controlPoints[i][j][k];
				tK += r.bernPolyPack[n][k][2] * controlPoints[i][j][k];
			}
			//tM += bern_poly(m,j,t) * tK;
			tM += r.bernPolyPack[m][j][1] * tK;
		}
		//tS += bern_poly(l,i,s) * tM;
		tS += r.bernPolyPack[l][i][0] * tM;
	}
	return tS;
}
void tyrFreeFormDef::reParamVertices(vector<RectCoord> &param, vector3 &S,vector3 &T, vector3 &U){
	printf("  - Reparameterizing vertices\n");

	int nVtx = _ply.GetNumVertices();
	point3D_t *vtx = _ply.GetVertices();

	vector3 min = _ply.GetMin();
	vector3 p0 = min;
	vector3 max = _ply.GetMax();

	S = vector3(max.x - min.x,0.0,0.0);
	T = vector3(0.0,max.y - min.y,0.0);
	U = vector3(0.0,0.0,max.z - min.z);


	vector3 TcU = T.Cross(U);
	vector3 ScU = S.Cross(U);
	vector3 ScT = S.Cross(T);

	double TcUdS = TcU.Dot(S);
	double ScUdT = ScU.Dot(T);
	double ScTdU = ScT.Dot(U);

	for(int v = 0; v < nVtx; v++){
		vector3 diff = vtx[v].ToVector3() - p0;

		RectCoord tmp;
		tmp.s = TcU.Dot(diff/TcUdS);
		tmp.t = ScU.Dot(diff/ScUdT);
		tmp.u = ScT.Dot(diff/ScTdU);
		tmp.p = p0 + (tmp.s * S) + (tmp.t * T) + (tmp.u * U);
		tmp.p0 = p0;
		
		{	///Pre-calculate bernstein polynomial expansion.  It only needs to be done once per parameterization
			for(int i = 0; i <= 5; i++){
				for(int j = 0; j <= 5; j++){
					
					for(int k = 0; k <= 5; k++){
						tmp.bernPolyPack[n][k][2] = bern_poly(n,k,tmp.u);
					}
					tmp.bernPolyPack[m][j][1] = bern_poly(m,j,tmp.t);
				}
				tmp.bernPolyPack[l][i][0] = bern_poly(l,i,tmp.s);
			}
		}
		param.push_back(tmp);
		if(tmp.p.Dist(vtx[v].ToVector3()) > Math::EPSILON){
			printf("     - Warning, vtx[%d] does not match it's parameterization.\n",v);
		}
	}
	for(int i = 0; i <= 5; i++){
		for(int j = 0; j <= 5; j++){
			vector3 tK(0.0f,0.0f,0.0f);
			for(int k = 0; k <= 5; k++){
				controlPoints[i][j][k] = createControlPoint(min,i,j,k,S,T,U);
				
			}
		}
	}
}
/*Update the animation sequence*/
void tyrFreeFormDef::updateAnimation(double dt){
	if(bAnimate){

		totalTime += (speed*dt)*deltaDir;
		if(totalTime >= 1.0f){
			totalTime = 1.0f;
			deltaDir *= -1.0f;
		} else if(totalTime <= 0.0f){
			deltaDir *= -1.0f;
			totalTime = 0.0f;
		}
		
		for(int i = 0; i <= 5; i++){
			for(int j = 0; j <= 5; j++){
				vector3 tK(0.0f,0.0f,0.0f);
				for(int k = 0; k <= 5; k++){
					controlPoints[i][j][k].update(totalTime);
					
				}
			}
		}
		transformVertices();
	}
}
void tyrFreeFormDef::render(double dt){
	
	glColor3f(0.5f, 0.5f, 0.5f);

	Face *faces = _ply.GetFaces();
	int nFace = _ply.GetNumFaces();


	for(int f = 0; f< nFace; f++){
		vector3 pt[3];
		pt[0] = vtxTransform[faces[f].verts[0]];
		pt[1] = vtxTransform[faces[f].verts[1]];
		pt[2] = vtxTransform[faces[f].verts[2]];
		
		vector3 edge1 = pt[1] - pt[0];
		vector3 edge2 = pt[2] - pt[0];
		vector3 normal = edge1.Cross(edge2);
		normal.Normalize();
		
		
		glBegin(GL_TRIANGLES);
			glNormal3f(normal.x,normal.y,normal.z);
			glVertex3f(pt[0].x,pt[0].y,pt[0].z);
			glVertex3f(pt[1].x,pt[1].y,pt[1].z);
			glVertex3f(pt[2].x,pt[2].y,pt[2].z);
		glEnd();
	}
/*
	for(int i = 0; i <= l; i++){
		for(int j = 0; j <= m; j++){
			for(int k = 0; k <= n; k++){
				glPushMatrix();
					glTranslatef(controlPoints[i][j][k].x,controlPoints[i][j][k].y,controlPoints[i][j][k].z);
					glColor3f(1.0f, 0.0f, 0.0f);
					glutSolidSphere(0.2,10,10);
				glPopMatrix();
			}
		}
	}
*/
}
bool tyrFreeFormDef::setControlPoint(int i,int j,int k,tdio_library::vector3 v){
	controlPoints[i][j][k] = v;
	return true;
}
bool tyrFreeFormDef::transControlPoint(int i,int j,int k,tdio_library::vector3 v){
	controlPoints[i][j][k] += v;
	return true;
}