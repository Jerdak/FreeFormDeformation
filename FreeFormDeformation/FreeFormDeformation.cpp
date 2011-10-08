// FreeFormDeformation.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

using namespace tdio_library;
using namespace std;


/** Ugly Globals **/
	///Freeform deformation controller
	tyrFreeFormDef ffd;

	///Barr's shape transformation controllers
	tyrBarrShape shape,shape2;

	///Generic objects for head, body, and arm for the Eva animation example
	tyrObject obj_head,obj_body,obj_arm;

	///Limb specific orientations for Eva animation example.
	quaternion armRightQ,armLeftQ,headQ;

	///Global translation
	Vector3 trans(0,0,-4);

	///Global rotation
	Vector3 rot;

	///Elapsed time since last frame
	double elapsedTime = 0.0;

	///Time direction (Used to loop in same time cycle). 
	double timeDir = 1;

	///Nodes for Eva body parts
	map<string,tyrNode*> nodes;

	///Global timer;
	NSC::Common::ProfTimer timer;

	///Chaining flags, determine if orientations propogate through parental heirarchy. (true = yes)
	bool bChain = false, bChain2 = false, bChain3 = false;

	///Reset animation flag (true = yes)
	bool bReset = false;

	///Run animation flag (true = yes);
	bool bRunAnimation = false;

/** Functions **/
	///Run Eva 'head drop' animation
	void animateHeadDrop(double dt);

	///Create GlutMenus, for Eva demo only.
	void createGLUTMenus();
	
	///Render Freeform Deformation scene, callback for GLUT
	void renderSceneFFD(void);
	
	///Render Eva demo scene, callback for GLUT
	void renderSceneEva(void);

	///Keyboard function callback (for GLUT)
	void keyboardFunc(unsigned char key, int x, int y);

	///Resize callback for GLUT
	void resize(int w, int h);

	///Drag lights
	void drawLights();

	///Update lights, if position changes
	void updateLights();

	///Helper function: Create node
	tyrNode *createNode(string name,tyrNode *parent,Vector3 pos= Vector3::ZERO, quaternion orientation = quaternion::IDENTITY,rgb_l color = rgb_l(0.5,0.5,0.5));
	
	///Helper function: Create node
	tyrNode *createNode(string name,string parent,Vector3 pos = Vector3::ZERO, quaternion orientation = quaternion::IDENTITY,rgb_l color = rgb_l(0.5,0.5,0.5));

	///Run Freeform Deformation Example
	void DemoFreeForm(int argc, char* argv[]);

	///Run Eva Demo
	void DemoeEva(int argc, char* argv[]);

