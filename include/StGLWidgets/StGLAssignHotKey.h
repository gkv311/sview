/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLAssignHotKey_h_
#define __StGLAssignHotKey_h_

#include <StGLWidgets/StGLMessageBox.h>
#include <StSlots/StAction.h>

/**
 * Simple widget to assign new hot key.
 */
class StGLAssignHotKey : public StGLMessageBox {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLAssignHotKey(StGLRootWidget*           theParent,
                                  const StHandle<StAction>& theAction,
                                  const int                 theHKeyIndex);

    /**
     * Initialize the widget, should be called once.
     */
    ST_CPPEXPORT void create();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLAssignHotKey();

    /**
     * Unassign current hot key from specified action.
     */
    ST_CPPEXPORT void unsetHotKey(StHandle<StAction>& theAction);

    /**
     * Assign new hot key.
     */
    ST_CPPEXPORT void doSave(const size_t );

    /**
     * Setup default value to the action.
     */
    ST_CPPEXPORT void doReset(const size_t );

    /**
     *
     */
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;

        protected:

    /**
     * Mandatory method to retrieve action associated with the key.
     */
    virtual StHandle<StAction> getActionForKey(unsigned int theHKey) const = 0;

        private:

    /**
     * Update text.
     */
    ST_LOCAL void updateText();

        protected:

    StString           myTitleFrmt;       //!< "Assign new Hot Key for\n{0}"
    StString           myConflictFrmt;    //!< "Conflicts with: {0}"
    StString           myAssignLab;       //!< "Assign"  key text
    StString           myDefaultLab;      //!< "Default" key text
    StString           myCancelLab;       //!< "Cancel"  key text

    StHandle<StAction> myAction;          //!< action to modify
    StHandle<StAction> myConflictAction;  //!< existing action with the same hot-key
    StGLTextArea*      myHKeyLabel;       //!< text label with current hot-key
    StGLTextArea*      myConflictLabel;   //!< text label with name of conflicting action
    int                myHKeyIndex;       //!< the hot-key index within action
    unsigned int       myKeyFlags;        //!< currently entered hot-key

};

#endif // __StGLAssignHotKey_h_
