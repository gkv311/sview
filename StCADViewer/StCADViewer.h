/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#ifndef __StCADViewer_h_
#define __StCADViewer_h_

#include <StTemplates/StHandle.h>
#include <StCore/StDrawerInterface.h>
#include <StCore/StWindow.h>
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
 * Base Drawer class for CAD Viewer plugin.
 */
class ST_LOCAL StCADViewer : public StDrawerInterface {

        private:

    StHandle<StWindow>       myWin;           //!< pointer to StWindow, created by Output plugin
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
    bool                     myToQuit;        //!< application quit flag

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    StWindow* getStWindow() { return myWin.operator->(); }

        public: //!< interface methods' implementations

    StCADViewer();
    virtual ~StCADViewer();
    virtual StDrawerInterface* getLibImpl() { return this; }
    virtual bool init(StWindowInterface* inStWin);
    virtual bool open(const StOpenInfo& stOpenInfo);
    virtual void parseCallback(StMessage_t* stMessages);
    virtual void stglDraw(unsigned int view);

        public: //!< callback Slots

    /**
     * Should be called when file in loading state.
     */
    void doUpdateStateLoading();

    /**
     * Should be called when file was loaded.
     */
    void doUpdateStateLoaded(bool isSuccess);

    /**
     * Load FIRST file in the playlist.
     */
    void doListFirst(const size_t dummy = 0);

    /**
     * Load PREVIOUS file in the playlist.
     */
    void doListPrev(const size_t dummy = 0);

    /**
     * Load NEXT file in the playlist.
     */
    void doListNext(const size_t dummy = 0);

    /**
     * Load LAST file in the playlist.
     */
    void doListLast(const size_t dummy = 0);

    /**
     * Fit ALL.
     */
    void doFitALL(const size_t dummy = 0);

        public: //!< Properties

    struct {

        StHandle<StBoolParam>  isFullscreen;    //!< fullscreen state
        StHandle<StBoolParam>  toShowNormals;   //!< show normals flag
        StHandle<StBoolParam>  toShowTrihedron; //!< show trihedron flag
        StHandle<StBoolParam>  isLightTwoSides; //!< if on both sides of the triangle will be enlighted
        StHandle<StInt32Param> projectMode;     //!< projection mode
        StHandle<StInt32Param> fillMode;        //!< fill mode

    } params;

        private: //!< private callback Slots

    void doFullscreen(const bool theIsFullscreen);
    void doShowNormals(const bool toShow);
    void doChangeProjection(const int32_t theProj);

};

#endif //__StDiagnostics_h_