int main(int argc, char* argv[])
{

	obj_head.loadPly("eva_head.ply");
	obj_body.loadPly("eva_body.ply");
	obj_arm.loadPly("eva_arm.ply");

	timer.Start();

	//TODO: Add flag to choose project to load
	DemoFreeForm(argc,argv);
	//DemoEva(argc,argv);
	
	glutMainLoop();
	return 0;
}
void DemoeEva(int argc, char* argv[]){
	Matrix3 mat;
	quaternion o;
	Vector3 p;

	
	{	///Create "root"
		tyrNode *tmpNode = new tyrNode();
		tmpNode->setName("root");
		nodes["root"] = tmpNode;
	}
	{	///Create "eva"
		mat.FromEulerAnglesXYZ(0,0,0);
		createNode("eva","root");
	}
	{	///Create "body"
		mat = Matrix3::IDENTITY;
		createNode("body","root")->addObject(&obj_body);
	}
	{	///Create "head"
		mat.FromEulerAnglesXYZ(0,0,0);
		createNode("head","body",Vector3(0,0.5,0),mat)->addObject(&obj_head);
	}

	{	///Create "right_arm"
		mat.FromEulerAnglesXYZ(0,0,-15);
		createNode("arm_right","body",Vector3(-0.1,0.25,0.0),mat,rgb_l(1,0,0));
		createNode("arm_right_mesh","arm_right",Vector3(0.0,0.0,0.0),mat,rgb_l(1,0,0))->addObject(&obj_arm);
		nodes["arm_right_mesh"]->translate(Vector3(-0.1,-0.2,0.0),tyrNode::RC_PARENT);
	}
	{	///Create "left_arm"
		mat.FromEulerAnglesXYZ(0,0,15);
		createNode("arm_left","body",Vector3(0.1,0.25,0.0),mat,rgb_l(1,0,0));
		createNode("arm_left_mesh","arm_left",Vector3(0.0,0.0,0.0),mat,rgb_l(0,1,0))->addObject(&obj_arm);
		nodes["arm_left_mesh"]->translate(Vector3(0.1,-0.2,0.0),tyrNode::RC_PARENT);
	}
	{
		armRightQ	= nodes["arm_right"]->getGlobalOrientation();
		armLeftQ	= nodes["arm_left"]->getGlobalOrientation();
		headQ		= nodes["head"]->getGlobalOrientation();
	}

	{	///Initialize glut window
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100,100);
		glutInitWindowSize(800,600);
		glutCreateWindow("Projects 4");

		///Call backs
		glutDisplayFunc(renderSceneEva);
		glutIdleFunc(renderSceneEva);
		glutKeyboardFunc(keyboardFunc);
		glutReshapeFunc(resize);

	}
	
	{	///Initialize lighting
		GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat mat_shininess[] = { 10.0 };
		GLfloat ambient[] = {0.5,0.5,0.5,0.5};
		GLfloat diffuse[] = {0.5,0.5,0.5,0.5};
		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClearDepth(1.0f);	
		glShadeModel (GL_SMOOTH);

		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_COLOR_MATERIAL);

		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	createGLUTMenus();
}
void DemoFreeForm(int argc, char* argv[]){
	ffd.loadPly("brain.ply");

	//Uncomment this line if you want a demo. animation that moves the control points in and out.
	ffd.dbgPaths();
	
	{	///Initialize glut window
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100,100);
		glutInitWindowSize(800,600);
		glutCreateWindow("Project 3");

		///Call backs
		glutDisplayFunc(renderSceneFFD);
		glutIdleFunc(renderSceneFFD);
		glutKeyboardFunc(keyboardFunc);
		glutReshapeFunc(resize);

	}
	
	{	///Initialize lighting
		GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat mat_shininess[] = { 10.0 };
		GLfloat ambient[] = {0.5,0.5,0.5,0.5};
		GLfloat diffuse[] = {0.5,0.5,0.5,0.5};
		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClearDepth(1.0f);	
		glShadeModel (GL_SMOOTH);

		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_COLOR_MATERIAL);

		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	createGLUTMenus();
}
tyrNode *createNode(string name,tyrNode *parent,Vector3 pos, quaternion orientation,rgb_l color){
	tyrNode *tmpNode = new tyrNode();
	tmpNode->setName(name);
	parent->addChild(tmpNode);
	tmpNode->translate(pos,tyrNode::RC_WORLD);
	tmpNode->rotate(orientation,tyrNode::RC_WORLD);
	tmpNode->setColor(color);
	
	nodes[name] = tmpNode;

	return tmpNode;
}
tyrNode *createNode(string name,string parent,Vector3 pos, quaternion orientation,rgb_l color){
	return createNode(name,nodes[parent],pos,orientation,color);
}
void updateLights(){
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 10.0 };
	GLfloat ambient[] = {0.5,0.5,0.5,0.5};
	GLfloat diffuse[] = {0.5,0.5,0.5,0.5};

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

double time0,time1;

quaternion last = quaternion::IDENTITY;

