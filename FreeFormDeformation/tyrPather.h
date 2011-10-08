#pragma once
#include <3DIO/3DIO.h>
#include <vector>
#include <string>

enum PATH_TYPE {PATH_BEZIER,PATH_LINEAR};

namespace Tyr{
	class tyrPathElement {
	public:
		tyrPathElement(){}
		tyrPathElement(const tyrPathElement& elem){*this = elem;}

		~tyrPathElement(){}
		
		void operator=(const tyrPathElement &elem){
			frame = elem.frame;
			point = elem.point;
			step_size = elem.step_size;
			dist_from_last = elem.dist_from_last;
			percent_traveled = elem.percent_traveled;
			time_stamp = elem.time_stamp;
		}

		int frame;
		tdio_library::Vector3 point;
		
		double time_stamp;

		double step_size;
		double dist_from_last;
		double percent_traveled;
	};
	class tyrPath {
	public:	
		tyrPath():_pathLength(0.0f),_stepLength(0.02){}
		~tyrPath(){
			clear();		
		}
		void clear(){
			std::vector<tyrPathElement*>::iterator iter = _elems.begin();
			while(iter != _elems.end()){
				tyrPathElement *elem = (*iter);
				if(elem){delete elem; elem = NULL;}
				iter++;
			}
			_elems.clear();
		}

		double	_stepLength;
		double	_pathLength;
		std::vector<tyrPathElement*> _elems;
		std::vector<double>			 _distAlongCurve;
	};

	class tyrPather
	{
	public:
		tyrPather(void);
		~tyrPather(void);
		
		void addFeaturePoint(const tdio_library::Vector3 &pt);
		bool buildPath();
		void dbgDrawPathToBmp(const std::string &name);
			
		bool getPoint(const float &time, tdio_library::Vector3 &pt, double &offset, double t1=0.25f,double t2=0.75f);
		
		void setGranularity(const double &d){_granularity = d;}
		void setPathType(PATH_TYPE t){_type = t;}
	protected:

		bool buildBezierPath();
		bool buildLinearPath();
		double calc_cat(double t, double p0,double p1,double p2,double p3);
	protected:
		PATH_TYPE _type;
		tyrPath	_path;
		std::vector<tdio_library::Vector3> _featurePoints;
		double _granularity;
	};
};
