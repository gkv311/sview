/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#include "StCADModel.h"
#include "StCADViewer.h"
#include "StCADViewerGUI.h"
#include "StCADLoader.h"

#include <StGLMesh/StGLMesh.h>
#include <StGLMesh/StGLPrism.h>
#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StBndCameraBox.h>

#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLMsgStack.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

namespace {
    static const StString ST_PARAM_NORMALS   = "showNormals";
    static const StString ST_PARAM_TRIHEDRON = "showTrihedron";
    static const StString ST_PARAM_PROJMODE  = "projMode";
    static const StString ST_PARAM_FILLMODE  = "fillMode";
};

class ST_LOCAL StGLCamera {

        private:

    StGLVec3   myEye;        //!< eye position
    StGLVec3   myCenter;     //!< look to center
    StGLDir3   myUp;         //!< camera up direction
    StGLMatrix myViewMatrix; //!< model view matrix

        public:

    StGLCamera()
    : ///myEye(StGLVec3::DZ()),
      myEye(0.0f, 0.0f, 10.0f),
      myCenter(0.0f, 0.0f, 0.0f),
      myUp(StGLVec3::DY()),
      myViewMatrix() {
        updateMatrix();
    }

    ~StGLCamera() {}

    const StGLVec3& getEye() const {
        return myEye;
    }

    void setEye(const StGLVec3& theEye) {
        myEye = theEye;
        updateMatrix();
    }

    const StGLVec3& getCenter() const {
        return myCenter;
    }

    void setCenter(const StGLVec3& theCenter) {
        myCenter = theCenter;
        updateMatrix();
    }

    const StGLVec3& getUp() const {
        return myUp;
    }

    void setUp(const StGLVec3& theUp) {
        myUp = theUp;
        updateMatrix();
    }

    StGLDir3 getForward() const {
        return myCenter - myEye;
    }

    StGLDir3 getSide() const {
        // side = forward x up
        return StGLVec3::cross(getForward(), myUp);
    }

    void updateMatrix() {
        // recompute up as: up = side x forward
        //myUp = StGLVec3::cross(getSide(), getForward());
        myViewMatrix.initIdentity();
        myViewMatrix.lookAt(myEye, myCenter, myUp);
    }

    void rotateX(GLfloat theAngleDegrees) {
        StGLMatrix aRotMat;
        aRotMat.translate(myCenter);
        aRotMat.rotate(theAngleDegrees, myUp);
        aRotMat.translate(-myCenter);
        StGLVec4 aVec = aRotMat * StGLVec4(myEye, 1.0f);
        myEye = aVec.xyz();
        updateMatrix();
    }

    void rotateY(GLfloat theAngleDegrees) {
        StGLMatrix aRotMat;
        StGLVec3 anEyeDir = myCenter - myEye;
        GLfloat anEyeMagnitude = anEyeDir.modulus();
        anEyeDir.normalize();

        StGLVec3 aRotVec = StGLVec3::cross(myUp, anEyeDir);
        aRotVec.normalize();

        aRotMat.rotate(theAngleDegrees, aRotVec);
        StGLVec4 aVec = aRotMat * StGLVec4(myUp, 1.0f);
        myUp = aVec.xyz();
        aVec = aRotMat * StGLVec4(anEyeDir, 1.0f);
        myEye = aVec.xyz();
        myEye *= anEyeMagnitude;
        myEye = myCenter - myEye;
        updateMatrix();
    }

    void rotateZ(GLfloat theAngleDegrees) {
        StGLMatrix aRotMat;
        StGLVec3 aFrontVec = myEye - myCenter;
        aFrontVec.normalize();
        aRotMat.rotate(theAngleDegrees, aFrontVec);
        StGLVec4 aVec = aRotMat * StGLVec4(myUp, 1.0f);
        myUp = aVec.xyz();
        updateMatrix();
    }

    operator const StGLMatrix&() const { return myViewMatrix; }
    operator const GLfloat*() const { return myViewMatrix; }

};


/**
 * Special class to render the normals arrays.
 */
class ST_LOCAL StGLNormalsMesh : public StGLMesh {

        public:

    StGLNormalsMesh()
    : StGLMesh(GL_LINES) {}

    virtual ~StGLNormalsMesh() {}

