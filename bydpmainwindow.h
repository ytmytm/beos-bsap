
#ifndef _BYDPMAINWINDOW_H
#define _BYDPMAINWINDOW_H

	#include <SpLocaleApp.h>
	#include <View.h>
	#include <Window.h>
	#include <TextControl.h>
	#include <TextView.h>
	#include <MenuItem.h>
	#include <FilePanel.h>
	#include "globals.h"
	#include "bydpconfig.h"
	#include "bydpconfigure.h"
	#include "bydpdictionary.h"
	#include "bydplistview.h"

	class BYdpMainWindow : public BWindow {
		public:
			BYdpMainWindow(const char *windowTitle);
			~BYdpMainWindow();
			virtual void FrameResized(float width, float height);
			virtual void MessageReceived(BMessage *Message);
			virtual void DispatchMessage(BMessage *message, BHandler *handler);
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
			bydpScrollBar *scrollBar;
			bydpListView *dictList;
			BMenuItem *menuFuzzy, *menuPlain, *menuEng, *menuPol, *menuClip;
			BMenuItem *menuFocus;
			BMenu *fontMenu;
			BMenuItem *currentFontItem;
			void SetFontSize(float fontSize);
			void SetFontStyle(const char *fontFamily, const char *fontStyle);

			ydpDictionary *myDict;
			bydpConfig *config;
			bydpConfigure *myDialog;

			BFilePanel *myPanel;
			bool firstStart;

	};

#endif
