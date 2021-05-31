/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLWidgetList_h_
#define __StGLWidgetList_h_

class StGLWidget;

/**
 * Double-linked list for widgets.
 */
class StGLWidgetList {

        public:

    /**
     * Default constructor (creates empty list).
     */
    ST_CPPEXPORT StGLWidgetList();

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StGLWidgetList();

    /**
     * @return first child in the list
     */
    ST_LOCAL inline const StGLWidget* getStart() const {
        return myFirst;
    }

    /**
     * @return first child in the list
     */
    ST_LOCAL inline StGLWidget* getStart() {
        return myFirst;
    }

    /**
     * Notice that this method will trivially iterate list till the last element
     * (value is not cached).
     * @return last child in the list
     */
    ST_CPPEXPORT StGLWidget* getLast();

    /**
     * @return last child in the list
     */
    ST_LOCAL inline StGLWidget* getLast() const {
        return const_cast<StGLWidgetList* >(this)->getLast();
    }

    /**
     * Append new item to the end of list.
     * Notice that this method doesn't check for duplicates
     * - appended item should be newly created widget.
     * @param theAddItem new item to append
     * @return this
     */
    ST_CPPEXPORT StGLWidgetList* add(StGLWidget* theAddItem);

    /**
     * Remove specified item from this list.
     * @param theRemItem item to remove
     * @return this
     */
    ST_CPPEXPORT StGLWidgetList* remove(StGLWidget* theRemItem);

    /**
     * Move widget to the topmost (last) position.
     */
    ST_CPPEXPORT void moveToTop(StGLWidget* theTopmost);

        private:

    StGLWidget* myFirst; //!< the first item in the list

};

#endif // __StGLWidgetList_h_
