#include "tw_list.h"
#include <algorithm>

#define LOG_TAG "TWList"
TWList::TWList(HWND hWnd,int zorder, int id, RECT rect,  LIST_STYLE style)
	: TWView(hWnd, zorder, id, rect){
	
	mBackground = NULL;
    mOnItemClickListener = NULL;
    mFirstVisablePos = 0;
    mStyle = style;
	
}

TWList::TWList(HWND hWnd, int zorder, int id, RECT rect,  LIST_STYLE style, const char* background)
	: TWView(hWnd, zorder, id, rect){

    mFirstVisablePos = 0;
    mOnItemClickListener = NULL;
    mStyle = style;
	if (background != NULL){
#ifdef  RESOURCE_MANAGER_SUPPORT
        mBackground = ResourceManager::loadImg(background);
#else
		mBackground = new BITMAP();
		
		LOGD("\n TWList load %s \n ", background);
		LoadBitmap (HDC_SCREEN, mBackground, background);
#endif
	}
	
}

TWList::~TWList(){
    
#ifndef  RESOURCE_MANAGER_SUPPORT
	if (mBackground != NULL){
		UnloadBitmap(mBackground);
		delete mBackground;
	}
#endif	
}

void TWList::onDraw(HDC dc){
	
	LISTVIEWS::iterator plist; 

    if (getVisable() != VISABLE)
        return;
	
	if (mBackground != NULL)
		FillBoxWithBitmap (dc, mRect.left, mRect.top, RECTW(mRect), RECTH(mRect), mBackground);
	
	for(plist = mChilds.begin(); plist != mChilds.end(); plist++)
		(*plist)->onDraw(dc);
}

void TWList::setOnItemClickListener(OnItemClickListener* listener){
    mOnItemClickListener = listener;
}

void TWList::resetScroll(){
    LISTVIEWS::iterator pBegin;
    LISTVIEWS::iterator pEnd; 

    if (mChilds.size() == 0)
    return;

    pBegin = mChilds.begin();
    pEnd = --mChilds.end();
    if (mStyle == VERTICAL){

        if (mRect.top <= (*pBegin)->getRect()->top
            && mRect.bottom >= (*pEnd)->getRect()->bottom)
            return ;//no view out of rect

        int offset = 0;

        if (mRect.top < (*pBegin)->getRect()->top - 5){
            offset = mRect.top - (*pBegin)->getRect()->top + 5;
        } else if (mRect.bottom > (*pEnd)->getRect()->bottom + 5) {
            offset = mRect.bottom - (*pEnd)->getRect()->bottom - 5;
        }

        for(LISTVIEWS::iterator plist = mChilds.begin(); plist != mChilds.end(); plist++){
            (*plist)->offset(0, offset);
        }
    } else {
        if (mRect.left <= (*pBegin)->getRect()->left
            && mRect.right >= (*pEnd)->getRect()->right)
            return ;//no view out of rect

        int offset = 0;

        if (mRect.left < (*pBegin)->getRect()->left - 5){
            offset = mRect.left - (*pBegin)->getRect()->left + 5;
        } else if (mRect.right > (*pEnd)->getRect()->right + 5) {
            offset = mRect.right - (*pEnd)->getRect()->right - 5;
        }

        for(LISTVIEWS::iterator plist = mChilds.begin(); plist != mChilds.end(); plist++){
            (*plist)->offset(offset, 0);
        }
    }

}

