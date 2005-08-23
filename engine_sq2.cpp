//
// DB schema:
// CREATE TABLE words (id INTEGER, key TEXT, desc TEXT, PRIMARY KEY(id,key)); 
//
// TODO:
// - check for errors and/or empty results set, use dbErrMsg
// - put both directions of translation into one DB?
// - decide about name
// - update docs

#include <stdio.h>
#include <string.h>
#include <Alert.h>
#include <SpLocaleApp.h>

#include "globals.h"
#include "engine_sq2.h"

//
// to prevent my confusion, everything from base class is prefixed with this: this-> (wow, that's recursive :)

EngineSQ2::EngineSQ2(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter) : ydpDictionary(output,dict,config,converter) {
	int i;
	for (i=0;i<2;i++) {
		dictCache_LL[i].ids = NULL;
	}
	dbErrMsg = 0;
}

EngineSQ2::~EngineSQ2() {
	int i,j;

	for (i=0;i<2;i++) {
		if (this->dictCache[i].wordCount>0) {
			if (this->dictCache[i].words) {
				for (j=0;j<this->dictCache[i].wordCount;j++) {
					delete [] dictCache_LL[j].ids;
				}
			}
		}
	}
}

int EngineSQ2::OpenDictionary(void) {
	int i;
	BString dat;

	dat = this->cnf->topPath;
	dat.Append("/");
	dat += this->cnf->toPolish ? "sap_en-pl.sq2" : "sap_pl-en.sq2";	// XXX what about these???

	dbData = sqlite_open(dat.String(), 0444, &dbErrMsg);
	if (dbErrMsg!=0) {
		BAlert *alert = new BAlert(APP_NAME, tr("Couldn't open data file."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}

	i = 0;
	if (!(this->cnf->toPolish)) i++;
	if (this->dictCache[i].wordCount>0) {
		ids = dictCache_LL[i].ids;
	} else {
		FillWordList();
		dictCache_LL[i].ids = ids;
	}
	return ydpDictionary::OpenDictionary();	// required call
}

void EngineSQ2::CloseDictionary(void) {
	sqlite_close(dbData);
	ydpDictionary::CloseDictionary();	// required call
}

void EngineSQ2::FillWordList(void) {
	int i, len, nRows, nCols;
	char **result;

	sqlite_get_table(dbData, "SELECT id, key FROM words", &result, &nRows, &nCols, &dbErrMsg);
	this->wordCount = nRows;
	this->words = new char* [this->wordCount];
	this->ids = new int [this->wordCount];
	for (i=1;i<=wordCount;i++) {
		len = strlen(result[i*2+1])+1;
		this->words[i-1] = new char[len];
		strcpy(this->words[i-1],result[i*2+1]);
		sscanf(result[i*2],"%i",&(this->ids[i-1]));
	}
	sqlite_free_table(result);
}

int EngineSQ2::ReadDefinition(int index) {
	BString sqlQuery;
	int nRows, nCols;
	char **result;

	this->curWord = this->words[index];
	sqlQuery << "SELECT desc FROM words WHERE id = " << this->ids[index];
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);

	delete [] this->curDefinition;
	this->curDefinition = new char [strlen(result[1])+1];
	strcpy(this->curDefinition, result[1]);

	sqlite_free_table(result);

	return 0;
}

const char *EngineSQ2::ColourFunctionName(int index) {
	switch(index) {
		case 0:
			return tr("Translation colour");
			break;
		case 1:
			return tr("Keywords colour");
			break;
		case 2:
			return tr("Qualifiers colour");
			break;
		case 3:
			return tr("Additional text colour");
			break;
		default:
			break;
	}
	return "Illegal index";
}

//
// parses format and (via UpdateAttr and convert) outputs data
void EngineSQ2::ParseRTF(void) {

	this->ClearView();
	this->textlen = 0;

	this->line.SetTo(this->curWord);
	this->UpdateAttr(A_BOLD|A_COLOR0);
	this->line += " - ";
	this->UpdateAttr(0);
	this->line.SetTo(this->curDefinition);

	this->UpdateAttr(0);
}

/////////////////////
// utf8 <-> anything convertion stuff below
//

ConvertSQ2::ConvertSQ2(void) {
}

ConvertSQ2::~ConvertSQ2() {
}
