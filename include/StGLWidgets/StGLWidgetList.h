/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLWidgetList_h_
#define __StGLWidgetList_h_

class StGLWidget;

/**
 * Double-linked list.
 */
class StGLWidgetList {

        public:

    ST_CPPEXPORT StGLWidgetList();
    ST_CPPEXPORT ~StGLWidgetList();

    inline StGLWidget* getStart() {
        return myFirst;
    }

    ST_CPPEXPORT StGLWidget* getLast();

    ST_CPPEXPORT StGLWidgetList* add(StGLWidget* theAddItem);
    ST_CPPEXPORT StGLWidgetList* remove(StGLWidget* theRemItem);

    /**
     * Move widget to the topmost (last) position.
     */
    ST_CPPEXPORT void moveToTop(StGLWidget* theTopmost);

        private:

    StGLWidget* myFirst;

};

#endif //__StGLWidgetList_h_
