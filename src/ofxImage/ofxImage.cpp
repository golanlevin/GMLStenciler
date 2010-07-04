/*
 *  ofxImage.cpp
 *
 *  Created by Pat Long (plong0) on 08/06/10.
 *  Copyright 2010 Tangible Interaction Inc. All rights reserved.
 *
 */
#include "ofxImage.h"

//------------------------------------
ofxImage::ofxImage():ofImage(){
	fileName = "";
}

//------------------------------------
string ofxImage::getFileName(){
	return fileName;
}

//------------------------------------
bool ofxImage::loadImage(string fileName){
	string localFileName = getFileName();
	if(fileName == "" && localFileName != "")
		fileName = localFileName;
	bool result = ofImage::loadImage(fileName);
	if(result && fileName != getFileName())
		setFileName(fileName);
	return result;
}

//------------------------------------
bool ofxImage::saveImage(string fileName){
	string localFileName = getFileName();
	if(fileName == "" && localFileName != "")
		fileName = localFileName;
	
	bool result = saveImageFromPixels(fileName, myPixels);

	if(result && fileName != getFileName())
		setFileName(fileName);
	
	return result;
}

//----------------------------------------------------------------
// copied directly from core ofImage::saveImageFromPixels, with added bool return value
bool ofxImage::saveImageFromPixels(string fileName, ofPixels &pix){
	bool result = false;
	if (pix.bAllocated == false){
		ofLog(OF_LOG_ERROR,"error saving image - pixels aren't allocated");
		return result;
	}
	
#ifdef TARGET_LITTLE_ENDIAN
	if (pix.bytesPerPixel != 1) swapRgb(pix);
#endif
	
	FIBITMAP * bmp	= getBmpFromPixels(pix);
	
#ifdef TARGET_LITTLE_ENDIAN
	if (pix.bytesPerPixel != 1) swapRgb(pix);
#endif
	
	fileName = ofToDataPath(fileName);
	if (pix.bAllocated == true){
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(fileName.c_str(), 0);
		if(fif == FIF_UNKNOWN) {
			// or guess via filename
			fif = FreeImage_GetFIFFromFilename(fileName.c_str());
		}
		if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
			result = FreeImage_Save(fif, bmp, fileName.c_str(), 0);
		}
	}
	
	if (bmp != NULL){
		FreeImage_Unload(bmp);
	}
	return result;
}


//------------------------------------
void ofxImage::setFileName(string fileName){
	this->fileName = fileName;
}

//------------------------------------
void ofxImage::mirror(bool horizontal, bool vertical){
	flipPixels(myPixels, horizontal, vertical);
	
	update();
}

//------------------------------------
void ofxImage::rotate(float angle){
	rotatePixels(myPixels, angle);
	
	tex.clear();
	if (bUseTexture == true){
		tex.allocate(myPixels.width, myPixels.height, myPixels.glDataType);
	}
	
	update();
}

//----------------------------------------------------
void ofxImage::flipPixels(ofPixels &pix, bool horizontal, bool vertical){
	if(!horizontal && !vertical)
		return;
	
	FIBITMAP * bmp               = getBmpFromPixels(pix);
	bool horSuccess = false, vertSuccess = false;
	
	if(horizontal)
		horSuccess = FreeImage_FlipHorizontal(bmp);
	if(vertical)
		vertSuccess = FreeImage_FlipVertical(bmp);
	
	if(horSuccess || vertSuccess)
		putBmpIntoPixels(bmp, pix);
	
	if (bmp != NULL)            FreeImage_Unload(bmp);
}

//----------------------------------------------------
void ofxImage::rotatePixels(ofPixels &pix, float angle){
	if(angle == 0.0)
		return;
	
	FIBITMAP * bmp               = getBmpFromPixels(pix);
	FIBITMAP * convertedBmp         = NULL;
	
	convertedBmp = FreeImage_RotateClassic(bmp, angle);
	putBmpIntoPixels(convertedBmp, pix);
	
	if (bmp != NULL)            FreeImage_Unload(bmp);
	if (convertedBmp != NULL)      FreeImage_Unload(convertedBmp);
}	
