
#ifndef _BYDPDICTIONARY_H
#define _BYDPDICTIONARY_H

#include <String.h>
#include <TextView.h>

#include "bydpconfig.h"
#include "bydplistview.h"
#include "bydpconverter.h"

#define A_BOLD 1
#define A_ITALIC 2
#define A_COLOR0 4
#define A_COLOR1 8
#define A_COLOR2 16
#define A_MARGIN 32
#define A_SUPER 64

	class ydpDictionary {
		public:
			ydpDictionary(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter);
			virtual ~ydpDictionary();
			virtual int OpenDictionary(void);
			virtual void CloseDictionary(void);
			virtual const char *ColourFunctionName(int);

			void ReGetDefinition(void);
			void GetDefinition(int index);
			int FindWord(const char *word);

			bydpConverter *cvt;		// to/from utf converter object
			int *wordPairs;			// # current dictList indexes (for fuzzy)

		private:
			virtual int ReadDefinition(int index);
			virtual void ParseRTF(void);
			void ClearFuzzyWordList(void);
			int ScoreWord(const char *w1, const char *w2);
			int BeginsFindWord(const char *word);
			int FuzzyFindWord(const char *word);
		    int min3(const int a, const int b, const int c);
		    int editDistance(const char*slowo1, const char*slowo2);

			// GUI data holders
			BTextView *outputView;
			bydpListView *dictList;
			// fuzzy search data
			int fuzzyWordCount;
			char **fuzzyWords;
			// caching stuff
			int lastIndex;
			bool dictionaryReady;

		protected:
			inline unsigned int fix32(unsigned int x);
			inline unsigned short fix16(unsigned short x);

			void ClearView(void);
			void UpdateAttr(int newattr);

			// configuration holder
			bydpConfig *cnf;

			// dictionary data
			int wordCount;
			char **words;
			char *curDefinition;

			// caching stuff
			struct dictionaryCache {
				int wordCount;
				char **words;
				char **definitions;
			} dictCache[2];

			// UpdateAttr globals (ParseRTF must use that)
			int textlen;
			BString line;
	};

#endif
