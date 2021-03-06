#include "tw_video.h"

#define ROTATE_DGREE 270

#define LOG_TAG "TWVideoView"

static unsigned int yuv2rgb_pixel(int y, int u, int v)
{    
    unsigned int rgb_24 = 0;    
    unsigned char *pixel = (unsigned char *)&rgb_24;    
    int r, g, b;
    #if 0    
    r = y + (1.370705 * (v - 128));    
    g = y - (0.698001 * (v - 128)) - (0.337633 * (u - 128));    
    b = y + (1.732446 * (u - 128));
    #else    
    r = y + ((351 * (v - 128))>>8);    
    g = y - ((179 * (v - 128))>>8) - ((86 * (u - 128))>>8);    
    b = y + ((444 * (u - 128))>>8);
    #endif    
    if(r > 255) r = 255;    
    if(g > 255) g = 255;    
    if(b > 255) b = 255;    
    if(r < 0) r = 0;    
    if(g < 0) g = 0;    
    if(b < 0) b = 0;    
    pixel[0] = b;    
    pixel[1] = g;    
    pixel[2] = r;    
    return rgb_24;
}


/* * YUV422 to RGB24 */
static int yuv2rgb(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in = 0, out = 0;
    unsigned int pixel_16;
    unsigned char pixel_24[3];
    unsigned int rgb_24;
    unsigned int y0, u, y1, v;
    for(in = 0; in < width * height * 2; in +=4) {
        pixel_16 = yuv[in + 3] << 24 
            |yuv[in + 2] << 16 
            |yuv[in + 1] << 8  
            |yuv[in + 0];
        #if 1        /* Output data mode:Y U Y V */        
        y0 = (pixel_16 & 0x000000ff);        
        u  = (pixel_16 & 0x0000ff00) >> 8;        
        y1 = (pixel_16 & 0x00ff0000) >> 16;        
        v  = (pixel_16 & 0xff000000) >> 24;
        #elif 0        /* Output data mode:Y V Y U */        
        y0 = (pixel_16 & 0x000000ff);        
        v  = (pixel_16 & 0x0000ff00) >> 8;        
        y1 = (pixel_16 & 0x00ff0000) >> 16;        
        u  = (pixel_16 & 0xff000000) >> 24;
        #else        /* Output data mode:U Y V Y */        
        u  = (pixel_16 & 0x000000ff);        
        y0 = (pixel_16 & 0x0000ff00) >> 8;        
        v  = (pixel_16 & 0x00ff0000) >> 16;        
        y1 = (pixel_16 & 0xff000000) >> 24;
        #endif        
        rgb_24 = yuv2rgb_pixel(y0, u, v);        
        pixel_24[0] = (rgb_24 & 0x000000ff);        
        pixel_24[1] = (rgb_24 & 0x0000ff00) >> 8;       
        pixel_24[2] = (rgb_24 & 0x00ff0000) >> 16;        
        rgb[out++] = pixel_24[0];        
        rgb[out++] = pixel_24[1];        
        rgb[out++] = pixel_24[2];        
        rgb_24 = yuv2rgb_pixel(y1, u, v);        
        pixel_24[0] = (rgb_24 & 0x000000ff);        
        pixel_24[1] = (rgb_24 & 0x0000ff00) >> 8;        
        pixel_24[2] = (rgb_24 & 0x00ff0000) >> 16;        
        rgb[out++] = pixel_24[0];        
        rgb[out++] = pixel_24[1];        
        rgb[out++] = pixel_24[2];    
    }    
    return 0;
}


#define DEST_AREA_PERCENT 5

