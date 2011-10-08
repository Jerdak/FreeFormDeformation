#pragma once

#include <3DIO/3DIO.h>
#include <vector>
#include <glut.h>

class tyrObject {
public:
	tyrObject(){}
	~tyrObject(){}

	virtual void render(double dt){
		tdio_library::point3D_t *vtx = _ply.GetVertices();

		tdio_library::Face *faces = _ply.GetFaces();
		int nFace = _ply.GetNumFaces();

		for(int f = 0; f< nFace; f++){
			tdio_library::Vector3 pt[3];
			pt[0] = vtx[faces[f].verts[0]];
			pt[1] = vtx[faces[f].verts[1]];
			pt[2] = vtx[faces[f].verts[2]];
			
			tdio_library::Vector3 edge1 = pt[1] - pt[0];
			tdio_library::Vector3 edge2 = pt[2] - pt[0];
			tdio_library::Vector3 normal = edge1.Cross(edge2);
			normal.Normalize();
			
			
			glBegin(GL_TRIANGLES);
				glNormal3f(normal.x,normal.y,normal.z);
				glVertex3f(pt[0].x,pt[0].y,pt[0].z);
				glVertex3f(pt[1].x,pt[1].y,pt[1].z);
				glVertex3f(pt[2].x,pt[2].y,pt[2].z);
			glEnd();
		}
	}
	virtual bool loadPly(const std::string file){
		if(!tdio_library::Reader::ReadPLY(file.c_str(),_ply)){
			printf("  - Could not load %s\n",file.c_str());
			return false;
		}
		return true;
	}
protected:
	///Raw ply data
	tdio_library::Object<tdio_library::PLY> _ply;
};
class tyrNode
{
public:
	enum Rotation_Center {RC_WORLD,RC_LOCAL,RC_PARENT};
	tyrNode(void);
	~tyrNode(void);

	void addChild(tyrNode *node);
	void addObject(tyrObject *obj);
	void removeChild(tyrNode *node){}

	void clearChildren();
	void clearObjects();

	std::string getName(){return _name;}

	void setOrientation(tdio_library::quaternion q){_localOrientation = q; _localOrientation.Normalize(); update();}
	void setPosition(tdio_library::Vector3 v){_localPosition = v; update();}

	tdio_library::quaternion getGlobalOrientation(){return _globalOrientation;}
	tdio_library::quaternion getLocalOrientation(){return _localOrientation;}

	tdio_library::Vector3 getGlobalPosition(){return _globalPosition;}
	tdio_library::Vector3 getLocalPosition(){return _localPosition;}

	void setParent(tyrNode *node){_parent = node;}
	void setColor(tdio_library::rgb_l c){_color = c;}	
	void setName(std::string n){_name = n;}

	void render(double dt);

	void translate(const tdio_library::Vector3& v,Rotation_Center r = RC_PARENT);
	void rotate(const tdio_library::quaternion& q,Rotation_Center r = RC_PARENT);

	void update();
protected:
	void applyParent();
	void requestUpdate();
	void updateChildren();

private:
	tdio_library::quaternion _localOrientation;
	tdio_library::quaternion _globalOrientation;

	tdio_library::Vector3 _localPosition;
	tdio_library::Vector3 _globalPosition;

	tyrNode *_parent;
	std::vector<tyrNode *> _childNodes;
	std::vector<tyrObject *> _objects;
	
	bool _bTriggerParentUpdate;

	tdio_library::rgb_l _color;

	std::string _name;
};
