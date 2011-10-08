FreeFormDeformation (FFD)
=========================
The code in this project was written for a class on animation.  

Contained herein are functions for: FreeFormDeformation, Barr shape transformations, and spline path finding.  I have only included the models for the FFD functions, the 'Eva' model is my own and not available for distribution.  

This is class code so don't expect too many comments or a clean base.  I'm always happy to answer questions if needs be and I've tried to be as verbose as possible where it matters.


Dependencies
------------

The following libraries are required to run the code.

* [GLUT](http://www.opengl.org/resources/libraries/glut/glut37.zip)
* OpenGL
* 3DIO -- This library is a part of my internal code base and is not available to the general public.  The quaternion, vector3, and matrix3 classes I used are almost identical to Ogre3D.  You can download Ogre3D and replace the calls as necessary.
* [EventTimer](http://github.com/Jerdak/EventTimer) -- A class to handle event timing.  