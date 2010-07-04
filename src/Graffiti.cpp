#include "Graffiti.h"
#include "GMLPoint.h"
#include "ofxImage.h"
#include <vector>
#include <math.h>
#include <limits.h>

//-----------------------------------------------------------------------------------------------

Graffiti::Graffiti(vars *v) {
	
	myFBO.allocate(ofGetWidth(),ofGetHeight());
	myVars = v;

	ofSetColor(0xFFFFFF);//background is black, fill is white. Why? Because ofxFBO likes it that way.
}

//-----------------------------------------------------------------------------------------------

bool Graffiti::loadGML(char gmlLocation[]) {
	if (gmlLocation == NULL) {
		return false;
	}

	strokes.clear();//clear so that sets of gml don't go on top of each other

	ofxXmlSettings GML;
	if (GML.loadFile(gmlLocation)) {
		if(!GML.tagExists("GML")) return false;
		GML.pushTag("GML");
		GML.pushTag("tag");
		GML.pushTag("drawing");
	
		int numStrTags = GML.getNumTags("stroke");//determines the number of strokes in the GML
		printf("there %s %d <stroke> tag%s\n",
			numStrTags > 1 ? "are" : "is", numStrTags, numStrTags > 1 ? "s" : "");//ternary operators are cool.
	
		float prevTime = 0.0;
		for (int i = 0; i < numStrTags; i++) {//iterate through strokes
			GML.pushTag("stroke",i);
			int numPtTags = GML.getNumTags("pt");//number of pts in the stroke

			vector<GMLPoint*> gmlPoints;//temporary vector to hold the points in a stroke
			GMLPoint* previousPoint = new GMLPoint(0,0,0,0,0);//holds the point before the current point.
			printf("there %s %d <pt> tag%s in <stroke> tag %d\n",
				numPtTags > 1 ? "are" : "is" ,numPtTags, numPtTags > 1 ? "s" : "",i+1);

			for (int j = 0; j < numPtTags; j++) {
				float x = GML.getValue("pt:x",0.0,j);
				float y = GML.getValue("pt:y",0.0,j);
				float x2 = y; //This is because OpenFrameworks
				float y2 = 1 - x;//uses the top-left as origin
				float time = GML.getValue("pt:time",0.0,j);
				float angle = atan2(y2 - previousPoint->y, x2 - previousPoint->x);
				float dist = ofDist(x2,y2,previousPoint->x,previousPoint->y);

				previousPoint = new GMLPoint(x2,y2,time - prevTime,angle,dist);
				gmlPoints.push_back(new GMLPoint(x2,y2,time - prevTime,angle,dist));
				prevTime = time;
			}

			Stroke* stroke = new Stroke(gmlPoints);
			strokes.push_back(stroke);
			GML.popTag();
		}
		myVars->bFileLoaded = true;
		return true;
	}
	return false;//if it couldn't load the XML...
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawSelf() {
	myFBO.clear();
	ofNoFill();
	myFBO.begin();

	for (int i = 0; i < strokes.size(); i++) {
		
		Stroke* currentStroke = strokes.at(i);
		ofBeginShape();
		for (int j = 0; j < currentStroke->gmlPoints.size(); j++) {
			ofVertex(
				currentStroke->gmlPoints.at(j)->x * myVars->scaleFactor,
				currentStroke->gmlPoints.at(j)->y * myVars->scaleFactor);
		}
		ofEndShape();
	}

	myFBO.end();
	myFBO.draw(myVars->xLoc,myVars->yLoc,ofGetWidth(),ofGetHeight());
}
//-----------------------------------------------------------------------------------------------
void Graffiti::saveSelf(char fileName[]) {
	
	//lots of code conveniently reused from findContours

	ofxImage graffitiRegular;
	int w = myFBO.getWidth();
	int h = myFBO.getHeight();

	graffitiRegular.setFromPixels((unsigned char*) myFBO.getPixels(),w,h,OF_IMAGE_COLOR_ALPHA);
	graffitiRegular.update();
	
	graffitiRegular.mirror(false,true);
	graffitiRegular.update();
	graffitiRegular.setImageType(OF_IMAGE_GRAYSCALE);

	//create a grayscale image for contour finding
	ofxCvGrayscaleImage graffitiGrayscale;
	graffitiGrayscale.allocate(w,h);
	graffitiGrayscale.setFromPixels(graffitiRegular.getPixels(),w,h);
	graffitiGrayscale.draw(myVars->xLoc,myVars->yLoc);
	
	//get the number of points for use in nConsidered
	int numPoints = 0;
	for(int i = 0; i < strokes.size(); i++) {
		numPoints += strokes.at(i)->gmlPoints.size();		
	}
	
	contourFinder.findContours(graffitiGrayscale,myVars->threshold,ofGetWidth() * ofGetHeight(),numPoints,true);
	
	bool bSmoothOutput = true;
	if (bSmoothOutput == false){
		// OLD METHOD. Produces gnarly lines. dont use. 
	
		output.beginEPS(fileName);
		output.noFill();
		for (int i = 0; i < contourFinder.blobs.size(); i++) {
			output.setColor(0x000000);
			output.beginShape();
			for (int j = 0; j < contourFinder.blobs.at(i).pts.size() - 1; j++) {
				ofPoint pt1 = contourFinder.blobs.at(i).pts.at(j);
				ofPoint pt2 = contourFinder.blobs.at(i).pts.at(j+1);
				output.line(pt1.x,pt1.y,pt2.x,pt2.y);
			}
			ofPoint pt3 = contourFinder.blobs.at(i).pts.at(contourFinder.blobs.at(i).pts.size() - 1);
			ofPoint pt4 = contourFinder.blobs.at(i).pts.at(0);
			output.line(pt3.x,pt3.y,pt4.x,pt4.y);
			output.endShape();
		}
		output.endEPS();
		
		
	} else {
		// HEY, NICE NEW METHOD. 
		// Perform local averaging to smooth marks somewhat.
		// Outputs a true polyline. 
		
		float c1 = 0.500; 
		float c0 = (1.0 - c1)/2.0;
		float c2 = c0; 
		float px, py;
	
		output.beginEPS(fileName);
		output.noFill();
		
		for (int i = 0; i < contourFinder.blobs.size(); i++) {
			output.setColor(0x000000);
			output.setLineWidth(0.35294122); // produces a 1-point line, for whatever reason
			output.beginShape();
			
			int nBlobPts = contourFinder.blobs.at(i).pts.size();
			if (nBlobPts > 3){
				
				for (int j = 0; j <= nBlobPts; j++) {
					ofPoint pt0 = contourFinder.blobs.at(i).pts.at((j-1+nBlobPts)%nBlobPts);
					ofPoint pt1 = contourFinder.blobs.at(i).pts.at((j           )%nBlobPts);
					ofPoint pt2 = contourFinder.blobs.at(i).pts.at((j+1         )%nBlobPts);
					
					// compute dot product to estimate angle
					float dx01 = pt0.x - pt1.x;
					float dy01 = pt0.y - pt1.y;
					float dh01 = sqrtf(dx01*dx01 + dy01*dy01);
					float dx21 = pt2.x - pt1.x;
					float dy21 = pt2.y - pt1.y;
					float dh21 = sqrtf(dx21*dx21 + dy21*dy21); 
					if ((dh21 > 0) && (dh01 > 0)){
						
						float cosTheta = (dx01*dx21 + dy01*dy21)/(dh01*dh21);
						if (ABS(cosTheta) < 0.7){ // keep angles sharper than ~45 degrees
							px = pt1.x;
							py = pt1.y;
						} else {                  // smooth lines otherwise. 
							px = c0*pt0.x + c1*pt1.x + c2*pt2.x;
							py = c0*pt0.y + c1*pt1.y + c2*pt2.y;
						}
						
						output.polyVertex(px,py);
					}
				}
			}
			output.endShape();
		}
		output.endEPS();
	}
	
	ofSetColor(0xFFFFFF);//otherwise we start drawing in black D:
}
//-----------------------------------------------------------------------------------------------
void Graffiti::thicken() {

	vector<Stroke*> rightStrokes;
	vector<Stroke*> leftStrokes;

	//determine the left and right points for each point in the graffiti
	for (int i = 0; i < strokes.size(); i++) {
		Stroke* currentStroke = strokes.at(i);
		Stroke* rightStroke = new Stroke();
		Stroke* leftStroke = new Stroke();

		leftStroke->gmlPoints.push_back(new GMLPoint(
			currentStroke->gmlPoints.at(0)->x,
			currentStroke->gmlPoints.at(0)->y,
			currentStroke->gmlPoints.at(0)->time,
			currentStroke->gmlPoints.at(0)->angle,
			currentStroke->gmlPoints.at(0)->dist));
		rightStroke->gmlPoints.push_back(new GMLPoint(
			currentStroke->gmlPoints.at(0)->x,
			currentStroke->gmlPoints.at(0)->y,
			currentStroke->gmlPoints.at(0)->time,
			currentStroke->gmlPoints.at(0)->angle,
			currentStroke->gmlPoints.at(0)->dist));

		for (int j = 1; j < currentStroke->gmlPoints.size()-1; j++) {
			GMLPoint* pt1 = currentStroke->gmlPoints.at(j);
			float newDist = (myVars->thickness) / 100.0 - ofClamp(pt1->dist,0.003,(myVars->thickness)-0.001);
			newDist = ofClamp(newDist,0.002,newDist);

			float leftX  = pt1->x-(sin(pt1->angle)* newDist);
			float leftY  = pt1->y+(cos(pt1->angle)* newDist);
			float rightX = pt1->x+(sin(pt1->angle)* newDist);
			float rightY = pt1->y-(cos(pt1->angle)* newDist);

			//finally, add them to the new strokes.
			leftStroke->gmlPoints.push_back (new GMLPoint(leftX, leftY,   pt1->time, pt1->angle,pt1->dist));
			rightStroke->gmlPoints.push_back(new GMLPoint(rightX, rightY, pt1->time, pt1->angle,pt1->dist));
		}

		
		leftStroke ->gmlPoints.push_back(new GMLPoint(
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->x,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->y,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->time,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->angle,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->dist));
		rightStroke->gmlPoints.push_back(new GMLPoint(
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->x,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->y,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->time,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->angle,
			currentStroke->gmlPoints.at(currentStroke->gmlPoints.size() - 1)->dist));


		average(.1,leftStroke,rightStroke);
		leftStrokes.push_back(leftStroke);
		rightStrokes.push_back(rightStroke);
	}

	//let's draw our two new strokes.
	myFBO.clear();
	ofFill();
	ofSetColor(0xFFFFFF);

	myFBO.begin();

	for (int i = 0; i < strokes.size(); i++) {
		Stroke* currentLeftStroke = leftStrokes.at(i);
		Stroke* currentRightStroke = rightStrokes.at(i);
		
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j < currentLeftStroke->gmlPoints.size(); j++) {
			glVertex2f(
				currentLeftStroke->gmlPoints.at(j)->x * myVars->scaleFactor,
				currentLeftStroke->gmlPoints.at(j)->y * myVars->scaleFactor);//left vertex
			glVertex2f(
				currentRightStroke->gmlPoints.at(j)->x * myVars->scaleFactor,
				currentRightStroke->gmlPoints.at(j)->y * myVars->scaleFactor);//right vertex
		}
		glEnd();
	}

	myFBO.end();
	myFBO.draw(myVars->xLoc,myVars->yLoc,ofGetWidth(),ofGetHeight());
}

