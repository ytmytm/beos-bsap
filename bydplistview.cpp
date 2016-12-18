
#include <stdio.h>

#include <Application.h>

#include "bydplistview.h"
#include "globals.h"

bydpListView::bydpListView(const char *name, BHandler *handler) : BListView(
		BRect(10,60,200,400),
		name,
		B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM) {

	valid = false;
	topIndex = -1;
//	printf("konstruktor %s\n",name);
	myHandler = handler;
	cvt = NULL;
	myBar = NULL;
	NewSize();
}

bydpListView::~bydpListView() {

}

void bydpListView::SetScrollBar(bydpScrollBar *newBar) {
// in theory this is unneeded - you just need to take child by hand...
	myBar = newBar;
}

void bydpListView::SetConverter(bydpConverter *converter) {
	cvt = converter;
}

void bydpListView::KeyDown(const char *bytes, int32 numBytes) {
	if (numBytes == 1) {
		switch (bytes[0]) {
			case B_HOME:
				ListRefresh(0);
				this->Select(0);
				myHandler->Looper()->PostMessage(new BMessage(MSG_LISTUP));
				break;
			case B_END:
				ListRefresh(wordCount-visible);
				this->Select(visible-1);
				myHandler->Looper()->PostMessage(new BMessage(MSG_LISTUP));
				break;
			case B_PAGE_UP:
				topIndex -= visible;
				ListRefresh();
				myHandler->Looper()->PostMessage(new BMessage(MSG_LISTUP));
				this->Select(0);
				break;
			case B_PAGE_DOWN:
				topIndex += visible;
				ListRefresh();
				myHandler->Looper()->PostMessage(new BMessage(MSG_LISTUP));
				this->Select(visible-1);
				break;
			case B_UP_ARROW:
//				printf("listupmsg\n");
				if (this->CurrentSelection(0) == 0) {
					myHandler->Looper()->PostMessage(new BMessage(MSG_LISTUP));
					List1Up();
				} else
					BListView::KeyDown(bytes,numBytes);
				break;
			case B_DOWN_ARROW:
//				printf("listdownmsg\n");
				if (this->CurrentSelection(0)+1 == this->CountItems()) {
					myHandler->Looper()->PostMessage(new BMessage(MSG_LISTDOWN));
					List1Down();
				} else
					BListView::KeyDown(bytes,numBytes);
				break;
			default:
				BListView::KeyDown(bytes,numBytes);
		}
	} else {
		BListView::KeyDown(bytes,numBytes);
	}
	myBar->BlockSignals(true);
	myBar->SetValue(topIndex+this->CurrentSelection(0));
	myBar->BlockSignals(false);
}

void bydpListView::NewData(int howmany, char **data, int current) {
//	printf("in newdata w/ %i,%i,%s\n",current,howmany,data[1]);
	words = data;
	wordCount = howmany;
	valid = true;

	NewSize();	// updates visible...

	ListRefresh();
	ListRefresh(current);	// forced refresh, required
	this->Select(current-topIndex);
}

void bydpListView::NewSize(void) {
	float dictSize;
	float itemSize;
	font_height myHeight;

	this->GetFontHeight(&myHeight);
	dictSize = this->Bounds().Height();
	itemSize = myHeight.leading+myHeight.ascent+myHeight.descent;
	visible = (int)(dictSize/itemSize-2);
	if (visible<1) visible = 1;
	if (visible>wordCount)
		visible=wordCount;
	if (valid) {
		if (visible<wordCount)
			myBar->SetRange(0.0,(float)wordCount-1);
		else
			myBar->SetRange(0.0, 0.0);
	}

//	printf("in newsize with %i\n",visible);

	int i;
	void *anItem;
	for (i=0; (anItem=this->ItemAt(i)); i++)
		delete anItem;
	this->MakeEmpty();

	for (i=0; i<visible; i++)
		this->AddItem(new BStringItem(""));

	ListRefresh();
}

void bydpListView::ListRefresh(int start=-1, bool update=true) {
	if (!valid)
		return;
//	printf("in refresh with %i\n",start);
	int i;
	if (start>=0) {
		if (topIndex == start)
			return;
		topIndex = start;
	}
	if ((topIndex+visible)>wordCount)
		topIndex = wordCount-visible;
	if (topIndex < 0)
		topIndex = 0;
	for (i=0; ((i<visible)&&(i+topIndex<wordCount)); i++)
		((BStringItem*)this->ItemAt(i))->SetText(cvt->ConvertToUtf(words[i+topIndex]));
	this->Invalidate();
	if (update)
		myBar->SetValue(topIndex);
}

void bydpListView::ListScrolled(int value) {
//	printf("got value:%i\n",value);
	ListRefresh(value,false);
	this->Select(value-topIndex);
}

void bydpListView::List1Up(void) {
	int i;

	if (topIndex == 0)
		return;
	topIndex--;
	for (i=visible-1; i!=0; i--)
		((BStringItem*)this->ItemAt(i))->SetText(((BStringItem*)this->ItemAt(i-1))->Text());
	((BStringItem*)this->ItemAt(0))->SetText(cvt->ConvertToUtf(words[topIndex]));
	this->Invalidate();
}

void bydpListView::List1Down(void) {
	int i;
	if (topIndex+visible == wordCount)
		return;
	topIndex++;
	for (i=0; i<visible-1; i++)
		((BStringItem*)this->ItemAt(i))->SetText(((BStringItem*)this->ItemAt(i+1))->Text());
	((BStringItem*)this->ItemAt(visible-1))->SetText(cvt->ConvertToUtf(words[topIndex+visible-1]));
	this->Invalidate();
}

bydpScrollBar::bydpScrollBar(BRect frame, const char *name, BHandler *handler) : BScrollBar (
	frame,
	name,
	NULL,
	0.0,
	100.0,
	B_VERTICAL) {

	myHandler = handler;
	blockSig = false;
	SetRange(0.0,1000.0);
}

bydpScrollBar::~bydpScrollBar() {

}

void bydpScrollBar::ValueChanged(float newValue) {
//	printf("value changed %f\n",newValue);
	if (!blockSig)
		myHandler->Looper()->PostMessage(new BMessage(MSG_SCROLL));
}

void bydpScrollBar::BlockSignals(bool block) {
	blockSig = block;
}
