/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifndef __StCADViewer_h_
#define __StCADViewer_h_

#include <StCore/StApplication.h>
#include <StGLMesh/StGLMesh.h>
#include <StGLStereo/StGLProjCamera.h>
#include <StSettings/StParam.h>

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
    ST_CPPEXPORT StCADViewer(const StNativeWin_t         theParentWin,
                             const StHandle<StOpenInfo>& theOpenInfo);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StCADViewer();

    /**
     * Open application.
     */
    ST_CPPEXPORT virtual bool open();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void beforeDraw();

    /**
     * Draw frame for requested view.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice();

        private: //! @name window events slots

    ST_LOCAL virtual void doResize   (const StSizeEvent&   theEvent);
    ST_LOCAL virtual void doKeyDown  (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doKeyHold  (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doKeyUp    (const StKeyEvent&    theEvent);
    ST_LOCAL virtual void doMouseDown(const StClickEvent&  theEvent);
    ST_LOCAL virtual void doMouseUp  (const StClickEvent&  theEvent);
    ST_LOCAL virtual void doFileDrop (const StDNDropEvent& theEvent);
    ST_LOCAL virtual void doNavigate (const StNavigEvent&  theEvent);

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
    ST_LOCAL void doFitALL(const size_t dummy = 0);

        public: //!< Properties

    struct {

        StHandle<StBoolParam>  isFullscreen;    //!< fullscreen state
        StHandle<StBoolParam>  ToShowFps;       //!< display FPS meter
        StHandle<StBoolParam>  toShowNormals;   //!< show normals flag
        StHandle<StBoolParam>  toShowTrihedron; //!< show trihedron flag
        StHandle<StBoolParam>  isLightTwoSides; //!< if on both sides of the triangle will be enlighted
        StHandle<StInt32Param> projectMode;     //!< projection mode
        StHandle<StInt32Param> fillMode;        //!< fill mode
        int                    TargetFps;       //!< limit or not rendering FPS

    } params;

        private:

    ST_LOCAL bool init();

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

        private: //!< private callback Slots

    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doShowNormals(const bool toShow);
    ST_LOCAL void doChangeProjection(const int32_t theProj);

        private:

    StHandle<StGLContext>    myContext;
    StHandle<StSettings>     mySettings;      //!< current plugin local settings
    StHandle<StCADViewerGUI> myGUI;           //!< GUI elements
    StHandle<StCADLoader>    myCADLoader;     //!< dedicated threaded class for load/save operations
    StHandle<StGLMesh>       myModel;         //!< current drawn CAD model
    StGLProjCamera           myProjection;    //!< projection setup
    StPointD_t               myPrevMouse;     //!< previous mouse click
    bool                     myIsLeftHold;
    bool                     myIsRightHold;
    bool                     myIsMiddleHold;
    bool                     myIsCtrlPressed;
    bool                     myIsCamIterative;

};

#endif //__StDiagnostics_h_
