
#ifndef _BYDPCONVERTER_H
#define _BYDPCONVERTER_H

	class bydpConverter {
		public:
			bydpConverter(void);
			virtual ~bydpConverter();
			// dictionary-dependent convertion routines
			virtual char tolower(const char c);
			virtual char *ConvertToUtf(const char *line);
			virtual char *ConvertFromUtf(const char *input);
	};

#endif