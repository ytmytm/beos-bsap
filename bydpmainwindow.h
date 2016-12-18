
#ifndef _BYDPMAINWINDOW_H
#define _BYDPMAINWINDOW_H

	#include <Application.h>
	#include <FilePanel.h>
	#include <MenuItem.h>
	#include <TextControl.h>
	#include <TextView.h>
	#include <View.h>
	#include <Window.h>
	
	#include "bydpconfig.h"
	#include "bydpconfigure.h"
	#include "bydpdictionary.h"
	#include "bydplistview.h"
	#include "globals.h"

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
//			void ConfigSQLTables(void);
			void TryToOpenDict(void);
			void SwitchEngine(int newengine);
			BTextView *outputView;
			BTextControl *wordInput;
			bydpScrollBar *scrollBar;
			bydpListView *dictList;
			BMenuItem *menuFuzzy, *menuPlain, *menuEng, *menuPol, *menuClip;
			BMenuItem *menuFocus, *menuSAP, *menuYDP/*, *menuSQ2*/;
			BMenuItem *menuCol0, *menuCol1, *menuCol2, *menuCol3;
			BMenu *fontMenu;
			BMenuItem *currentFontItem;
			void SetFontSize(float fontSize);
			void SetFontStyle(const char *fontFamily, const char *fontStyle);

			ydpDictionary *myDict, *sapDict, *ydpDict/*, *sq2Dict*/;
			bydpConverter *myConverter, *sapConv, *ydpConv/*, *sq2Conv*/;
			bydpConfig *config;
			bydpConfigure *myDialog;

			BFilePanel *myPanel;
			bool firstStart;

	};

#endif
