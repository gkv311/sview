/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StApplicationImpl_h_
#define __StApplicationImpl_h_

#include <StCore/StApplicationInterface.h>
#include <StCore/StMessageList.h>
#include <StCore/StRendererPlugin.h>
#include <StLibrary.h>

class ST_LOCAL StApplicationImpl : public StApplicationInterface {

        public: //! @name StApplicationInterface implementation

    virtual StApplicationInterface* getLibImpl() { return this; }
    StApplicationImpl();
    virtual ~StApplicationImpl();
    virtual bool isOpened() { return myIsOpened; }
    virtual bool isFullscreen() { return myIsFullscreen; }
    virtual bool create(const StNativeWin_t* );
    virtual bool open(const StOpenInfo& );
    virtual void callback(StMessage_t* );

        private: //! @name auxiliary private methods

    bool chooseRendererPlugin();
    void parseProcessArguments();

        private: //! @name class fields

    StRendererPlugin   myRenderer;   //!< renderer plugin
    StString           myStCorePath; //!< StCore root folder
    StString           myParamRenderer;
    StString           myParamDeviceString;
    int                myParamDeviceInt;
    StOpenInfo         myDrawerInfo; // StDrawer plugin to open (may be empty for autodetection)
    StOpenInfo         myOpenFileInfo; // file to open
    StNativeWin_t      myNativeWinParent;
    StString           myShowHelpString;
    StMessage_t        myMessages[StMessageList::BUFFER_SIZE + 1];
    bool               myIsOpened;
    bool               myIsFullscreen;
    bool               myToQuit;

};

#endif //__StApplication_h_
