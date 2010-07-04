#ifndef _VARS
#define _VARS
#include "string.h"

class vars {

	public:

	void setupDefault();

	//location for drawing.
	float xLoc;
	float yLoc;

	int controlWidth;
	int controlHeight;

	float thickness;
	float contourThickness;
	float threshold;
	float scaleFactor;

	bool thickenOn;//holds whether or not the Graffiti is thickened.
	bool contoursOn;
	bool save;
	bool showHoles;

	char* fileName;
	bool fileChanged;
	bool bFileLoaded;

	//variables for bridge types
	bool findClosestBridge;
	bool highestBridge;
	bool lowestBridge;
	bool leftMostBridge;
	bool rightMostBridge;
};

#endif