    /**
     * Will extract vertices and normals from the mesh.
     */
    bool init(StGLContext&    theCtx,
              const StGLMesh& theMesh) {
        // reset current state
        clearRAM();
        clearVRAM(theCtx);

        // verify input mesh
        const StArrayList<StGLVec3>& aVertices = theMesh.getVertices();
        const StArrayList<StGLVec3>& aNormals  = theMesh.getNormals();
        if(aVertices.isEmpty() || aVertices.size() != aNormals.size()) {
            return false;
        }

        // compute normals scale factor (length)
        GLfloat aScaleFactor = 1.0f;
        StBndBox aBndBox;
        aBndBox.enlarge(aVertices);
        aScaleFactor = (aBndBox.getDX()+ aBndBox.getDY() + aBndBox.getDZ()) * 0.333f * 0.01f;

        myVertices.initList(aVertices.size() * 2);
        for(size_t aVertId = 0; aVertId < aVertices.size(); ++aVertId) {
            const StGLVec3& aNode   = aVertices.getValue(aVertId);
            const StGLVec3& aNormal = aNormals.getValue(aVertId);
            myVertices.add(aNode);
            myVertices.add(aNode + aNormal * aScaleFactor);
        }
        myVertexBuf.init(theCtx, myVertices);

        // no need to store this list in RAM
        //clearRAM();
        return true;
    }

};

static StGLCamera myCam; /// TODO move to...
static StGLPrism myPrism; /// debug bnd box
static StGLNormalsMesh* myNormalsMesh = NULL; /// TODO move to...

const StString StCADViewer::ST_DRAWER_PLUGIN_NAME = "StCADViewer";

StCADViewer::StCADViewer(const StNativeWin_t         theParentWin,
                         const StHandle<StOpenInfo>& theOpenInfo)
: StApplication(theParentWin, theOpenInfo),
  mySettings(new StSettings(ST_DRAWER_PLUGIN_NAME)),
  myIsLeftHold(false),
  myIsRightHold(false),
  myIsMiddleHold(false),
  myIsCtrlPressed(false),
  myIsCamIterative(false) {
    //
    myTitle = "sView - CAD Viewer";
    //
    params.isFullscreen = new StBoolParam(false);
    params.isFullscreen->signals.onChanged.connect(this, &StCADViewer::doFullscreen);
    params.toShowNormals = new StBoolParam(false);
    params.toShowNormals->signals.onChanged.connect(this, &StCADViewer::doShowNormals);
    params.toShowTrihedron = new StBoolParam(true);
    params.isLightTwoSides = new StBoolParam(true);
    params.projectMode = new StInt32Param(ST_PROJ_STEREO);
    params.projectMode->signals.onChanged.connect(this, &StCADViewer::doChangeProjection);
    params.fillMode = new StInt32Param(ST_FILL_MESH);

    myGUI = new StCADViewerGUI(this);

    addRenderer(new StOutAnaglyph(theParentWin));
    addRenderer(new StOutDual(theParentWin));
    addRenderer(new StOutIZ3D(theParentWin));
    addRenderer(new StOutInterlace(theParentWin));
    addRenderer(new StOutPageFlipExt(theParentWin));
    addRenderer(new StOutDistorted(theParentWin));
}

