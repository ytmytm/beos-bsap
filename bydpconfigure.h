
#ifndef _BYDPCONFIGURE
#define _BYDPCONFIGURE

#define BYDPCONF_COLOUR		0		// anything >0 in fact
#define BYDPCONF_DISTANCE	-1
#define BYDPCONF_SQL		-2

	#include <View.h>
	#include <Window.h>
	#include <ColorControl.h>
	#include <Slider.h>
	#include "bydpconfig.h"

	class bydpConfigure : public BWindow {
		public:
			bydpConfigure(const char *title, BHandler *handler);
			~bydpConfigure();
			virtual void MessageReceived(BMessage *msg);
			virtual bool QuitRequested();
			void SetConfig(bydpConfig *config);
			void SetupDialog(int type, int param = -1);
		private:
			void SetupColourDialog(int colour);
			void SetupDistanceDialog(void);
			void SetupSQLDialog(void);
			void ConfigUpdate(void);
			void CopyNewColours(rgb_color *to);
			void UpdateExampleColour();
			BHandler *myHandler;
			bydpConfig *myConfig;
			int myColour;
			int dialogType;

			BColorControl *myCColor;
			BStringView *exampleText;
			BSlider *mySlider;
			BView *mainView;
	};

	const uint32 MSG_COLOURUPDATE =		'CNUp';
	const uint32 MSG_FUZZYUPDATE =		'CNFu';

#endif
