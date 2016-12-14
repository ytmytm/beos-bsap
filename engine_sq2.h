
#ifndef _ENGINESQ2_H
#define _ENGINESQ2_H

#include <File.h>

#include <sqlite3.h>

#include "bydpdictionary.h"
#include "bydpconverter.h"

	class EngineSQ2 : public ydpDictionary {
		public:
			EngineSQ2(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter);
			~EngineSQ2();
			int OpenDictionary(void);
			void CloseDictionary(void);
			void FlushCache(void);
			const char *ColourFunctionName(int);
			const char *AppBarName(void);

		private:
			int ReadDefinition(int index);
			void ParseRTF(void);
			void FillWordList(void);

			// dictionary data
			sqlite3 *db;
			char *dbErrMsg;

			int *ids;
			char *curWord;
			// caching stuff
			struct LL_dictionaryCache {
				int *ids;
			} dictCache_LL[2];
	};

	class ConvertSQ2 : public bydpConverter {
		public:
			ConvertSQ2(void);
			~ConvertSQ2();
	};

#endif
