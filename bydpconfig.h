
#ifndef _BYDPCONFIG_H
#define _BYDPCONFIG_H

#include <File.h>
#include <Font.h>
#include <Rect.h>
#include <String.h>

#define SEARCH_BEGINS	1
#define SEARCH_FUZZY	2

#define DICTIONARY_SAP	1
#define DICTIONARY_YDP	2
#define DICTIONARY_SQ2	3

#define CONFIG_NAME "/boot/home/config/settings/bsap"

	class bydpConfig {
		public:
			bydpConfig();
			~bydpConfig();

			BString topPath;
			bool toPolish;
			bool clipboardTracking;
			bool setFocusOnSelf;
			// SQL dictionaries [0]==toPolish, [1]==!toPolish
			int sqlDictionary[2];
			int distance;	// for fuzzy search
			int searchmode;
			int dictionarymode;
			rgb_color colour, colour0, colour1, colour2;
			BFont currentFont;
			BRect position;

			void load(void);
			void save(void);
			void setDefaultConfiguration(void);
		private:
			BFile conf;
			BString cfgname;

			void readValue(const char *buf, const char *token, rgb_color *result);
			void readValue(const char *buf, const char *token, bool *result);
			void readValue(const char *buf, const char *token, int *result);
			char *readValue(const char *buf, const char *token);
			void readValue(const char *buf, const char *token, BFont *result);
			void readValue(const char *buf, const char *token, BRect *result);

			void writeValue(BString variable, rgb_color value);
			void writeValue(BString variable, int value);
			void writeValue(BString variable, BString value);
			void writeValue(BString variable, bool value);
			void writeValue(BString variable, BFont font);
			void writeValue(BString variable, BRect value);
	};

#endif
