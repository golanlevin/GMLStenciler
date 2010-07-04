#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxDirList.h"
#include "ofxXmlSettings.h"

#include "Graffiti.h"
#include "graffitiControlPanel.h"
#include "vars.h"

class testApp : public ofBaseApp{

	public:
	
		Graffiti* myGraffiti;
		graffitiControlPanel* control;
		vars* myVars;
		char *handyStr;

		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void resized(int w, int h);

};

#endif
