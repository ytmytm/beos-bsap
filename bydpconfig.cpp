
#include "bydpconfig.h"
#include <stdio.h>
#include <stdlib.h>

bydpConfig::bydpConfig() {
	load();
}

bydpConfig::~bydpConfig() {

}

char *bydpConfig::readString(const char *buf, const char *token) {
	static char buf2[256];
	char *s, *t;
	s = strstr(buf,token);
	if (s!=NULL) {
		memset(buf2,0,sizeof(buf2));
		strncpy(buf2,&s[strlen(token)],sizeof(buf2)-1);
		t = strchr(buf2,'\n');
		if (t!=NULL)
			*t = '\0';
		return buf2;
	} else {
		return NULL;
	}
}

void bydpConfig::readInt(const char *buf, const char *token, int *result) {
	char *s = strstr(buf,token);
	if (s!=NULL) {
		*result = strtol(&s[strlen(token)],NULL,10);
	}
}

void bydpConfig::readRGB(const char *buf, const char *token, rgb_color *result) {
	BString tmp;
	int res;
	tmp = token;
	tmp += ".red=";
	readInt(buf,tmp.String(),&res);
	result->red = res;
	tmp = token;
	tmp += ".green=";
	readInt(buf,tmp.String(),&res);
	result->green = res;
	tmp = token;
	tmp += ".blue=";
	readInt(buf,tmp.String(),&res);
	result->blue = res;
}

void bydpConfig::readBoolean(const char *buf, const char *token, bool *result) {
	char *res = readString(buf,token);
	if (res != NULL)
		*result = (!strcmp(res,"true"));
}

void bydpConfig::load(void) {
	static char buf[1024];
	char *result;

	setDefaultConfiguration();
	if (conf.SetTo(CONFIG_NAME,B_READ_ONLY) != B_OK) {
		printf("error opening config file for load\n");
		return;
	}
	memset(buf,0,sizeof(buf));
	conf.Read(buf,sizeof(buf)-1);
	conf.Unset();
	result = readString(buf,"topPath=");
	if (result!=NULL) topPath = result;
	readBoolean(buf,"toPolish=",&toPolish);
	readBoolean(buf,"clipboardTracking=",&clipboardTracking);
	readBoolean(buf,"setFocusOnSelf=",&setFocusOnSelf);
	readInt(buf,"distance=",&distance);
	readInt(buf,"searchmode=",&searchmode);
	readInt(buf,"todisplay=",&todisplay);
	readRGB(buf,"colour",&colour);
	readRGB(buf,"colour0",&colour0);
	readRGB(buf,"colour1",&colour1);
	readRGB(buf,"colour2",&colour2);
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
	writeBoolean("setFocusOnSelf",setFocusOnSelf);
	writeInt("distance",distance);
	writeInt("searchmode",searchmode);
	writeInt("todisplay",todisplay);
	writeRGB("colour",colour);
	writeRGB("colour0",colour0);
	writeRGB("colour1",colour1);
	writeRGB("colour2",colour2);
	conf.Unset();
	updateFName();	// don't remove this - needs to be up-to-date for langswitch
}

void bydpConfig::setDefaultConfiguration(void) {
	topPath = "/boot/home/Desktop/beos/sap/lib";
	toPolish = true;
	clipboardTracking = true;
	setFocusOnSelf = true;
	searchmode = SEARCH_BEGINS;
	distance = 3;
	todisplay = 23;

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
//	indexFName = toPolish ? "dict100.idx" : "dict101.idx";
	dataFName = toPolish ? "dvp_1.dic" : "dvp_2.dic";
}
