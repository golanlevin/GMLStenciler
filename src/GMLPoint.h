#ifndef _GMLPOINT
#define _GMLPOINT


#include "ofMain.h"

class GMLPoint {
	public:	
	float x;
	float y;
	float time;
	float angle;
	float dist;//distance to the previous point
	GMLPoint(float x1, float y1, float t, float a, float dist);
};

#endif