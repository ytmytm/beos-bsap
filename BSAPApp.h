
#ifndef APP_H
#define APP_H

#include <Application.h>
#include <Window.h>

class BYdpApp : public BApplication {
private:
	BWindow *window;

public:
	BYdpApp();
	~BYdpApp();
};

#endif
