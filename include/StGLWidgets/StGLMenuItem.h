/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StGLMenuItem : public StGLTextArea {

        public:

    // recursively delete all submenus and that this item itself
    // should be used only for dynamic menu recreation
    static void DeleteWithSubMenus(StGLMenuItem* theMenuItem);

    StGLMenuItem(StGLMenu* theParent,
                 const int theLeft = 32, const int theTop = 32,
                 StGLMenu* theSubMenu = NULL);

    virtual ~StGLMenuItem();

    virtual const StString& getClassName();
    virtual void stglUpdate(const StPointD_t& theCursorZo);
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int view);
    virtual bool tryClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemClicked);
    virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemUnclicked);

    const int computeTextWidth();

    StGLMenu* getParentMenu() {
        return (StGLMenu* )StGLWidget::getParent();
    }

    StGLMenu* getSubMenu() {
        return mySubMenu;
    }

    void setSubMenu(StGLMenu* theSubMenu) {
        mySubMenu = theSubMenu;
    }

    bool hasSubMenu() {
       return mySubMenu != NULL;
    }

    bool isSelected() const {
        return myIsItemSelected;
    }

    void setSelected(bool theToSelect);

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on menu item click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onItemClick;
    } signals;

        private: //! @name callback Slots (private overriders)

    void doMouseUnclick(const int btnId);

        private: //! @name private methods

    void stglResize();

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
