#ifndef _GRAFFITICONTROLPANEL
#define _GRAFFITICONTROLPANEL

#include "ofxControlPanel.h"
#include "ofmain.h"
#include "vars.h"

class graffitiControlPanel {
	
	public:
		vars* myVars;
		ofxControlPanel *gui;
		simpleFileLister files;

		graffitiControlPanel(vars* v);

		void startGUI();
		void updateGUI();
		void drawGUI();

		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
};

#endif