int TWList::onTouch(POINT pt, int action){

    static POINT last = pt;
    static int last_action = TOUCH_UP;

    if (PtInRect(&mRect, pt.x, pt.y)){

        switch (action){
            case TOUCH_UP:
               if (last_action == TOUCH_MOVE)
                    resetScroll();
            case TOUCH_DOWN:{
                LISTVIEWS::iterator plist; 
            	
            	for(plist = mChilds.begin(); plist != mChilds.end(); plist++){
            		
            		if ((*plist)->onTouch(pt, action) == SUCCESS){
            			
            			break;
            		}	
            	}	
            }break;
            case TOUCH_MOVE:{
                if (last_action != TOUCH_MOVE)
                    break;

                LISTVIEWS::iterator plist; 
            	
            	for(plist = mChilds.begin(); plist != mChilds.end(); plist++){
            		
            		if ((*plist)->onTouch(pt, action) == SUCCESS){
            			
            			break;
            		}	
            	}	
                
                if (mStyle == HORIZONTAL)
                    scroll(pt.x - last.x, 0);
                else
                    scroll(0, pt.y - last.y);
                }break;
            default:
                break;
        }

        last = pt;
        last_action = action;
        
        return SUCCESS;
    }

    return FAILED;
}

void TWList::setBackground(const char* background){

#ifdef  RESOURCE_MANAGER_SUPPORT
    mBackground = ResourceManager::loadImg(background);
#else

	if (mBackground != NULL){
		UnloadBitmap(mBackground);
		delete mBackground;
	}
	
	if (background != NULL){
		mBackground = new BITMAP();
		LoadBitmap (HDC_SCREEN, mBackground, background);
	}
#endif
}

int TWList::getFirstVisiblePosition(){
    return mFirstVisablePos;
}

int TWList::getLastVisiblePosition(){
    return mFirstVisablePos + getChildCount();
}

int TWList::getChildCount(){
    return mChilds.size();
}

void TWList::setAdapter(BaseAdapter* adapter){
    mAdapter = adapter;

    mAdapter->setListener(this);
}

int TWList::onClickListener(TWView* view){
    if (mOnItemClickListener != NULL)
        mOnItemClickListener->onItemClick(view, view->getExtra());

    return SUCCESS;
}

void TWList::setFirstVisiblePosition(int pos){
    mFirstVisablePos = pos;
}

bool TWList::canScroll(int x_offset, int y_offset){

    LISTVIEWS::iterator plistEnd; 
    LISTVIEWS::iterator plistBegin;

    plistBegin = mChilds.begin();
    plistEnd = --mChilds.end(); 

    LOGD("\n canScroll begin rect [%d, %d, %d %d] mFirstVisablePos %d\n", (*plistBegin)->getRect()->left, (*plistBegin)->getRect()->top,
                (*plistBegin)->getRect()->right,(*plistBegin)->getRect()->bottom, mFirstVisablePos);

    LOGD("\n canScroll end rect [%d, %d, %d %d] getLastVisiblePosition()%d mAdapter->getCount() %d\n", (*plistEnd)->getRect()->left, (*plistEnd)->getRect()->top,
                (*plistEnd)->getRect()->right,(*plistEnd)->getRect()->bottom, getLastVisiblePosition(), mAdapter->getCount());

    if (mStyle == HORIZONTAL){
        if (x_offset > 0){
        
           return mFirstVisablePos > 0 
               || (*plistBegin)->getRect()->left - 30 < mRect.left;
           
       } else if (x_offset < 0){
       
           return getLastVisiblePosition() < mAdapter->getCount() -1
               || (*plistEnd)->getRect()->right + 30 > mRect.right;
       }
   } else if (mStyle == VERTICAL){

       if (y_offset > 0){
        
           return mFirstVisablePos > 0
               || (*plistBegin)->getRect()->top - 30 < mRect.top;
           
       } else if (y_offset < 0){
       
           return getLastVisiblePosition() < mAdapter->getCount() -1
               || (*plistEnd)->getRect()->bottom + 30 > mRect.bottom;

       }

    }

    return false;
}


