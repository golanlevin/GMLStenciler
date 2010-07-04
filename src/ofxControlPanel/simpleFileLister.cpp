#include "simpleFileLister.h"

simpleFileLister::simpleFileLister(){
	bRevSort        = false;
	selectedChanged = false;
	selected        = 0;
	lastDirectory   = "";
}

//-------------------------------------------------------------
int simpleFileLister::refreshDir(){

	if( lastDirectory != ""){
		 int num =  simpleFileLister::listDir(lastDirectory);
		 return num;
	}
	else return 0;
}


//-------------------------------------------------------------
int simpleFileLister::listDirWithExtension(string directory, string fileExtension){
	
	bRevSort        = false;
	entries.clear();
	
	ofxDirList::reset();
	int numFiles = ofxDirList::listDir(directory);
	int countWithExtension = 0; 
	for (int i=0; i<numFiles; i++){
		string aFileName = ofxDirList::getName(i);
		int posExtension = aFileName.find (fileExtension, 0);
		if (posExtension != aFileName.npos){
			countWithExtension++;
		}
	}
	printf("Found %d %s files in the 'data' folder.\n", countWithExtension, fileExtension.c_str());
	entries.assign(countWithExtension, entry());
	
	int count = 0; 
	for (int i = 0; i < numFiles; i++){
		string aFileName = ofxDirList::getName(i);
		int posExtension = aFileName.find (fileExtension, 0);
		if (posExtension != aFileName.npos){
			entries[count].filename = ofxDirList::getName(i);
			entries[count].fullpath = ofxDirList::getPath(i); 
			count++;
		}
	}
	
	lastDirectory = directory;
	return entries.size();
}



//-------------------------------------------------------------
int simpleFileLister::listDir(string directory){
	bRevSort        = false;
	entries.clear();

	ofxDirList::reset();
	int numFiles = ofxDirList::listDir(directory);
	
	entries.assign(numFiles, entry());

	for(int i = 0; i < numFiles; i++){
		entries[i].filename = ofxDirList::getName(i);
		entries[i].fullpath = ofxDirList::getPath(i);
	}

	lastDirectory = directory;
	return entries.size();
}

//-------------------------------------------------------------
void simpleFileLister::reverseOrder(){
	if( entries.size() ){
		reverse(entries.begin(), entries.end());
		bRevSort = !bRevSort;
	}
}

//-------------------------------------------------------------
bool simpleFileLister::selectedHasChanged(){
	return selectedChanged;
}

//-------------------------------------------------------------
void simpleFileLister::clearChangedFlag(){
	selectedChanged = false;
}

//------------------------------------------------
string simpleFileLister::getName(int which){
	if( which >= 0 && which < entries.size() ){
		return entries[which].filename;
	}else{
		return "";
	}
}

//------------------------------------------------
string simpleFileLister::getPath(int which){
	if( which >= 0 && which < entries.size() ){
		return entries[which].fullpath;
	}else{
		return "";
	}
}

//------------------------------------------------
void simpleFileLister::setSelectedFile(int which){
	if( which >= 0 && which < entries.size() ){
		selected    = which;
		selectedChanged = true;
	}
}

//------------------------------------------------
string simpleFileLister::getSelectedName(){
	if( selected >= 0 && selected < entries.size() ){
		return entries[selected].filename;
	}else{
		return "";
	}
}

//------------------------------------------------
string simpleFileLister::getSelectedPath(){
	if( selected >= 0 && selected < entries.size() ){
		return entries[selected].fullpath;
	}else{
		return "";
	}
}
