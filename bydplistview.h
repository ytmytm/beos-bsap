
#ifndef _BYDPLISTVIEW
#define _BYDPLISTVIEW

	#include <ScrollBar.h>

	class bydpScrollBar : public BScrollBar {
		public:
			bydpScrollBar(BRect frame, const char *name, BHandler *handler);
			~bydpScrollBar();
			virtual void ValueChanged(float newValue);
			void BlockSignals(bool block);
		private:
			BHandler *myHandler;
			bool blockSig;
	};

	#include <ListView.h>

	class bydpListView : public BListView {
		public:
			bydpListView(const char *name, BHandler *handler);
			~bydpListView();
//			virtual void MessageReceived(BMessage *Message);
			virtual void KeyDown(const char *bytes, int32 numBytes);
//			virtual void MakeFocus(bool focused);
			void SetScrollBar(bydpScrollBar *newBar);
			void NewSize(void);
			void NewData(int howmany, char **data, int current);
			void ListRefresh(int start=-1, bool update=true);
			void ListScrolled(int value);

			int topIndex;
		private:
			BHandler *myHandler;
			char **words;
			int wordCount;
			int visible;
			bool valid;
			bydpScrollBar *myBar;

			void List1Up(void);
			void List1Down(void);
	};

	const uint32 MSG_LISTUP =	'LVUp';
	const uint32 MSG_LISTDOWN =	'LVDo';
	const uint32 MSG_SCROLL =	'SBSc';

    char tolower(const char c);
	char *ConvertToUtf(const char *line);
	char *ConvertFromUtf(const char *input);

#endif