int TWList::onDataChange(){

    TWView* view = NULL;

    TWView* last = NULL;

    LISTVIEWS newViews;

    if (mAdapter == NULL)
        return 0;

    LOGD("\n TWList::onDataChange mFirstVisablePos %d\n ", mFirstVisablePos);
    
    for (int index = mFirstVisablePos; index < mAdapter->getCount(); index ++){
        
         view = mAdapter->getView(index, getChild(index - mFirstVisablePos));

         if (view == NULL){
            LOGE("\n getView is NULL\n ");
            break;
         } 

         view->setOnClickListener(this);

         if (last != NULL){
            if (mStyle == HORIZONTAL)
                view->offset(last->getRect()->right - view->getRect()->left, 0);                 
            else
                view->offset(0 , last->getRect()->bottom - view->getRect()->top);
         }else if (!DoesIntersect(&mRect, view->getRect())){//first is not in list rect
             if (mStyle == HORIZONTAL)
                view->offset(mRect.left - view->getRect()->left, 0);                 
            else
                view->offset(0 , mRect.top - view->getRect()->top);
         }

         if (!DoesIntersect(&mRect, view->getRect())){

            LISTVIEWS::iterator plist = find(mReservedChilds.begin(), mReservedChilds.end(), view);

            if (plist ==  mReservedChilds.end())
                mReservedChilds.push_back(view);
            //delete view;

            LOGD ("mFirstVisablePos %d, mChilds.size()%d index %d isCovered false \n", mFirstVisablePos, mChilds.size(), index);
            //release over views in mChilds
            /*for (int i = index - mFirstVisablePos + 1; i < mChilds.size() ; i++){
                view = getChild(i);
                plist = find(mReservedChilds.begin(), mReservedChilds.end(), view );

                if (plist ==  mReservedChilds.end()){
                    mReservedChilds.push_back(view);
                    printf (" mReservedChilds.push_back(view) mFirstVisablePos %d, mChilds.size()%d i %d  \n", mFirstVisablePos, mChilds.size(), i);
                }
            }*/
            
            break;                
         } 
         
         last = view;
         newViews.push_back(view);
         
    }  
    mChilds.clear();

    mChilds = newViews;
    

    return mChilds.size();
     
}

TWView* TWList::getChild(int index){
    
    if (index < mChilds.size()){
        
        LISTVIEWS::iterator plist = mChilds.begin(); 

        advance(plist, index);
        
        return *plist;
    } else if (!mReservedChilds.empty()){
    
        LISTVIEWS::iterator plist = mReservedChilds.begin();

        TWView* view = *plist;

        mReservedChilds.pop_front();
        return view;
    }

    return NULL;
}

