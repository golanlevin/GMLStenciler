#include "vars.h"

void vars::setupDefault() {
	
	xLoc = 2.0;
	yLoc = 2.0;

	controlWidth = 390;
	controlHeight = 560;

	thickness = 1.8;
	contourThickness = 2.0;
	threshold = 100.0;
	scaleFactor = 250.0;

	thickenOn = false;
	contoursOn = false;
	save = false;
	showHoles = false;

	fileChanged = false;
	bFileLoaded = false;

	findClosestBridge = false;
	highestBridge = false;
	lowestBridge = false;
	leftMostBridge = false;
	rightMostBridge = false;
}