//-----------------------------------------------------------------------------------------------

void Graffiti::showHoles() {
	//need to load into an ofImage, then ofxCvGrayscaleImage, so as to solve problems w/ openCv images.
	ofxImage graffitiRegular;
	int w = myFBO.getWidth();
	int h = myFBO.getHeight();

	graffitiRegular.setFromPixels((unsigned char*) myFBO.getPixels(),w,h,OF_IMAGE_COLOR_ALPHA);
	graffitiRegular.update();
	
	graffitiRegular.mirror(false,true);
	graffitiRegular.update();
	graffitiRegular.setImageType(OF_IMAGE_GRAYSCALE);

	//create a grayscale image for contour finding
	ofxCvGrayscaleImage graffitiGrayscale;
	graffitiGrayscale.allocate(w,h);
	graffitiGrayscale.setFromPixels(graffitiRegular.getPixels(),w,h);
	graffitiGrayscale.draw(myVars->xLoc,myVars->yLoc);
	
	//get the number of points for use in nConsidered
	int numPoints = 0;
	for(int i = 0; i < strokes.size(); i++) {
		numPoints += strokes.at(i)->gmlPoints.size();		
	}
	
	contourFinder.findContours(graffitiGrayscale,myVars->threshold,ofGetWidth() * ofGetHeight(),numPoints,true);
	
	int holeCount = 0;
	for (int i = 0; i < contourFinder.blobs.size(); i++) {
		if (!contourFinder.blobs.at(i).hole) holeCount++;
	}
	for (int i = 0; i < contourFinder.blobs.size(); i++) {
		ofNoFill();
		if (contourFinder.blobs.at(i).hole) { ofSetColor(0x000FFF); 
		} else { ofSetColor(0xFF0099); }
		ofBeginShape();
		for (int j = 0; j < contourFinder.blobs.at(i).pts.size(); j++) {
			ofVertex(contourFinder.blobs.at(i).pts.at(j).x + myVars->xLoc,contourFinder.blobs.at(i).pts.at(j).y + myVars->yLoc);
		}
		ofEndShape(false);
		
		ofRect(contourFinder.blobs.at(i).boundingRect.x + myVars->xLoc, contourFinder.blobs.at(i).boundingRect.y + myVars->yLoc,
			contourFinder.blobs.at(i).boundingRect.width,contourFinder.blobs.at(i).boundingRect.height);

		char numStr[1024];
		sprintf(numStr,"contour #%d",i + 1);
		ofDrawBitmapString(numStr, contourFinder.blobs.at(i).boundingRect.x + myVars->xLoc, contourFinder.blobs.at(i).boundingRect.y + myVars->yLoc);
	}
	ofSetColor(0xFFFFFF);
	
}

