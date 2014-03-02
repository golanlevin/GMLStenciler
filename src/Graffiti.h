#ifndef _GRAFFITI
#define _GRAFFITI

#include "ofxXmlSettings.h"
#include "GMLPoint.h"
#include <vector>
#include "Stroke.h"
#include "ofxVectorGraphics.h"
#include "ofxCvGrayscaleImage.h"
#include "ofxCvColorImage.h"
#include "ofxCvContourFinder.h"
#include "vars.h"

class Graffiti {
	public:

	vector<Stroke*> strokes;//what is maximum number of strokes? 1000 is arbitrary
	float scaleFactor;
	ofFbo myFBO;
	ofxCvContourFinder contourFinder;
	ofxVectorGraphics output;
	vars* myVars;

	//-------------------------------------------------------------------------------------

	Graffiti( vars* v);//the graffiti constructor.
	bool loadGML(char gmlLocation[]);//constructs the stroke array.
	void drawSelf();//writes out an image of self based on strokes
	void thicken();//visibly thickens the image.
	void findContours();
	void showHoles();
	void saveSelf(char fileName[]);
	void average(float pct, Stroke* left,  Stroke* right);
	void smoothStroke(int resolution, Stroke* src);

	void drawCircle(float x, float y, float r);
	void drawCircle(GMLPoint* point, float r);
	void drawCircle(ofPoint point, float r);
	
	void drawLine(ofPoint pt1, ofPoint pt2);
	void drawLine(GMLPoint* pt1, GMLPoint* pt2);
	void drawLine(ofPoint pt1, ofPoint pt2, ofPoint pt3,
		ofPoint pt4, ofPoint pt5, ofPoint pt6);
	
	float getWidth();
	float getHeight();
};

#endif