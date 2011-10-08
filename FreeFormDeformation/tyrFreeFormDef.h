#pragma once

#include <vector>
#include <string>
#include <3DIO/3DIO.h>

#include "tyrPather.h"
#include "tyrNode.h"
///Coordinate in s,t,u space.
class RectCoord {
public:
	RectCoord():s(0.0),t(0.0),u(0.0),p(tdio_library::Vector3(0.0,0.0,0.0)),p0(tdio_library::Vector3(0.0,0.0,0.0)){}
	RectCoord(const RectCoord &r){*this = r;}

	void operator=(const RectCoord &r){
		s = r.s;
		t = r.t;
		u = r.u;
		p = r.p;
		p0 = r.p0;
		
		for(int i = 0; i < 5; i++){
			for(int j = 0; j < 5; j++){
				for(int k = 0; k < 3; k++){
					bernPolyPack[i][j][k] = r.bernPolyPack[i][j][k];
				}
			}
		}
	}
	///bernstein polynomial packing 
	double bernPolyPack[6][6][3];

	///Point after applying s,t,u to p0, should result in original point
	tdio_library::Vector3 p;

	///Origin
	tdio_library::Vector3 p0;
	
	///Distances along S/T/U axes
	double s,t,u;
};
class vector3_pathd : public tdio_library::Vector3 {
public:
	vector3_pathd():bPath(false){}
	~vector3_pathd(){}

	tdio_library::Vector3 getPoint(double dt){
		if(bPath){
			tdio_library::Vector3 ret;
			double offset;
			_path.getPoint(dt,ret,offset);
			
			return ret;
		} else {
			return tdio_library::Vector3(x,y,z);
		}
	}
	void update(double dt){
		if(bPath){
			tdio_library::Vector3 ret;
			double offset;
			_path.getPoint(dt,ret,offset,0,1);
			*this = ret;
		} 
	}
	tdio_library::Vector3 getVec()const{return Vector3(x,y,z);}
	void operator=(const tdio_library::Vector3 &v){
		x = v.x;
		y = v.y;
		z = v.z;

		originalVec = v;
	}
	void operator=(const vector3_pathd &v){
		x = v.x;
		y = v.y;
		z = v.z;

		originalVec = v.getVec();

	}
	bool bPath;
	Tyr::tyrPather _path;
	tdio_library::Vector3 originalVec;
};
class tyrFreeFormDef : public tyrObject
{
public:
	tyrFreeFormDef(void);
	~tyrFreeFormDef(void);

	/*Load ply file*/
	bool loadPly(const std::string file);
	
	/*Load obj file*/
	bool saveObj(const std::string file);

	/*Get control poiint*/
	bool getControlPoint(int i,int j,int k);
	
	std::vector<tdio_library::Vector3> *getModifiedVertices(){return &vtxTransform;}
		
	/*Set control point*/
	bool setControlPoint(int i,int j,int k,tdio_library::Vector3 v);
	
	/*Translate control point*/
	bool transControlPoint(int i,int j,int k,tdio_library::Vector3 v);

	/*Apply bezier interpolation to original points*/
	void transformVertices();

	/*OpenGL rendering call*/
	void render(double dt,bool bDrawControls = false);

	/*Update the animation sequence*/
	void updateAnimation(double dt);

	void dbgPaths();

	
protected:
	///bernstein polynomial
	static double bern_poly(int n, int v, double x);
	
	///fast binomial coefficient for up to 4 factorial
	static double binom_coeff4(int k);

private:

	///Reparameterize the vertices in to s,t,u space (position related to global axes)
	void reParamVertices(std::vector<RectCoord> &param, tdio_library::Vector3 &S,tdio_library::Vector3 &T, tdio_library::Vector3 &U);
	
	///Get global vertice through Bezier interpolation
	tdio_library::Vector3 getGlobalVertice(RectCoord &r,tdio_library::Vector3 &S,tdio_library::Vector3 &T,tdio_library::Vector3 &U);
	
	///Create a new control point
	tdio_library::Vector3 createControlPoint(tdio_library::Vector3 p0,int i,int j, int k,const tdio_library::Vector3 &S,const tdio_library::Vector3 &T,const tdio_library::Vector3 &U);

	///Raw ply data
	tdio_library::Object<tdio_library::PLY> _ply;
	
	///World axes (don't have to be orthogonal but we use them as such)
	tdio_library::Vector3 S,T,U;

	///Control points
	vector3_pathd controlPoints[6][6][6];
	
	///Reparamterized vertices
	std::vector<RectCoord> vtxParam;

	///Bezier transformed vertices
	std::vector<tdio_library::Vector3> vtxTransform;
	
	///Max range along S,T,U axes
	int l,m,n;

	bool bAnimate;

	int deltaDir;

	double speed;
};
