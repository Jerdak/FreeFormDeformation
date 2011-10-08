#include "stdafx.h"
#include "tyrBarrShape.h"


using namespace tdio_library;
using namespace std;

tyrBarrShape::tyrBarrShape(void):_elapsedTime(0.0f),_deltaTime(0.01f),_twistAngle(0.0),bAnimate(false),deltaDir(1),speed(0.9f)
{
}

tyrBarrShape::~tyrBarrShape(void)
{
}
void tyrBarrShape::dbgAnimation(){
	bAnimate = true;
	_path.addFeaturePoint(Vector3(0,0,0));
	_path.addFeaturePoint(Vector3(0,360,0));
	_path.buildPath();
}
bool tyrBarrShape::loadPly(const std::string file){
	printf("\n[Barr Shape ply loader]\n");

	if(!Reader::ReadPLY(file.c_str(),_ply)){
		printf("  - Could not load %s\n",file.c_str());
		return false;
	}
	transformVertices();
	return true;
}
void tyrBarrShape::chainTransformVertices(std::vector<tdio_library::Vector3> *vtx){
	vtxTransform.clear();
	
	Vector3 min = _ply.GetMin();
	Vector3 max = _ply.GetMax();
	Vector3 range = max - min;


	for(int i = 0; i < vtx->size(); i++){
		Vector3 p = (*vtx)[i];
		double offset = p.y - min.y;
		offset /= (max.y-min.y);
		
		Matrix3 mat;
		mat.FromEulerAnglesXYZ(0,_twistAngle * offset,0);

		Vector3 p2 = mat * p;
		vtxTransform.push_back(p2);
		//Vector3 P = getGlobalVertice(vtxParam[i],S,T,U);
		//vtxTransform.push_back(P);
	}
}
void tyrBarrShape::transformVertices(){
	vtxTransform.clear();
	point3D_t *vtx = _ply.GetVertices();
	int nVtx = _ply.GetNumVertices();

	Vector3 min = _ply.GetMin();
	Vector3 max = _ply.GetMax();
	Vector3 range = max - min;


	for(int i = 0; i < nVtx; i++){
		Vector3 p = vtx[i];
		double offset = p.y - min.y;
		offset /= (max.y-min.y);
		
		Matrix3 mat;
		mat.FromEulerAnglesXYZ(0,_twistAngle * offset,0);

		Vector3 p2 = mat * p;
		vtxTransform.push_back(p2);

	}
}

void tyrBarrShape::updateAnimation(double dt,bool manualApply){
	if(bAnimate){
		_elapsedTime += (speed*dt) * deltaDir;
		if( _elapsedTime >= 1.0f){
			deltaDir *= -1.0f;
			_elapsedTime = 1.0f;
		} else if(_elapsedTime <= 0.0f){
			deltaDir *= -1.0f;
			_elapsedTime = 0.0f;
		}
		Vector3 p;
		double offset;
		_path.getPoint(_elapsedTime,p,offset,0,1);

		_twistAngle = p.y;
		if(!manualApply)transformVertices();
	}

}
void tyrBarrShape::render(double dt){

	glColor3f(0.5f, 0.5f, 0.5f);
		
	Face *faces = _ply.GetFaces();
	int nFace = _ply.GetNumFaces();

	for(int f = 0; f< nFace; f++){
		Vector3 pt[3];
		pt[0] = vtxTransform[faces[f].verts[0]];
		pt[1] = vtxTransform[faces[f].verts[1]];
		pt[2] = vtxTransform[faces[f].verts[2]];
		
		Vector3 edge1 = pt[1] - pt[0];
		Vector3 edge2 = pt[2] - pt[0];
		Vector3 normal = edge1.Cross(edge2);
		normal.Normalize();
		
		
		glBegin(GL_TRIANGLES);
			glNormal3f(normal.x,normal.y,normal.z);
			glVertex3f(pt[0].x,pt[0].y,pt[0].z);
			glVertex3f(pt[1].x,pt[1].y,pt[1].z);
			glVertex3f(pt[2].x,pt[2].y,pt[2].z);
		glEnd();
	}
}