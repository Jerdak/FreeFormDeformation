#include "stdafx.h"
#include "tyrNode.h"

using namespace std;
using namespace tdio_library;

tyrNode::tyrNode(void):_parent(NULL),_bTriggerParentUpdate(false)
{
	_localPosition = Vector3::ZERO;
	_localOrientation = quaternion::IDENTITY;
	_globalOrientation = quaternion::IDENTITY;

	_color = rgb_l(0.5,0.5,0.5);
	_name = "n/a";
}

tyrNode::~tyrNode(void)
{
	//clearChildren();
	//clearObjects();
}

void tyrNode::addChild(tyrNode *node){
	printf("Adding child %s to %s\n",node->getName().c_str(),_name.c_str());
	_childNodes.push_back(node);
	node->setParent(this);
	node->update();
}
void tyrNode::addObject(tyrObject *obj){
	_objects.push_back(obj);
}

void tyrNode::clearChildren(){
	for(int i = 0; i < _childNodes.size(); i++){
		tyrNode *tmp = _childNodes[i];
		if(tmp){delete tmp; tmp = NULL;}
	}
	_childNodes.clear();
}
void tyrNode::clearObjects(){
	for(int i = 0; i < _objects.size(); i++){
		tyrObject *tmp = _objects[i];
		if(tmp){delete tmp; tmp = NULL;}
	}
	_objects.clear();
}

void ToRealArray4_2(Matrix3 mat,float ret[16]){
	for(int r = 0; r < 3; r++){
		for(int c = 0; c < 3; c++){
			ret[c * 4 + r] = mat.m[r][c];
		}
	}
	ret[3] = ret[7] = ret[11] = ret[12] = ret[13] = ret[14] =  0.0f;
	ret[15] = 1.0f;
}
void tyrNode::render(double dt){
	quaternion o	= _globalOrientation;
	Vector3 p		= _globalPosition;
	Matrix3 mat;
	float realMat[16];

	o.ToRotationMatrix(mat);
	//mat.ToRealArray4(realMat);
	ToRealArray4_2(mat,realMat);

	glPushMatrix();
		glColor3f(_color.r,_color.g,_color.b);

		glTranslatef(p.x,p.y,p.z);
		glMultMatrixf(realMat);
		
		for(int i = 0; i < _objects.size(); i++){
			tyrObject *tmp = _objects[i];
			if(tmp)tmp->render(dt);
		}
	glPopMatrix();

		for(int i = 0; i < _childNodes.size(); i++){
			tyrNode *tmp = _childNodes[i];
			tmp->render(dt);
		}
	
	
}
void tyrNode::rotate(const quaternion& q,Rotation_Center r){
	// Normalise quaternion to avoid drift
	quaternion qnorm = q;
	qnorm.Normalize();
	//printf("[%s] RPY Before: %f %f %f\n",_name.c_str(),_globalOrientation.getRoll(),_globalOrientation.getPitch(),_globalOrientation.getYaw());

    switch(r){
		case RC_PARENT:
			// Rotations are normally relative to local axes, transform up
			_localOrientation = qnorm * _localOrientation;
			break;
		case RC_WORLD:
			// Rotations are normally relative to local axes, transform up
			_localOrientation = _localOrientation * getGlobalOrientation().Inverse()* qnorm * getGlobalOrientation();
			break;
		case RC_LOCAL:
			// Note the order of the mult, i.e. q comes after
			_localOrientation = _localOrientation * qnorm;
			break;
    }
    update();


}
void tyrNode::translate(const Vector3& v,Rotation_Center r){
//	printf("\n\n");
	Vector3 tVec;
//	printf("Local Position Before: %f %f %f\n",_localPosition.x,_localPosition.y,_localPosition.z);
    switch(r){
		case RC_PARENT:
			tVec = (_localOrientation * v);
			// Rotations are normally relative to local axes, transform up
			_localPosition = _localPosition + (_localOrientation * v);
			break;
		case RC_WORLD:
			 // position is relative to parent so transform upwards
            if (_parent) {
		        _localPosition += (_parent->getGlobalOrientation().Inverse() * v);
            } else {
		        _localPosition = _localPosition + v;
            }
			break;
		case RC_LOCAL:
			// Note the order of the mult, i.e. q comes after
			 _localPosition = _localPosition + v;
			break;
    }

    update();
}
void tyrNode::updateChildren(){
//	printf("  - Updating children[%d]\n",_childNodes.size());
	for(int i = 0; i < _childNodes.size(); i++){
		tyrNode *node = _childNodes[i];
		node->update();
	}
}
void tyrNode::update(){
	applyParent();
	updateChildren();
}
void tyrNode::applyParent(void) 
{
    if (_parent)
    {
        // Update orientation
        const quaternion& parentOrientation = _parent->getGlobalOrientation();  
		_globalOrientation = parentOrientation * _localOrientation;
    
        // Change position vector based on parent's orientation
        _globalPosition = (parentOrientation * _localPosition);

        // Add altered position vector to parents
        _globalPosition += _parent->getGlobalPosition();
    } else {
		_globalOrientation	= _localOrientation; 
		_globalPosition		= _localPosition;
    }
	printf("[%s] Pos: %f %f %f\n",_name.c_str(),_globalPosition.x,_globalPosition.y,_globalPosition.z);
	printf("[%s] RPY: %f %f %f\n",_name.c_str(),_globalOrientation.GetRoll(),_globalOrientation.GetPitch(),_globalOrientation.GetYaw());

}