
#include <Button.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>

//#include <sqlite.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bydpconfigure.h"
#include "globals.h"

const uint32 CHOOSEDICT0 =		'ChD0';
const uint32 CHOOSEDICT1 =		'ChD1';
const uint32 BUTTON_OK =		'BuOK';
const uint32 BUTTON_CANCEL =	'BuCA';
const uint32 CCOLOR_MSG =		'ColM';
const uint32 SLIDER =			'Slid';

bydpConfigure::bydpConfigure(const char *title, BHandler *handler) : BWindow(
		BRect(62, 100, 62+370, 260),
		title,
		B_TITLED_WINDOW,
		B_NOT_RESIZABLE ) {

	myHandler = handler;
	myColour = -1;

	mainView = new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0);

	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(mainView);
}

void bydpConfigure::SetupDialog(int type, int param) {

	dialogType = type;

	switch (dialogType) {
/*		case BYDPCONF_SQL:
			SetupSQLDialog();
			break; */
		case BYDPCONF_DISTANCE:
			SetupDistanceDialog();
			break;
		default:
			SetupColourDialog(param);
			break;
	}
}

bydpConfigure::~bydpConfigure() {

}

void bydpConfigure::SetupDistanceDialog(void) {
	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", "Cancel", new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);
	BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok","OK", new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(OKButton);
	mySlider = new BSlider(BRect(44,20,22+285,20+100), "slider", "Fuzzy factor", new BMessage(SLIDER), 1, 5);
	mySlider->SetLimitLabels("low", "high");
	mySlider->SetHashMarks(B_HASH_MARKS_BOTH);
	mySlider->SetHashMarkCount(5);
	mySlider->SetValue(myConfig->distance);
	mainView->AddChild(mySlider);
}

void bydpConfigure::SetupColourDialog(int colour) {

	myColour = colour;

	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", "Cancel", new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);
	BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok","OK", new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(OKButton);
	exampleText = new BStringView(BRect(22,91,22+258,91+19),"example","Example text.", B_FOLLOW_LEFT, B_WILL_DRAW);
	exampleText->SetAlignment(B_ALIGN_CENTER);
	mainView->AddChild(exampleText);
	myCColor = new BColorControl(BPoint(22,20),B_CELLS_32x8, 8.0, "ccontrol", new BMessage(CCOLOR_MSG), false);
	switch(colour) {
		case 0:
			myCColor->SetValue(myConfig->colour);
			break;
		case 1:
			myCColor->SetValue(myConfig->colour0);
			break;
		case 2:
			myCColor->SetValue(myConfig->colour1);
			break;
		case 3:
			myCColor->SetValue(myConfig->colour2);
		default:
			break;
	}
	mainView->AddChild(myCColor);
	OKButton->MakeFocus(true);
	UpdateExampleColour();
}

/*void bydpConfigure::SetupSQLDialog(void) {

	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", "Cancel", new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);

	// this is ripped from engine_sq2
	sqlite *dbData;
	char *dbErrMsg;
	BString dat;
	BFile fData;

	dat = myConfig->topPath;
	dat.Append("/");
	dat += "bsapdict.sq2";

	// fData test wouldn't be necessary if sqlite_open worked as advertised or I don't understand it
	int fResult = fData.SetTo(dat.String(), B_READ_ONLY);
	dbData = sqlite_open(dat.String(), 0444, &dbErrMsg);
	if ((dbData==0)||(dbErrMsg!=0)||(fResult!=B_OK)) {
		// clean up after sqlite_open - file didn't exist before it, but it exists now
		unlink(dat.String());
		BStringView *sqlMessageText = new BStringView(BRect(22,22,22+258,22+19),"example","Could not open data file.", B_FOLLOW_LEFT, B_WILL_DRAW);
		sqlMessageText->SetAlignment(B_ALIGN_CENTER);
		mainView->AddChild(sqlMessageText);
	} else {
		// suck SQL dictdata
		char **result;
		int nRows, nCols;
		sqlite_get_table(dbData, "SELECT id, name FROM dictionaries ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
//		printf("got: %ix%i\n",nRows,nCols);
		if (nRows<1) {
		// show error
			BStringView *sqlMessageText = new BStringView(BRect(22,22,22+258,22+19),"example","There are no dictionaries defined in database.", B_FOLLOW_LEFT, B_WILL_DRAW);
			sqlMessageText->SetAlignment(B_ALIGN_CENTER);
			mainView->AddChild(sqlMessageText);		
		} else {
			// add into two popupmenus
			int i, j, id, type;
			BPopUpMenu *dictMenu;
			BMenuField *menuField;
			BMenuItem *menuItem;
			BMessage *msg;
			BString tmp, tmp2;
			for (j=0;j<=1;j++) {
				dictMenu = new BPopUpMenu("unknown");
				for (i=1;i<=nRows;i++) {
					type = (j==0) ? CHOOSEDICT0 : CHOOSEDICT1;
					id = strtol(result[i*2],NULL,10);
					msg = new BMessage(type);
					msg->AddInt32("_dictid",id);
					menuItem = new BMenuItem(result[i*2+1], msg);
					menuItem->SetMarked(id==myConfig->sqlDictionary[j]);
					dictMenu->AddItem(menuItem);
//					printf("added %s,%i\n",result[i*2+1],id);
				}
				tmp = "Dictionary "; tmp2 = "menuField";
				tmp << j+1; tmp2 << j;
				menuField = new BMenuField(BRect(10,10+30*j,300,10+30*j+22), tmp2.String(), tmp.String(), dictMenu, B_FOLLOW_LEFT, B_WILL_DRAW);
				mainView->AddChild(menuField);
			}
			BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok","OK", new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
			mainView->AddChild(OKButton);
		}
		sqlite_free_table(result);
	}
	mySqlDict[0] = myConfig->sqlDictionary[0];
	mySqlDict[1] = myConfig->sqlDictionary[1];
} */

