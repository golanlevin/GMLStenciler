#include "Stroke.h"

Stroke::Stroke(GMLPoint* pointAr[]) {
	gmlPoints.assign(pointAr, pointAr+ (sizeof(pointAr)/sizeof(GMLPoint*)));
}

Stroke::Stroke(vector<GMLPoint*> pointVect) {
	gmlPoints = pointVect;
}

Stroke::Stroke() {
	;
}