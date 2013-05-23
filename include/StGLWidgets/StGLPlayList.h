/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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
class StGLPlayList : public StGLMenu {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLPlayList(StGLWidget*                 theParent,
                              const StHandle<StPlayList>& theList);

    ST_CPPEXPORT virtual ~StGLPlayList();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);
    ST_CPPEXPORT virtual void stglResize(const StRectI_t& theWinRectPx);

        public:  //!< @name Signals

    struct {
        /**
         * Emit new item signal.
         */
        StSignal<void (void )> onOpenItem;
    } signals;

        private: //!< @name callback slots

    ST_LOCAL void updateList();
    ST_LOCAL void doResetList();
    ST_LOCAL void doItemClick(const size_t );
    ST_LOCAL void doMouseUnclick(const int theBtnId);
    ST_LOCAL void resizeWidth();

        private:

    StHandle<StPlayList> myList;         //!< handle to playlist
    size_t               myFromId;       //!< id in playlist of first item displayed on screen
    int                  myItemsNb;      //!< number of items displayed on screen
    volatile bool        myToUpdateList; //!< playlist has been changed

};

#endif // __StGLPlayList_h_
