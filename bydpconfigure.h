
#ifndef _BYDPCONFIGURE
#define _BYDPCONFIGURE

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
			void SetupColourDialog(int colour);
			void SetupDistanceDialog(void);
		private:
			void ConfigUpdate(void);
			void CopyNewColours(rgb_color *to);
			void UpdateExampleColour();
			BHandler *myHandler;
			bydpConfig *myConfig;
			int myColour;

			BColorControl *myCColor;
			BStringView *exampleText;
			BSlider *mySlider;
			BView *mainView;
	};

	const uint32 MSG_COLOURUPDATE =		'CNUp';
	const uint32 MSG_FUZZYUPDATE =		'CNFu';

#endif
