/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StBrowserPlugin NPAPI plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StBrowserPlugin NPAPI plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StBrowserPlugin_H__
#define __StBrowserPlugin_H__

#include "NSPluginBase.h"

#include <StCore/StApplication.h>

class StApplication;
class StThread;

/**
 * Plugin implementation.
 */
class StBrowserPlugin : public NSPluginBase {

        public:

    // exports
    ST_LOCAL StBrowserPlugin(NSPluginCreateData* aCreateDataStruct);
    ST_LOCAL virtual ~StBrowserPlugin();
    ST_LOCAL virtual bool init(NPWindow* );
    ST_LOCAL virtual bool isInitialized();
    ST_LOCAL virtual NPError streamNew(NPMIMEType , NPStream* , NPBool , uint16_t* );
    ST_LOCAL virtual void streamAsFile(NPStream* , const char* );
#if(defined(__linux__) || defined(__linux))
    ST_LOCAL virtual NPError getValue(NPPVariable , void* );
#endif

    // locals
    ST_LOCAL const char* getVersion();
    ST_LOCAL void stWindowLoop();

        private:

    ST_LOCAL static void doLoadFullSize(void* thePlugin);
    ST_LOCAL void        doLoadFullSize();

        private:

    NPP                     nppInstance;
    StNativeWin_t           myParentWin;      //!< handle to native window for this ActiveX component
    StHandle<StThread>      myThread;         //!< dedicated thread for this plugin instance
    StHandle<StApplication> myStApp;          //!< StCore application instance worked in dedicated thread
    StMutex                 myMutex;          //!< mutex for thread-safety
    StOpenInfo              myOpenInfo;       //!< info for file to load
    StString                myPreviewUrl;     //!< url  to preview   image
    StString                myPreviewUrlUtf8; //!< url  to preview   image (with decoded Unicode symbols)
    StString                myFullUrl;        //!< url  to full-size image
    StString                myPreviewPath;    //!< path to preview   image in local cache
    StString                myFullPath;       //!< path to full-size image in local cache
    StArrayList<StString>   myTmpFiles;
    bool                    myToLoadFull;     //!< flag indicates that plugin was switched into fullscreen and full-size image required
    volatile bool           myToQuit;         //!< flag to perform termination

};

#endif //__StBrowserPlugin_H__
