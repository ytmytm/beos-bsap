
#ifndef _BYDPCONFIG_H
#define _BYDPCONFIG_H

#include <String.h>
#include <Font.h>
#include <File.h>

#define SEARCH_BEGINS	1
#define SEARCH_FUZZY	2

#define CONFIG_NAME "/boot/home/config/settings/bsap"

	class bydpConfig {
		public:
			bydpConfig();
			~bydpConfig();

			BString topPath;
			BString dataFName;
			bool toPolish;
			bool clipboardTracking;
			bool setFocusOnSelf;
			int distance;	// for fuzzy search
			int searchmode;
			int todisplay;	// how many to show on list with plain search
			rgb_color colour, colour0, colour1, colour2;
			BFont currentFont;

			void load(void);
			void save(void);
			void setDefaultConfiguration(void);
		private:
			BFile conf;
			BString cfgname;
			void updateFName(void);

			void readValue(const char *buf, const char *token, rgb_color *result);
			void readValue(const char *buf, const char *token, bool *result);
			void readValue(const char *buf, const char *token, int *result);
			char *readValue(const char *buf, const char *token);
			void readValue(const char *buf, const char *token, BFont *result);

			void writeValue(BString variable, rgb_color value);
			void writeValue(BString variable, int value);
			void writeValue(BString variable, BString value);
			void writeValue(BString variable, bool value);
			void writeValue(BString variable, BFont font);
	};

#endif
