//
// TODO (w porzadku waznosci):
// LATER:
//	- nie ma odswiezenia outputView po zmianie kolorow (jakos to sie pieprzy)
//	- po wyszukiwaniu pierwszy klik na liste nie dziala
//		- przychodzi msg o zmianie inputa!
//	- lista wyrazow przy nie-fuzzy zachowuje sie nieintuicyjnie, na razie
//	  musi wystarczyc, w przyszlosci pewnie lepiej byloby sportowac kydpdict

#include "bydpmainwindow.h"
#include <ScrollView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Path.h>
#include <Clipboard.h>
#include <stdio.h>

const uint32 MSG_MODIFIED_INPUT =	'MInp';	// wpisanie litery
const uint32 MSG_LIST_SELECTED =	'LSel'; // klik na liscie
const uint32 MSG_LIST_INVOKED =		'LInv'; // dwuklik na liscie
const uint32 MSG_CLEAR_INPUT =		'IClr';	// shortcut - czyszczenie linii

const uint32 MENU_SWITCH =			'MSwi';
const uint32 MENU_ENG2POL =			'ME2P';
const uint32 MENU_POL2ENG =			'MP2E';
const uint32 MENU_FUZZY =			'MFuz';
const uint32 MENU_PLAIN =			'MPla';
const uint32 MENU_PATH =			'MPat';
const uint32 MENU_COLOR0 =			'MCo0';
const uint32 MENU_COLOR1 =			'MCo1';
const uint32 MENU_COLOR2 =			'MCo2';
const uint32 MENU_COLOR3 =			'MCo3';
const uint32 MENU_CLIP =			'MCli';
const uint32 MENU_FOCUS =			'MFoc';
const uint32 MENU_ABOUT =			'MAbo';
const uint32 MENU_DISTANCE =		'MDis';
const uint32 FONT_SIZE =			'MFsi';
const uint32 FONT_FAMILY =			'MFam';
const uint32 FONT_STYLE =			'MFst';

