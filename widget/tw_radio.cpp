#include "tw_radio.h"
#include <string.h>

#define LOG_TAG "TWRadioButton"

TWRadioButton::TWRadioButton(HWND hWnd, int zorder, int id, RECT rect, const string* path, int size)
	: TWButton(hWnd, zorder, id, rect, path, size){
    setOnClickListener(this);
}

TWRadioButton::TWRadioButton(HWND hWnd, int zorder, int id, RECT rect, PBITMAP* imges, int size)
   : TWButton(hWnd, zorder, id, rect, imges, size){
   setOnClickListener(this);
}


TWRadioButton::~TWRadioButton(){
	LOGD("\n TWRadioButton::~TWRadioButton() id %d\n ", mId);

}

void TWRadioButton::setChecked(bool check){
    setState(check ? TW_BUTTON_PRESSED : TW_BUTTON_NORMAL);
}

bool TWRadioButton::isChecked(){
    return getState() == TW_BUTTON_PRESSED;
}

int TWRadioButton::onTouch(POINT pt, int action){
	
	return TWView::onTouch(pt, action);
}

int TWRadioButton::onClickListener(TWView* view){
    setChecked(!isChecked());
}