
#include <stdlib.h>
#include <Path.h>
#include <Alert.h>
#include <SpLocaleApp.h>
#include "globals.h"
#include "bydpconfig.h"

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

void bydpConfig::readValue(const char *buf, const char *token, BFont *result) {
	BString tmp;
	char *res;
	int size;

	tmp = token;
	tmp += ".family=";
	res = readValue(buf,tmp.String());
	result->SetFamilyAndStyle(res,NULL);
	tmp = token;
	tmp += ".style=";
	res = readValue(buf,tmp.String());
	result->SetFamilyAndStyle(NULL,res);
	tmp = token;
	tmp += ".size=";
	readValue(buf,tmp.String(),&size);
	result->SetSize(size);
}

void bydpConfig::readValue(const char *buf, const char *token, BRect *result) {
	BString tmp;
	int pos;
	tmp = token;
	tmp += ".top=";
	readValue(buf,tmp.String(),&pos);
	result->top = pos;
	tmp = token;
	tmp += ".bottom=";
	readValue(buf,tmp.String(),&pos);
	result->bottom = pos;
	tmp = token;
	tmp += ".left=";
	readValue(buf,tmp.String(),&pos);
	result->left = pos;
	tmp = token;
	tmp += ".right=";
	readValue(buf,tmp.String(),&pos);
	result->right = pos;
}

void bydpConfig::load(void) {
	static char buf[1024];
	char *result;

	setDefaultConfiguration();
	if (conf.SetTo(CONFIG_NAME,B_READ_ONLY) != B_OK)
		return;

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
	readValue(buf,"dictionarymode=",&dictionarymode);
	readValue(buf,"colour",&colour);
	readValue(buf,"colour0",&colour0);
	readValue(buf,"colour1",&colour1);
	readValue(buf,"colour2",&colour2);
	readValue(buf,"currentFont",&currentFont);
	readValue(buf,"position",&position);
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

void bydpConfig::writeValue(BString variable, BFont font) {
	BString line;
	BString val;
	font_family family;
	font_style style;
	float fsize;
	int size;

	font.GetFamilyAndStyle(&family,&style);
	line = variable;
	line += ".family";
	val = family;
	writeValue(line,val);
	line = variable;
	line += ".style";
	val = style;
	writeValue(line,val);
	line = variable;
	line += ".size";
	fsize = font.Size();
	size = int(fsize);
	writeValue(line,size);
}

void bydpConfig::writeValue(BString variable, BRect value) {
	BString line;
	int pos;
	line = variable;
	line += ".top";
	pos = int(value.top);
	writeValue(line,pos);
	line = variable;
	line += ".bottom";
	pos = int(value.bottom);
	writeValue(line,pos);
	line = variable;
	line += ".left";
	pos = int(value.left);
	writeValue(line,pos);
	line = variable;
	line += ".right";
	pos = int(value.right);
	writeValue(line,pos);
}

void bydpConfig::save(void) {
	if (conf.SetTo(CONFIG_NAME,B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, tr("Error writing configuration file."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return;
	}
	writeValue("topPath",topPath);
	writeValue("toPolish",toPolish);
	writeValue("clipboardTracking",clipboardTracking);
	writeValue("setFocusOnSelf",setFocusOnSelf);
	writeValue("distance",distance);
	writeValue("searchmode",searchmode);
	writeValue("dictionarymode",dictionarymode);
	writeValue("colour",colour);
	writeValue("colour0",colour0);
	writeValue("colour1",colour1);
	writeValue("colour2",colour2);
	writeValue("currentFont",currentFont);
	writeValue("position",position);
	conf.Unset();
}

void bydpConfig::setDefaultConfiguration(void) {
	BPath path("./");
	topPath = path.Path();
	toPolish = true;
	clipboardTracking = true;
	setFocusOnSelf = true;
	searchmode = SEARCH_BEGINS;
	dictionarymode = DICTIONARY_SAP;
	distance = 3;
	currentFont = be_plain_font;

	colour.red = colour.green = colour.blue = 0;
	colour0.red = colour0.green = 0;
	colour0.blue = 255;
	colour1.red = 255;
	colour1.green = colour1.blue = 0;
	colour2.green = 255;
	colour2.red = colour2.blue = 0;

	position.left = 64; position.top = 64;
	position.right = 585; position.bottom = 480;

}
