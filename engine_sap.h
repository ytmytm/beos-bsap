
#ifndef _ENGINESAP_H
#define _ENGINESAP_H

#include <File.h>

#include "bydpconverter.h"
#include "bydpdictionary.h"

	class EngineSAP : public ydpDictionary {
		public:
			EngineSAP(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter);
			~EngineSAP();
			int OpenDictionary(void);
			void CloseDictionary(void);
			const char *ColourFunctionName(int);
			const char *AppBarName(void);

		private:
			int ReadDefinition(int index);
			void ParseRTF(void);
			void FillWordList(void);

			// dictionary data
			BFile fData;
			char **definitions;
			char *curWord;
			// caching stuff
			struct LL_dictionaryCache {
				char **definitions;
			} dictCache_LL[2];
	};

	class ConvertSAP : public bydpConverter {
		public:
			ConvertSAP(void);
			~ConvertSAP();
			char tolower(const char c);
			char *ConvertToUtf(const char *line);
			char *ConvertFromUtf(const char *input);

		private:
			const char **utf8_table;
	};

#endif
