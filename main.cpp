
#include <Application.h>

#include "bydpdict.h"

int AppReturnValue(0);

int main (void) {
	AppReturnValue = B_ERROR;
	BApplication* myApp = new BYdpApp();
	myApp->Run();
	return AppReturnValue;
}