void bydpConfigure::SetConfig(bydpConfig *config) {
	myConfig = config;
}

void bydpConfigure::CopyNewColours(rgb_color *to) {
	to->red = myCColor->ValueAsColor().red;
	to->green = myCColor->ValueAsColor().green;
	to->blue = myCColor->ValueAsColor().blue;
}

void bydpConfigure::UpdateExampleColour(void) {
	exampleText->SetHighColor(myCColor->ValueAsColor());
	exampleText->Invalidate();
}

void bydpConfigure::ConfigUpdate(void) {
//	printf("update config in dialog\n");
	switch (dialogType) {
/*		case BYDPCONF_SQL:
//			printf("update sql\n");
			myConfig->sqlDictionary[0] = mySqlDict[0];
			myConfig->sqlDictionary[1] = mySqlDict[1];
			break; */
		case BYDPCONF_DISTANCE:
//			printf("update dist\n");
			myConfig->distance = mySlider->Value();
			break;
		case BYDPCONF_COLOUR:
		default:
//		printf("update colour %i\n",myColour);
			switch(myColour) {
				case 0:
					CopyNewColours(&myConfig->colour);
					break;
				case 1:
					CopyNewColours(&myConfig->colour0);
					break;
				case 2:
					CopyNewColours(&myConfig->colour1);
					break;
				case 3:
					CopyNewColours(&myConfig->colour2);
					break;
				default:
					break;
			}
			break;
	}
	myConfig->save();
}

void bydpConfigure::MessageReceived(BMessage * Message) {
	switch(Message->what)
	{
		case BUTTON_OK:
//			printf("ok\n");
			ConfigUpdate();
			switch (dialogType) {
/*				case BYDPCONF_SQL:
					myHandler->Looper()->PostMessage(new BMessage(MSG_SQLTABLESUPDATE));
					break; */
				case BYDPCONF_DISTANCE:
					myHandler->Looper()->PostMessage(new BMessage(MSG_FUZZYUPDATE));
					break;
				case BYDPCONF_COLOUR:
				default:
					myHandler->Looper()->PostMessage(new BMessage(MSG_COLOURUPDATE));
					break;
			}
			QuitRequested();
			break;
		case BUTTON_CANCEL:
//			printf("cancel\n");
			QuitRequested();
			break;
		case CCOLOR_MSG:
//			printf("color msg\n");
			UpdateExampleColour();
			break;
		case SLIDER:
			break;
/*		case CHOOSEDICT0:
			{
				int32 id = 0;
				if (Message->FindInt32("_dictid",&id) == B_OK)
					mySqlDict[0] = id;
			}
			break;
		case CHOOSEDICT1:
			{
				int32 id = 0;
				if (Message->FindInt32("_dictid",&id) == B_OK)
					mySqlDict[1] = id;
			}
			break; */
		default:
		  BWindow::MessageReceived(Message);
		  break;
	}
}

bool bydpConfigure::QuitRequested() {
	Hide();
	return BWindow::QuitRequested();
}
