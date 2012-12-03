/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StCoreImpl_h_
#define __StCoreImpl_h_

#include <StCore/StCore.h>
#include <StCore/StDrawer.h>
#include <StCore/StWindowInterface.h>

// Exported class-methods wpappers in StCore library (here!)
ST_EXPORT StWindowInterface* StWindow_new();
ST_EXPORT void StWindow_del(StWindowInterface* );

class ST_LOCAL StCoreImpl : public StRendererInterface {

        public: //! @name StRendererInterface implementation

    virtual StRendererInterface* getLibImpl() { return this; }
    StCoreImpl();
    virtual ~StCoreImpl();
    virtual StWindowInterface* getStWindow() { return myWindow; }
    virtual bool init(const StString& theRendererPath, const int& theDeviceId, const StNativeWin_t* );
    virtual bool open(const StOpenInfo& stOpenInfo);
    virtual void callback(StMessage_t* theMessages) {
        myWindow->callback(theMessages);
        if(!myDrawer.isNull()) {
            myDrawer->parseCallback(theMessages);
        }
    }
    virtual void stglDraw(unsigned int theViews) {
        if(!myDrawer.isNull()) {
            myDrawer->stglDraw(theViews);
        }
    }

        private:

    void closeDrawer();

        private:

    StWindowInterface* myWindow;     //!< real StWindow instaniated here, window created in Render plugin
    StHandle<StDrawer> myDrawer;     //!< current Drawer instance
    StString           myStCorePath; //!< main sView libraries path
    StString           myDrawerPath; //!< current Drawer library path

};

#endif //__StCoreImpl_h_
