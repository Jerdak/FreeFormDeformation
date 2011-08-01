#include "tyrPather.h"

using namespace Tyr;
using namespace tdio_library;
using namespace std;
tyrPather::tyrPather(void):_type(PATH_LINEAR),_granularity(0.1f)
{
}

tyrPather::~tyrPather(void)
{
}
void tyrPather::addFeaturePoint(const tdio_library::vector3 &pt){
	_featurePoints.push_back(pt);
}

bool tyrPather::buildPath(){
	switch(_type){
		case PATH_BEZIER:return buildBezierPath();
		case PATH_LINEAR:return buildLinearPath();
	};

	return false;
}

bool tyrPather::buildBezierPath(){
	if(_featurePoints.size() < 4){
		printf("  - Error, not enough feature points for a bezier path.  Must have 4 or more.\n");
		return false;
	}
	int frame = 0;
	double time = 0.0f;
	printf("[Building Path]\n");

	///Loop through control points to build up our 
	printf("  - Creating points along curve....");

	for(int c = 0; c < _featurePoints.size()-3; c++){
		vector3 p0,p1,p2,p3;
		p0 = _featurePoints[c];
		p1 = _featurePoints[c+1];
		p2 = _featurePoints[c+2];
		p3 = _featurePoints[c+3];
		for(float i = 0; i <= 1; i+=_granularity){
			vector3 pt;
			pt.x = calc_cat(i,p0.x,p1.x,p2.x,p3.x);
			pt.y = calc_cat(i,p0.y,p1.y,p2.y,p3.y);
			pt.z = calc_cat(i,p0.z,p1.z,p2.z,p3.z);

			{	///Create new element
				tyrPathElement *elem = new tyrPathElement();
				elem->frame = frame;
				elem->point = pt;
				elem->step_size = _granularity;
				elem->time_stamp = time;

				if(_path._elems.size()==0){
					elem->dist_from_last = 0.0;
				} else {
					vector3 ptLast = _path._elems[elem->frame-1]->point;
					double tmpDist = abs(pt.Dist(ptLast));
					elem->dist_from_last = tmpDist;
					_path._pathLength += tmpDist;
				}
				_path._elems.push_back(elem);
			}
			time += _granularity;
			frame++;
		}
	}
	printf("Complete\n");

	///Next up we finish building out table
	std::vector<tyrPathElement*>::iterator iter = _path._elems.begin();
	double dist = 0.0f;
	double p = 0.0;
	printf("  - Filling path table...");
	while(iter != _path._elems.end()){
		tyrPathElement *elem = (*iter);
		if(elem->frame == 0){
			elem->percent_traveled = 0.0;
		} else {		
			elem->percent_traveled = elem->dist_from_last / _path._pathLength;
			
		}
		iter++;
	}
	printf("Complete.\n");
	double tmpT = 0.0f;
	double tmpP = 0.0f;
	double tmpTT = 0.0f;



	_path._stepLength = (1.0f/(float)frame);
	printf("  - Step length: %f\n",_path._stepLength);
	iter = _path._elems.begin();
	_path._distAlongCurve.push_back(0.0f);
	while(iter != _path._elems.end()){
		tyrPathElement *elem = (*iter);
		tmpT += elem->dist_from_last;
		tmpP += elem->percent_traveled;

		if(tmpT >= _path._stepLength){
			tmpTT+= _path._stepLength;
			p+= tmpP;
			_path._distAlongCurve.push_back(tmpP);
			tmpP = 0.0f;
			tmpT = 0.0f;
			
			printf("T[%f]: %f\n",tmpTT,_path._distAlongCurve[_path._distAlongCurve.size()-1]);
		}
		iter++;
	}
	printf("Complete.\n");
	printf("  - p: %f\n",p);
	printf("  - Total Time to complete curve: %f\n",time);
	printf("  - Curve Length: %f\n",_path._pathLength);
	printf("  - # of frames: %d\n",_path._elems.size());
	return true;

}

