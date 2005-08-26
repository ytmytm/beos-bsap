
#include "bydpconfigure.h"

#include <Button.h>
#include <StringView.h>
#include <SpLocaleApp.h>
#include "globals.h"
#include <stdio.h>

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

void bydpConfigure::SetupDialog(int type, int param = -1) {

	dialogType = type;

	switch (dialogType) {
		case BYDPCONF_SQL:
			SetupSQLDialog();
			break;
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
	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", tr("Cancel"), new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);
	BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok",tr("OK"), new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(OKButton);
	mySlider = new BSlider(BRect(44,20,22+285,20+100), "slider", tr("Fuzzy factor"), new BMessage(SLIDER), 1, 5);
	mySlider->SetLimitLabels(tr("low"), tr("high"));
	mySlider->SetHashMarks(B_HASH_MARKS_BOTH);
	mySlider->SetHashMarkCount(5);
	mySlider->SetValue(myConfig->distance);
	mainView->AddChild(mySlider);
}

void bydpConfigure::SetupColourDialog(int colour) {

	myColour = colour;

	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", tr("Cancel"), new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);
	BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok",tr("OK"), new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(OKButton);
	exampleText = new BStringView(BRect(22,91,22+258,91+19),"example",tr("Example text."), B_FOLLOW_LEFT, B_WILL_DRAW);
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

void bydpConfigure::SetupSQLDialog(void) {
	printf("in setup sql dialog\n");
}

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
		case BYDPCONF_SQL:
			printf("update sql\n");
			break;
		case BYDPCONF_DISTANCE:
			printf("update dist\n");
			myConfig->distance = mySlider->Value();
			break;
		default:
		printf("update colour %i\n",myColour);
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
				case BYDPCONF_SQL:
					printf("ok - sql\n");
					break;
				case BYDPCONF_DISTANCE:
					myHandler->Looper()->PostMessage(new BMessage(MSG_FUZZYUPDATE));
					break;
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
		default:
		  BWindow::MessageReceived(Message);
		  break;
	}
}

bool bydpConfigure::QuitRequested() {
	Hide();
	return BWindow::QuitRequested();
}
