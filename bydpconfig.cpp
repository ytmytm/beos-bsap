
#include "bydpconfig.h"
#include <stdio.h>
#include <stdlib.h>

bydpConfig::bydpConfig() {
	load();
}

bydpConfig::~bydpConfig() {

}

char *bydpConfig::readValue(const char *buf, const char *token) {
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

void bydpConfig::readValue(const char *buf, const char *token, int *result) {
	char *s = strstr(buf,token);
	if (s!=NULL) {
		*result = strtol(&s[strlen(token)],NULL,10);
	}
}

void bydpConfig::readValue(const char *buf, const char *token, rgb_color *result) {
	BString tmp;
	int res;
	tmp = token;
	tmp += ".red=";
	readValue(buf,tmp.String(),&res);
	result->red = res;
	tmp = token;
	tmp += ".green=";
	readValue(buf,tmp.String(),&res);
	result->green = res;
	tmp = token;
	tmp += ".blue=";
	readValue(buf,tmp.String(),&res);
	result->blue = res;
}

void bydpConfig::readValue(const char *buf, const char *token, bool *result) {
	char *res = readValue(buf,token);
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
	result = readValue(buf,"topPath=");
	if (result!=NULL) topPath = result;
	readValue(buf,"toPolish=",&toPolish);
	readValue(buf,"clipboardTracking=",&clipboardTracking);
	readValue(buf,"setFocusOnSelf=",&setFocusOnSelf);
	readValue(buf,"distance=",&distance);
	readValue(buf,"searchmode=",&searchmode);
	readValue(buf,"todisplay=",&todisplay);
	readValue(buf,"colour",&colour);
	readValue(buf,"colour0",&colour0);
	readValue(buf,"colour1",&colour1);
	readValue(buf,"colour2",&colour2);
	updateFName();
}

void bydpConfig::writeValue(BString variable, rgb_color value) {
	BString line;
	line = variable; line += ".red";
	writeValue(line,value.red);
	line = variable; line += ".green";
	writeValue(line,value.green);
	line = variable; line += ".blue";
	writeValue(line,value.blue);
}

void bydpConfig::writeValue(BString variable, int value) {
	BString line;
	line = variable;
	line += "=";
	line << value;
	line += "\n";
	conf.Write(line.String(),line.Length());
}

void bydpConfig::writeValue(BString variable, BString value) {
	BString line;
	line = variable;
	line += "=";
	line += value;
	line += "\n";
	conf.Write(line.String(),line.Length());
}

void bydpConfig::writeValue(BString variable, bool value) {
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
	writeValue("topPath",topPath);
	writeValue("toPolish",toPolish);
	writeValue("clipboardTracking",clipboardTracking);
	writeValue("setFocusOnSelf",setFocusOnSelf);
	writeValue("distance",distance);
	writeValue("searchmode",searchmode);
	writeValue("todisplay",todisplay);
	writeValue("colour",colour);
	writeValue("colour0",colour0);
	writeValue("colour1",colour1);
	writeValue("colour2",colour2);
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
	dataFName = toPolish ? "dvp_1.dic" : "dvp_2.dic";
}
