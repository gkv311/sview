/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StCADViewer_h_
#define __StCADViewer_h_

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <V3d_View.hxx>
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>

#include <StCore/StApplication.h>
#include <StGLMesh/StGLMesh.h>
#include <StGLStereo/StGLProjCamera.h>
#include <StSettings/StParam.h>
#include <StSettings/StTranslations.h>

//#include "StCADViewerGUI.h"

enum {
    ST_PROJ_ORTHO,  //!< orthogonal projection matrix
    ST_PROJ_PERSP,  //!< perspective projection matrix
    ST_PROJ_STEREO, //!< stereoscopic projection matrix
};

// forward declarations
class StSettings;
class StCADViewerGUI;
class StCADLoader;

/**
 * CAD Viewer application.
 */
class StCADViewer : public StApplication {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

        public: //!< interface methods' implementations

    /**
     * Constructor.
     */
    ST_CPPEXPORT StCADViewer(const StHandle<StResourceManager>& theResMgr,
                             const StNativeWin_t                theParentWin,
                             const StHandle<StOpenInfo>&        theOpenInfo);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StCADViewer();

    /**
     * Open application.
     */
    ST_CPPEXPORT virtual bool open() ST_ATTR_OVERRIDE;

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void beforeDraw() ST_ATTR_OVERRIDE;

    /**
     * Draw frame for requested view.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice() ST_ATTR_OVERRIDE;

        private: //! @name window events slots

    ST_LOCAL virtual void doPause    (const StPauseEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doResize   (const StSizeEvent&   theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyDown  (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyHold  (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doKeyUp    (const StKeyEvent&    theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doMouseDown(const StClickEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doMouseUp  (const StClickEvent&  theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doGesture  (const StGestureEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doScroll   (const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doFileDrop (const StDNDropEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void doNavigate (const StNavigEvent&  theEvent) ST_ATTR_OVERRIDE;

        public: //!< callback Slots

    /**
     * Should be called when file in loading state.
     */
    ST_LOCAL void doUpdateStateLoading();

    /**
     * Should be called when file was loaded.
     */
    ST_LOCAL void doUpdateStateLoaded(bool isSuccess);

    /**
     * Load FIRST file in the playlist.
     */
    ST_LOCAL void doListFirst(const size_t dummy = 0);

    /**
     * Load PREVIOUS file in the playlist.
     */
    ST_LOCAL void doListPrev(const size_t dummy = 0);

    /**
     * Load NEXT file in the playlist.
     */
    ST_LOCAL void doListNext(const size_t dummy = 0);

    /**
     * Load LAST file in the playlist.
     */
    ST_LOCAL void doListLast(const size_t dummy = 0);

    /**
     * Fit ALL.
     */
    ST_LOCAL void doFitAll(const size_t dummy = 0);

        public: //!< Properties

    struct {

        StHandle<StBoolParam>  isFullscreen;    //!< fullscreen state
        StHandle<StBoolParam>  ToShowFps;       //!< display FPS meter
        StHandle<StBoolParam>  toShowTrihedron; //!< show trihedron flag
        StHandle<StInt32Param> projectMode;     //!< projection mode
        int                    TargetFps;       //!< limit or not rendering FPS

    } params;

        private:

    /**
     * Initialize GUI.
     */
    ST_LOCAL bool createGui();

    /**
     * Initialize the OCCT viewer.
     */
    ST_LOCAL bool initOcctViewer();

    /**
     * Perform initialization.
     */
    ST_LOCAL bool init();

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();
    ST_LOCAL void saveGuiParams();
    ST_LOCAL void saveAllParams();

        private: //!< private callback Slots

    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doChangeProjection(const int32_t theProj);
    ST_LOCAL void doZoomIn (const double theValue);
    ST_LOCAL void doZoomOut(const double theValue);
    ST_LOCAL void doStereoZFocusCloser(const double theValue);
    ST_LOCAL void doStereoZFocusFarther(const double theValue);
    ST_LOCAL void doStereoIODDec(const double theValue);
    ST_LOCAL void doStereoIODInc(const double theValue);

        public:

    /**
     * Actions identifiers.
     */
    enum ActionId {
        Action_Fullscreen,
        Action_ShowFps,
        Action_FileInfo,
        Action_ListFirst,
        Action_ListLast,
        Action_ListPrev,
        Action_ListNext,
        //Action_DeleteFile,
        Action_FitAll,
        Action_ProjOrthogonal,
        Action_ProjPerspective,
        Action_ProjStereo,
        Action_ZoomIn,
        Action_ZoomOut,
        Action_StereoZFocusCloser,
        Action_StereoZFocusFarther,
        Action_StereoIODDec,
        Action_StereoIODInc,
    };

        private:

    StHandle<StGLContext>    myContext;
    StHandle<StSettings>     mySettings;      //!< current plugin local settings
    StHandle<StTranslations> myLangMap;       //!< translated strings map
    StHandle<StCADViewerGUI> myGUI;           //!< GUI elements
    StHandle<StCADLoader>    myCADLoader;     //!< dedicated threaded class for load/save operations
    StGLProjCamera           myProjection;    //!< projection setup
    StPointD_t               myPrevMouse;     //!< previous mouse click
    bool                     myIsLeftHold;
    bool                     myIsRightHold;
    bool                     myIsMiddleHold;
    bool                     myIsCtrlPressed;

    Handle(V3d_Viewer)             myViewer;     //!< main viewer
    Handle(V3d_View)               myView;       //!< main view
    Handle(AIS_InteractiveContext) myAisContext; //!< interactive context containing displayed objects

    Handle(XCAFApp_Application)    myXCafApp;    //!< OCAF application instance
    Handle(TDocStd_Document)       myXCafDoc;    //!< OCAF document    instance

        private:

    friend class StCADViewerGUI;

};

#endif //__StDiagnostics_h_