bool StCADViewer::resetDevice() {
    if(myGUI.isNull()
    || myCADLoader.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myMessages[0].uin = StMessageList::MSG_EXIT;
    myMessages[1].uin = StMessageList::MSG_NULL;
    myWindow->processEvents(myMessages);

    myCADLoader->doRelease();
    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StCADViewer::releaseDevice() {
    if(!myGUI.isNull()) {
        mySettings->saveParam(ST_PARAM_NORMALS,   params.toShowNormals);
        mySettings->saveParam(ST_PARAM_TRIHEDRON, params.toShowTrihedron);
        mySettings->saveParam(ST_PARAM_PROJMODE,  params.projectMode);
        mySettings->saveParam(ST_PARAM_FILLMODE,  params.fillMode);
    }

    if(!myContext.isNull()) {
        if(myNormalsMesh != NULL) {
            myNormalsMesh->release(*myContext);
            delete myNormalsMesh;
            myNormalsMesh = NULL;
        }
        if(!myModel.isNull()) {
            myModel->release(*myContext);
        }
        myPrism.release(*myContext);
    }

    // release GUI data and GL resources before closing the window
    myGUI.nullify();
    myContext.nullify();
}

StCADViewer::~StCADViewer() {
    releaseDevice();
    // wait working threads to quit and release resources
    myCADLoader.nullify();
}

bool StCADViewer::init() {
    const bool isReset = !myCADLoader.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError("CADViewer, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError("CADViewer, OpenGL2.0+ not available!");
        return false;
    }

    myWindow->setTargetFps(50.0);
    myWindow->setStereoOutput(params.projectMode->getValue() == ST_PROJ_STEREO);

    myGUI->setContext(myContext);
    if(!myGUI->stglInit()) {
        stError("CADViewer, GUI initialization failed!");
        return false;
    }
    myGUI->stglResize(myWindow->getPlacement());

    // create working threads
    if(!isReset) {
        myCADLoader = new StCADLoader(StHandle<StLangMap>::downcast(myGUI->myLangMap));
    }
    myCADLoader->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);

    // load settings
    mySettings->loadParam(ST_PARAM_NORMALS,   params.toShowNormals);
    mySettings->loadParam(ST_PARAM_TRIHEDRON, params.toShowTrihedron);
    mySettings->loadParam(ST_PARAM_PROJMODE,  params.projectMode);
    mySettings->loadParam(ST_PARAM_FILLMODE,  params.fillMode);
    return true;
}

bool StCADViewer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
        return false;
    }

    if(isReset) {
        myCADLoader->doLoadNext();
        return true;
    }

    //parseArguments(myOpenFileInfo.getArgumentsMap());
    const StMIME anOpenMIME = myOpenFileInfo->getMIME();
    if(myOpenFileInfo->getPath().isEmpty()) {
        // open drawer without files
        return true;
    }

    // clear playlist first
    myCADLoader->getPlayList().clear();

    if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myCADLoader->getPlayList().addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myCADLoader->getPlayList().open(myOpenFileInfo->getPath());
    }

    if(!myCADLoader->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myCADLoader->doLoadNext();
    }

    return true;
}

void StCADViewer::doResize(const StSizeEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    const StRectI_t aWinRect = myWindow->getPlacement();
    myGUI->stglResize(aWinRect);
    myProjection.resize(*myContext, aWinRect.width(), aWinRect.height());
}

void StCADViewer::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    myGUI->tryClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);

    if(theEvent.Button == ST_MOUSE_LEFT) {
        myIsLeftHold = true; ///
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
    } else if(theEvent.Button == ST_MOUSE_RIGHT) {
        myIsRightHold = true; ///
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
    } else if(theEvent.Button == ST_MOUSE_MIDDLE) {
        myIsMiddleHold = true; ///
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
    }
}

void StCADViewer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    switch(theEvent.Button) {
        case ST_MOUSE_LEFT: {
            myIsLeftHold = false;
            break;
        }
        case ST_MOUSE_RIGHT: {
            if(myIsRightHold && myIsCtrlPressed) {
                // rotate
                StPointD_t aPt = myWindow->getMousePos();
                StGLVec2 aFlatMove( 2.0f * GLfloat(aPt.x() - myPrevMouse.x()),
                                   -2.0f * GLfloat(aPt.y() - myPrevMouse.y()));

                myCam.rotateX(-aFlatMove.x() * 90.0f);
                myCam.rotateY(-aFlatMove.y() * 90.0f);
            }
            myIsRightHold = false;
            break;
        }
        case ST_MOUSE_MIDDLE: {
            if(!myIsCtrlPressed) {
                params.isFullscreen->reverse();
            }
            myIsMiddleHold = false;
            break;
        }
        case ST_MOUSE_SCROLL_V_UP: {
            if(myIsCtrlPressed) {
                myProjection.setZScreen(myProjection.getZScreen() + 1.1f);
            } else {
                myProjection.setZoom(myProjection.getZoom() * 0.9f);
            }
            break;
        }
        case ST_MOUSE_SCROLL_V_DOWN: {
            if(myIsCtrlPressed) {
                myProjection.setZScreen(myProjection.getZScreen() - 1.1f);
            } else {
                myProjection.setZoom(myProjection.getZoom() * 1.1f);
            }
            break;
        }
        default: break;
    }
    myGUI->tryUnClick(StPointD_t(theEvent.PointX, theEvent.PointY), theEvent.Button);
}

