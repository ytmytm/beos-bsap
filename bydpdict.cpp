
#include "bydpdict.h"
#include "globals.h"

BYdpApp::BYdpApp() : BApplication(APP_SIGNATURE) {
	myMainWindow = new BYdpMainWindow("BSAP");
	if (myMainWindow != NULL) {
		myMainWindow->Show();
	} else {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}

BYdpApp::~BYdpApp() {
	if (myMainWindow != NULL)
		if (myMainWindow->LockWithTimeout(30000000) == B_OK)
			myMainWindow->Quit();
}

void BYdpApp::ReadyToRun() {
	// sth etc. just before running Run()
}
