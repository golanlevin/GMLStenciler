#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	myVars = new vars();
	myVars->setupDefault();
	
	control = new graffitiControlPanel(myVars);
	control->startGUI();
	
	myGraffiti = new Graffiti(myVars);
	printf("Please select a GML file from the list to get started...\n");
	handyStr = new char[128];
}

//--------------------------------------------------------------
void testApp::update(){
	
	control->updateGUI();
	if (myVars->save) {
		if (myVars->bFileLoaded){
			string currentInputFilePath = myVars->fileName;
			int lastSlashIndex = 1 + currentInputFilePath.find_last_of ('/');
			int lastDotIndex =       currentInputFilePath.find_last_of ('.');
			int len = lastDotIndex - lastSlashIndex;
			string currentInputFileName = currentInputFilePath.substr (lastSlashIndex, len);
			sprintf(handyStr, "stencil_%s.eps", currentInputFileName.c_str());
			
			myGraffiti->saveSelf(handyStr);
			printf("Exported EPS Stencil to %s!\n", handyStr);
			
		}
	}
	
	if (myVars->fileChanged) {
		printf("\nLoading a new GML...\n");
		if(myGraffiti->loadGML(myVars->fileName)) printf("GML loaded!\n");
		else printf("GML could not be loaded.\n");
		myVars->fileChanged = false;
	}
}


//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(24,24,24);
	if (myVars->thickenOn) {
		myGraffiti->thicken();
	}
	if (myVars->contoursOn) {
		myGraffiti->findContours();
	} 
	if (myVars->showHoles) {
		myGraffiti->showHoles();
	}
	
	if (!myVars->contoursOn && !myVars->showHoles && !myVars->thickenOn) {
		myGraffiti->drawSelf();
	}
	
	control->drawGUI();
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	control->mouseDragged(x, y, button); 
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	control->mousePressed(x, y, button); 
	
	/*
	if (myVars->bFileLoaded){
		if(x < (ofGetWidth() - myVars->controlWidth)) {
			myVars->xLoc = x - ((1.0/2.0) * myGraffiti->getWidth());
			myVars->yLoc = y - ((1.0/2.0) * myGraffiti->getHeight());
		}
	}
	 */
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	control->mouseReleased(); 
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}

