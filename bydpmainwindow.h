
#ifndef _BYDPMAINWINDOW_H
#define _BYDPMAINWINDOW_H

	#include <Application.h>
	#include <View.h>
	#include <Window.h>
	#include <TextControl.h>
	#include <TextView.h>
	#include <ListView.h>
	#include <MenuItem.h>
	#include <FilePanel.h>
	#include "globals.h"
	#include "bydpconfig.h"
	#include "bydpconfigure.h"
	#include "bydpdictionary.h"

	class BYdpMainWindow : public BWindow {
		public:
			BYdpMainWindow(const char *windowTitle);
			~BYdpMainWindow();
			virtual void FrameResized(float width, float height);
			virtual void MessageReceived(BMessage *Message);
			virtual bool QuitRequested();
			virtual void RefsReceived(BMessage *Message);
		private:
			void NewClipData(void);
			void HandleModifiedInput(bool force);
			void UpdateMenus(void);
			void UpdateLanguages(bool newlang);
			void ConfigPath(void);
			void ConfigColour(int number);
			void ConfigDistance(void);
			void TryToOpenDict(void);
			BTextView *outputView;
			BTextControl *wordInput;
			BListView *dictList;
			BMenuItem *menuFuzzy, *menuPlain, *menuEng, *menuPol, *menuClip;
			BMenuItem *menuFocus;

			ydpDictionary *myDict;
			bydpConfig *config;
			bydpConfigure *myDialog;

			BFilePanel *myPanel;
			bool firstStart;

	};

#endif