void renderSceneFFD(void){
	timer.Stop();
	double dt = timer.GetDurationInSecs();
	
	//Manually step animation in 15ms increments. (Using time between calls of renderSceneFFD leads to 'spikey' timing)
	while(dt < 0.015){
		timer.Stop();
		dt += timer.GetDurationInSecs();
	}

	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(trans.x,trans.y,trans.z);
	glRotatef(rot.y,0,1,0);
	ffd.render(dt,true);
	ffd.updateAnimation(dt);
	glFlush();
	timer.Start();
	glutSwapBuffers();
}
void renderSceneEva(void) {
	timer.Stop();
	quaternion orientation;orientation.Identity();

	double dt = timer.GetDurationInSecs();
	while(dt < 0.02){
		timer.Stop();
		dt += timer.GetDurationInSecs();
	}
	if(bRunAnimation){
		glMatrixMode(GL_MODELVIEW);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		updateLights();
		glLoadIdentity();
			glTranslatef(trans.x,trans.y,trans.z);
			glRotatef(rot.y,0,1,0);
			glPushMatrix();
			animateHeadDrop(dt);
		glPopMatrix();
		glFlush();
	} else {
		if(bReset){
			elapsedTime = 0.0f;
			timeDir = 1.0f;
			bReset = false;


			nodes["arm_right"]->setOrientation(armRightQ);
			nodes["arm_left"]->setOrientation(armLeftQ);
			nodes["head"]->setOrientation(headQ);
		}
		elapsedTime += 0.5 * timeDir;
		
		quaternion qTmp;

		Matrix3 mat;	
		Matrix3 mat2;
		Matrix3 mat3;
		
		if(0)
		{
			if(elapsedTime < 90 && elapsedTime >= 0)mat.FromEulerAnglesXYZ(0.5 * timeDir,0,0.0);		else mat.Identity();
			if(elapsedTime < 90 && elapsedTime >= 0)mat2.FromEulerAnglesXYZ(0,0,-0.5 * timeDir);		else mat2.Identity();
			if(elapsedTime < 90 && elapsedTime >= 0)mat3.FromEulerAnglesXYZ(0.5 * timeDir,0.0f,0.0f);	else mat3.Identity();
		} else 
		{
			if(elapsedTime >= 90 || elapsedTime <= 0)timeDir *= -1;
			mat.FromEulerAnglesXYZ(-0.5 * timeDir,0,0.0);	
			mat2.FromEulerAnglesXYZ(0,0,-0.5 * timeDir);
			mat3.FromEulerAnglesXYZ(0.5 * timeDir,0.0f,0.0f);
		}
		
		{	///Create arm rotation slerp
			Matrix3 mTmp; mTmp.FromEulerAnglesXYZ(-60,-90,0);
			quaternion qFrom = quaternion::IDENTITY;
			quaternion qTo(mTmp);
			qTmp = quaternion::Slerp(elapsedTime / 90.0f,qFrom,qTo);
		}
		glMatrixMode(GL_MODELVIEW);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		updateLights();
		glLoadIdentity();
		
		
		glTranslatef(trans.x,trans.y,trans.z);
		glRotatef(rot.y,0,1,0);
		glPushMatrix();
			tyrNode::Rotation_Center r; r = tyrNode::RC_LOCAL;
			if(bChain)nodes["arm_right"]->rotate(mat2,r);
			if(bChain2)nodes["arm_left"]->rotate(qTmp,r);
			if(bChain3)nodes["head"]->rotate(mat3,tyrNode::RC_LOCAL);
			{
				nodes["root"]->render(dt);
			}
			if(bChain2)nodes["arm_left"]->setOrientation(armLeftQ);
		glPopMatrix();
		glFlush();
	}

	Sleep(1);
	glutSwapBuffers();
	timer.Start();
}
Vector3 lastPos;