void StCADViewer::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    switch(theEvent.VKey) {
        case ST_VK_ESCAPE:
            StApplication::exit(0);
            return;
        case ST_VK_RETURN:
            params.isFullscreen->reverse();
            return;

        // switch projection matrix
        case ST_VK_M:
            params.projectMode->setValue(ST_PROJ_PERSP);
            return;
        case ST_VK_S:
            params.projectMode->setValue(ST_PROJ_STEREO);
            return;
        case ST_VK_O:
            params.projectMode->setValue(ST_PROJ_ORTHO);
            return;

        // separation
        case ST_VK_MULTIPLY: {
            if(theEvent.Flags == ST_VF_NONE) {
                myProjection.setIOD(myProjection.getIOD() + 0.1f);
                ST_DEBUG_LOG("Sep. inc, " + myProjection.toString());
            }
            return;
        }
        case ST_VK_DIVIDE:
            if(theEvent.Flags == ST_VF_NONE) {
                myProjection.setIOD(myProjection.getIOD() - 0.1f);
                ST_DEBUG_LOG("Sep. dec, " + myProjection.toString());
            }
            return;

        case ST_VK_C:
            myIsCamIterative = !myIsCamIterative;
            ST_DEBUG_LOG("Iterative camera " + (myIsCamIterative ? "ON" : "OFF"));
            return;

        case ST_VK_LEFT:
            myCam.rotateX(-1.0f);
            return;
        case ST_VK_RIGHT:
            myCam.rotateX(1.0f);
            return;
        case ST_VK_UP:
            myCam.rotateY(-1.0f);
            return;
        case ST_VK_DOWN:
            myCam.rotateY(1.0f);
            return;
        case ST_VK_Q:
            myCam.rotateZ(-1.0f);
            return;
        case ST_VK_W:
            myCam.rotateZ(1.0f);
            return;

        // call fit all
        case ST_VK_F:
            doFitALL();
            return;

        // show normals
        case ST_VK_N:
            params.toShowNormals->reverse();
            return;

        // playlist navigation
        case ST_VK_PRIOR:
            doListPrev();
            return;
        case ST_VK_NEXT:
            doListNext();
            return;
        case ST_VK_HOME:
            doListFirst();
            return;
        case ST_VK_END:
            doListLast();
            return;

        // shading mode
        case ST_VK_1:
            params.fillMode->setValue(ST_FILL_MESH);
            return;
        case ST_VK_2:
            params.fillMode->setValue(ST_FILL_SHADING);
            return;
        case ST_VK_3:
            params.fillMode->setValue(ST_FILL_SHADED_MESH);
            return;

        default:
            break;
    }
}

