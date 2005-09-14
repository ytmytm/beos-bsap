//
// DB schema:
// CREATE TABLE dictionaries (id INTEGER, name TEXT, desc TEXT, PRIMARY KEY(id));
// CREATE TABLE words (id INTEGER, dictionary INTEGER, key TEXT, desc TEXT, PRIMARY KEY(id,dictionary,key)); 
//
// TODO:
// - update docs, put info about schema, population (BeAcc, QueryLite) and default dict0/1,
//   put commands for sample sqldata, info about utf8 encoding, and weak parser
//   info about docs
// - update translation, remove obsolete, commit updates

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
	dbData = 0;
}

EngineSQ2::~EngineSQ2() {
	int i,j;

	for (i=0;i<2;i++) {
		if (this->dictCache[i].wordCount>0) {
			if (this->dictCache[i].words) {
//				for (j=0;j<this->dictCache[i].wordCount;j++) {
//					delete [] dictCache_LL[j].ids;
//				}
				delete [] dictCache_LL[i].ids;
			}
		}
	}
}

void EngineSQ2::FlushCache(void) {
	int i,j;
	printf ("in flush\n");
	for (i=0;i<2;i++) {
		printf("flush :%i\n",i);
		if (this->dictCache[i].wordCount>0) {
			if (this->dictCache[i].words) {
				printf("have words %i\n", this->dictCache[i].wordCount);
				for (j=0;j<this->dictCache[i].wordCount;j++) {
//					printf("%i\n",j);
					delete [] this->dictCache[i].words[j];
				}
				delete [] this->dictCache_LL[i].ids;
				delete [] this->dictCache[i].words;
			}
		}
	}
	printf("after flush\n");
	this->dictCache[0].wordCount = 0;
	this->dictCache[1].wordCount = 0;
}

int EngineSQ2::OpenDictionary(void) {
	int i;
	BString dat;
	BFile fData;

	dat = this->cnf->topPath;
	dat.Append("/");
	dat += "bsapdict.sq2";

	// fData test wouldn't be necessary if sqlite_open worked as advertised or I don't understand it
	int fResult = fData.SetTo(dat.String(), B_READ_ONLY);
	dbData = sqlite_open(dat.String(), 0444, &dbErrMsg);
	if ((dbData==0)||(dbErrMsg!=0)||(fResult!=B_OK)) {
		// clean up after sqlite_open - file didn't exist before it, but it exists now
		unlink(dat.String());
		BString message;
		message = tr("Couldn't open data file.");
		message << "\n" << dbErrMsg;
		BAlert *alert = new BAlert(APP_NAME, message.String(), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
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
	BString sqlQuery;

	sqlQuery = "SELECT words.id,words.key FROM words,dictionaries WHERE dictionaries.id = words.dictionary AND dictionaries.id =";
	if (this->cnf->toPolish)
		i = this->cnf->sqlDictionary[0];
	else
		i = this->cnf->sqlDictionary[1];
	sqlQuery << i;

	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	this->wordCount = nRows;
	this->words = new char* [this->wordCount];
	this->ids = new int [this->wordCount];
	if (nRows<2) {
		BString message;
		message = tr("Database query returned no results. Please read included documentation for what might be the cause. Program will end now.\n");
		BAlert *alert = new BAlert(APP_NAME, message.String(), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		exit(-1);
	}
	for (i=1;i<=wordCount;i++) {
		len = strlen(result[i*2+1])+1;
		this->words[i-1] = new char[len];
		strcpy(this->words[i-1],result[i*2+1]);
		this->ids[i-1] = strtol(result[i*2],NULL,10);
	}
	sqlite_free_table(result);
}

int EngineSQ2::ReadDefinition(int index) {
	BString sqlQuery;
	int nRows, nCols;
	char **result;
	int i;

	if (this->cnf->toPolish)
		i = this->cnf->sqlDictionary[0];
	else
		i = this->cnf->sqlDictionary[1];
	this->curWord = this->words[index];
	sqlQuery << "SELECT desc FROM words WHERE id = " << this->ids[index];
	sqlQuery << " AND dictionary = " << i;
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

const char *EngineSQ2::AppBarName(void) {

	if (dbData == 0)
		return "No dictionary";

	int i;
	char **result;
	int nRows, nCols;

	if (this->cnf->toPolish)
		i = this->cnf->sqlDictionary[0];
	else
		i = this->cnf->sqlDictionary[1];

	BString sqlQuery = "SELECT name FROM dictionaries WHERE id = ";
	sqlQuery << i;
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	static BString dictName;
	if (nRows<1) {
		dictName = "no name";
	} else {
		dictName = result[1];
	}
	sqlite_free_table(result);
	return dictName.String();
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
	char *c = this->curDefinition;
	while (*c) {
		switch (*c) {
			case '\\':
				this->line += '\n';
				c++;
				c++;
				continue;
			case '[':
				this->UpdateAttr(0);
				c++;
				continue;
			case ']':
				this->UpdateAttr(A_COLOR1);
				c++;
				continue;
		}
		this->line += *c++;
	}
	this->UpdateAttr(0);
}

/////////////////////
// utf8 <-> anything convertion stuff below
//

ConvertSQ2::ConvertSQ2(void) {
}

ConvertSQ2::~ConvertSQ2() {
}
