//
// BUGS
//
// TODO (order of importance):
//	- direct utf8 convert for ydp phonetic signs
//	- if resize would be enabled (find XXX) => menubar is invisible (reposition it there?)
//	- write brief docs about my classes and derivatives
// LATER:
//	- change config dialog into sth like BeIDE project properties
//	- rewrite bydpconfig so readValue would return value always the same way
//	  and use defaults



#include <Alert.h>
#include <Clipboard.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Path.h>
#include <ScrollView.h>

#include <stdio.h>

#include "bydpmainwindow.h"
#include "engine_sap.h"
#include "engine_ydp.h"
//#include "engine_sq2.h"

const uint32 MSG_MODIFIED_INPUT =	'MInp';	// wpisanie litery
const uint32 MSG_LIST_SELECTED =	'LSel'; // klik na liscie
const uint32 MSG_LIST_INVOKED =		'LInv'; // dwuklik na liscie
const uint32 MSG_CLEAR_INPUT =		'IClr';	// shortcut - czyszczenie linii

const uint32 MENU_SWITCH =			'MSwi';
const uint32 MENU_ENG2POL =			'ME2P';
const uint32 MENU_POL2ENG =			'MP2E';
const uint32 MENU_FUZZY =			'MFuz';
const uint32 MENU_PLAIN =			'MPla';
const uint32 MENU_ENGINESAP =		'MESA';
const uint32 MENU_ENGINEYDP =		'MEYD';
//const uint32 MENU_ENGINESQ2 =		'MES2';
const uint32 MENU_PATH =			'MPat';
const uint32 MENU_COLOR0 =			'MCo0';
const uint32 MENU_COLOR1 =			'MCo1';
const uint32 MENU_COLOR2 =			'MCo2';
const uint32 MENU_COLOR3 =			'MCo3';
const uint32 MENU_CLIP =			'MCli';
const uint32 MENU_FOCUS =			'MFoc';
const uint32 MENU_ABOUT =			'MAbo';
const uint32 MENU_DISTANCE =		'MDis';
//const uint32 MENU_SQL =				'MSQL';
const uint32 FONT_SIZE =			'MFsi';
const uint32 FONT_FAMILY =			'MFam';
const uint32 FONT_STYLE =			'MFst';

