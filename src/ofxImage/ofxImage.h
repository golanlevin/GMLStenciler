/*
 *  ofxImage
 *
 *  Provides extended functionality to the core ofImage class.
 *  
 *  Created by Pat Long (plong0) on 08/06/10.
 *  Copyright 2010 Tangible Interaction Inc. All rights reserved.
 *
 */
#ifndef _OFX_IMAGE
#define _OFX_IMAGE

#include "ofMain.h"

class ofxImage : public ofImage{
protected:
	string fileName;
	
	void flipPixels(ofPixels &pix, bool horizontal, bool vertical);
	void rotatePixels(ofPixels &pix, float angle);
	bool saveImageFromPixels(string fileName, ofPixels &pix);
	
public:
	ofxImage();
	
	string getFileName();
	bool loadImage(string fileName="");
	bool saveImage(string fileName="");
	void setFileName(string fileName);
	
	void mirror(bool horizontal, bool vertical);
	void rotate(float angle);
};

#endif
