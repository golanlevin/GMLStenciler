#ifndef _STROKE
#define _STROKE

#include "GMLPoint.h"
#include <vector>

class Stroke {
	public:
	vector<GMLPoint*> gmlPoints;
	Stroke(GMLPoint* pointAr[]);
	Stroke(vector<GMLPoint*> pointVect);
	Stroke();


};

#endif