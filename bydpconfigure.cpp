
#include "bydpconfigure.h"
//#include <stdio.h>

#include <Button.h>
#include <StringView.h>

const uint32 BUTTON_OK = 'BuOK';
const uint32 BUTTON_CANCEL = 'BuCA';
const uint32 CCOLOR_MSG = 'ColM';

bydpConfigure::bydpConfigure(const char *title, void *par) : BWindow(
		BRect(62, 100, 62+370, 260),
		title,
		B_TITLED_WINDOW,
		B_NOT_RESIZABLE ) {

	parent = par;
	myColour = -1;

	mainView = new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0);

	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(mainView);
}

bydpConfigure::~bydpConfigure() {

}

void bydpConfigure::SetupColourDialog(int colour) {

	myColour = colour;

	BButton *CancelButton = new BButton(BRect(22,123,23+75,123+24), "cancel", "Anuluj", new BMessage(BUTTON_CANCEL), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(CancelButton);
	BButton *OKButton = new BButton(BRect(285,123,285+75,123+24),"ok","OK", new BMessage(BUTTON_OK), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(OKButton);
	exampleText = new BStringView(BRect(22,91,22+258,91+19),"example","Przykładowy tekst.", B_FOLLOW_LEFT, B_WILL_DRAW);
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
	if (myColour > -1) {
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
	}
	myConfig->save();
}

void bydpConfigure::MessageReceived(BMessage * Message)
{
	switch(Message->what)
	{
		case BUTTON_OK:
//			printf("ok\n");
			ConfigUpdate();
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
		default:
		  BWindow::MessageReceived(Message);
		  break;
	}
}

bool bydpConfigure::QuitRequested() {
///	((BYdpMainWindow*)parent)->ConfigUpdate();
	Hide();
	return BWindow::QuitRequested();
}