void StCADViewer::processEvents(const StMessage_t* theEvents) {
    size_t evId(0);
    for(; theEvents[evId].uin != StMessageList::MSG_NULL; ++evId) {
        switch(theEvents[evId].uin) {
            case StMessageList::MSG_DRAGNDROP_IN: {
                StString aFilePath;
                int aFilesCount = myWindow->getDragNDropFile(-1, aFilePath);
                if(aFilesCount > 0) {
                    myWindow->getDragNDropFile(0, aFilePath);
                    if(myCADLoader->getPlayList().checkExtension(aFilePath)) {
                        myCADLoader->getPlayList().open(aFilePath);
                        doUpdateStateLoading();
                        myCADLoader->doLoadNext();
                    }
                }
                break;
            }
            case StMessageList::MSG_CLOSE:
            case StMessageList::MSG_EXIT: {
                StApplication::exit(0);
                break;
            }
            case StMessageList::MSG_KEYS: {
                const bool* aKeys = (bool* )theEvents[evId].data;
                myIsCtrlPressed = aKeys[ST_VK_CONTROL];
                break;
            }
        }
    }

    if(myIsMiddleHold && myIsCtrlPressed) {
        // move
        StPointD_t aPt = myWindow->getMousePos();
        StGLVec2 aFlatMove( 2.0f * GLfloat(aPt.x() - myPrevMouse.x()),
                           -2.0f * GLfloat(aPt.y() - myPrevMouse.y()));

        StRectD_t aSect;
        myProjection.getZParams(aSect);

        StGLVec3 aCenter   = myCam.getCenter();
        StGLVec3 anEye     = myCam.getEye();
        StGLVec3 aMoveSide = StGLVec3(myCam.getSide()) * 1.0f * aFlatMove.x() * GLfloat(aSect.right());
        StGLVec3 aMoveUp   = myCam.getUp() * 1.0f * aFlatMove.y() * GLfloat(aSect.top());

        aCenter -= aMoveSide;
        anEye   -= aMoveSide;
        aCenter -= aMoveUp;
        anEye   -= aMoveUp;

        myCam.setCenter(aCenter);
        myCam.setEye(anEye);

        myPrevMouse = aPt;
    }

    StHandle<StGLMesh> aNewMesh;
    if(myCADLoader->getNextShape(aNewMesh)) {
        if(!aNewMesh.isNull()) {
            if(!myModel.isNull()) {
                myModel->release(*myContext);
            }
            myModel = aNewMesh;

            ST_DEBUG_LOG(myContext->stglFullInfo()); ///
            myModel->initVBOs(*myContext);
            ST_DEBUG_LOG(myContext->stglFullInfo()); ///

            // update normals representation
            if(params.toShowNormals->getValue()) {
                if(myNormalsMesh == NULL) {
                    myNormalsMesh = new StGLNormalsMesh();
                }
                myNormalsMesh->init(*myContext, *myModel);
            }
            // call fit all
            doFitALL();
        }
        doUpdateStateLoaded(!aNewMesh.isNull());
    }

    myGUI->setVisibility(myWindow->getMousePos(), true);
}

