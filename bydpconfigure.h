
#ifndef _BYDPCONFIGURE
#define _BYDPCONFIGURE

	#include <View.h>
	#include <Window.h>
	#include <ColorControl.h>
	#include <Slider.h>
	#include "bydpconfig.h"

	class bydpConfigure : public BWindow {
		public:
			bydpConfigure(const char *title, void *parent);
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
			void *parent;
			bydpConfig *myConfig;
			int myColour;

			BColorControl *myCColor;
			BStringView *exampleText;
			BSlider *mySlider;
			BView *mainView;
	};

#endif
