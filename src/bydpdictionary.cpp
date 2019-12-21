
#include <Alert.h>
#include <ByteOrder.h>

#include <stdio.h>
#include <string.h>

#include "bydpdictionary.h"
#include "globals.h"

ydpDictionary::ydpDictionary(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter) {
	int i;

	outputView = output;
	dictList = dict;
	cnf = config;
	cvt = converter;

	dictionaryReady = false;

	for (i=0;i<2;i++) {
		dictCache[i].wordCount = -1;
		dictCache[i].words = NULL;
	}
	fuzzyWordCount = -1;
}

ydpDictionary::~ydpDictionary() {
	FlushCache();
}

void ydpDictionary::ReGetDefinition(void) {
	if (ReadDefinition(lastIndex) == 0) {
		ParseRTF();
	} else {
		BAlert *alert = new BAlert(APP_NAME, "Data file read error.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
	}
}

void ydpDictionary::GetDefinition(int index) {
	if (!dictionaryReady) {
		BAlert *alert = new BAlert(APP_NAME, "Please setup path to dictionary files.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
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
		BAlert *alert = new BAlert(APP_NAME, "Data file read error.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
	}
}

int ydpDictionary::OpenDictionary(void) {
	int i = 0;

	if (!(cnf->toPolish)) i++;
	if (dictCache[i].wordCount>0) {
		wordCount = dictCache[i].wordCount;
		words = dictCache[i].words;
		delete [] wordPairs;
		delete [] fuzzyWords;
	} else {
		dictCache[i].wordCount = wordCount;
		dictCache[i].words = words;
	}
	wordPairs = new int [wordCount];
	fuzzyWords = new char* [wordCount];

	lastIndex = -1;
	dictionaryReady = true;
	return 0;		// never fails because only inherited may fail
}

void ydpDictionary::FlushCache(void) {
	int i,j;

	for (i=0;i<1;i++) {
		if (dictCache[i].wordCount>0) {
			if (dictCache[i].words) {
				for (j=0;j<dictCache[i].wordCount;j++) {
					delete [] dictCache[i].words[j];
				}
				delete [] dictCache[i].words;
			}
		}
	}
	dictCache[0].wordCount = 0;
}

void ydpDictionary::CloseDictionary(void) {
	ClearFuzzyWordList();
}

const char *ydpDictionary::ColourFunctionName(int index) {
	switch(index) {
		case 0:
			return "colour 0";
			break;
		case 1:
			return "colour 1";
			break;
		case 2:
			return "colour 2";
			break;
		case 3:
			return "colour 3";
			break;
		default:
			break;
	}
	return "illegal index";
}

const char *ydpDictionary::AppBarName(void) {
	return "No engine";
}

int ydpDictionary::ReadDefinition(int index) {
	// dummy one, should be made pure virtual?
	return 0;
}

inline unsigned int ydpDictionary::fix32(unsigned int x)
{
#if (B_HOST_IS_LENDIAN == 1)
	return x;
#else
	return (unsigned int)
	     (((x & (unsigned int) 0x000000ffU) << 24) |
          ((x & (unsigned int) 0x0000ff00U) << 8) |
          ((x & (unsigned int) 0x00ff0000U) >> 8) |
          ((x & (unsigned int) 0xff000000U) >> 24));
#endif
}

inline unsigned short ydpDictionary::fix16(unsigned short x)
{
#if (B_HOST_IS_LENDIAN == 1)
	return x;
#else
	 return (unsigned short)
		  (((x & (unsigned short) 0x00ffU) << 8) |
           ((x & (unsigned short) 0xff00U) >> 8));
#endif
}

void ydpDictionary::ParseRTF(void) {
	// dummy one, should be made pure virtual?
}

void ydpDictionary::ClearView(void) {
	outputView->SetText("");
}

//
// appends 'line' to output widget with proper attributes
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
	line = cvt->ConvertToUtf(line.String());
	outputView->Insert(textlen,line.String(),line.Length());
	textlen+=line.Length();
	line="";
}

////////////////
// search stuff below

int ydpDictionary::FindWord(const char *wordin) {
	int result,i;

	if (!dictionaryReady) {
		outputView->SetText("Please setup path to dictionary files.");
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
		if (cvt->tolower(w1[i])!=cvt->tolower(w2[i]))
			break;
	return i;
}

int ydpDictionary::BeginsFindWord(const char *wordin) {
	char *word = cvt->ConvertFromUtf(wordin);
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

	char *word = cvt->ConvertFromUtf(wordin);

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
	static int rowx[512];					// speedup!
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
//			cost = (cvt->tolower(slowo1[i-1])==cvt->tolower(slowo2[j-1])) ? 0 : 1; /// za wolno!!!
			row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
		}
		row = row0;
		row0 = row1;
		row1 = row;
	}
	cost = row0[n];
	return cost;
}
