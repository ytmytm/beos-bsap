
#include <stdio.h>
#include <string.h>
#include <Application.h>
#include "bydplistview.h"

#define TABLE_UTF8 { \
		"~", \
		".", ".", "<o>", "<3>", ".", "<|>", "<E>", "<^>", "<e>", \
		"θ", "<i>", "<a>", ".", ":", "´", ".", "ŋ", \
		".", ".", ".", ".", ".", ".", "ð", "æ", \
		".", ".", ".", ".", ".", ".", ".", ".", \
		"Ą", ".", "Ł", "¤", "Ľ", "Ś", "§", "¨", \
		"Š", "Ş", "Ť", "Ź", "­", "Ž", "Ż", "°", \
		"ą", ".", "ł", "´", "ľ", "ś", ".", "¸", \
		"ą", "ş", "ť", "ź", ".", "ž", "ż", "Ŕ", \
		"Á", "Â", "Ă", "Ä", "Ľ", "Ć", "Ç", "Č", \
		"É", "Ę", "Ë", "Ĕ", "Í", "Î", "Ď", "Ð", \
		"Ń", "Ň", "Ó", "Ô", "Õ", "Ö", "×", "Ř", \
		"Ù", "Ú", "Û", "Ü", "Ý", "Ţ", "ß", "ŕ", \
		"á", "â", "ã", "ä", "ľ", "ć", "ç", "č", \
		"é", "ę", "ë", "ì", "í", "î", "ď", "đ", \
		"ń", "ň", "ó", "ô", "õ", "ö", "÷", "ř", \
		"ù", "ú", "û", "ü", "ý", "ţ", "." }

bydpListView::bydpListView(const char *name, BHandler *handler) : BListView(
		BRect(10,60,200,400),
		name,
		B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM) {

	valid = false;
	topIndex = -1;
//	printf("konstruktor %s\n",name);
	myHandler = handler;
	myBar = NULL;
	NewSize();
}

bydpListView::~bydpListView() {

}

void bydpListView::SetScrollBar(bydpScrollBar *newBar) {
// teoretycznie niepotrzebne - wystarczy wziac dziecko...
	myBar = newBar;
}

//void bydpListView::MessageReceived(BMessage *Message) {
//
//	switch(Message->what) {
//	/// XXX te komunikaty nigdy tu nie docieraja
//		case MSG_SCROLL:
//			printf("lv scroll value changed\n");
//			if (myBar->Value() != topIndex)
//				ListRefresh(myBar->Value());
//			break;
//		default:
//			BListView::MessageReceived(Message);
//			break;
//		}
//}

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

//void bydpListView::MakeFocus(bool focused) {
//	printf("in makefocus ");
//	if (focused)
//		printf(" focused\n");
//	else
//		printf(" unfocused\n");
//	BListView::MakeFocus(focused);
//}

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
		((BStringItem*)this->ItemAt(i))->SetText(ConvertToUtf(words[i+topIndex]));
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
	((BStringItem*)this->ItemAt(0))->SetText(ConvertToUtf(words[topIndex]));
	this->Invalidate();
}

void bydpListView::List1Down(void) {
	int i;
	if (topIndex+visible == wordCount)
		return;
	topIndex++;
	for (i=0; i<visible-1; i++)
		((BStringItem*)this->ItemAt(i))->SetText(((BStringItem*)this->ItemAt(i+1))->Text());
	((BStringItem*)this->ItemAt(visible-1))->SetText(ConvertToUtf(words[topIndex+visible-1]));
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

/////////////////////
// utf8 <-> cp1250 convertion stuff below
//

const char *utf8_table[] = TABLE_UTF8;
const char upper_cp[] = "A�BC�DE�FGHIJKL�MN�O�PQRS�TUVWXYZ��";
const char lower_cp[] = "a�bc�de�fghijkl�mn�o�pqrs�tuvwxyz��";

char tolower(const char c) {
    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

char *ConvertToUtf(const char *line) {
	static char buf[1024];
	static char letter[2] = "\0";
	unsigned char *inp;
	memset(buf, 0, sizeof(buf));

	inp = (unsigned char *)line;
	for (; *inp; inp++) {
		if (*inp > 126) {
			strncat(buf, utf8_table[*inp - 127], sizeof(buf) - strlen(buf) - 1);
		} else {
			letter[0] = *inp;
			strncat(buf, letter, sizeof(buf) - strlen(buf) - 1);
		}
	}
	return buf;
}

char *ConvertFromUtf(const char *input) {
	static char buf[1024];
	unsigned char *inp, *inp2;
	memset(buf, 0, sizeof(buf));
	int i,k;
	char a,b;
	bool notyet;

	k=0;
	inp = (unsigned char*)input;
	inp2 = inp; inp2++;
	for (; *inp; inp++, inp2++) {
		a = *inp;
		b = *inp2;
		i=0;
		notyet=true;
		while ((i<129) && (notyet)) {
			if (a==utf8_table[i][0]) {
				if (utf8_table[i][1]!=0) {
					if (b==utf8_table[i][1]) {
						inp++;
						inp2++;
						notyet=false;
					}
				} else {
					notyet=false;
				}
			}
			i++;
		}
		if (notyet)
			buf[k]=a;
		else
			buf[k]=i+126;
		k++;
	}
	buf[k]='\0';
	return buf;
}
