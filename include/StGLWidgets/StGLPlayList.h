/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLPlayList_h_
#define __StGLPlayList_h_

#include <StGLWidgets/StGLMenu.h>

#include <StGL/StPlayList.h>

/**
 * PlayList widget.
 */
class StGLPlayList : public StGLWidget {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLPlayList(StGLWidget*                 theParent,
                              const StHandle<StPlayList>& theList);

    ST_CPPEXPORT virtual ~StGLPlayList();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo);
    ST_CPPEXPORT virtual void stglResize();

        public:  //! @name Signals

    struct {
        /**
         * Emit new item signal.
         */
        StSignal<void (void )> onOpenItem;
    } signals;

        private: //! @name callback slots

    ST_LOCAL void updateList();
    ST_LOCAL void doResetList();
    ST_LOCAL void doChangeItem(const size_t );
    ST_LOCAL void doItemClick(const size_t );
    ST_LOCAL void doMouseClick  (const int theBtnId);
    ST_LOCAL void doMouseUnclick(const int theBtnId);
    ST_LOCAL void resizeWidth();

        protected:

    ST_LOCAL StGLMenuItem* addItem();
    ST_LOCAL void stglDrawScrollBar(unsigned int theView);

        private:

    StGLMenu*            myMenu;         //!< menu with items

    StGLVertexBuffer     myBarVertBuf;   //!< vertices buffer
    StGLVec4             myBarColor;     //!< color of scroll bar

    StHandle<StPlayList> myList;         //!< handle to playlist
    size_t               myFromId;       //!< id in playlist of first item displayed on screen
    int                  myItemsNb;      //!< number of items displayed on screen
    volatile bool        myToResetList;  //!< playlist has been reseted
    volatile bool        myToUpdateList; //!< playlist has been changed

    bool       myIsLeftClick; //!< flag to perform dragging - some item has been clicked (but not yet unclicked)
    StPointD_t myClickPntZo;  //!< remembered mouse click position
    StTimer    myDragTimer;   //!< timer between dragging animation
    int64_t    myDragDone;    //!< the number of dragged items (sign means direction)
    double     myFlingAccel;  //!< (positive) fling acceleration
    StPointD_t myFlingPntZo;  //!< remembered mouse unclick position
    StTimer    myFlingTimer;  //!< timer for dragging inertia
    double     myFlingYSpeed; //!< the dragging velocity for inertial scrolling

};

#endif // __StGLPlayList_h_
