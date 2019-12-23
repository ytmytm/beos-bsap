
#ifndef _ENGINEYDP_H
#define _ENGINEYDP_H

#include <File.h>

#include "bydpconverter.h"
#include "bydpdictionary.h"

	class EngineYDP : public ydpDictionary {
		public:
			EngineYDP(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter);
			~EngineYDP();
			int OpenDictionary(void);
			void CloseDictionary(void);
			const char *ColourFunctionName(int);
			const char *AppBarName(void);

		private:
			int ReadDefinition(int index);
			void ParseRTF(void);
			void FillWordList(void);
			char* ParseToken(char *def);

			// dictionary data
			BFile fIndex, fData;
			unsigned long *indexes;
			// caching stuff
			struct LL_dictionaryCache {
				unsigned long *indexes;
			} dictCache_LL[2];
			// parser variables
			int newline_, newattr, newphon;
	};

	class ConvertYDP : public bydpConverter {
		public:
			ConvertYDP(void);
			~ConvertYDP();
			char tolower(const char c);
			char *ConvertToUtf(const char *line);
			char *ConvertFromUtf(const char *input);

		private:
			const char **utf8_table;
	};

#endif
