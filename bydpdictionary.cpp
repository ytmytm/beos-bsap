
#include "bydpdictionary.h"

#include <stdio.h>
#include <string.h>

ydpDictionary::ydpDictionary(BTextView *output, bydpListView *dict, bydpConfig *config) {
	int i;

	outputView = output;
	dictList = dict;
	cnf = config;

	dictionaryReady = false;

	for (i=0;i<2;i++) {
		dictCache[i].wordCount = -1;
		dictCache[i].words = NULL;
		dictCache[i].definitions = NULL;
	}
	fuzzyWordCount = -1;
}

ydpDictionary::~ydpDictionary() {
	int i,j;

	for (i=0;i<1;i++) {
		if (dictCache[i].wordCount>0) {
			if (dictCache[i].words) {
				for (j=0;j<dictCache[i].wordCount;j++) {
					delete [] dictCache[i].words[j];
					delete [] dictCache[i].definitions;
				}
				delete [] dictCache[i].words;
			}
		}
	}
}

void ydpDictionary::ReGetDefinition(void) {
	if (ReadDefinition(lastIndex) == 0) {
		ParseRTF();
	} else {
		outputView->SetText("BÅ‚Ä…d przy odczycie pliku danych!");
	}
}

void ydpDictionary::GetDefinition(int index) {
	if (!dictionaryReady) {
		outputView->SetText("ProszÄ™ skonfigurowaÄ‡ Å›cieÅ¼kÄ™ dostÄ™pu do plikÃ³w sÅ‚ownika.\n");
		return;
	}
	if (index < 0)
		index = lastIndex;
	if (index == lastIndex)
		return;
	lastIndex = index;
	if (ReadDefinition(index) == 0) {
		ParseRTF();
	} else {
		outputView->SetText("BÅ‚Ä…d przy odczycie pliku danych!");
	}
}

int ydpDictionary::OpenDictionary(const char *data) {
	int i;

	if ((fData.SetTo(data, B_READ_ONLY)) != B_OK) {
		outputView->SetText("BÅ‚Ä…d przy otwieraniu pliku danych!");
		return -1;
	}

	i = 0;
	if (!(cnf->toPolish)) i++;
	if (dictCache[i].wordCount>0) {
		wordCount = dictCache[i].wordCount;
		words = dictCache[i].words;
		definitions = dictCache[i].definitions;
		delete [] wordPairs;
		delete [] fuzzyWords;
		wordPairs = new int [wordCount];
		fuzzyWords = new char* [wordCount];
	} else {
		FillWordList();
		dictCache[i].wordCount = wordCount;
		dictCache[i].words = words;
		dictCache[i].definitions = definitions;
	}
	lastIndex = -1;
	dictionaryReady = true;
	return 0;
}

int ydpDictionary::OpenDictionary(void) {
	BString dat;

	dat = cnf->topPath;
	dat.Append("/");
	dat += cnf->dataFName;
	return OpenDictionary(dat.String());
}

void ydpDictionary::CloseDictionary(void) {
	fData.Unset();
	ClearFuzzyWordList();
}

unsigned int fix32(unsigned int x) {
	return x;
}

unsigned short fix16(unsigned short x) {
	return x;
}