double chaseTime = 0;
bool chaseStart = false;
bool animationSetup = true;
Tyr::tyrPather headPath;
void animateHeadDrop(double dt) {
	if(animationSetup) {
		Vector3 base = nodes["head"]->getGlobalPosition();
		headPath.addFeaturePoint(Vector3(0,0,0));
		headPath.addFeaturePoint(Vector3(0,0,0));
		headPath.addFeaturePoint(Vector3(0,3,0));
		headPath.addFeaturePoint(Vector3(0,3,2));
		headPath.addFeaturePoint(Vector3(0,-base.y,5));
		headPath.addFeaturePoint(Vector3(0,-base.y*2,5));
		headPath.setPathType(PATH_BEZIER);
		headPath.buildPath();
		elapsedTime = 0.0f;
		animationSetup = false;
	}
	quaternion qHead = nodes["head"]->getGlobalOrientation();
	Vector3 vHead = nodes["head"]->getGlobalPosition();
	elapsedTime += dt;

	Vector3 vTmp;
	double offset;
	if(headPath.getPoint(elapsedTime/5,vTmp,offset,0.1,0.90)){
		nodes["head"]->translate(vTmp-lastPos,tyrNode::RC_WORLD);
		nodes["root"]->render(dt);
		lastPos = vTmp;
	} else {
		vTmp = Vector3(0,0,0.1);
		nodes["head"]->translate(vTmp,tyrNode::RC_WORLD);
		nodes["root"]->render(dt);

		if(nodes["head"]->getGlobalPosition().z > 10){
			if(!chaseStart){

				nodes["head"]->setParent(NULL);
				
				Matrix3 tmp; tmp.FromEulerAnglesXYZ(15,0,0);
				nodes["body"]->rotate(tmp,tyrNode::RC_LOCAL);

				tmp.FromEulerAnglesXYZ(90,0,0);
				nodes["head"]->rotate(tmp,tyrNode::RC_LOCAL);

				tmp.FromEulerAnglesXYZ(-95,0,0);
				nodes["arm_right"]->rotate(tmp,tyrNode::RC_LOCAL);

				tmp.FromEulerAnglesXYZ(-95,0,0);
				nodes["arm_left"]->rotate(tmp,tyrNode::RC_LOCAL);
				chaseStart = true;
			}
			nodes["body"]->translate(vTmp,tyrNode::RC_WORLD);
		}
	}
}
void drawLights(){
	GLfloat light_position[] = { 2.0f, 2.0f, -17.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

}
void processMenuEvents(int option);
#define MENU_CHAIN 0
#define MENU_CHAIN2 1
#define MENU_CHAIN3 2
#define MENU_CHAIN4 3
void createGLUTMenus() {

	int menu;

	// create the menu and
	// tell glut that "processMenuEvents" will 
	// handle the events
	menu = glutCreateMenu(processMenuEvents);
	
	//add entries to our menu
	glutAddMenuEntry("Chain Right Arm",MENU_CHAIN);
	
	//add entries to our menu
	glutAddMenuEntry("Chain Left Arm",MENU_CHAIN2);

		//add entries to our menu
	glutAddMenuEntry("Chain Head",MENU_CHAIN3);

	//add entries to our menu
	glutAddMenuEntry("Run Animation",MENU_CHAIN4);

	// attach the menu to the right button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void processMenuEvents(int option) {

	switch (option) {
		case MENU_CHAIN :
			bChain = !bChain;
			bReset = true;
			break;
		case MENU_CHAIN2 :
			bChain2 = !bChain2;
			bReset = true;
			break;
		case MENU_CHAIN3 :
			bChain3 = !bChain3;
			bReset = true;
			break;
		case MENU_CHAIN4 :
			bRunAnimation = true;
			break;
	}
}
void resize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,1,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,0.0,5.0, 
		      0.0,0.0,-1.0,
			  0.0f,1.0f,0.0f);


}

void keyboardFunc(unsigned char key, int x, int y){
	switch(key){
		case 'q':
			exit(0);
			break;
		case 'w':
			trans.z += 0.3;
			break;
		case 's':
			trans.z -= 0.3;
			break;
		case 'z':
			ffd.transControlPoint(1,1,4,Vector3(0,0,-1));
			ffd.transformVertices();
			break;
		case 'x':
			ffd.transControlPoint(1,1,4,Vector3(0,0,1));
			ffd.transformVertices();
			break;
		case 'c':
			ffd.transControlPoint(2,4,4,Vector3(1,1,0));
			ffd.transformVertices();
			break;
		case 'v':
			ffd.transControlPoint(2,4,4,Vector3(-1,-1,0));
			ffd.transformVertices();
			break;
		case 'a':
			rot.y += 5.0;
			break;
		case 'd':
			rot.y -= 5.0;
			break;
		default:
			break;
	};
}