void StCADViewer::stglDraw(unsigned int theView) {
    myGUI->getCamera()->setView(theView);
    myProjection.setView(myWindow->isStereoOutput() ? theView : ST_DRAW_MONO);
    if(theView == ST_DRAW_LEFT) {
        myGUI->stglUpdate(myWindow->getMousePos());
    }

    // clear the screen and the depth buffer
    myContext->core11fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/// setup the projection matrix
    myProjection.updateFrustum(); ///
    myProjection.setupFixed(*myContext);

/// draw tunnel
    /*glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    StGLPrism stTunnel;
    stTunnel.setWireframe(0, 0, 10);
    stTunnel.setVisibilityZ(false, false);
    const GLfloat aPrecision = 0.001f;
    GLfloat aZDelta =  myProjection.getZFar() - myProjection.getZNear() - aPrecision;
    GLfloat aZ0  = -myProjection.getZNear();
    GLfloat aZ1  =  aZ0 - aZDelta;
    GLfloat aX0 = myProjection.getMonoFrustrum()->xLeft;
    GLfloat aX1 = myProjection.getMonoFrustrum()->xRight - aPrecision;
    GLfloat aY0 = myProjection.getMonoFrustrum()->yTop   - aPrecision;
    GLfloat aY1 = myProjection.getMonoFrustrum()->yBottom;
    stTunnel.init(StGLVec3(aX0, aY0, aZ0), // rectangle in near Z plane
                  StGLVec3(aX1, aY0, aZ0),
                  StGLVec3(aX1, aY1, aZ0),
                  StGLVec3(aX0, aY1, aZ0),
                  StGLVec3(aX0, aY0, aZ1), // rectangle in far Z plane
                  StGLVec3(aX1, aY0, aZ1),
                  StGLVec3(aX1, aY1, aZ1),
                  StGLVec3(aX0, aY1, aZ1));
    stTunnel.drawFixed();*/
///

    /// setup the camera
    StGLCamera aCam = myCam;
    if(myIsRightHold && myIsCtrlPressed) {
        // rotation preview
        const StPointD_t aPt = myWindow->getMousePos();
        StGLVec2 aFlatMove( 2.0f * GLfloat(aPt.x() - myPrevMouse.x()),
                           -2.0f * GLfloat(aPt.y() - myPrevMouse.y()));

        aCam.rotateX(-aFlatMove.x() * 90.0f);
        aCam.rotateY(-aFlatMove.y() * 90.0f);

        if(myIsCamIterative) {
            myCam = aCam;
            myPrevMouse = aPt;
        }
    }
    myContext->core11->glMatrixMode(GL_MODELVIEW);
    myContext->core11->glLoadIdentity();
    myContext->core11->glMultMatrixf(aCam);

    /// draw boxes
    /*StGLPrism aBox111, aBox011, aBox211, aBox001, aBox112, aBox110;
    aBox011.init(StGLVec3(-2.0f,  0.0f,  0.0f), 1.0f, 1.0f, 1.0f); aBox011.drawFixed();
    aBox211.init(StGLVec3( 2.0f,  0.0f,  0.0f), 1.0f, 1.0f, 1.0f); aBox211.drawFixed();
    aBox112.init(StGLVec3( 0.0f,  0.0f,  2.0f), 1.0f, 1.0f, 1.0f); aBox112.drawFixed();
    aBox110.init(StGLVec3( 0.0f,  0.0f, -2.0f), 1.0f, 1.0f, 1.0f); aBox110.drawFixed();*/

    /// draw the model
    if(!myModel.isNull()) {
        GLfloat mat_shininess[] = { 50.0 };
        glMaterialfv(GL_FRONT, GL_SPECULAR, StGLVec4(1.0f, 1.0f, 1.0f, 1.0f));
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        StGLVec4 aLightPos = StGLVec4(aCam.getEye(), 1.0f);
        glLightfv(GL_LIGHT0, GL_POSITION, aLightPos);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        GLint aLModel = params.isLightTwoSides->getValue() ? 1 : 0; glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, &aLModel); // light both sides
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        glEnable(GL_DEPTH_TEST);

        glColor3f(1.0f, 0.84314f, 0.0f); // golden
        if(params.fillMode->getValue() == ST_FILL_SHADING
        || params.fillMode->getValue() == ST_FILL_SHADED_MESH) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            myModel->drawFixed(*myContext);
        }
        if(params.fillMode->getValue() == ST_FILL_MESH
        || params.fillMode->getValue() == ST_FILL_WIREFRAME) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            myModel->drawFixed(*myContext);
        }
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);

        if(params.fillMode->getValue() == ST_FILL_SHADED_MESH) {
            glColor3f(1.0f, 1.0f, 1.0f); // white
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            myModel->drawFixed(*myContext);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if(myNormalsMesh != NULL) {
            glColor3f(1.0f, 1.0f, 1.0f); // white
            myNormalsMesh->drawFixed(*myContext);
        }

/// draw bnd box+
/*        glColor3f(1.0f, 0.0f, 1.0f);
        StGLPrism aBndBox;
        aBndBox.init(myModel->myBndBox);
        aBndBox.drawFixed();*/
/// draw bnd box2
/*        glColor3f(0.0f, 1.0f, 1.0f);
        myPrism.drawFixed();*/
/// draw bnd sphere
/*        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            StGLUVSphere sBndSphere;
            sBndSphere.init(myModel->myBndSphere, 20);
            sBndSphere.drawFixed();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
///
    }

    /// draw trihedron
    myContext->core11fwd->glDisable(GL_DEPTH_TEST);
    if(params.toShowTrihedron->getValue()) {
        StRectD_t aCurrSect; myProjection.getZParams(myProjection.getZScreen(), aCurrSect);
        GLfloat aLineLen = GLfloat(std::abs(aCurrSect.top()) * 0.2);
        StGLVec3 aTrihCenter = aCam.getCenter();
        aTrihCenter -= aCam.getSide() * ((GLfloat )std::abs(aCurrSect.left()) * 1.0f - aLineLen); // move to left
        aTrihCenter -= aCam.getUp()   * ((GLfloat )std::abs(aCurrSect.top())  * 1.0f - aLineLen); // move to bottom
        glBegin(GL_LINES);
            // DX
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3fv(aTrihCenter);
            glVertex3fv(aTrihCenter + StGLVec3::DX() * aLineLen);
            // DY
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3fv(aTrihCenter);
            glVertex3fv(aTrihCenter + StGLVec3::DY() * aLineLen);
            // DZ
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3fv(aTrihCenter);
            glVertex3fv(aTrihCenter + StGLVec3::DZ() * aLineLen);
        glEnd();
    }

    // draw GUI
    myContext->core11fwd->glDisable(GL_DEPTH_TEST);
    myGUI->stglDraw(theView);
}

void StCADViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myCADLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - CAD Viewer");
    } else {
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StCADViewer::doUpdateStateLoaded(bool isSuccess) {
    const StString aFileLoaded = myCADLoader->getPlayList().getCurrentTitle();
    if(aFileLoaded.isEmpty()) {
        myWindow->setTitle("sView - CAD Viewer");
    } else {
        myWindow->setTitle(aFileLoaded + (isSuccess ? StString() : StString(" FAIL to open")) + " - sView");
    }
}

void StCADViewer::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StCADViewer::doListFirst(const size_t ) {
    if(myCADLoader->getPlayList().walkToFirst()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListPrev(const size_t ) {
    if(myCADLoader->getPlayList().walkToPrev()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListNext(const size_t ) {
    if(myCADLoader->getPlayList().walkToNext()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListLast(const size_t ) {
    if(myCADLoader->getPlayList().walkToLast()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doFitALL(const size_t ) {
    if(myModel.isNull() || myModel->getBndSphere().isVoid()) {
        return;
    }

    // remember current camera direction
    StGLVec3 anEyeDir = myCam.getCenter() - myCam.getEye();
    anEyeDir.normalize();

    // got the boundary sphere for first fit all approximation
    StGLVec3 aFitCenter = myModel->getBndSphere().getCenter();
    GLfloat aFitRadius  = myModel->getBndSphere().getRadius();

    // aim the camera to the boundary sphere center
    myCam.setCenter(aFitCenter);
    // move camera out to avoid clipping by nearest Z-clipping plane
    anEyeDir *= 2.0f * aFitRadius;
    StGLVec3 anEye = myCam.getCenter() - anEyeDir;
    myCam.setEye(anEye);

    // setup the save far Z-clipping plane
    myProjection.setZScreen(2.0f * aFitRadius);
    myProjection.setZFar(4.0f * aFitRadius); // limit important for orthogonal projection!
    myProjection.updateFrustum();

    StBndCameraBox aCamBox(myCam);
    aCamBox.enlarge(myModel->getVertices());
    aCamBox.getPrism(*myContext, myPrism);

    GLdouble aZScr = anEyeDir.modulus() - aCamBox.getDZ() * 0.5f;
    StRectD_t aCurrSect; myProjection.getZParams(aZScr, aCurrSect);
    GLfloat aCurrDY = 2.0f * GLfloat(aCurrSect.top()) / myProjection.getZoom();

    // enlarge the rectangle to fit sphere
    GLfloat anAspect = myProjection.getAspect();
    GLfloat aNewDX = aCamBox.getDX();
    GLfloat aNewDY = aCamBox.getDY();
    // apply camera aspect ratio
    if(anAspect * aNewDY > aNewDX) {
        aNewDX = anAspect * aNewDY;
    } else {
        aNewDY = aNewDX / anAspect;
    }
    // compute linear scale factor
    myProjection.setZoom(aNewDY / aCurrDY);
    // move camera center to the boundary box center
    myCam.setCenter(aCamBox.getCenterGlobal());
    // move camera out from the boundary sphere center to avoid clipping by nearest Z-clipping plane
    anEye = myCam.getCenter() - anEyeDir;
    myCam.setEye(anEye);
}

void StCADViewer::doShowNormals(const bool toShow) {
    if(toShow && myNormalsMesh == NULL && !myModel.isNull()) {
        myNormalsMesh = new StGLNormalsMesh();
        myNormalsMesh->init(*myContext, *myModel);
    } else if(!toShow && myNormalsMesh != NULL) {
        myNormalsMesh->release(*myContext);
        delete myNormalsMesh;
        myNormalsMesh = NULL;
    }
}

void StCADViewer::doChangeProjection(const int32_t theProj) {
    if(myWindow.isNull()) {
        return;
    }

    switch(theProj) {
        case ST_PROJ_ORTHO: {
            myWindow->setStereoOutput(false);
            myProjection.setPerspective(false);
            break;
        }
        case ST_PROJ_PERSP: {
            myWindow->setStereoOutput(false);
            myProjection.setPerspective(true);
            break;
        }
        case ST_PROJ_STEREO: {
            myWindow->setStereoOutput(true);
            myProjection.setPerspective(true);
            break;
        }
    }
}
