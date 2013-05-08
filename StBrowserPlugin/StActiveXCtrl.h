/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StActiveX plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StActiveX plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StActiveXCtrl_H__
#define __StActiveXCtrl_H__

#ifdef _MSC_VER

#include <afxctl.h> // MFC support for ActiveX Controls
#include <objsafe.h>

#include <StCore/StApplication.h>
#include <StThreads/StCondition.h>

class StApplication;
class StThread;

/**
 * ActiveX Control.
 */
class StActiveXCtrl : public COleControl {

    DECLARE_DYNCREATE(StActiveXCtrl)

        public:

    /**
     * Constructor.
     */
    StActiveXCtrl();

    virtual void OnDraw(CDC*         theDevCtx,
                        const CRect& theRectBounds,
                        const CRect& theRectInvalid);

    /**
     * Read properties from object.
     */
    virtual void DoPropExchange(CPropExchange* thePropEx);

    /**
     * Reset control to default state
     */
    virtual void OnResetState();

        protected:

    /**
     * Destructor.
     */
    virtual ~StActiveXCtrl();

    DECLARE_OLECREATE_EX(StActiveXCtrl) // Class factory and guid
    DECLARE_OLETYPELIB  (StActiveXCtrl) // GetTypeInfo
    DECLARE_OLECTLTYPE  (StActiveXCtrl) // Type name and misc status

    DECLARE_MESSAGE_MAP()   // Message maps
    DECLARE_DISPATCH_MAP()  // Dispatch maps
    DECLARE_EVENT_MAP()     // Event maps
    DECLARE_INTERFACE_MAP()

    /**
     * Exported method.
     * Just show about message box.
     */
    afx_msg void AboutBox();

    /**
     * Exported method.
     * Destroy StApplication instance.
     */
    afx_msg void Dispose();

    // IObjectSafety
    BEGIN_INTERFACE_PART(ObjectSafety, IObjectSafety)
    INIT_INTERFACE_PART(CActiveXViewer3dControlCtrl, ObjectSafety)
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID theRefIID, DWORD* theSupportedOptions, DWORD* theEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID theRefIID, DWORD  theOptionSetMask,    DWORD  theEnabledOptions);
    END_INTERFACE_PART_STATIC(ObjectSafety)

        public:

    /**
     * WM_Create handler creates control, set its window and assign URL to it.
     */
    afx_msg int OnCreate(LPCREATESTRUCT theCreateStruct);

    afx_msg void OnDestroy();

    /**
     * Working loop for StCore application (in dedicated thread).
     */
    void stWindowLoop();

        protected:

    /**
     * Load URL and return path to file on local system.
     */
    StString loadURL(const CString& theUrl);

    virtual LRESULT WindowProc(UINT theMsg, WPARAM theParamW, LPARAM theParamL);

        private:

    StNativeWin_t           myParentWin;      //!< handle to native window for this ActiveX component
    StHandle<StThread>      myThread;         //!< dedicated thread for this plugin instance
    StHandle<StApplication> myStApp;          //!< StCore application instance worked in dedicated thread
    StOpenInfo              myOpenInfo;       //!< info for file to load
    StCondition             myOpenEvent;      //!< event to be emitted when myOpenInfo configured with new file
    CString                 myUrlFull;        //!< url to full-size image
    CString                 myUrlPreview;     //!< url to preview   image
    CString                 myMimeType;       //!< MIME type
    bool                    myHasPreview;     //!< has dedicated URL for smaller preview image
    bool                    myToBlockMsg;     //!< MFC stuff
    volatile bool           myToQuit;         //!< flag to perform termination

};

#endif // _MSC_VER
#endif // __StActiveXCtrl_H__
