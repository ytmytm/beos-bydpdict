
#include "bydpconfig.h"
#include <stdio.h>

bydpConfig::bydpConfig() {
	load();
}

bydpConfig::~bydpConfig() {

}

void bydpConfig::load(void) {
	char buf[1024];
	setDefaultConfiguration();
	if (conf.SetTo(CONFIG_NAME,B_READ_ONLY) != B_OK) {
		printf("error opening config file for load\n");
		return;
	}
	conf.Read(buf,1024);
	conf.Unset();
	printf("read config:%s\n",buf);
	// podzial na stringi przez #10
	// parsowanie wynikow
	updateFName();
}

void bydpConfig::writeRGB(BString variable, rgb_color value) {
	BString line;
	line = variable; line += ".red";
	writeInt(line,value.red);
	line = variable; line += ".green";
	writeInt(line,value.green);
	line = variable; line += ".blue";
	writeInt(line,value.blue);
}

void bydpConfig::writeInt(BString variable, int value) {
	BString line;
	line = variable;
	line += "=";
	line << value;
	line += "\n";
	conf.Write(line.String(),line.Length());
}

void bydpConfig::writeString(BString variable, BString value) {
	BString line;
	line = variable;
	line += "=";
	line += value;
	line += "\n";
	conf.Write(line.String(),line.Length());
}

void bydpConfig::writeBoolean(BString variable, bool value) {
	BString line;
	line = variable;
	line += "=";
	if (value)
		line += "true\n";
	else
		line += "false\n";
	conf.Write(line.String(),line.Length());
}

void bydpConfig::save(void) {
	if (conf.SetTo(CONFIG_NAME,B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE) != B_OK) {
		printf("error opening config file for save\n");
		return;
	}
	writeString("topPath",topPath);
	writeBoolean("toPolish",toPolish);
	writeBoolean("clipboardTracking",clipboardTracking);
	writeInt("distance",distance);
	writeInt("searchmode",searchmode);
	writeInt("todisplay",todisplay);
	writeRGB("colour",colour);
	writeRGB("colour0",colour0);
	writeRGB("colour1",colour1);
	writeRGB("colour2",colour2);
	conf.Unset();
}

void bydpConfig::setDefaultConfiguration(void) {
//	topPath = "/boot/home/Desktop/beos/kydpdict/";
	topPath = "/boot/home/Desktop/beos/";
	toPolish = true;
	clipboardTracking = false;
	searchmode = SEARCH_BEGINS;
//	searchmode = SEARCH_FUZZY;
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
