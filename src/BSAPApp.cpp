
#include "BSAPApp.h"
#include "bydpmainwindow.h"
#include "globals.h"

int AppReturnValue(0);

BYdpApp::BYdpApp(void) : BApplication(APP_SIGNATURE) 
{
	BYdpMainWindow *myMainWindow = new BYdpMainWindow(APP_NAME);
	if (myMainWindow != NULL) {
		myMainWindow->Show();
	} else {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}

BYdpApp::~BYdpApp() {
	BYdpMainWindow* myMainWindow;
	if (myMainWindow != NULL)
		if (myMainWindow->LockWithTimeout(30000000) == B_OK)
			myMainWindow->Quit();
}

int main() {
	AppReturnValue = B_ERROR;
	BYdpApp *myApp = new BYdpApp();
	myApp->Run();
	return AppReturnValue;
}



