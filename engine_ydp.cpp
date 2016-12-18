
#include <Alert.h>

#include <stdio.h>
#include <string.h>

#include "engine_ydp.h"
#include "globals.h"

//
// to prevent my confusion, everything from base class is prefixed with this: this-> (wow, that's recursive :)

EngineYDP::EngineYDP(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter) : ydpDictionary(output,dict,config,converter) {
	int i;
	for (i=0;i<2;i++) {
		dictCache_LL[i].indexes = NULL;
	}
}

EngineYDP::~EngineYDP() {
	int i;

	for (i=0;i<2;i++) {
		if (dictCache_LL[i].indexes) delete [] dictCache_LL[i].indexes;
	}
}

int EngineYDP::OpenDictionary(void) {
	int i;
	BString idx, dat;

	dat = this->cnf->topPath;
	dat.Append("/");
	dat += this->cnf->toPolish ? "dict100.dat" : "dict101.dat";
	idx = this->cnf->topPath;
	idx.Append("/");
	idx += this->cnf->toPolish ? "dict100.idx" : "dict101.idx";
	if ((fIndex.SetTo(idx.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, "Couldn't open index file.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}
	if ((fData.SetTo(dat.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, "Couldn't open data file.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}

	i = 0;
	if (!(this->cnf->toPolish)) i++;
	if (this->dictCache[i].wordCount>0) {
		indexes = dictCache_LL[i].indexes;
	} else {
		FillWordList();
		dictCache_LL[i].indexes = indexes;
	}

	return ydpDictionary::OpenDictionary();	// required call
}

void EngineYDP::CloseDictionary(void) {
	fIndex.Unset();
	fData.Unset();
	ydpDictionary::CloseDictionary();	// required call
}

void EngineYDP::FillWordList(void) {
	unsigned long pos;
	unsigned long index[2];
	unsigned short wcount;
	int current = 0;

	// read # of words
	wcount = 0;
	this->wordCount = 0;
	fIndex.Seek(0x08, SEEK_SET);
	fIndex.Read(&wcount, 2);
	this->wordCount = (int)fix16(wcount);

	indexes = new unsigned long [this->wordCount+2];
	this->words = new char* [this->wordCount+2];

	this->words[this->wordCount]=0;

	// read index table offset
	fIndex.Seek(0x10, SEEK_SET);
	fIndex.Read(&pos, sizeof(unsigned long));
	pos=fix32(pos);

	fIndex.Seek(pos, SEEK_SET);
	do {
		fIndex.Read(&index[0], 8);
		indexes[current]=fix32(index[1]);
		this->words[current] = new char [fix32(index[0]) & 0xff];
		fIndex.Read(this->words[current], fix32(index[0]) & 0xff);
	} while (++current < this->wordCount);
	// XXX fix dictionary index errors (Provencial?)
}

int EngineYDP::ReadDefinition(int index) {
	unsigned long dsize, size;
	char *def;

	dsize = 0;
	fData.Seek(indexes[index], SEEK_SET);
	fData.Read((char*)&dsize, sizeof(unsigned long));
	dsize = fix32(dsize);

	def = new char [dsize+1];
	if ((size = fData.Read(def,dsize)) != dsize) return -1;
	def[size] = 0;

	if (this->curDefinition) delete [] this->curDefinition;
	this->curDefinition = def;
	return 0;
}

const char *EngineYDP::ColourFunctionName(int index) {
	switch(index) {
		case 0:
			return "Plain text colour";
			break;
		case 1:
			return "Examples colour";
			break;
		case 2:
			return "Translation colour";
			break;
		case 3:
			return "Qualifiers colour";
			break;
		default:
			break;
	}
	return "Illegal index";
}

const char *EngineYDP::AppBarName(void) {
	if (cnf->toPolish)
		return "Eng->Pol";
	else
		return "Pol->Eng";
}

//
// parses format and (via UpdateAttr and convert) outputs data
void EngineYDP::ParseRTF(void) {
	char *def = this->curDefinition;
	int level=0, attr=0, attrs[16], phon;

	newline_=0; newattr=0; newphon=0;

	this->textlen=0;
	this->ClearView();

	while(*def) {
		switch(*def) {
			case '{':
				if (level<16) attrs[level++] = attr;
				break;
			case '\\':
				def = ParseToken(def);
				this->UpdateAttr(attr);
				attr = newattr;
				break;
			case '}':
				newattr = attrs[--level];
				this->UpdateAttr(attr);
				break;
			default:
				this->line += *def;
				break;
		}
		def++;
		if (newline_) {
			if (newattr & A_MARGIN) {
				this->line.Prepend("\t\t",2);
			}
			this->line.Append("\n",1);
			this->UpdateAttr(attr);
			newline_ = 0;
		}
		attr = newattr;
		phon = newphon;
	}
	this->UpdateAttr(attr);
}

char* EngineYDP::ParseToken(char *def) {
    char token[64];
    int tp;

    def++; tp = 0;
    while((*def >= 'a' && *def <= 'z') || (*def >='0' && *def <= '9'))
	    token[tp++] = *def++;
    token[tp] = 0;
    if (*def == ' ') def++;
    if (!strcmp(token, "par") || !strcmp(token, "line"))
		newline_ = 1;
    if (!strcmp(token, "pard")) {
		newline_ = 1; newattr = (newattr & !A_MARGIN);
	}

    if (!strcmp(token, "b")) newattr |= A_BOLD;
    if (!strcmp(token, "cf0")) newattr |= A_COLOR0;
    if (!strcmp(token, "cf1")) newattr |= A_COLOR1;
    if (!strcmp(token, "cf2")) newattr |= A_COLOR2;
    if (!strcmp(token, "i")) newattr |= A_ITALIC;
    if (!strncmp(token, "sa", 2))	newattr |=A_MARGIN;
    if (token[0] == 'f') newphon = 0;
    if (!strcmp(token, "f1")) newphon = 1;
//    	if (!strcmp(token, "qc")) newattr |= 0x8000; /* nie wyswietlac */
    if (!strcmp(token, "super")) newattr |=A_SUPER;

    def--;
    return def;
}

/////////////////////
// utf8 <-> anything convertion stuff below
//

#define TABLE_UTF8 { \
		"~", \
		".", ".", "<o>", "<3>", ".", "<|>", "<E>", "<^>", "<e>", \
		"θ", "<i>", "<a>", "Ś", ":", "´", "Ź", "ŋ", \
		".", ".", ".", ".", ".", ".", "ð", "æ", \
		".", ".", ".", "ś", ".", ".", "ź", ".", \
		".", ".", "Ł", "¤", "Ą", "Ś", "§", "¨", \
		"Š", "Ş", "Ť", "Ź", "­", "Ž", "Ż", "°", \
		".", ".", "ł", "´", "ľ", "ś", ".", "¸", \
		"ą", "ş", "ť", "ź", ".", "ž", "ż", "Ŕ", \
		"Á", "Â", "Ă", "Ä", "Ľ", "Ć", "Ç", "Č", \
		"É", "Ę", "Ë", "Ĕ", "Í", "Î", "Ď", "Ð", \
		"Ń", "Ň", "Ó", "Ô", "Õ", "Ö", "×", "Ř", \
		"Ù", "Ú", "Û", "Ü", "Ý", "Ţ", "ß", "ŕ", \
		"á", "â", "ã", "ä", "ľ", "ć", "ç", "č", \
		"é", "ę", "ë", "ì", "í", "î", "ď", "đ", \
		"ń", "ň", "ó", "ô", "õ", "ö", "÷", "ř", \
		"ù", "ú", "û", "ü", "ý", "ţ", "." }

ConvertYDP::ConvertYDP(void) {
	static const char *utf_table[] = TABLE_UTF8;
	utf8_table = utf_table;
}

ConvertYDP::~ConvertYDP() {
}

char ConvertYDP::tolower(const char c) {
	const static char upper_cp[] = "A�BC�DE�FGHIJKL�MN�O�PQRS�TUVWXYZ��";
	const static char lower_cp[] = "a�bc�de�fghijkl�mn�o�pqrs�tuvwxyz��";
    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

char *ConvertYDP::ConvertToUtf(const char *line) {
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

char *ConvertYDP::ConvertFromUtf(const char *input) {
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