void TWList::scroll(int x_offset, int y_offset){

    LISTVIEWS::iterator plist; 
    
    if (!canScroll(x_offset, y_offset)){
        LOGW("\n can't scroll x_offset %d, y_offset %d\n", x_offset, y_offset);
        return;
    }
    
	for(plist = mChilds.begin(); plist != mChilds.end(); plist++){
		(*plist)->offset(x_offset, y_offset);
	}

    LISTVIEWS::iterator plistEnd; 
    LISTVIEWS::iterator plistBegin;    

    //check head is out of list rect
    for(plistBegin = mChilds.begin(), plistEnd = --mChilds.end(); 
        plistBegin != plistEnd && !mChilds.empty() && (x_offset < 0 || y_offset < 0); 
        plistBegin = mChilds.begin(), plistEnd = --mChilds.end()){    
         
        if (DoesIntersect(&mRect, (*plistBegin)->getRect())){
            break;
        } else {
            // *plist out of list
            RECT* end = (*plistEnd)->getRect();
            RECT* begin = (*plistBegin)->getRect();

            if (mStyle == HORIZONTAL){
                (*plistBegin)->offset(end->right - begin->left, 0);
            } else {
                (*plistBegin)->offset(0, end->bottom - begin->top);
            }
            
            if (mFirstVisablePos + mChilds.size() <= mAdapter->getCount()){
                mFirstVisablePos ++;
                
                mChilds.push_back(*plistBegin);
                mChilds.pop_front();  
            } else {
                LOGD("\n move to reseved plistBegin\n");
                //delete *plistBegin;
                mReservedChilds.push_back(*plistBegin);
                mChilds.pop_front();  
            }           

        }
            
    }
    
    //check tail is out of list rect
    for(plistBegin = mChilds.begin(), plistEnd = --mChilds.end(); 
        plistBegin != plistEnd && (x_offset > 0 || y_offset > 0); 
        plistBegin = mChilds.begin(), plistEnd = --mChilds.end()){    
       
        if (DoesIntersect(&mRect, (*plistEnd)->getRect())){
            break;
        } else {
            // *plist out of list
            RECT* end = (*plistEnd)->getRect();
            RECT* begin = (*plistBegin)->getRect();

            if (mStyle == HORIZONTAL){
                (*plistEnd)->offset(begin->left - end->right, 0);                
            } else {                
                (*plistEnd)->offset(0, begin->top - end->bottom );
            }

          
            if (mFirstVisablePos > 0){
                mFirstVisablePos --;                
                mChilds.push_front(*plistEnd);
                mChilds.pop_back(); 
                
            } else {
                LOGD("\n move to reseved plistEnd\n");
                //delete *plistEnd;
                mReservedChilds.push_back(*plistEnd);
                mChilds.pop_back(); 
            }

        }
            
    }


    //check head is sub of list rect
    for(plistBegin = mChilds.begin(), plistEnd = --mChilds.end(); 
        plistBegin != plistEnd && !mChilds.empty() 
            && (x_offset > 0 || y_offset > 0) 
            && mFirstVisablePos > 0; 
        plistBegin = mChilds.begin(), plistEnd = --mChilds.end()){ 

        if (mStyle == HORIZONTAL){

            if (mRect.left > (*plistBegin)->getRect()->left){
                break;
            }
        } else {
            if (mRect.top > (*plistBegin)->getRect()->top){
                break;
            }
        }

        LOGD("\n fill new before head\n");

        if (mReservedChilds.empty()){
            TWView* view = mAdapter->getView(mFirstVisablePos, NULL);
            mReservedChilds.push_back(view);
        }

        plist = mReservedChilds.begin();

        RECT* src = (*plist)->getRect();
        RECT* begin = (*plistBegin)->getRect();

        if (mStyle == HORIZONTAL){
            (*plist)->offset(begin->left - src->right, 0);
        } else {
            (*plist)->offset(0, begin->top - src->bottom);
        }
        mFirstVisablePos --;
        mChilds.push_front(*plist);
        mReservedChilds.pop_front();

    }

    
    //check tail is sub of list rect
    for(plistBegin = mChilds.begin(), plistEnd = --mChilds.end(); 
        plistBegin != plistEnd && !mChilds.empty() && (x_offset < 0 || y_offset < 0); 
        plistBegin = mChilds.begin(), plistEnd = --mChilds.end()){ 

        if (mStyle == HORIZONTAL){

            if (mRect.right <= (*plistEnd)->getRect()->right){
                break;
            }
        } else {
            if (mRect.bottom <= (*plistEnd)->getRect()->bottom){
                break;
            }
        }

        LOGD("\n fill new after tail\n");
        if (mReservedChilds.empty()){
            TWView* view = mAdapter->getView(mFirstVisablePos, NULL);
            mReservedChilds.push_back(view);
        }

        plist = mReservedChilds.begin();

        RECT* src = (*plist)->getRect();
        RECT* end = (*plistEnd)->getRect();

        if (mStyle == HORIZONTAL){
            (*plist)->offset(end->right - src->left, 0);
        } else {
            (*plist)->offset(0, end->bottom - src->top);
        }

        mChilds.push_back(*plist);
        mReservedChilds.pop_front();

    }

    for(plist = mChilds.begin(); plist != mChilds.end(); plist++){
           LOGD("\n after all rect [%d, %d, %d %d]\n", (*plist)->getRect()->left, (*plist)->getRect()->top,
                (*plist)->getRect()->right,(*plist)->getRect()->bottom);
    }

    onDataChange();
}


