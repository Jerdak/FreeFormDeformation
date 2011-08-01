#pragma once

#include <3DIO/3DIO.h>
#include <vector>
#include <string>
#include "tyrPather.h"
#include "tyrNode.h"

class tyrBarrShape : public tyrObject
{
public:
	tyrBarrShape(void);
	~tyrBarrShape(void);

	void dbgAnimation();

	/*Load ply file*/
	bool loadPly(const std::string file);
	
	/*Load obj file*/
	bool saveObj(const std::string file);

	/*Render*/
	void render(double dt);

	void transformVertices();
	void chainTransformVertices(std::vector<tdio_library::vector3> *vtx);

	void updateAnimation(double dt,bool manualApply=false);
	void twist(double angle){_twistAngle += angle;}

protected:
	bool bAnimate;

	double _twistAngle;

	float _elapsedTime,_deltaTime;

	///Raw ply data
	tdio_library::Object<tdio_library::PLY> _ply;

	///Bezier transformed vertices
	std::vector<tdio_library::vector3> vtxTransform;

	Tyr::tyrPather _path;

	int deltaDir;

	double speed;
};