//-----------------------------------------------------------------------------------------------

void Graffiti::findContours() {
	thicken();//in case it hasn't been thickened- only finding contours on a thickened image
	
	//need to load into an ofImage, then ofxCvGrayscaleImage, so as to solve problems w/ openCv images.
	ofxImage graffitiRegular;
	int w = myFBO.getWidth();
	int h = myFBO.getHeight();

	graffitiRegular.setFromPixels((unsigned char*) myFBO.getPixels(),w,h,OF_IMAGE_COLOR_ALPHA);
	graffitiRegular.update();
	
	graffitiRegular.mirror(false,true);
	graffitiRegular.update();
	graffitiRegular.setImageType(OF_IMAGE_GRAYSCALE);

	//create a grayscale image for contour finding
	ofxCvGrayscaleImage graffitiGrayscale;
	graffitiGrayscale.allocate(w,h);
	graffitiGrayscale.setFromPixels(graffitiRegular.getPixels(),w,h);
	
	//get the number of points for use in nConsidered
	int numPoints = 0;
	for(int i = 0; i < strokes.size(); i++) {
		numPoints += strokes.at(i)->gmlPoints.size();		
	}
	
	contourFinder.findContours(graffitiGrayscale,myVars->threshold,ofGetWidth() * ofGetHeight(),numPoints,true);
	
	int testsCount = 5;//number of times to attempt to bridge before giving up.
	int holeCount = 0;
	for (int i = 0; i < contourFinder.blobs.size(); i++) {
		if (!contourFinder.blobs.at(i).hole) holeCount++;
	}

	while (holeCount > 1 && testsCount > 0) {
		ofFill();
		myFBO.begin();

		//graffitiGrayscale.draw(myVars->xLoc,myVars->yLoc);
	
		for (int i = 0; i < contourFinder.blobs.size(); i++) {//for each blob in the contourFinder's model
			ofxCvBlob blob = contourFinder.blobs.at(i);
			float blobsLowestX = INT_MAX;
			float blobsLowestY = INT_MAX;
			float blobsHighestX = INT_MIN;
			float blobsHighestY = INT_MIN;
	
			for (int k = 0; k < blob.pts.size(); k++) {
				ofPoint nextPoint = blob.pts.at(k);
				if (nextPoint.x < blobsLowestX) blobsLowestX = nextPoint.x;
				if (nextPoint.y < blobsLowestY) blobsLowestY = nextPoint.y;
				if (nextPoint.x > blobsHighestX) blobsHighestX = nextPoint.x;
				if (nextPoint.y > blobsHighestY) blobsHighestY = nextPoint.y;
			}

			if (!blob.hole) {//apparently, hole means it's not a hole?
				ofxCvBlob surroundingBlob;
			
				//Now we determine the blob that surrounds 
				for (int j = 0; j < contourFinder.blobs.size(); j++) {
					if (j != i && contourFinder.blobs.at(j).hole) {
						ofxCvBlob nextBlob = contourFinder.blobs.at(j);
						float sBlobsLowestX = INT_MAX;
						float sBlobsLowestY = INT_MAX;
						float sBlobsHighestX = INT_MIN;
						float sBlobsHighestY = INT_MIN;
					
						//is nextBlob blob's surrounding blob?
						//test to see if centroid is within four outermost points
						for (int k = 0; k < nextBlob.pts.size(); k++) {
							ofPoint nextPoint = nextBlob.pts.at(k);
							if (nextPoint.x < sBlobsLowestX) sBlobsLowestX = nextPoint.x;
							if (nextPoint.y < sBlobsLowestY) sBlobsLowestY = nextPoint.y;
							if (nextPoint.x > sBlobsHighestX) sBlobsHighestX = nextPoint.x;
							if (nextPoint.y > sBlobsHighestY) sBlobsHighestY = nextPoint.y;
						}

						if (blobsLowestX > sBlobsLowestX && blobsHighestX < sBlobsHighestX &&
							blobsLowestY > sBlobsLowestY && blobsLowestY < sBlobsHighestY) {
								surroundingBlob = contourFinder.blobs.at(j);
						}
					}
				}
			
				//now we know surroundingBlob surrounds blob.

				//we can find the two closest points in the two.
				if (myVars->findClosestBridge) {
					float lowestDifference =  INT_MAX;
					ofPoint blobsClosestPoint;
					ofPoint SurroundingBlobsClosestPoint;
					
					int closestPointLoc = 0;
					int surroundingPointLoc = 0;
					
					int nBlobPoints = blob.pts.size();
					int nSurroundingBlobPoints = surroundingBlob.pts.size();
					float pointDistSq;
					float lowestDistSq = INT_MAX;
					float pointDist;

					for (int j = 0; j < nBlobPoints; j++) {
						ofPoint pt1 = blob.pts.at(j);
						
						for (int k = 0; k < nSurroundingBlobPoints; k++) {
							ofPoint pt2 = surroundingBlob.pts.at(k);
							pointDistSq = (pt1.x-pt2.x)*(pt1.x-pt2.x) + 
								          (pt1.y-pt2.y)*(pt1.y-pt2.y);

							if (pointDistSq < lowestDistSq) {
								lowestDistSq = pointDistSq;
								blobsClosestPoint = pt1;
								closestPointLoc = j;
								SurroundingBlobsClosestPoint = pt2;
								surroundingPointLoc = k;
							}
						}
					}

					//draw a line between the two points
					if (blob.area > 0) ofSetColor(0x000000);//if it's negative space
					else if (blob.area < 0) ofSetColor(0xFFFFFF);//if it's positive space
					drawLine (blobsClosestPoint,SurroundingBlobsClosestPoint);
					//drawLine(blob.pts.at(closestPointLoc - 2),blobsClosestPoint,
						//blob.pts.at(closestPointLoc + 2),
						//surroundingBlob.pts.at(surroundingPointLoc - 2),
						//SurroundingBlobsClosestPoint,
						//surroundingBlob.pts.at(surroundingPointLoc + 2));
				}

				//we can draw a straight line up from the highest point
				if (myVars->highestBridge) {
					ofPoint blobsHighestPoint = blob.pts.at(0);
					ofPoint surroundingBlobsHighestPoint = surroundingBlob.pts.at(0);
	
					for (int j = 1; j < blob.pts.size(); j++) {
						if (blob.pts.at(j).y < blobsHighestPoint.y)
							blobsHighestPoint = blob.pts.at(j);
					}

					//now we move into the raster domain in order to find closest black pixel in proper direction
					int yTemp = blobsHighestPoint.y;
					int index = blobsHighestPoint.y * graffitiGrayscale.getWidth() + blobsHighestPoint.x;
					unsigned char level = graffitiGrayscale.getPixels()[index];
					while((level > 0) && yTemp > 0) {
						yTemp--;
						index = yTemp * graffitiGrayscale.getWidth()
							+ blobsHighestPoint.x;
						level = graffitiGrayscale.getPixels()[index];
					}

					surroundingBlobsHighestPoint.x = blobsHighestPoint.x;
					surroundingBlobsHighestPoint.y = yTemp;
					blobsHighestPoint.y = blobsHighestPoint.y + 1;//change to make sure full line

					//draw a line between the two points
					if (blob.area > 0) ofSetColor(0x000000);//if it's negative space
					else if (blob.area < 0) ofSetColor(0xFFFFFF);//if it's positive space
					drawLine(blobsHighestPoint,surroundingBlobsHighestPoint);
				}

				//lowest bridges
				if (myVars->lowestBridge) {
					ofPoint blobsLowestPoint = blob.pts.at(0);
					ofPoint surroundingBlobsLowestPoint = surroundingBlob.pts.at(0);
	
					for (int j = 1; j < blob.pts.size(); j++) {
						if (blob.pts.at(j).y > blobsLowestPoint.y)
							blobsLowestPoint = blob.pts.at(j);
					}

					//now we move into the raster domain in order to find closest black pixel in proper direction
					int yTemp = blobsLowestPoint.y;
					int index = blobsLowestPoint.y * graffitiGrayscale.getWidth() + blobsLowestPoint.x;
					unsigned char level = graffitiGrayscale.getPixels()[index];
					while((level > 0) && yTemp < graffitiGrayscale.getHeight()) {
						yTemp++;
						index = yTemp * graffitiGrayscale.getWidth()
							+ blobsLowestPoint.x;
						level = graffitiGrayscale.getPixels()[index];
					}

					surroundingBlobsLowestPoint.x = blobsLowestPoint.x;
					surroundingBlobsLowestPoint.y = yTemp;
					blobsLowestPoint.y = blobsLowestPoint.y - 1;//change to make sure full line

					//draw a line between the two points
					if (blob.area > 0) ofSetColor(0x000000);//if it's negative space
					else if (blob.area < 0) ofSetColor(0xFFFFFF);//if it's positive space
					drawLine(blobsLowestPoint,surroundingBlobsLowestPoint);
				}

				//left most bridges
				if (myVars->leftMostBridge) {
					ofPoint blobsLeftMostPoint = blob.pts.at(0);
					ofPoint surroundingBlobsLeftMostPoint = surroundingBlob.pts.at(0);
	
					for (int j = 1; j < blob.pts.size(); j++) {
						if (blob.pts.at(j).x < blobsLeftMostPoint.x)
							blobsLeftMostPoint = blob.pts.at(j);
					}

					//now we move into the raster domain in order to find closest black pixel in proper direction
					int xTemp = blobsLeftMostPoint.x;
					int index = blobsLeftMostPoint.y * graffitiGrayscale.getWidth() + blobsLeftMostPoint.x;
					unsigned char level = graffitiGrayscale.getPixels()[index];
					while(level > 0 && xTemp > 0) {
						xTemp--;
						index = blobsLeftMostPoint.y * graffitiGrayscale.getWidth()
							+ xTemp;
						level = graffitiGrayscale.getPixels()[index];
					}

					surroundingBlobsLeftMostPoint.x = xTemp;
					surroundingBlobsLeftMostPoint.y = blobsLeftMostPoint.y;
					blobsLeftMostPoint.x = blobsLeftMostPoint.x + 1;//change to make sure full line

					//draw a line between the two points
					if (blob.area > 0) ofSetColor(0x000000);//if it's negative space
					else if (blob.area < 0) ofSetColor(0xFFFFFF);//if it's positive space
					drawLine(blobsLeftMostPoint,surroundingBlobsLeftMostPoint);
				}

				//right most bridges
				if (myVars->rightMostBridge) {
					ofPoint blobsRightMostPoint = blob.pts.at(0);
					ofPoint surroundingBlobsRightMostPoint = surroundingBlob.pts.at(0);
	
					for (int j = 1; j < blob.pts.size(); j++) {
						if (blob.pts.at(j).x > blobsRightMostPoint.x)
							blobsRightMostPoint = blob.pts.at(j);
					}
					
										//now we move into the raster domain in order to find closest black pixel in proper direction
					int xTemp = blobsRightMostPoint.x;
					int index = blobsRightMostPoint.y * graffitiGrayscale.getWidth() + blobsRightMostPoint.x;
					unsigned char level = graffitiGrayscale.getPixels()[index];
					while(level > 0 && xTemp < graffitiGrayscale.getWidth()) {
						xTemp++;
						index = blobsRightMostPoint.y * graffitiGrayscale.getWidth()
							+ xTemp;
						level = graffitiGrayscale.getPixels()[index];
					}

					surroundingBlobsRightMostPoint.x = xTemp;
					surroundingBlobsRightMostPoint.y = blobsRightMostPoint.y;
					blobsRightMostPoint.x = blobsRightMostPoint.x - 1;//change to make sure full line

					//draw a line between the two points
					if (blob.area > 0) ofSetColor(0x000000);//if it's negative space
					else if (blob.area < 0) ofSetColor(0xFFFFFF);//if it's positive space
					drawLine(blobsRightMostPoint,surroundingBlobsRightMostPoint);
				}
			}
		}
		ofSetColor(0xFFFFFF);//reset to white so we don't get all black at next cycle
		myFBO.end();
		myFBO.draw(myVars->xLoc,myVars->yLoc,ofGetWidth(),ofGetHeight());

		testsCount--;
		holeCount = 0;
		for (int i = 0; i < contourFinder.blobs.size(); i++) {
			if (contourFinder.blobs.at(i).hole) holeCount++;
		}
	}
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawCircle(float x, float y, float r) {
	ofCircle(x,y,r);
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawCircle(GMLPoint* point, float r) {
	ofCircle(point->x * myVars->scaleFactor, point->y * myVars->scaleFactor, r);
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawCircle(ofPoint point, float r) {
	ofCircle(point.x, point.y, r);
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawLine(ofPoint pt1, ofPoint pt2) {
	// this is used to draw the bridges on the stencils

	float totDist = ofDist(pt1.x,pt1.y,pt2.x,pt2.y);
	float increment = totDist/(myVars->contourThickness * 2.0) * 10.0;
	float xDist = (pt2.x - pt1.x) / increment;
	float yDist = (pt2.y - pt1.y) / increment;
		
	for (float k = 0; k <= increment; k++) {
		float cx = pt1.x + xDist* k;
		float cy = pt1.y + yDist* k;
		float cr = myVars->contourThickness;

		drawCircle(cx,cy,cr);
		//it's contour thickness here because it only ever gets used w/ contour lines- sloppy, but works.
	}
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawLine(GMLPoint* pt1, GMLPoint* pt2) {
	
	float totDist = ofDist(pt1->x,pt1->y,pt2->x,pt2->y);
	float increment = totDist/(myVars->contourThickness * 2) * 10;
	float xDist = (pt2->x - pt1->x) * myVars->scaleFactor / increment;
	float yDist = (pt2->y - pt1->y) * myVars->scaleFactor / increment;
	
	//drawCircle(pt1,(myVars->contourThickness));
	for(float k = 0; k < increment - 1; k++) {
		drawCircle(pt1->x * myVars->scaleFactor + xDist* k,pt1->y * myVars->scaleFactor + yDist*k,(myVars->contourThickness));
	}
}

//-----------------------------------------------------------------------------------------------

void Graffiti::drawLine(ofPoint pt1, ofPoint pt2, ofPoint pt3, ofPoint pt4, ofPoint pt5, ofPoint pt6) {
	ofBeginShape();
		ofVertex(pt1.x,pt1.y);
		ofVertex(pt2.x,pt2.y);
		ofVertex(pt3.x,pt3.y);
		//we add two, because otherwise it doesn't quite reach the outside.
		ofVertex(pt4.x + 2,pt4.y + 2);
		ofVertex(pt5.x + 2,pt5.y + 2);
		ofVertex(pt6.x + 2,pt6.y + 2);
	ofEndShape();

}

//-----------------------------------------------------------------------------------------------

float Graffiti::getHeight() {
	ofxImage graffitiRegular;
	int w = myFBO.getWidth();
	int h = myFBO.getHeight();

	graffitiRegular.setFromPixels((unsigned char*) myFBO.getPixels(),w,h,OF_IMAGE_COLOR_ALPHA);
	graffitiRegular.update();
	
	graffitiRegular.mirror(false,true);
	graffitiRegular.update();
	graffitiRegular.setImageType(OF_IMAGE_GRAYSCALE);

	//create a grayscale image for contour finding
	ofxCvGrayscaleImage graffitiGrayscale;
	graffitiGrayscale.allocate(w,h);
	graffitiGrayscale.setFromPixels(graffitiRegular.getPixels(),w,h);
	
	//get the number of points for use in nConsidered
	int numPoints = 0;
	for(int i = 0; i < strokes.size(); i++) {
		numPoints += strokes.at(i)->gmlPoints.size();		
	}
	contourFinder.findContours(graffitiGrayscale,myVars->threshold,ofGetWidth() * ofGetHeight(),numPoints,true);
	return contourFinder.blobs.at(0).boundingRect.height;
}

//-----------------------------------------------------------------------------------------------

float Graffiti::getWidth() {
	ofxImage graffitiRegular;
	int w = myFBO.getWidth();
	int h = myFBO.getHeight();

	graffitiRegular.setFromPixels((unsigned char*) myFBO.getPixels(),w,h,OF_IMAGE_COLOR_ALPHA);
	graffitiRegular.update();
	
	graffitiRegular.mirror(false,true);
	graffitiRegular.update();
	graffitiRegular.setImageType(OF_IMAGE_GRAYSCALE);

	//create a grayscale image for contour finding
	ofxCvGrayscaleImage graffitiGrayscale;
	graffitiGrayscale.allocate(w,h);
	graffitiGrayscale.setFromPixels(graffitiRegular.getPixels(),w,h);
	
	//get the number of points for use in nConsidered
	int numPoints = 0;
	for(int i = 0; i < strokes.size(); i++) {
		numPoints += strokes.at(i)->gmlPoints.size();		
	}
	contourFinder.findContours(graffitiGrayscale,myVars->threshold,ofGetWidth() * ofGetHeight(),numPoints,true);
	return contourFinder.blobs.at(0).boundingRect.width;
}

//-----------------------------------------------------------------------------------------------

//average code courtesy of dustTag 2.0
void Graffiti::average(float pct, Stroke* left,  Stroke* right) {
    float pctSis = (1-pct) * .5f;
	
    for( int i = 1; i < left->gmlPoints.size(); i++) {
		
		if( i >= 2  ) {
			left->gmlPoints[i-1] -> x = (
				 pctSis * left->gmlPoints[i-2]->x) +
				(pct * left->gmlPoints[i-1]->x) +
				(pctSis * left->gmlPoints[i]->x
				);
			
			left->gmlPoints[i-1] -> y = (
				 pctSis * left->gmlPoints[i-2]->y) +
				(pct * left->gmlPoints[i-1]->y) +
				(pctSis * left->gmlPoints[i]->y
				);
		
		} else if( i== 1 ) {
			left->gmlPoints[i-1] -> x = (
				 pctSis * left->gmlPoints[i+1]->x) +
				(pct * left->gmlPoints[i-1]->x) + 
				(pctSis * left->gmlPoints[i]->x
				);
			
			left->gmlPoints[i-1] -> y = (
				 pctSis * left->gmlPoints[i+1]->y) +
				(pct * left->gmlPoints[i-1]->y) +
				(pctSis * left->gmlPoints[i]->y
				);
		}
		
		if( i == left->gmlPoints.size()-1 ) {
			left->gmlPoints[i] -> x = (
				pctSis * left->gmlPoints[i-2]->x) +
				(pctSis * left->gmlPoints[i-1]->x) +
				(pct * left->gmlPoints[i]->x
				);
			
			left->gmlPoints[i] -> y = (
				pctSis * left->gmlPoints[i-2]->y) +
				(pctSis * left->gmlPoints[i-1]->y) +
				(pct * left->gmlPoints[i]->y
				);
		}
    }
	
	
    for( int i = 1; i < right->gmlPoints.size(); i++) {
        if( i >= 2 ) {
			right->gmlPoints[i-1]->x = (pctSis * right->gmlPoints[i-2]->x) +
				(pct * right->gmlPoints[i-1]->x) + (pctSis * right->gmlPoints[i]->x);
			right->gmlPoints[i-1]->y = (pctSis * right->gmlPoints[i-2]->y) +
				(pct * right->gmlPoints[i-1]->y) + (pctSis * right->gmlPoints[i]->y);
		} else if( i == 1 ) {
			right->gmlPoints[i-1]->x = (pctSis * right->gmlPoints[i+1]->x) +
				(pct * right->gmlPoints[i-1]->x) + (pctSis * right->gmlPoints[i]->x);
			right->gmlPoints[i-1]->y = (pctSis * right->gmlPoints[i+1]->y) +
				(pct * right->gmlPoints[i-1]->y) + (pctSis * right->gmlPoints[i]->y);
		}		
		if( i == right->gmlPoints.size()-1 ) {
			right->gmlPoints[i]->x = (pctSis * right->gmlPoints[i-2]->x) +
				(pctSis * right->gmlPoints[i-1]->x) + (pct * right->gmlPoints[i]->x);
			right->gmlPoints[i]->y = (pctSis * right->gmlPoints[i-2]->y) +
				(pctSis * right->gmlPoints[i-1]->y) + (pct * right->gmlPoints[i]->y);
		}
    }
}