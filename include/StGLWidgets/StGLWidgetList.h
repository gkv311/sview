/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StGLWidgetList {

        private:

    StGLWidget* myFirst;

        public:

    StGLWidgetList();
    ~StGLWidgetList();

    StGLWidget* getStart() {
        return myFirst;
    }

    StGLWidget* getLast();

    StGLWidgetList* add(StGLWidget* theAddItem);
    StGLWidgetList* remove(StGLWidget* theRemItem);

    /**
     * Move widget to the topmost (last) position.
     */
    void moveToTop(StGLWidget* theTopmost);

};

#endif //__StGLWidgetList_h_
