
#include "bydpdict.h"

int AppReturnValue(0);

int main (void) {
	AppReturnValue = B_ERROR;
	BYdpApp myApp;
	myApp.Run();
	return AppReturnValue;
}