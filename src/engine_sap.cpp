
#include <Alert.h>

#include <stdio.h>
#include <string.h>

#include "engine_sap.h"
#include "globals.h"

//
// to prevent my confusion, everything from base class is prefixed with this: this-> (wow, that's recursive :)

EngineSAP::EngineSAP(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter) : ydpDictionary(output,dict,config,converter) {
	int i;
	for (i=0;i<2;i++) {
		dictCache_LL[i].definitions = NULL;
	}
}

EngineSAP::~EngineSAP() {
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

int EngineSAP::OpenDictionary(void) {
	int i;
	BString dat;

	dat = this->cnf->topPath;
	dat.Append("/");
	dat += this->cnf->toPolish ? "dvp_1.dic" : "dvp_2.dic";

	if ((fData.SetTo(dat.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, "Couldn't open data file.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
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

void EngineSAP::CloseDictionary(void) {
	fData.Unset();
	ydpDictionary::CloseDictionary();	// required call
}

void EngineSAP::FillWordList(void) {
	int magic;
	int npages;
	int *pages_offsets;
	int wordspp, dsize, dvoffset;
	char *body;
	int curpage, curword, i;
	short wordlen, wordspps, dsizes, dvoffsets;

	this->wordCount = 0;
	fData.Read(&magic,4);
	fData.Read(&this->wordCount,4);
	fData.Read(&npages,4);
	this->wordCount = this->fix32(this->wordCount);
	npages = this->fix32(npages);
	magic = this->fix32(magic);
	this->words = new char* [this->wordCount];
	definitions = new char* [this->wordCount];
	pages_offsets = new int [4*npages];
	fData.Read(pages_offsets,4*npages);

	curword = 0;
	body = new char [16384];
	for (curpage=0;curpage<npages;curpage++) {
		fData.Seek(pages_offsets[curpage],SEEK_SET);
		wordspp = dsize = dvoffset = 0;
		wordspps = dsizes = dvoffsets = 0;
		fData.Read(&wordspps,2); wordspp = fix16(wordspps);
		fData.Read(&dsizes,2); dsize = fix16(dsizes);
		fData.Read(&dvoffsets,2); dvoffset = fix16(dvoffsets);
		fData.Read(body,dsize);
		char *current = &body[2*wordspp];
		for (i=0;i<wordspp;i++) {	
			wordlen = strlen(current);
			this->words[curword+i] = new char [wordlen+1];
			strcpy(this->words[curword+i],current);
			this->words[curword+i][wordlen] = '\0';
			current += wordlen +1;
		}
		for (i=0;i<wordspp;i++) {
			wordlen = *(short*)&body[i*2];
			definitions[curword] = new char [wordlen+1];
			memcpy(definitions[curword],current,wordlen);
			definitions[curword][wordlen] = '\0';
			current += wordlen;
			++curword;
		}
	}
	delete body;
}

int EngineSAP::ReadDefinition(int index) {
	this->curWord = this->words[index];
	this->curDefinition = definitions[index];
	return 0;
}

const char *EngineSAP::ColourFunctionName(int index) {
	switch(index) {
		case 0:
			return "Translation colour";
			break;
		case 1:
			return "Keywords colour";
			break;
		case 2:
			return "Qualifiers colour";
			break;
		case 3:
			return "Additional text colour";
			break;
		default:
			break;
	}
	return "Illegal index";
}

const char *EngineSAP::AppBarName(void) {
	if (cnf->toPolish)
		return "Eng->Pol";
	else
		return "Pol->Eng";
}

/// XXX
/// this is ugly - change it (polish letters are ISO8859-2 encoded because they pass
/// through UpdateAttr convert routine)
struct okreslenia { char *nazwa; int flagi; int maska; } okreslenia[]={
{"przymiotnik",1,15},
{"przys��wek",2,15},
{"sp�jnik",3,15},
{"liczebnik",4,15},
{"partyku�a",5,15},
{"przedrostek",6,15},
{"przyimek",7,15},
{"zaimek",8,15},
{"rzeczownik",9,15},
{"czasownik posi�kowy",10,15},
{"czasownik nieprzechodni",11,15},
{"czasownik nieosobowy",12,15},
{"czasownik zwrotny",13,15},
{"czasownik przechodni",14,15},	/* 16 - 4 bajty */
{"czasownik",15,15},

{"rodzaj �e�ski",16,0x30},		/* 4 bajty */
{"rodzaj m�ski",32,0x30},
{"rodzaj nijaki",16+32,0x30},

{"liczba pojedyncza",64,0xc0},
{"liczba mnoga",128,0xc0},
{"tylko liczba mnoga",128+64,0xc0},

{"czas przesz�y",2048,2048|4096|8192|16384},
{"czas tera�niejszy",4096,2048|4096|8192|16384},
{"czas przysz�y",2048+4096,2048|4096|8192|16384},
{"bezokolicznik",8192,2048|4096|8192|16384},

{"stopie� najwy�szy",16384,2048|4096|8192|16384},
{"regularny",256,256},	/* 3 bajty */
{"wyraz potoczny",1024,1024},
{"skr�t",512,512},
{"stopie� wy�szy",16384|8192,2048|4096|8192|16384},
{0,0}};

//
// parses format and (via UpdateAttr and convert) outputs data
void EngineSAP::ParseRTF(void) {
	char *c = this->curDefinition;
	int cdoll = 0;
	int len; int v;
	int i, mc;
	int level=0, attr[16];

	this->ClearView();
	this->textlen = 0;
	len = 0;
	attr[level] = 0;

	this->line.SetTo(this->curWord);
	this->UpdateAttr(A_BOLD|A_COLOR0);
	this->line += " - ";
	this->UpdateAttr(0);
	while ((*c)&&(*c!='\n')) {
//		printf("got:%c\n",*c);
		switch (*c) {
			case ',':
			case '.':
			case ')':
				this->line += *c++;
				if (!strchr(".,",*c)) this->line += ' ';
				continue;
			case '(':
				this->line += *c++;
				this->UpdateAttr(attr[level]);
				continue;
			case '$':
				if (cdoll) this->line += "; "; else this->line += '\n';
				this->UpdateAttr(attr[level]);
				this->line += '\n';
				this->line += 'a'+cdoll;
				this->line += ")\n\t";
				this->UpdateAttr(A_ITALIC|A_BOLD|A_COLOR2);
				cdoll++; c++;
				continue;
			case '-':
				this->line += " - ";
				c++;
				this->UpdateAttr(A_COLOR2);
				continue;
			case '{':
				level++;
				attr[level]=attr[level-1];
				c++;
				continue;
			case '}':
				cdoll = 0;
				this->UpdateAttr(A_COLOR0);
				this->line += " - ";
				level--;
				UpdateAttr(attr[level]);
				c++;
				continue;
			case '*':
				this->line += curWord;
				this->UpdateAttr(attr[level]);
				c++;
				continue;
			case '=':
				this->line += "patrz: ";
				this->UpdateAttr(A_COLOR2);
				c++;
				continue;
			case '#':
				c++;
				v = ((*c++)<<8) & 0xff00;
				v |= (*c++)&255;
				for (i=mc=0; okreslenia[i].nazwa; i++)
					if ((v & okreslenia[i].maska)==okreslenia[i].flagi) {
						if (mc) this->line+= ',';
						this->line += okreslenia[i].nazwa;
						this->line += ' ';
						mc = 1;
					}
				this->UpdateAttr(A_COLOR1);
				continue;
		}
		this->line += *c++;
		len++;
	}
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

ConvertSAP::ConvertSAP(void) {
	static const char *utf_table[] = TABLE_UTF8;
	utf8_table = utf_table;
}

ConvertSAP::~ConvertSAP() {
}

char ConvertSAP::tolower(const char c) {
	const static char upper_cp[] = "A�BC�DE�FGHIJKL�MN�O�PQRS�TUVWXYZ��";
	const static char lower_cp[] = "a�bc�de�fghijkl�mn�o�pqrs�tuvwxyz��";
    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

char *ConvertSAP::ConvertToUtf(const char *line) {
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

char *ConvertSAP::ConvertFromUtf(const char *input) {
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
