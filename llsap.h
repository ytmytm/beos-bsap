
#ifndef _LLSAP_H
#define _LLSAP_H

#include <File.h>

#include "bydpdictionary.h"
#include "bydpconverter.h"

	class LLSAP : public ydpDictionary {
		public:
			LLSAP(BTextView *output, bydpListView *dict, bydpConfig *config, bydpConverter *converter);
			~LLSAP();
			int OpenDictionary(void);
			void CloseDictionary(void);
			const char *ColourFunctionName(int);

		private:
			int ReadDefinition(int index);
			void ParseRTF(void);
			void FillWordList(void);

			// dictionary data
			BFile fData;
			char **definitions;
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