bool tyrPather::buildLinearPath(){
	if(_featurePoints.size() < 2){
		printf("  - Error, not enough feature points for a linear path.  Must have 4 or more.\n");
		return false;
	}
	int frame = 0;
	double time = 0.0f;
	printf("[Building Linear Path]\n");

	///Loop through control points to build up our 
	printf("  - Creating points along curve....");

	for(int c = 0; c < _featurePoints.size()-1; c++){
		vector3 p0,p1;
		p0 = _featurePoints[c];
		p1 = _featurePoints[c+1];

		Ray ray(p0,p1-p0);

		for(float i = 0; i <= 1; i+=_granularity){
			vector3 pt;
			pt = ray.GetPoint(i);

			{	///Create new element
				tyrPathElement *elem = new tyrPathElement();
				elem->frame = frame;
				elem->point = pt;
				elem->step_size = _granularity;
				elem->time_stamp = time;

				if(_path._elems.size()==0){
					elem->dist_from_last = 0.0;
				} else {
					vector3 ptLast = _path._elems[elem->frame-1]->point;
					double tmpDist = abs(pt.Dist(ptLast));
					elem->dist_from_last = tmpDist;
					_path._pathLength += tmpDist;
				}
				_path._elems.push_back(elem);
			}
			time += _granularity;
			frame++;
		}
	}
	printf("Complete\n");

	///Next up we finish building out table
	std::vector<tyrPathElement*>::iterator iter = _path._elems.begin();
	double dist = 0.0f;
	double p = 0.0;
	printf("  - Filling path table...");
	while(iter != _path._elems.end()){
		tyrPathElement *elem = (*iter);
		if(elem->frame == 0){
			elem->percent_traveled = 0.0;
		} else {		
			elem->percent_traveled = elem->dist_from_last / _path._pathLength;
			
		}
		iter++;
	}
	printf("Complete.\n");
	double tmpT = 0.0f;
	double tmpP = 0.0f;
	double tmpTT = 0.0f;



	_path._stepLength = (1.0f/(float)frame);
	printf("  - Step length: %f\n",_path._stepLength);
	iter = _path._elems.begin();
	_path._distAlongCurve.push_back(0.0f);
	while(iter != _path._elems.end()){
		tyrPathElement *elem = (*iter);
		tmpT += elem->dist_from_last;
		tmpP += elem->percent_traveled;

		if(tmpT >= _path._stepLength){
			tmpTT+= _path._stepLength;
			p+= tmpP;
			_path._distAlongCurve.push_back(tmpP);
			tmpP = 0.0f;
			tmpT = 0.0f;
			
			printf("T[%f]: %f\n",tmpTT,_path._distAlongCurve[_path._distAlongCurve.size()-1]);
		}
		iter++;
	}
	printf("Complete.\n");
	printf("  - p: %f\n",p);
	printf("  - Total Time to complete curve: %f\n",time);
	printf("  - Curve Length: %f\n",_path._pathLength);
	printf("  - # of frames: %d\n",_path._elems.size());
	return true;
}
bool tyrPather::getPoint(const float &aTime, vector3 &pt, double &offset, double t1, double t2){
	double time = aTime;
	if(time > 1.0f){
		return false;
	}
	if(t1 >= t2){
		printf("  - Warning, t1 >= t2 [%f >= %f\n",t1,t2);
		t1 = 0.25;
		t2 = 0.75;
	}
	double d = 0.0f;
	double v0 = 2/(t2- t1 + 1);

	if(time <= t1){
		d = v0*((time * time)/2*t1);
	}else if(time > t1 && time <= t2){
		d = v0*((t1/2.0f) + (time - t1));
	} else {
		d = v0*(t1 / 2)		+ \
			v0*(t2 - t1)	+ \
			(v0-(v0*((time-t2)/(1-t2)))/2.0f) * (time - t2);
	}
	double maxDist = _path._pathLength * d;
	double totalDist = 0.0;
	tyrPathElement *elem,*elemLast= NULL;
	for(int i = 0; i < _path._elems.size(); i++){
		tyrPathElement *elem = _path._elems[i];
		totalDist += elem->dist_from_last;

		if(totalDist >= maxDist){
			if(elemLast){
				vector3 pt1 = elemLast->point;
				vector3 pt2 = elem->point;
				Ray ray(pt1,pt2-pt1);

				double nextDist = totalDist;
				totalDist -= elem->dist_from_last;
				nextDist -= totalDist;

				double diff = (maxDist - totalDist) / nextDist;
				offset = diff;
				pt = ray.GetPoint(diff);
			} else {
				offset = 0;
				pt = elem->point;
			}
			return true;
		}
		offset = 0;
		elemLast = elem;
	}
	printf("You should never see me.\n");
	return false;
}
double tyrPather::calc_cat(double t, double p0,double p1,double p2,double p3){
	double t2 = t*t;
	double t3 = t2 * t;
	return (0.5 *(    	(2 * p1) + (-p0 + p2) * t +(2*p0 - 5*p1 + 4*p2 - p3) * t2 +(-p0 + 3*p1- 3*p2 + p3) * t3));
}
/*
#pragma warning(push)
	#pragma warning(disable:4793)	///Ignore native code generation warning
	#define UINT unsigned int		///tdio_library defines a UINT that CImg can't resolve
	#include <cimg.h>
#pragma warning(pop)
*/
void tyrPather::dbgDrawPathToBmp(const std::string &name){
/*	using namespace cimg_library;

	CImg<unsigned char> image(500,400,1,3,0);
	unsigned char red[] = {255,0,0};

	image.fill(0);
	std::vector<tyrPathElement*>::iterator iter = _path._elems.begin();

	while(iter != _path._elems.end()){
		tyrPathElement *elem = (*iter);
		vector3 pt = elem->point;
		if(elem->frame + 1 < _path._elems.size()){
			vector3 pt2 = _path._elems[elem->frame+1]->point;
			image.draw_line((int)pt.x,400-(int)pt.y,(int)pt2.x,400-(int)pt2.y,red);
		}
		iter++;
	}

	image.save("Curve.bmp");*/
}