//
// TODO:
//	- dekodowanie gramatyki '#'
//	- stos atrybutow (np. { , , } - przecinki w srodku psuja)
//	- prawidlowo/ladnie rozlozone atrybuty (kolory, styl)
//	- def_legnths do wywalenia prawdopodobnie

#include "bydpdictionary.h"

#include <stdio.h>
#include <string.h>

ydpDictionary::ydpDictionary(BTextView *output, BListView *dict, bydpConfig *config) {
	int i;

	outputView = output;
	dictList = dict;
	cnf = config;

	dictionaryReady = false;

	for (i=0;i<2;i++) {
		dictCache[i].wordCount = -1;
		dictCache[i].def_lengths = NULL;
		dictCache[i].words = NULL;
		dictCache[i].definitions = NULL;
	}
}

ydpDictionary::~ydpDictionary() {
	int i,j;

	for (i=0;i<1;i++) {
		if (dictCache[i].wordCount>0) {
			if (dictCache[i].def_lengths) delete [] dictCache[i].def_lengths;
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

void ydpDictionary::GetDefinition(int index) {
	if (!dictionaryReady) {
		outputView->SetText("Proszƒô skonfigurowaƒá ≈õcie≈ºkƒô dostƒôpu do plik√≥w s≈Çownika.\n");
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
		outputView->SetText("B≈ÇƒÖd przy odczycie pliku danych!");
	}
}

int ydpDictionary::OpenDictionary(const char *index, const char *data) {
	int i;

	if ((fData.SetTo(data, B_READ_ONLY)) != B_OK) {
		outputView->SetText("B≈ÇƒÖd przy otwieraniu pliku danych!");
		return -1;
	}

	i = 0;
	if (!(cnf->toPolish)) i++;
	if (dictCache[i].wordCount>0) {
		wordCount = dictCache[i].wordCount;
		def_lengths = dictCache[i].def_lengths;
		words = dictCache[i].words;
		definitions = dictCache[i].definitions;
	} else {
		FillWordList();
		dictCache[i].wordCount = wordCount;
		dictCache[i].def_lengths = def_lengths;
		dictCache[i].words = words;
		dictCache[i].definitions = definitions;
	}
	lastIndex = -1;
	dictionaryReady = true;
	printf("dictionary ready, exiting opendictionary\n");
	return 0;
}

int ydpDictionary::OpenDictionary(void) {
	BString idx, dat;

	dat = cnf->topPath;
	dat.Append("/");
	dat += cnf->dataFName;
	return OpenDictionary(idx.String(),dat.String());
}

void ydpDictionary::CloseDictionary(void) {
	fData.Unset();
	ClearWordList();
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
	printf("magic %x, wcount %i, pages %i\n",magic,wordCount,npages);
	wordPairs = new int [wordCount];
	words = new char* [wordCount];
	definitions = new char* [wordCount];
	def_lengths = new int [wordCount];
	pages_offsets = new int [4*npages];
	fData.Read(pages_offsets,4*npages);
	printf("offset to 1: %x, offset to 2: %x\n",pages_offsets[0],pages_offsets[1]);

	curword = 0;
	body = new char [16384];
	for (curpage=0;curpage<npages;curpage++) {
		fData.Seek(pages_offsets[curpage],SEEK_SET);
		wordspp = dsize = dvoffset = 0;
		wordspps = dsizes = dvoffsets = 0;
		fData.Read(&wordspps,2); wordspp = fix16(wordspps);
		fData.Read(&dsizes,2); dsize = fix16(dsizes);
		fData.Read(&dvoffsets,2); dvoffset = fix16(dvoffsets);
		printf("about to read %x body\n",dsize);
		fData.Read(body,dsize);
		printf("page in memory\n");
		char *current = &body[2*wordspp];
		for (i=0;i<wordspp;i++) {	
			wordlen = strlen(current);
			words[curword+i] = new char [wordlen+1];
			strcpy(words[curword+i],current);
			words[curword+i][wordlen] = '\0';
			current += wordlen +1;
		}
		for (i=0;i<wordspp;i++) {
//		printf("definition for word %s: %s\n", words[curword], current);
			wordlen = *(short*)&body[i*2];
			definitions[curword] = new char [wordlen+1];
			memcpy(definitions[curword],current,wordlen);
			def_lengths[curword] = wordlen;
			definitions[curword][wordlen] = '\0';
			current += wordlen;
			++curword;
		}
	}
	delete body;
	printf("should be %i, is %i\n",wordCount,curword);
	printf("%s - %s,%i",words[1],&definitions[1001][2],def_lengths[1001]);
}

int ydpDictionary::ReadDefinition(int index) {
	printf("reading definition %i\n", index);
	curWord = words[index];
	curDefinition = definitions[index];
	curDefLength = def_lengths[index];
	return 0;
}

//
// parsuje rtf i od razu (via UpdateAttr) wstawia na wyjscie
//
void ydpDictionary::ParseRTF(void) {
	char *c; int cdoll = 0;
	int len; int v;
	c = curDefinition;

	printf("in parsertf\n");

	outputView->SetText("");
	textlen = 0;
	len = 0;

	line.SetTo(curWord);
	UpdateAttr(A_BOLD);
	line += " - ";
	UpdateAttr(0);
	// walk through the definition, resolve tokens and colours
	// use curDefinition up to curDefLength
	while ((*c)&&(*c!='\n')) {
		printf("got:%c\n",*c);
		switch (*c) {
			case ',':
			case '.':
			case ')':
				line += *c++;
				if (!strchr(".,",*c)) line += ' ';
				UpdateAttr(0);
				continue;
			case '$':	// youngster
				if (cdoll) line += "; ";
				UpdateAttr(0);
				line += '\n';
				line += 'a'+cdoll;
				line += ")\n\t";
				UpdateAttr(A_ITALIC);
				cdoll++; c++;
				continue;
			case '-':
				line += " - ";
				c++;
				UpdateAttr(0);
				continue;
			case '{':
				c++;
				continue;
			case '}':
				cdoll = 0;
				UpdateAttr(A_BOLD);
				line += " - ";
				UpdateAttr(0);
				c++;
				continue;
			case '*':
				line += curWord;
				UpdateAttr(0);
				c++;
				continue;
			case '=':
				line += "patrz: ";
				UpdateAttr(A_COLOR2);
				c++;
				continue;
			case '#':
				// gramatyka, markpol
				c++;
				v = ((*c++)<<8) & 0xff00;
				v |= (*c++)&255;
				printf("got %i as grammar\n",v);
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
// (parametr newattr jest zbedny, liczy sie tylko oldattr)
void ydpDictionary::UpdateAttr(int newattr) {
//	printf("adding line, oldattr %i, newattr %i, line:%s:\n",oldattr,newattr,line.String());
	if (line.Length() == 0)
		return;
	rgb_color *colour;
	BFont myfont(be_plain_font);
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
	line = ConvertToUtf(line);
	outputView->Insert(textlen,line.String(),line.Length());
	textlen+=line.Length();
	line="";
}

/////////////////////
// utf8 <-> cp1250 convertion stuff below
//

const char *utf8_table[] = TABLE_UTF8;
const char upper_cp[] = "A•BC∆DE FGHIJKL£MN—O”PQRSåTUVWXYZØè";
const char lower_cp[] = "aπbcÊdeÍfghijkl≥mnÒoÛpqrsútuvwxyzøü";

char ydpDictionary::tolower(const char c) {
    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

char *ydpDictionary::ConvertToUtf(BString line) {
	static char buf[1024];
	static char letter[2] = "\0";
	unsigned char *inp;
	memset(buf, 0, sizeof(buf));

	inp = (unsigned char *)line.String();
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

char *ydpDictionary::ConvertFromUtf(const char *input) {
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

////////////////
// search stuff below

int ydpDictionary::FindWord(const char *wordin)
{
	int result,i,j;

	if (!dictionaryReady) {
		outputView->SetText("Proszƒô skonfigurowaƒá ≈õcie≈ºkƒô dostƒôpu do plik√≥w s≈Çownika.\n");
		return -1;
	}

	switch (cnf->searchmode) {
		case SEARCH_FUZZY:
			printf("fuzzy\n");
			return FuzzyFindWord(wordin);
			break;
		case SEARCH_BEGINS:
		default:
			printf("default\n");
			result = BeginsFindWord(wordin);
			ClearWordList();
			j = 0;
			i = result-(cnf->todisplay/2-1); if (i<0) i=0;
			for (;(i<wordCount) && (j<cnf->todisplay);i++) {
				dictList->AddItem(new BStringItem(ConvertToUtf(words[i])));
				wordPairs[j]=i;
				j++;
			}
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

void ydpDictionary::ClearWordList(void) {
	int i;
	void *anItem;
	for (i=0; (anItem=dictList->ItemAt(i)); i++)
		delete anItem;
	dictList->MakeEmpty();
}

int ydpDictionary::FuzzyFindWord(const char *wordin)
{
	int i, numFound, best, score, hiscore;

    if ((wordCount<0) || (words == NULL))
		return -1;
	if (strlen(wordin)==0)
		return -1;

	char *word = ConvertFromUtf(wordin);

	ClearWordList();

    numFound = 0;
    best = 0;
    hiscore = cnf->distance;
    for (i=0;i<wordCount;i++)
		if ((score=editDistance(word,words[i])) < cnf->distance) {
			dictList->AddItem(new BStringItem(ConvertToUtf(words[i])));
			wordPairs[numFound] = i;
			numFound++;
			if (score<hiscore) {
				best = i;
				hiscore = score;
			}
		}
	return best;
}

int ydpDictionary::min3(const int a, const int b, const int c)
{
    int min=a;
    if (b<min) min = b;
    if (c<min) min = c;
    return min;
}

int ydpDictionary::editDistance(const char*slowo1, const char*slowo2)
{
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
	    row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
	}
	row = row0;
	row0 = row1;
	row1 = row;
    }
    cost = row0[n];
    return cost;
}
