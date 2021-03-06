#ifndef ITODAY_VIDEO_H
#define ITODAY_VIDEO_H

#include "tw_view.h"

enum VIDEO_SURFACE{
    SURFACE_FORGROUND,
    SURFACE_BACKGROUND,
    SURFACE_SIZE,
};

class OnVideoFrameReceive{
    public:
        virtual void onFrameReceive(char* buf, int width, int height) = 0;
        virtual int capture(string filename) = 0;
        virtual void onFacePosition(int left, int top, int right, int bottom) = 0;
};

class TWVideoView : public TWView, public OnVideoFrameReceive{
	
	protected:	
        MYBITMAP  mBmp;  

        HDC mDc;

        RECT mFaceRect;

        RECT mDestRect;
	
	public:
		TWVideoView(HWND hWnd, int zorder, int id, RECT rect);

		~TWVideoView();

        virtual void onFrameReceive(char* buf, int width, int height);

        virtual void onFacePosition(int left, int top, int right, int bottom);
		
		virtual void onDraw(HDC dc);

        virtual int capture(string filename);

};

#endif