BYdpMainWindow::BYdpMainWindow(const char *windowTitle) : BWindow(
	BRect(64, 64, 585, 480), windowTitle, B_DOCUMENT_WINDOW, B_OUTLINE_RESIZE ) {

	this->Hide();
	config = new bydpConfig();

	BView *MainView(
		new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0) );

	if (MainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	MainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(MainView);
	wordInput = new BTextControl(
		BRect(5,24,210,45), "wordInput", NULL, "text", new BMessage(MSG_MODIFIED_INPUT));
	wordInput->SetModificationMessage(new BMessage(MSG_MODIFIED_INPUT));
	MainView->AddChild(wordInput);

	outputView = new BTextView(
		BRect(220,24,506,402), "outputView", BRect(10,10,300,200), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW|B_PULSE_NEEDED);
	outputView->SetText("output");
	outputView->MakeEditable(false);
	outputView->SetStylable(true);
	MainView->AddChild(new BScrollView("scrolloutput",outputView,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, 0, true, true));

	dictList = new bydpListView("listView", this);
	MainView->AddChild(new BScrollView("scrollview", dictList, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, false, B_FANCY_BORDER));
	dictList->SetInvocationMessage(new BMessage(MSG_LIST_INVOKED));
	dictList->SetSelectionMessage(new BMessage(MSG_LIST_SELECTED));
	BRect barr = dictList->Bounds();
	barr.left = barr.right-B_V_SCROLL_BAR_WIDTH;
	scrollBar = new bydpScrollBar(barr, "scrollbar", dictList);
	dictList->AddChild(scrollBar);
	dictList->SetScrollBar(scrollBar);

	ydpConv = new ConvertYDP();
	sapConv = new ConvertSAP();
//	sq2Conv = new ConvertSQ2();
	ydpDict = new EngineYDP(outputView, dictList, config, ydpConv);
	sapDict = new EngineSAP(outputView, dictList, config, sapConv);
//	sq2Dict = new EngineSQ2(outputView, dictList, config, sq2Conv);
	switch(config->dictionarymode) {
/*		case DICTIONARY_SQ2:
			myDict = sq2Dict;
			myConverter = sq2Conv;
			break; */
		case DICTIONARY_YDP:
			myDict = ydpDict;
			myConverter = ydpConv;
			break;
		case DICTIONARY_SAP:
		default:
			myDict = sapDict;
			myConverter = sapConv;
			break;
	}
	dictList->SetConverter(myConverter);

	BRect r;
	r = MainView->Bounds();
	r.bottom = 19;
	BMenuBar *menubar = new BMenuBar(r, "menubar");
	MainView->AddChild(menubar);

	BMenu *menu = new BMenu("File");
	menu->AddItem(new BMenuItem("About...", new BMessage(MENU_ABOUT), 'O'));
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menubar->AddItem(menu);

	BMenu *engineMenu;
	menu = new BMenu("Dictionary");
	menu->AddItem(new BMenuItem("Switch", new BMessage(MENU_SWITCH), 'J'));
	menu->AddItem(menuEng = new BMenuItem("Eng -> Pol", new BMessage(MENU_ENG2POL), 'E'));
	menu->AddItem(menuPol = new BMenuItem("Pol -> Eng", new BMessage(MENU_POL2ENG), 'P'));
	menu->AddSeparatorItem();
	menu->AddItem(engineMenu = new BMenu("Dictionary engine"));
	engineMenu->AddItem(menuSAP = new BMenuItem("SAP", new BMessage(MENU_ENGINESAP)));
	engineMenu->AddItem(menuYDP = new BMenuItem("YDP", new BMessage(MENU_ENGINEYDP)));
//	engineMenu->AddItem(menuSQ2 = new BMenuItem("SQ2", new BMessage(MENU_ENGINESQ2)));
	menubar->AddItem(menu);

	menu = new BMenu("Search type");
	menu->AddItem(menuPlain = new BMenuItem("Plain", new BMessage(MENU_PLAIN), 'Z'));
	menu->AddItem(menuFuzzy = new BMenuItem("Fuzzy", new BMessage(MENU_FUZZY), 'R'));
	menubar->AddItem(menu);

	menu = new BMenu("Settings");
	menu->AddItem(new BMenuItem("Path to dictionary", new BMessage(MENU_PATH), 'S'));
	menu->AddSeparatorItem();
	menu->AddItem(menuCol0 = new BMenuItem(myDict->ColourFunctionName(0), new BMessage(MENU_COLOR0)));
	menu->AddItem(menuCol1 = new BMenuItem(myDict->ColourFunctionName(1), new BMessage(MENU_COLOR1)));
	menu->AddItem(menuCol2 = new BMenuItem(myDict->ColourFunctionName(2), new BMessage(MENU_COLOR2)));
	menu->AddItem(menuCol3 = new BMenuItem(myDict->ColourFunctionName(3), new BMessage(MENU_COLOR3)));
	menu->AddSeparatorItem();
	menu->AddItem(menuClip = new BMenuItem("Clipboard tracking", new BMessage(MENU_CLIP), 'L'));
	menu->AddItem(menuFocus = new BMenuItem("Popup window", new BMessage(MENU_FOCUS), 'F'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Fuzzy factor", new BMessage(MENU_DISTANCE)));
//	menu->AddItem(new BMenuItem("SQL data source", new BMessage(MENU_SQL)));
	menu->AddSeparatorItem();
	menubar->AddItem(menu);

	BMessage *fontMessage;
	fontMenu = new BMenu("Font");
	menu->AddItem(fontMenu);

	BMenu* fontSizeMenu = new BMenu("Size");
	fontSizeMenu->SetRadioMode(true);
	fontMenu->AddItem(fontSizeMenu);
	fontMenu->AddSeparatorItem();

	fontSizeMenu->AddItem(new BMenuItem("9", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size", 9.0);
	fontSizeMenu->AddItem(new BMenuItem("10", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",10.0);
	fontSizeMenu->AddItem(new BMenuItem("11", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",11.0);
	fontSizeMenu->AddItem(new BMenuItem("12", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",12.0);
	fontSizeMenu->AddItem(new BMenuItem("14", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",14.0);
	fontSizeMenu->AddItem(new BMenuItem("18", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",18.0);	
	fontSizeMenu->AddItem(new BMenuItem("24", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",24.0);
	fontSizeMenu->AddItem(new BMenuItem("36", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",36.0);
	fontSizeMenu->AddItem(new BMenuItem("48", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",48.0);
	fontSizeMenu->AddItem(new BMenuItem("72", fontMessage = new BMessage(FONT_SIZE)));
	fontMessage->AddFloat("size",72.0);

	font_family plain_family;
	font_style plain_style;
	config->currentFont.GetFamilyAndStyle(&plain_family,&plain_style);

	BMenu *subMenu;
	BMenuItem *menuItem;
	currentFontItem = 0;

	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ ) {
		font_family localfamily;
		if ( get_font_family ( i, &localfamily ) == B_OK ) {
			subMenu = new BMenu(localfamily);
			subMenu->SetRadioMode(true);
			fontMenu->AddItem(menuItem = new BMenuItem(subMenu, new BMessage(FONT_FAMILY)));
			if (!strcmp(plain_family,localfamily)) {
				menuItem->SetMarked(true);
				currentFontItem = menuItem;
			}
			int32 numStyles=count_font_styles(localfamily);
			for(int32 j = 0;j<numStyles;j++){
				font_style style;
				uint32 flags;
				if( get_font_style(localfamily,j,&style,&flags)==B_OK){
					subMenu->AddItem(menuItem = new BMenuItem(style, new BMessage(FONT_STYLE)));
					if (!strcmp(plain_style,style)) {
						menuItem->SetMarked(true);
					}
				}
			}
		}
	}

	this->FrameResized(0.0, 0.0);
	UpdateMenus();

	wordInput->MakeFocus(true);
	firstStart = true;
	TryToOpenDict();

	BMessenger mesg(this);
	be_clipboard->StartWatching(mesg);
}

BYdpMainWindow::~BYdpMainWindow() {
}

void BYdpMainWindow::NewClipData(void) {
	const char *text; 
	int32 textLen; 
	BString result;
	static BString lastResult;
	BMessage *clip = (BMessage *)NULL; 
	int i;
	char c;

	if (!config->clipboardTracking)
		return;

	if (be_clipboard->Lock()) { 
		if ((clip = be_clipboard->Data()))
			clip->FindData("text/plain", B_MIME_TYPE,(const void **)&text, &textLen);
		be_clipboard->Unlock();
		result = "";
		for (i=0;i<textLen;i++) {
			c = text[i];
			if ((c!=' ')&&(c!='.')&&(c!=',')&&(c!='\t')&&(c!='\'')&&(c!='"'))
				result += c;
		}
		if (lastResult.Compare(result) != 0) {
			lastResult = result;
			wordInput->SetText(result.String());
			if (config->setFocusOnSelf)
				this->Activate();
		}
//		const char *tmp = result.String();
//		printf("got:%s:clip:%i,%i,%i\n",tmp,tmp[0],tmp[1],tmp[2]);
	}
}

void BYdpMainWindow::HandleModifiedInput(bool force) {
	static BString lastinput;
	if ((!force)&&(!strcmp(lastinput.String(),wordInput->Text()))) {
		return;
	}
	lastinput = wordInput->Text();
	int item = myDict->FindWord(lastinput.String());
//	printf("new input %s\n",lastinput.String());
	myDict->GetDefinition(item);
}

void BYdpMainWindow::UpdateMenus(void) {
	menuPlain->SetMarked(config->searchmode == SEARCH_BEGINS);
	menuFuzzy->SetMarked(config->searchmode == SEARCH_FUZZY);
	menuEng->SetMarked(config->toPolish);
	menuPol->SetMarked(!config->toPolish);
	menuClip->SetMarked(config->clipboardTracking);
	menuFocus->SetMarked(config->setFocusOnSelf);
	menuFocus->SetEnabled(config->clipboardTracking);
	menuSAP->SetMarked(config->dictionarymode == DICTIONARY_SAP);
	menuYDP->SetMarked(config->dictionarymode == DICTIONARY_YDP);
//	menuSQ2->SetMarked(config->dictionarymode == DICTIONARY_SQ2);
	menuCol0->SetLabel(myDict->ColourFunctionName(0));
	menuCol1->SetLabel(myDict->ColourFunctionName(1));
	menuCol2->SetLabel(myDict->ColourFunctionName(2));
	menuCol3->SetLabel(myDict->ColourFunctionName(3));

	BString name = APP_NAME;
	name += ": ";
	name += myDict->AppBarName();
	this->SetTitle(name.String());
//	if (config->toPolish)
//		this->SetTitle(APP_NAME ": Eng->Pol");
//	else
//		this->SetTitle(APP_NAME ": Pol->Eng");
}

void BYdpMainWindow::UpdateLanguages(bool newlang) {
	myDict->CloseDictionary();
	config->toPolish = newlang;
	config->save();
	myDict->OpenDictionary();
	UpdateMenus();
	HandleModifiedInput(true);
}

void BYdpMainWindow::ConfigColour(int number) {
//	printf("configure colour %i\n", number);
	myDialog = new bydpConfigure("Colour settings", this);
	myDialog->SetConfig(config);
	myDialog->SetupDialog(BYDPCONF_COLOUR,number);
	myDialog->Show();
}

void BYdpMainWindow::ConfigDistance(void) {
	myDialog = new bydpConfigure("Fuzzy search factor", this);
	myDialog->SetConfig(config);
	myDialog->SetupDialog(BYDPCONF_DISTANCE);
	myDialog->Show();
}

/* void BYdpMainWindow::ConfigSQLTables(void) {
//	printf("in csql\n");
	myDialog = new bydpConfigure("Choose SQL dictionaries", this);
	myDialog->SetConfig(config);
	myDialog->SetupDialog(BYDPCONF_SQL);
	myDialog->Show();
} */

void BYdpMainWindow::SwitchEngine(int newengine) {
	myDict->CloseDictionary();
	config->dictionarymode = newengine;
	config->save();
	TryToOpenDict();
}

void BYdpMainWindow::TryToOpenDict(void) {
//		printf("about to reopen dict\n");
	switch (config->dictionarymode) {
/*		case DICTIONARY_SQ2:
			myDict = sq2Dict;
			myConverter = sq2Conv;
			break; */
		case DICTIONARY_YDP:
			myDict = ydpDict;
			myConverter = ydpConv;
			break;
		case DICTIONARY_SAP:
		default:
			myDict = sapDict;
			myConverter = sapConv;
			break;
	}
	if (myDict->OpenDictionary() < 0) {
//		printf("failed\n");
		ConfigPath();
	} else {
//		printf("success\n");
		firstStart = false;
		dictList->SetConverter(myConverter);
		wordInput->SetText("A");
		HandleModifiedInput(true);
		UpdateMenus();
		this->MoveTo(BPoint(config->position.left, config->position.top));
//		XXX if this is enabled - menubar is lost
//		this->ResizeTo(config->position.Width(),config->position.Height());

		this->Show();
	}
}

void BYdpMainWindow::ConfigPath(void) {
//	printf("configure path\n");
	BMessenger mesg(this);
	myPanel = new BFilePanel(B_OPEN_PANEL,
			&mesg, NULL, B_DIRECTORY_NODE, false, NULL, NULL, true, true);
	myPanel->Show();
	myPanel->Window()->SetTitle("Choose directory with dictionary files");
}

void BYdpMainWindow::RefsReceived(BMessage *Message) {
	int ref_num;
	entry_ref ref;
	status_t err;
	ref_num = 0;
	do {
		if ((err = Message->FindRef("refs", ref_num, &ref)) != B_OK)
			return;
		BPath path;
		BEntry myEntry(&ref);
		myEntry.GetPath(&path);
//		printf("got new path %s\n", path.Path());
		config->topPath = path.Path();
		config->save();
		TryToOpenDict();
		ref_num++;
	} while (1);
}

void BYdpMainWindow::MessageReceived(BMessage *Message) {
	int item;
	this->DisableUpdates();
	switch (Message->what) {
		case MSG_MODIFIED_INPUT:
			HandleModifiedInput(false);
			break;
		case MSG_LIST_INVOKED:
			item = dictList->CurrentSelection(0);
			wordInput->SetText(((BStringItem*)dictList->ItemAt(item))->Text());
			break;
		case MSG_LIST_SELECTED:
			item = dictList->CurrentSelection(0);
			if (item>dictList->CountItems())
				item = dictList->CountItems();
			if (item>=0)
				myDict->GetDefinition(myDict->wordPairs[item+dictList->topIndex]);
			break;
		case MSG_LISTUP:
		case MSG_LISTDOWN:
//			printf("got listup\n");
			myDict->GetDefinition(myDict->wordPairs[dictList->topIndex+dictList->CurrentSelection(0)]);
			break;
//		case MSG_LISTDOWN:
//			printf("got listdown\n");
//			myDict->GetDefinition(myDict->wordPairs[dictList->topIndex+dictList->CurrentSelection(0)]);
//			break;
		case MSG_CLEAR_INPUT:
			wordInput->SetText("");
			wordInput->MakeFocus();
			break;
		case MENU_ENG2POL:
			UpdateLanguages(true);
			break;
		case MENU_POL2ENG:
			UpdateLanguages(false);
			break;
		case MENU_SWITCH:
			UpdateLanguages(!config->toPolish);
			break;
		case MENU_FUZZY:
			config->searchmode = SEARCH_FUZZY;
			config->save();
			HandleModifiedInput(true);
			UpdateMenus();
			break;
		case MENU_PLAIN:
			config->searchmode = SEARCH_BEGINS;
			config->save();
			HandleModifiedInput(true);
			UpdateMenus();
			break;
/*		case MENU_ENGINESQ2:
			SwitchEngine(DICTIONARY_SQ2);
			break; */
		case MENU_ENGINESAP:
			SwitchEngine(DICTIONARY_SAP);
			break;
		case MENU_ENGINEYDP:
			SwitchEngine(DICTIONARY_YDP);
			break;
		case MENU_PATH:
			ConfigPath();
			break;
		case MENU_COLOR0:
			ConfigColour(0);
			break;
		case MENU_COLOR1:
			ConfigColour(1);
			break;
		case MENU_COLOR2:
			ConfigColour(2);
			break;
		case MENU_COLOR3:
			ConfigColour(3);
			break;
		case MENU_CLIP:
			config->clipboardTracking = !config->clipboardTracking;
			config->save();
			UpdateMenus();
			break;
		case MENU_FOCUS:
			config->setFocusOnSelf = !config->setFocusOnSelf;
			config->save();
			UpdateMenus();
			break;
		case MENU_DISTANCE:
			ConfigDistance();
			break;
/*		case MENU_SQL:
			ConfigSQLTables();
			break; */
		case MENU_ABOUT: {
			BString about;
			about = "\n\n" APP_NAME " " APP_VERSION "\n";
			about += "English-Polish, Polish-English dictionary\n";
			about += "\n\nBeOS version:\n";
			about += "Maciej Witkowiak <ytm@elysium.pl>";
			about += "\n\nSAP engine based on sap v0.2b\n";
			about += "(c) 1998 Bohdan R. Rau,\n(c) 2001 Daniel Mealha Cabrita";
			about += "\nYDP and SQL engines by Maciej Witkowiak\n";
			about += "\n\nSoftware released under GNU/GPL license";
			about += "\n\nvisit:\n";
			about += "http://home.elysium.pl/ytm/html/beos.html\n";
			about += "\nDevelopment has been encouraged by:\n";
			about += "http://www.haiku-os.pl";
			outputView->SetText(about.String()); }
			break;
		case FONT_SIZE:
		{
			float fontSize;
			Message->FindFloat("size",&fontSize);
			SetFontSize(fontSize);
		}
		break;
		case FONT_FAMILY:
		{
			const char * fontFamily = 0, * fontStyle = 0;
			void * ptr;
			if (Message->FindPointer("source",&ptr) == B_OK) {
				currentFontItem = static_cast<BMenuItem*>(ptr);
				fontFamily = currentFontItem->Label();
			}
			SetFontStyle(fontFamily, fontStyle);
		}
		case FONT_STYLE:
		{
			const char * fontFamily = 0, * fontStyle = 0;
			void * ptr;
			if (Message->FindPointer("source",&ptr) == B_OK) {
				BMenuItem * item = static_cast<BMenuItem*>(ptr);
				fontStyle = item->Label();
				BMenu * menu = item->Menu();
				if (menu != 0) {
					currentFontItem = menu->Superitem();
					if (currentFontItem != 0) {
						fontFamily = currentFontItem->Label();
					}
				}
			}
			SetFontStyle(fontFamily, fontStyle);
		}
		case MSG_SCROLL:
//			printf("scroll value changed\n");
			if (scrollBar->Value() != dictList->topIndex) {
				dictList->ListScrolled(scrollBar->Value());
				myDict->GetDefinition(myDict->wordPairs[dictList->topIndex+dictList->CurrentSelection(0)]);
			}
			break;
		case MSG_COLOURUPDATE:
//			printf("colour update\n");
			myDict->ReGetDefinition();
			break;
		case MSG_FUZZYUPDATE:
//			printf("fuzzy update\n");
			if (config->searchmode == SEARCH_FUZZY)
				HandleModifiedInput(true);
			break;
/*		case MSG_SQLTABLESUPDATE:
//			printf("sql tables updated\n");
			myDict->FlushCache();
			SwitchEngine(config->dictionarymode);
			break; */
		case B_CLIPBOARD_CHANGED:
			NewClipData();
			break;
		case B_REFS_RECEIVED:
			RefsReceived(Message);
			break;
		case B_CANCEL:
//			printf("canceled\n");
			if (firstStart) {
				config->dictionarymode = DICTIONARY_SAP;	// will be saved in QuitRequested below
				BAlert *alert = new BAlert(APP_NAME, "Couldn't open dictionary. Dictionary engine has been reset to SAP.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->Go();
				QuitRequested();
			}
			else
				delete myPanel;
//			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
	this->EnableUpdates();
}

bool BYdpMainWindow::QuitRequested() {
	config->position = this->Frame();
	config->save();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

void BYdpMainWindow::FrameResized(float width, float height) {
	dictList->NewSize();
}

void BYdpMainWindow::SetFontSize(float fontSize) {
	config->currentFont.SetSize(fontSize);
	config->save();
	myDict->ReGetDefinition();
}

void BYdpMainWindow::SetFontStyle(const char *fontFamily, const char *fontStyle) {
	font_family oldFamily;
	font_style oldStyle;

	config->currentFont.GetFamilyAndStyle(&oldFamily,&oldStyle);
	if (strcmp(oldFamily,fontFamily)) {
		BMenuItem * oldItem = fontMenu->FindItem(oldFamily);
		if (oldItem != 0)
			oldItem->SetMarked(false);
	}
	config->currentFont.SetFamilyAndStyle(fontFamily,fontStyle);

	BMenuItem * superItem;
	superItem = fontMenu->FindItem(fontFamily);
	if (superItem != 0)
		superItem->SetMarked(true);
	config->save();
	myDict->ReGetDefinition();
}

void BYdpMainWindow::DispatchMessage(BMessage *message, BHandler *handler) {
	int8 key;
	status_t result;

	if (message->what == B_KEY_DOWN) {
		result = message->FindInt8("byte", 0, &key);
		if (result == B_OK) {
			if (key == B_ESCAPE){
				message->MakeEmpty();
				message->what=MSG_CLEAR_INPUT;
			}
		}
	}
	BWindow::DispatchMessage(message,handler);	
}
