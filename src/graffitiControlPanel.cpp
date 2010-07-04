#include "graffitiControlPanel.h"

graffitiControlPanel::graffitiControlPanel(vars *v) {
	myVars = v;
}

void graffitiControlPanel::startGUI() {
	
	files.listDirWithExtension(".", ".gml");
	//files.listDir(".");//our GMLs should be in data by default anyway. Add user input later
	
	gui = new ofxControlPanel();
	gui->notify();	
	gui->setup("GMLStenciler", ofGetWidth() - myVars->controlWidth - 4, 4, myVars->controlWidth, myVars->controlHeight);

	string instructionString = " INSTRUCTIONS:\n"; 
	instructionString += " [All in/out files are in 'data' folder]\n"; 
	instructionString += " Load a .gml file from list below;\n"; 
	instructionString += " Scale drawing using scale slider;\n"; 
	instructionString += " Enable & adjust thickness;\n";
	instructionString += " Enable bridges & select bridge type;\n"; 
	instructionString += " Adjust bridge thickness if desired;\n"; 
	instructionString += " Export with 'SAVE EPS' button.\n"; 
	gui->addPanel(instructionString, 2, false);	
	gui->setWhichPanel(0);
	
	//----------------------------
	gui->setWhichColumn(0);
	gui->addSlider("Thickness","THICKNESS",myVars->thickness,1,12,false);
	gui->addSlider("Hole threshold","THRESHOLD",myVars->threshold,0,5000,false);
	gui->addSlider("Bridge thickness","CONTOUR_THICKNESS", myVars->contourThickness, 1,20,false);
	gui->addSlider("Scale factor","SCALE_FACTOR", myVars->scaleFactor, 1, 1000,false);
	gui->addFileLister("Loadable GML files",&files,180,100);
	gui->addToggle("SAVE EPS!","SAVE",false);
	
	//----------------------------
	gui->setWhichColumn(1);
	gui->addToggle("Enable Thickness","THICKEN?",false);
	gui->addToggle("Show holes","SHOW_HOLES?",false);
	gui->addToggle("Enable bridges","CONTOURS?",false);
	gui->addToggle("-- Bridge Close","CBRIDGE",false);
	gui->addToggle("-- Bridge High" ,"HBRIDGE",false);
	gui->addToggle("-- Bridge Low"  ,"BBRIDGE",false);
	gui->addToggle("-- Bridge Left" ,"LBRIDGE",false);
	gui->addToggle("-- Bridge Right","RBRIDGE",false);
	
}

void graffitiControlPanel::updateGUI() {
	myVars->thickness			= gui->getValueF("THICKNESS");
	myVars->contourThickness	= gui->getValueF("CONTOUR_THICKNESS");
	myVars->threshold			= gui->getValueF("THRESHOLD");
	myVars->scaleFactor			= gui->getValueF("SCALE_FACTOR");
		
	myVars->thickenOn			= gui->getValueB("THICKEN?");
	myVars->contoursOn			= gui->getValueB("CONTOURS?");
	myVars->showHoles			= gui->getValueB("SHOW_HOLES?");

	myVars->findClosestBridge	= gui->getValueB("CBRIDGE");
	myVars->highestBridge		= gui->getValueB("HBRIDGE");
	myVars->lowestBridge		= gui->getValueB("BBRIDGE");
	myVars->leftMostBridge		= gui->getValueB("LBRIDGE");
	myVars->rightMostBridge		= gui->getValueB("RBRIDGE");

	myVars->save				= gui->getValueB("SAVE");
	
	if(files.selectedHasChanged()) { 
		char *a = new char[files.getSelectedPath().size()+1];
		a[files.getSelectedPath().size()] = 0;
		memcpy(a,files.getSelectedPath().c_str(),files.getSelectedPath().size());
		myVars->fileName = a;
		myVars->fileChanged = true;

		//all the values get reset when you load a new image in order to make it faster to load
		gui->setValueB("THICKEN?",false);
		gui->setValueB("CONTOURS?",false);
		gui->setValueB("SHOW_HOLES?",false);

		gui->setValueB("CBRIDGE",false);
		gui->setValueB("HBRIDGE",false);
		gui->setValueB("BBRIDGE",false);
		gui->setValueB("LBRIDGE",false);
		gui->setValueB("RBRIDGE",false);
	}

	gui->setValueB("SAVE",false);
	gui->update();
	files.clearChangedFlag();
}

void graffitiControlPanel::drawGUI() {
	gui->draw();
}

void graffitiControlPanel::mouseDragged(int x, int y, int button) {
	gui->mouseDragged(x,y,button);
}

void graffitiControlPanel::mousePressed(int x, int y, int button) {
	gui->mousePressed(x,y,button);
}

void graffitiControlPanel::mouseReleased() {
	gui->mouseReleased();
}