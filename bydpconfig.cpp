
#include "bydpconfig.h"

bydpConfig::bydpConfig() {
	load();
}

bydpConfig::~bydpConfig() {

}

void bydpConfig::load(void) {
	setDefaultConfiguration();
	// real load here
	updateFName();
}

void bydpConfig::save(void) {
	updateFName();
}

void bydpConfig::setDefaultConfiguration(void) {
	topPath = "/boot/home/Desktop/beos/kydpdict/";
	toPolish = true;
	clipboardTracking = false;
//	searchmode = SEARCH_BEGINS;
	searchmode = SEARCH_FUZZY;
	distance = 3;
	todisplay = 20;

	colour.red = colour.green = colour.blue = 0;
	colour0.red = colour0.green = 0;
	colour0.blue = 255;
	colour1.red = 255;
	colour1.green = colour1.blue = 0;
	colour2.green = 255;
	colour2.red = colour2.blue = 0;

	updateFName();
}

void bydpConfig::updateFName(void) {
	indexFName = toPolish ? "dict100.idx" : "dict101.idx";
	dataFName = toPolish ? "dict100.dat" : "dict101.dat";
}
