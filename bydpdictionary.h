
#ifndef _BYDPDICTIONARY_H
#define _BYDPDICTIONARY_H

#include <String.h>
#include <File.h>
#include <TextView.h>
///#include <ListView.h>
#include <Font.h>

#include "bydpconfig.h"
#include "bydplistview.h"

#define A_BOLD 1
#define A_ITALIC 2
#define A_COLOR0 4
#define A_COLOR1 8
#define A_COLOR2 16
#define A_MARGIN 32
#define A_SUPER 64

	class ydpDictionary {
		public:
			ydpDictionary(BTextView *output, bydpListView *dict, bydpConfig *config);
			~ydpDictionary();

			void ReGetDefinition(void);
			void GetDefinition(int index);
			int OpenDictionary(const char *index, const char *data);
			int OpenDictionary(void);
			int FindWord(const char *word);
			void CloseDictionary(void);

			int *wordPairs;		// # indeksow na liscie dictList

		private:
			int ReadDefinition(int index);
			void FillWordList(void);
			void ParseRTF(void);
			void UpdateAttr(int newattr);
			void ClearFuzzyWordList(void);
			int ScoreWord(const char *w1, const char *w2);
			int BeginsFindWord(const char *word);
			int FuzzyFindWord(const char *word);
		    int min3(const int a, const int b, const int c);
		    int editDistance(const char*slowo1, const char*slowo2);

			// GUI data holders
			BTextView *outputView;
			bydpListView *dictList;

			// configuration holder
			bydpConfig *cnf;

			// dictionary data
			BFile fIndex, fData;
			int wordCount, fuzzyWordCount;
			char **words;
			char **fuzzyWords;
			char **definitions;
			char *curDefinition;
			char *curWord;

			// caching stuff
			struct dictionaryCache {
				int wordCount;
				char **words;
				char **definitions;
			} dictCache[2];
			int lastIndex;
			bool dictionaryReady;

			// parser variables
			int textlen;
			BString line;
	};

#endif