void ydpDictionary::FillWordList(void) {
	int magic;
	int npages;
	int *pages_offsets;
	int wordspp, dsize, dvoffset;
	char *body;
	int curpage, curword, i;
	short wordlen, wordspps, dsizes, dvoffsets;

	wordCount = 0;
	fData.Read(&magic,4);
	fData.Read(&wordCount,4);
	fData.Read(&npages,4);
	wordCount = fix32(wordCount);
	npages = fix32(npages);
	magic = fix32(magic);
	wordPairs = new int [wordCount];
	fuzzyWords = new char * [wordCount];
	words = new char* [wordCount];
	definitions = new char* [wordCount];
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
			words[curword+i] = new char [wordlen+1];
			strcpy(words[curword+i],current);
			words[curword+i][wordlen] = '\0';
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

int ydpDictionary::ReadDefinition(int index) {
	curWord = words[index];
	curDefinition = definitions[index];
	return 0;
}

/// XXX
/// to jest ohydne - do zmiany (polskie litery w ISO, bo tlumaczone w UpdateAttr)
struct okreslenia { char *nazwa; int flagi; int maska; } okreslenia[]={
{"przymiotnik",1,15},
{"przys³ówek",2,15},
{"spójnik",3,15},
{"liczebnik",4,15},
{"partyku³a",5,15},
{"przedrostek",6,15},
{"przyimek",7,15},
{"zaimek",8,15},
{"rzeczownik",9,15},
{"czasownik posi³kowy",10,15},
{"czasownik nieprzechodni",11,15},
{"czasownik nieosobowy",12,15},
{"czasownik zwrotny",13,15},
{"czasownik przechodni",14,15},	/* 16 - 4 bajty */
{"czasownik",15,15},

{"rodzaj ¿eñski",16,0x30},		/* 4 bajty */
{"rodzaj mêski",32,0x30},
{"rodzaj nijaki",16+32,0x30},

{"liczba pojedyncza",64,0xc0},
{"liczba mnoga",128,0xc0},
{"tylko liczba mnoga",128+64,0xc0},

{"czas przesz³y",2048,2048|4096|8192|16384},
{"czas tera¼niejszy",4096,2048|4096|8192|16384},
{"czas przysz³y",2048+4096,2048|4096|8192|16384},
{"bezokolicznik",8192,2048|4096|8192|16384},

{"stopieñ najwy¿szy",16384,2048|4096|8192|16384},
{"regularny",256,256},	/* 3 bajty */
{"wyraz potoczny",1024,1024},
{"skrót",512,512},
{"stopieñ wy¿szy",16384|8192,2048|4096|8192|16384},
{0,0}};

//
// parsuje rtf i od razu (via UpdateAttr i konwersje) wstawia na wyjscie
void ydpDictionary::ParseRTF(void) {
	char *c; int cdoll = 0;
	int len; int v;
	int i, mc;
	int level=0, attr[16];

	c = curDefinition;
	outputView->SetText("");
	textlen = 0;
	len = 0;
	attr[level] = 0;

	line.SetTo(curWord);
	UpdateAttr(A_BOLD|A_COLOR0);
	line += " - ";
	UpdateAttr(0);
	while ((*c)&&(*c!='\n')) {
//		printf("got:%c\n",*c);
		switch (*c) {
			case ',':
			case '.':
			case ')':
				line += *c++;
				if (!strchr(".,",*c)) line += ' ';
				continue;
			case '(':
				line += *c++;
				UpdateAttr(attr[level]);
				continue;
			case '$':
				if (cdoll) line += "; "; else line += '\n';
				UpdateAttr(attr[level]);
				line += '\n';
				line += 'a'+cdoll;
				line += ")\n\t";
				UpdateAttr(A_ITALIC|A_BOLD|A_COLOR2);
				cdoll++; c++;
				continue;
			case '-':
				line += " - ";
				c++;
				UpdateAttr(A_COLOR2);
				continue;
			case '{':
				level++;
				attr[level]=attr[level-1];
				c++;
				continue;
			case '}':
				cdoll = 0;
				UpdateAttr(A_COLOR0);
				line += " - ";
				level--;
				UpdateAttr(attr[level]);
				c++;
				continue;
			case '*':
				line += curWord;
				UpdateAttr(attr[level]);
				c++;
				continue;
			case '=':
				line += "patrz: ";
				UpdateAttr(A_COLOR2);
				c++;
				continue;
			case '#':
				c++;
				v = ((*c++)<<8) & 0xff00;
				v |= (*c++)&255;
				for (i=mc=0; okreslenia[i].nazwa; i++)
					if ((v & okreslenia[i].maska)==okreslenia[i].flagi) {
						if (mc) line+= ',';
						line += okreslenia[i].nazwa;
						line += ' ';
						mc = 1;
					}
				UpdateAttr(A_COLOR1);
				continue;
		}
		line += *c++;
		len++;
	}
	UpdateAttr(0);
	return;
}

//
// wstawia na koniec tekst z line z odpowiednimi parametrami
void ydpDictionary::UpdateAttr(int newattr) {
	if (line.Length() == 0)
		return;
	rgb_color *colour;
	BFont myfont = cnf->currentFont;
	colour = &cnf->colour;
	if (newattr & A_SUPER) {
		myfont.SetSize(10.0);
	}
	if (newattr & A_BOLD) {
		myfont.SetFace(B_BOLD_FACE);
	}
	if (newattr & A_ITALIC) {
		myfont.SetFace(B_ITALIC_FACE);
	}
	if (newattr & A_COLOR0) {
		colour = &cnf->colour0;
	}
	if (newattr & A_COLOR1) {
		colour = &cnf->colour1;
	}
	if (newattr & A_COLOR2) {
		colour = &cnf->colour2;
	}
	outputView->SetFontAndColor(&myfont,B_FONT_ALL,colour);
	line = ConvertToUtf(line.String());
	outputView->Insert(textlen,line.String(),line.Length());
	textlen+=line.Length();
	line="";
}

////////////////
// search stuff below

int ydpDictionary::FindWord(const char *wordin) {
	int result,i;

	if (!dictionaryReady) {
		outputView->SetText("ProszÄ™ skonfigurowaÄ‡ Å›cieÅ¼kÄ™ dostÄ™pu do plikÃ³w sÅ‚ownika.\n");
		return -1;
	}

	switch (cnf->searchmode) {
		case SEARCH_FUZZY:
			return FuzzyFindWord(wordin);
			break;
		case SEARCH_BEGINS:
		default:
			result = BeginsFindWord(wordin);
			for (i=0;i<wordCount;i++) wordPairs[i]=i;
			dictList->NewData(wordCount,words,result);
			return result;
			break;
	}
}

int ydpDictionary::ScoreWord(const char *w1, const char *w2) {
	int i = 0;
	int len1 = strlen(w1);
	int len2 = strlen(w2);
	for (; ((i<len1) && (i<len2)); i++)
		if (tolower(w1[i])!=tolower(w2[i]))
			break;
	return i;
}

int ydpDictionary::BeginsFindWord(const char *wordin) {
	char *word = ConvertFromUtf(wordin);
	int i, score, maxscore=0, maxitem=0;

	for (i=0;i<wordCount;i++) {
		score = ScoreWord(word, words[i]);
		if (score>maxscore) {
			maxscore = score;
			maxitem = i;
		}
	}
	return maxitem;
}

void ydpDictionary::ClearFuzzyWordList(void) {
	int i;
	if (fuzzyWordCount>0)
		for (i=0;i<fuzzyWordCount;i++)
			delete [] fuzzyWords[i];
	fuzzyWordCount = 0;
}

int ydpDictionary::FuzzyFindWord(const char *wordin) {
	int i, numFound, best, score, hiscore;

    if ((wordCount<0) || (words == NULL))
		return -1;
	if (strlen(wordin)==0)
		return -1;

	char *word = ConvertFromUtf(wordin);

	ClearFuzzyWordList();

    numFound = 0;
    best = 0;
    hiscore = cnf->distance;
    for (i=0;i<wordCount;i++)
		if ((score=editDistance(word,words[i])) < cnf->distance) {
			fuzzyWords[numFound] = new char [strlen(words[i])+1];
			strcpy(fuzzyWords[numFound],words[i]);
			wordPairs[numFound] = i;
			numFound++;
			if (score<hiscore) {
				best = i;
				hiscore = score;
			}
		}
	fuzzyWordCount = numFound;
	dictList->NewData(fuzzyWordCount,fuzzyWords,best);
	return best;
}

int ydpDictionary::min3(const int a, const int b, const int c) {
	int min=a;
	if (b<min) min = b;
	if (c<min) min = c;
	return min;
}

int ydpDictionary::editDistance(const char*slowo1, const char*slowo2) {
	int *row0, *row1, *row;
	int cost,i,j,m,n;
	static int rowx[512];		// speedup!
	static int rowy[512];

	n = strlen(slowo1);	if (n>510) n=510;	// n+1 is used
	m = strlen(slowo2);

	row0 = rowx;
	row1 = rowy;

	for (i=0;i<=n;i++)
		row0[i] = i;

	for (j=1;j<=m;j++) {
		row1[0] = j;
		for (i=1;i<=n;i++) {
			cost = (slowo1[i-1]==slowo2[j-1]) ? 0 : 1;
//			cost = (tolower(slowo1[i-1])==tolower(slowo2[j-1])) ? 0 : 1; /// za wolno!!!
			row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
		}
		row = row0;
		row0 = row1;
		row1 = row;
	}
	cost = row0[n];
	return cost;
}