BYdpMainWindow::BYdpMainWindow(const char *windowTitle) : BWindow(
	BRect(64, 64, 585, 480), windowTitle, B_TITLED_WINDOW, B_OUTLINE_RESIZE ) {

	BView *MainView(
		new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0) );

	if (MainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	this->Hide();
	this->AddShortcut(B_ESCAPE,B_SHIFT_KEY,new BMessage(MSG_CLEAR_INPUT));

	config = new bydpConfig();

	MainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(MainView);
	wordInput = new BTextControl(
		BRect(5,24,210,45), "wordInput", NULL, "text", new BMessage(MSG_MODIFIED_INPUT));
	wordInput->SetModificationMessage(new BMessage(MSG_MODIFIED_INPUT));
	MainView->AddChild(wordInput);

	outputView = new BTextView(
		BRect(220,24,500,400), "outputView", BRect(10,10,300,200), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW|B_PULSE_NEEDED);
	outputView->SetText("output");
	outputView->MakeEditable(false);
	outputView->SetStylable(true);
	MainView->AddChild(new BScrollView("scrolloutput",outputView,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, 0, true, true));

	dictList = new BListView(
		BRect(10,60,200,400), "listView", B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM);
	MainView->AddChild(new BScrollView("scollbar", dictList, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	dictList->SetInvocationMessage(new BMessage(MSG_LIST_INVOKED));
	dictList->SetSelectionMessage(new BMessage(MSG_LIST_SELECTED));

	BRect r;
	r = MainView->Bounds();
	r.bottom = 19;
	BMenuBar *menubar = new BMenuBar(r, "menubar");
	MainView->AddChild(menubar);

	BMenu *menu = new BMenu("Plik");
	menu->AddItem(new BMenuItem("O programie...", new BMessage(MENU_ABOUT), 'O'));
	menu->AddItem(new BMenuItem("Zakończ", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menubar->AddItem(menu);

	menu = new BMenu("Język");
	menu->AddItem(new BMenuItem("Przełącz język", new BMessage(MENU_SWITCH), 'J'));
	menu->AddItem(menuEng = new BMenuItem("Eng -> Pol", new BMessage(MENU_ENG2POL), 'E'));
	menu->AddItem(menuPol = new BMenuItem("Pol -> Eng", new BMessage(MENU_POL2ENG), 'P'));
	menubar->AddItem(menu);

	menu = new BMenu("Wyszukiwanie");
	menu->AddItem(menuPlain = new BMenuItem("Zwykłe", new BMessage(MENU_PLAIN), 'Z'));
	menu->AddItem(menuFuzzy = new BMenuItem("Rozmyte", new BMessage(MENU_FUZZY), 'R'));
	menubar->AddItem(menu);

	menu = new BMenu("Ustawienia");
	menu->AddItem(new BMenuItem("Ścieżka do słownika", new BMessage(MENU_PATH), 'S'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Kolor tłumaczenia", new BMessage(MENU_COLOR0)));
	menu->AddItem(new BMenuItem("Kolor słów kluczowych", new BMessage(MENU_COLOR1)));
	menu->AddItem(new BMenuItem("Kolor kwalifikatorów", new BMessage(MENU_COLOR2)));
	menu->AddItem(new BMenuItem("Kolor dodatkowyego tekstu", new BMessage(MENU_COLOR3)));
	menu->AddSeparatorItem();
	menu->AddItem(menuClip = new BMenuItem("Śledzenie schowka", new BMessage(MENU_CLIP), 'L'));
	menu->AddItem(menuFocus = new BMenuItem("Wyskakujące okno", new BMessage(MENU_FOCUS), 'F'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Stopień rozmycia", new BMessage(MENU_DISTANCE)));
	menubar->AddItem(menu);

	BMessage *fontMessage;
	fontMenu = new BMenu("Czcionka");
	menu->AddItem(fontMenu);

	BMenu* fontSizeMenu = new BMenu("Rozmiar");
	fontSizeMenu->SetRadioMode(true);
	fontMenu->AddItem(fontSizeMenu);

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

	myDict = new ydpDictionary(outputView, dictList, config);
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

	if (!config->clipboardTracking)
		return;

	if (be_clipboard->Lock()) { 
		if ((clip = be_clipboard->Data()))
			clip->FindData("text/plain", B_MIME_TYPE,(const void **)&text, &textLen);
		be_clipboard->Unlock();
		result.SetTo(text,textLen);
		result.RemoveSet(" ");
		if (lastResult.Compare(result) != 0) {
			lastResult = result;
			wordInput->SetText(result.String());
			if (config->setFocusOnSelf)
				this->Activate();
		}
//		printf("got:%s:clip\n",result.String());
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
	if (config->toPolish)
		this->SetTitle("BSAP: Eng->Pol");
	else
		this->SetTitle("BSAP: Pol->Eng");
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
	myDialog = new bydpConfigure("Ustawienie kolorów", this);
	myDialog->SetConfig(config);
	myDialog->SetupColourDialog(number);
	myDialog->Show();
}

void BYdpMainWindow::ConfigDistance(void) {
	myDialog = new bydpConfigure("Stopień rozmycia wyszukiwania", this);
	myDialog->SetConfig(config);
	myDialog->SetupDistanceDialog();
	myDialog->Show();
}

void BYdpMainWindow::TryToOpenDict(void) {
//	printf("about to reopen dict\n");
	if (myDict->OpenDictionary() < 0) {
//		printf("failed\n");
		ConfigPath();
	} else {
//		printf("success\n");
		firstStart = false;
		wordInput->SetText("A");
		this->Show();
	}
}

void BYdpMainWindow::ConfigPath(void) {
//	printf("configure path\n");
	BMessenger mesg(this);
	myPanel = new BFilePanel(B_OPEN_PANEL,
			&mesg, NULL, B_DIRECTORY_NODE, false, NULL, NULL, true, true);
	myPanel->Show();
	myPanel->Window()->SetTitle("Wybierz katalog z plikami słownika");
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
				myDict->GetDefinition(myDict->wordPairs[item]);
			break;
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
		case MENU_ABOUT:
			outputView->SetText("\n\nBSAP 0.5 (XII 2004)\nsłownik angielsko-polski, polsko-angielski\n"
				"\n\nwersja dla BeOSa:\nMaciej Witkowiak <ytm@elysium.pl>"
				"\n\nna podstawie sap v0.2b\n(c) 1998 Bohdan R. Rau,\n(c) 2001 Daniel Mealha Cabrita"
				"\n\nProgram na licencji GNU/GPL"
				"\n\nodwiedź:\nhttp://home.elysium.pl/ytm/html/beos.html");
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
		case B_CLIPBOARD_CHANGED:
			NewClipData();
			break;
		case B_REFS_RECEIVED:
			RefsReceived(Message);
			break;
		case B_CANCEL:
//			printf("canceled\n");
			if (firstStart)
				QuitRequested();
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
	config->save();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

void BYdpMainWindow::FrameResized(float width, float height) {
	float dictSize;
	float itemSize;
	int spacefor;
	font_height myHeight;
	dictList->GetFontHeight(&myHeight);
	dictSize = dictList->Bounds().Height();
	itemSize = myHeight.leading+myHeight.ascent+myHeight.descent;
	spacefor = (int)(dictSize/itemSize-2);
	if (spacefor<1) spacefor = 1;
	config->todisplay = spacefor;
//	printf("spacefor: %i\n",spacefor);
	if (config->searchmode == SEARCH_BEGINS)
		HandleModifiedInput(true);
}

void BYdpMainWindow::SetFontSize(float fontSize) {
	config->currentFont.SetSize(fontSize);
	config->save();
}

void BYdpMainWindow::SetFontStyle(const char *fontFamily, const char *fontStyle) {
	font_family oldFamily;
	font_style oldStyle;

	config->currentFont.GetFamilyAndStyle(&oldFamily,&oldStyle);
	// clear that family's bit on the menu, if necessary
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
}
