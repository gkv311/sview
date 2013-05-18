/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMenuItem_h_
#define __StGLMenuItem_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLTextArea.h>
#include <StGL/StGLVertexBuffer.h>

class StGLMenu;
class StGLMenuProgram;

/**
 * Widget for item in the menu.
 */
class StGLMenuItem : public StGLTextArea {

        public:

    // recursively delete all submenus and that this item itself
    // should be used only for dynamic menu recreation
    ST_CPPEXPORT static void DeleteWithSubMenus(StGLMenuItem* theMenuItem);

    ST_CPPEXPORT StGLMenuItem(StGLMenu* theParent,
                              const int theLeft = 32,
                              const int theTop = 32,
                              StGLMenu* theSubMenu = NULL);

    ST_CPPEXPORT virtual ~StGLMenuItem();

    ST_CPPEXPORT virtual const StString& getClassName();
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo);
    ST_CPPEXPORT virtual void stglResize(const StRectI_t& theWinRectPx);
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int view);
    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemUnclicked);

    ST_CPPEXPORT const int computeTextWidth();

    inline StGLMenu* getParentMenu() {
        return (StGLMenu* )StGLWidget::getParent();
    }

    inline StGLMenu* getSubMenu() {
        return mySubMenu;
    }

    inline void setSubMenu(StGLMenu* theSubMenu) {
        mySubMenu = theSubMenu;
    }

    inline bool hasSubMenu() {
       return mySubMenu != NULL;
    }

    inline bool isSelected() const {
        return myIsItemSelected;
    }

    ST_CPPEXPORT void setSelected(bool theToSelect);

    ST_CPPEXPORT void setFocus(const bool theValue);

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on menu item click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onItemClick;
    } signals;

        private: //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int btnId);

        private: //! @name private methods

    ST_LOCAL void stglResize();

        private: //! @name private fields

    typedef enum tagState {
        PASSIVE,
        HIGHLIGHT,
        CLICKED,
    } State;

    StGLMenu*                  mySubMenu;        //!< child menu
    StGLShare<StGLMenuProgram> myProgram;        //!< GLSL program
    StGLVertexBuffer           myBackVertexBuf;  //!< background vertices
    StGLVec4                   myBackColor[3];   //!< background color per state
    bool                       myIsItemSelected; //!< navigation selection flag

};

#endif //__StGLMenuItem_h_
