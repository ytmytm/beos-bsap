#include <Catalog.h>

#ifndef _globals_h
#define _globals_h
	// application signature
	#define APP_NAME "BSAP"
	#define APP_VERSION "0.8 (20050917)"
	#define APP_SIGNATURE "application/x-vnd.generic-BSAP"
	extern int AppReturnValue;
	// gettext-style translation helper
//	#define _(x) SpTranslate(x)
	// Qt-style translation helper
//	#define tr(x) SpTranslate(x)
	// Haiku's translation helper
	#undef B_TRANSLATION_CONTEXT
    #define B_TRANSLATION_CONTEXT "BSAP"
	#define _(x) B_TRANSLATE(x)
	#define tr(x) B_TRANSLATE(x)
#endif