TWVideoView::TWVideoView(HWND hWnd,int zorder, int id, RECT rect)
	: TWView(hWnd, zorder, id, rect){
	
	mDc = GetDC(hWnd);
    
    mBmp.flags = MYBMP_TYPE_BGR | MYBMP_FLOW_DOWN | MYBMP_RGBSIZE_3;
    mBmp.frames = 1;
    mBmp.depth = 24;
    mBmp.alpha = 0;
    mBmp.transparent = 0;
    mBmp.w = RECTW(rect);
    mBmp.h = RECTH(rect);
    mBmp.pitch = RECTW(rect) * 3;
    mBmp.size = RECTW(rect) * RECTH(rect) * 3;
    mBmp.bits = new BYTE[RECTW(rect) * RECTH(rect) * 3];

    memset(&mFaceRect, 0, sizeof(mFaceRect));

    mDestRect.left = mRect.left + RECTW(mRect) / DEST_AREA_PERCENT;
    mDestRect.top = mRect.top + RECTH(mRect) / DEST_AREA_PERCENT;
    mDestRect.right = mRect.right - RECTW(mRect) / DEST_AREA_PERCENT;
    mDestRect.bottom = mRect.bottom - RECTH(mRect) / DEST_AREA_PERCENT;
    
}


TWVideoView::~TWVideoView(){

    LOGD("\n TWVideoView::~TWVideoView() id %d\n ", mId);
	delete[] mBmp.bits;
	ReleaseDC(mDc);

}

void TWVideoView::onFrameReceive(char* buf, int width, int height){
    
    yuv2rgb((unsigned char*)buf, mBmp.bits, width, height);    

    onDraw(mDc);
}

void TWVideoView::onFacePosition(int left, int top, int right, int bottom){
    
    mFaceRect.top = top ;
    mFaceRect.left = left;
    mFaceRect.right = right;
    mFaceRect.bottom = bottom;

    OffsetRect(&mFaceRect, mRect.left, mRect.top);

    if (mFaceRect.top < mRect.top)
        mFaceRect.top = mRect.top;

    if (mFaceRect.left < mRect.left)
        mFaceRect.left = mRect.left;

    if (mFaceRect.right > mRect.right)
        mFaceRect.right = mRect.right;

#if (ROTATE_DGREE == 270 || ROTATE_DGREE == 90)
    if (mFaceRect.bottom > mRect.bottom - (RECTH(mRect) - RECTW(mRect))/ 2)
        mFaceRect.bottom = mRect.bottom - (RECTH(mRect) - RECTW(mRect))/ 2;

    if (mFaceRect.top < mRect.top + (RECTH(mRect) - RECTW(mRect))/ 2)
        mFaceRect.top = mRect.top + (RECTH(mRect) - RECTW(mRect))/ 2;
#else
    if (mFaceRect.bottom > mRect.bottom)
        mFaceRect.bottom = mRect.bottom;

    if (mFaceRect.top < mRect.top)
        mFaceRect.top = mRect.top;
#endif
}


void TWVideoView::onDraw(HDC dc){

    if (mVisable != VISABLE)
        return;    
	BITMAP bmp;
    ExpandMyBitmap (dc, &bmp, &mBmp, NULL, 0);

    RotateScaledBitmap(dc,&bmp, mRect.left, mRect.top, ROTATE_DGREE * 64/*270*/, RECTW(mRect), RECTH(mRect));

    Uint32 oldWidth = SetPenWidth(dc, 20);
    Uint32 oldColor = SetPenColor (dc,PIXEL_green);

    if (RECTW(mFaceRect) > 5){
        Rectangle(dc, mFaceRect.left, mFaceRect.top, mFaceRect.right, mFaceRect.bottom);
        Rectangle(dc, mFaceRect.left + 1, mFaceRect.top + 1, mFaceRect.right - 1, mFaceRect.bottom - 1);
    }
    SetPenColor (dc,PIXEL_red);
    Rectangle(dc, mDestRect.left, mDestRect.top, mDestRect.right, mDestRect.bottom);
    Rectangle(dc, mDestRect.left + 1, mDestRect.top + 1, mDestRect.right - 1, mDestRect.bottom - 1);

    SetPenColor(dc, oldColor);
    SetPenWidth(dc, oldWidth);
	
    UnloadBitmap(&bmp); 
    
}

int TWVideoView::capture(string filename){
    int ret;    
    char *name;
    int width = RECTW(mRect); 
    int height = RECTH(mRect); 

    BITMAP bmp;
    ExpandMyBitmap (mDc, &bmp, &mBmp, NULL, 0);

    ret = SaveBitmapToFile(mDc, &bmp, filename.c_str());

    UnloadBitmap(&bmp); 

    return ret;
}


