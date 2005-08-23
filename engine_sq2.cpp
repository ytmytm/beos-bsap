
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
		dictCache_LL[i].definitions = NULL;
	}
	dbErrMsg = 0;
	dbSqlVm = 0;
	dbSqlTail = 0;
}

EngineSQ2::~EngineSQ2() {
	int i,j;

	for (i=0;i<2;i++) {
		if (this->dictCache[i].wordCount>0) {
			if (this->dictCache[i].words) {
				for (j=0;j<this->dictCache[i].wordCount;j++) {
					delete [] dictCache_LL[j].definitions;
				}
			}
		}
	}
}

int EngineSQ2::OpenDictionary(void) {
	int i;
	BString dat;

	printf("open dict\n");

	dat = this->cnf->topPath;
	dat.Append("/");
	dat += this->cnf->toPolish ? "sap_en-pl.sq2" : "sap_pl-en.sq2";

	dbData = sqlite_open(dat.String(), 0444, &dbErrMsg);
	printf("err: %s, db:%i\n", dbErrMsg, dbData);
	if (dbData < 0) {
//	if ((fData.SetTo(dat.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, tr("Couldn't open data file."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}

	i = 0;
	if (!(this->cnf->toPolish)) i++;
	if (this->dictCache[i].wordCount>0) {
		definitions = dictCache_LL[i].definitions;
	} else {
		FillWordList();
		dictCache_LL[i].definitions = definitions;
	}

	return ydpDictionary::OpenDictionary();	// required call
}

void EngineSQ2::CloseDictionary(void) {
//	fData.Unset();
	printf("close\n");
	sqlite_close(dbData);
	ydpDictionary::CloseDictionary();	// required call
}

void EngineSQ2::FillWordList(void) {
static char blabla[] = "blbalba";
	this->wordCount = 1;
	this->words = new char* [this->wordCount];
//	definitions = new char* [this->wordCount];
	this->words[0] = blabla;
}

int EngineSQ2::ReadDefinition(int index) {
	return 0;

	this->curWord = this->words[index];
	this->curDefinition = definitions[index];
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
	char *c = this->curDefinition;
	this->ClearView();
	this->textlen = 0;

	this->line.SetTo(this->curWord);
	this->UpdateAttr(A_BOLD|A_COLOR0);
	this->line += " - ";
	this->UpdateAttr(0);
	this->line.SetTo("testing");

	this->UpdateAttr(0);
}

/////////////////////
// utf8 <-> anything convertion stuff below
//

#define TABLE_UTF8 { \
		"~", \
		".", ".", "<o>", "<3>", ".", "<|>", "<E>", "<^>", "<e>", \
		"θ", "<i>", "<a>", ".", ":", "´", ".", "ŋ", \
		".", ".", ".", ".", ".", ".", "ð", "æ", \
		".", ".", ".", ".", ".", ".", ".", ".", \
		"Ą", ".", "Ł", "¤", "Ľ", "Ś", "§", "¨", \
		"Š", "Ş", "Ť", "Ź", "­", "Ž", "Ż", "°", \
		"ą", ".", "ł", "´", "ľ", "ś", ".", "¸", \
		"ą", "ş", "ť", "ź", ".", "ž", "ż", "Ŕ", \
		"Á", "Â", "Ă", "Ä", "Ľ", "Ć", "Ç", "Č", \
		"É", "Ę", "Ë", "Ĕ", "Í", "Î", "Ď", "Ð", \
		"Ń", "Ň", "Ó", "Ô", "Õ", "Ö", "×", "Ř", \
		"Ù", "Ú", "Û", "Ü", "Ý", "Ţ", "ß", "ŕ", \
		"á", "â", "ã", "ä", "ľ", "ć", "ç", "č", \
		"é", "ę", "ë", "ì", "í", "î", "ď", "đ", \
		"ń", "ň", "ó", "ô", "õ", "ö", "÷", "ř", \
		"ù", "ú", "û", "ü", "ý", "ţ", "." }

ConvertSQ2::ConvertSQ2(void) {
	static const char *utf_table[] = TABLE_UTF8;
	utf8_table = utf_table;
}

ConvertSQ2::~ConvertSQ2() {
}

char ConvertSQ2::tolower(const char c) {
	const static char upper_cp[] = "A�BC�DE�FGHIJKL�MN�O�PQRS�TUVWXYZ��";
	const static char lower_cp[] = "a�bc�de�fghijkl�mn�o�pqrs�tuvwxyz��";
    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

char *ConvertSQ2::ConvertToUtf(const char *line) {
	static char buf[1024];
	static char letter[2] = "\0";
	unsigned char *inp;
	memset(buf, 0, sizeof(buf));

	inp = (unsigned char *)line;
	for (; *inp; inp++) {
		if (*inp > 126) {
			strncat(buf, utf8_table[*inp - 127], sizeof(buf) - strlen(buf) - 1);
		} else {
			letter[0] = *inp;
			strncat(buf, letter, sizeof(buf) - strlen(buf) - 1);
		}
	}
	return buf;
}

char *ConvertSQ2::ConvertFromUtf(const char *input) {
	static char buf[1024];
	unsigned char *inp, *inp2;
	memset(buf, 0, sizeof(buf));
	int i,k;
	char a,b;
	bool notyet;

	k=0;
	inp = (unsigned char*)input;
	inp2 = inp; inp2++;
	for (; *inp; inp++, inp2++) {
		a = *inp;
		b = *inp2;
		i=0;
		notyet=true;
		while ((i<129) && (notyet)) {
			if (a==utf8_table[i][0]) {
				if (utf8_table[i][1]!=0) {
					if (b==utf8_table[i][1]) {
						inp++;
						inp2++;
						notyet=false;
					}
				} else {
					notyet=false;
				}
			}
			i++;
		}
		if (notyet)
			buf[k]=a;
		else
			buf[k]=i+126;
		k++;
	}
	buf[k]='\0';
	return buf;
}
