
#include "bydpconverter.h"

/////////////////////
// utf8 <-> anything convertion stuff below
//

bydpConverter::bydpConverter(void) {
}

bydpConverter::~bydpConverter() {
}

char bydpConverter::tolower(const char c) {
	return c;
}

char *bydpConverter::ConvertToUtf(const char *line) {
	return (char*)line;
}

char *bydpConverter::ConvertFromUtf(const char *input) {
	return (char*)input;
}
