/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#include "StCADModel.h"
#include "StCADViewer.h"
#include "StCADViewerGUI.h"
#include "StCADLoader.h"

#include <StCore/StCore.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>

#include <StGLEW.h>
#include <StGL/StGLMemInfo.h>

#include <StGLMesh/StGLMesh.h>
#include <StGLMesh/StGLPrism.h>
#include <StGLMesh/StGLUVSphere.h>
#include <StGLMesh/StBndCameraBox.h>

#include <StGLWidgets/StGLMsgStack.h>

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
    bool init(const StGLMesh& theMesh) {
        // reset current state
        clearRAM();
        clearVRAM();

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
        myVertexBuf.init(myVertices);

        // no need to store this list in RAM
        //clearRAM();
        return true;
    }

};

static StGLCamera myCam; /// TODO move to...
static StGLPrism myPrism; /// debug bnd box
static StGLNormalsMesh* myNormalsMesh = NULL; /// TODO move to...

const StString StCADViewer::ST_DRAWER_PLUGIN_NAME = "StCADViewer";

StCADViewer::StCADViewer()
: myWin(),
  mySettings(),
  myGUI(),
  myCADLoader(),
  myModel(),
  myProjection(),
  myIsLeftHold(false),
  myIsRightHold(false),
  myIsMiddleHold(false),
  myIsCtrlPressed(false),
  myIsCamIterative(false),
  myToQuit(false) {
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

    /// force thread-safe OCCT memory management
    Standard::SetReentrant(Standard_True);
}

StCADViewer::~StCADViewer() {
    if(!myGUI.isNull() && !mySettings.isNull()) {
        mySettings->saveParam(ST_PARAM_NORMALS,   params.toShowNormals);
        mySettings->saveParam(ST_PARAM_TRIHEDRON, params.toShowTrihedron);
        mySettings->saveParam(ST_PARAM_PROJMODE,  params.projectMode);
        mySettings->saveParam(ST_PARAM_FILLMODE,  params.fillMode);
    }

    // release GUI data and GL resources before closing the window
    myGUI.nullify();
    // wait working threads to quit and release resources
    myCADLoader.nullify();
    // destroy other objects
    mySettings.nullify();
    // now destroy the window
    myWin.nullify();
    // release libraries
    StCore::FREE();
    StSettings::FREE();
}

bool StCADViewer::init(StWindowInterface* theWindow) {
    if(!StVersionInfo::checkTimeBomb("sView - CAD Viewer plugin")) {
        // timebomb for alpha versions
        return false;
    } else if(theWindow == NULL) {
        stError("StCADViewer, Invalid window from StRenderer plugin!");
        return false;
    } else if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("StCADViewer, Core library not available!");
        return false;
    } else if(StSettings::INIT() != STERROR_LIBNOERROR) {
        stError("StCADViewer, Settings plugin not available!");
        return false;
    }

    // create window wrapper
    myWin = new StWindow(theWindow);
    myWin->setTitle("sView - CAD Viewer");
    if(!StGLEW::init()) {
        stError("StCADViewer, OpenGL context is probably broken");
        return false;
    }

    // load settings
    mySettings = new StSettings(ST_DRAWER_PLUGIN_NAME);
    mySettings->loadParam(ST_PARAM_NORMALS,   params.toShowNormals);
    mySettings->loadParam(ST_PARAM_TRIHEDRON, params.toShowTrihedron);
    mySettings->loadParam(ST_PARAM_PROJMODE,  params.projectMode);
    mySettings->loadParam(ST_PARAM_FILLMODE,  params.fillMode);

    myWin->stglSetTargetFps(50.0);
    myWin->setStereoOutput(params.projectMode->getValue() == ST_PROJ_STEREO);

    // create working threads
    myCADLoader = new StCADLoader(StHandle<StLangMap>::downcast(myGUI->myLangMap));
    // connect show messages slot
    myCADLoader->signals.onError.connect(myGUI->myMsgStack, &StGLMsgStack::doPushMessage);

    return myGUI->stglInit();
}

bool StCADViewer::open(const StOpenInfo& stOpenInfo) {
    //parseArguments(stOpenInfo.getArgumentsMap());
    StMIME stOpenMIME = stOpenInfo.getMIME();
    if(stOpenMIME == StDrawerInfo::DRAWER_MIME() || stOpenInfo.getPath().isEmpty()) {
        // open drawer without files
        return true;
    }

    // clear playlist first
    myCADLoader->getPlayList().clear();

    if(!stOpenMIME.isEmpty()) {
        // create just one-file playlist
        myCADLoader->getPlayList().addOneFile(stOpenInfo.getPath(), stOpenMIME);
    } else {
        // create playlist from file's folder
        myCADLoader->getPlayList().open(stOpenInfo.getPath());
    }

    if(!myCADLoader->getPlayList().isEmpty()) {
        doUpdateStateLoading();
        myCADLoader->doLoadNext();
    }

    return true;
}

void StCADViewer::parseCallback(StMessage_t* stMessages) {
    if(myToQuit) {
        stMessages[0].uin = StMessageList::MSG_EXIT;
        stMessages[1].uin = StMessageList::MSG_NULL;
    }

    size_t evId(0);
    for(; stMessages[evId].uin != StMessageList::MSG_NULL; ++evId) {
        switch(stMessages[evId].uin) {
            case StMessageList::MSG_RESIZE: {
                StRectI_t aWinRect = myWin->getPlacement();
                myGUI->stglResize(aWinRect);
                myProjection.resize(aWinRect.width(), aWinRect.height());
                break;
            }
            case StMessageList::MSG_DRAGNDROP_IN: {
                int aFilesCount = myWin->getDragNDropFile(-1, NULL, 0);
                if(aFilesCount > 0) {
                    stUtf8_t aBuffFile[4096];
                    stMemSet(aBuffFile, 0, sizeof(aBuffFile));
                    if(myWin->getDragNDropFile(0, aBuffFile, (4096 * sizeof(stUtf8_t))) == 0) {
                        StString aBuffString(aBuffFile);
                        if(myCADLoader->getPlayList().checkExtension(aBuffString)) {
                            myCADLoader->getPlayList().open(aBuffString);
                            doUpdateStateLoading();
                            myCADLoader->doLoadNext();
                        }
                    }
                }
                break;
            }
            case StMessageList::MSG_CLOSE:
            case StMessageList::MSG_EXIT: {
                stMessages[0].uin = StMessageList::MSG_EXIT;
                stMessages[1].uin = StMessageList::MSG_NULL;
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = (bool* )stMessages[evId].data;

                myIsCtrlPressed = keysMap[ST_VK_CONTROL];

                if(keysMap[ST_VK_ESCAPE]) {
                    // we could parse Escape key in other way
                    stMessages[0].uin = StMessageList::MSG_EXIT;
                    stMessages[1].uin = StMessageList::MSG_NULL;
                    return;
                }
                if(keysMap[ST_VK_RETURN]) {
                    params.isFullscreen->reverse();
                    keysMap[ST_VK_RETURN] = false;
                }

                // switch projection matrix
                if(keysMap[ST_VK_M]) {
                    params.projectMode->setValue(ST_PROJ_PERSP);
                    keysMap[ST_VK_M] = false;
                }
                if(keysMap[ST_VK_S]) {
                    params.projectMode->setValue(ST_PROJ_STEREO);
                    keysMap[ST_VK_S] = false;
                }
                if(keysMap[ST_VK_O]) {
                    params.projectMode->setValue(ST_PROJ_ORTHO);
                    keysMap[ST_VK_O] = false;
                }

                // separation
                if(keysMap[ST_VK_MULTIPLY] && !keysMap[ST_VK_CONTROL]) {
                    myProjection.setIOD(myProjection.getIOD() + 0.1f);
                    ST_DEBUG_LOG("Sep. inc, " + myProjection.toString());
                    keysMap[ST_VK_MULTIPLY] = false;
                }
                if(keysMap[ST_VK_DIVIDE] && !keysMap[ST_VK_CONTROL]) {
                    myProjection.setIOD(myProjection.getIOD() - 0.1f);
                    ST_DEBUG_LOG("Sep. dec, " + myProjection.toString());
                    keysMap[ST_VK_DIVIDE] = false;
                }

                ///
                if(keysMap[ST_VK_C]) {
                    myIsCamIterative = !myIsCamIterative;
                    ST_DEBUG_LOG("Iterative camera " + (myIsCamIterative ? "ON" : "OFF"));
                    keysMap[ST_VK_C] = false;
                }

                if(keysMap[ST_VK_LEFT]) {
                    myCam.rotateX(-1.0f);
                    keysMap[ST_VK_LEFT] = false;
                }
                if(keysMap[ST_VK_RIGHT]) {
                    myCam.rotateX(1.0f);
                    keysMap[ST_VK_RIGHT] = false;
                }
                if(keysMap[ST_VK_UP]) {
                    myCam.rotateY(-1.0f);
                    keysMap[ST_VK_UP] = false;
                }
                if(keysMap[ST_VK_DOWN]) {
                    myCam.rotateY(1.0f);
                    keysMap[ST_VK_DOWN] = false;
                }

                if(keysMap[ST_VK_Q]) {
                    myCam.rotateZ(-1.0f);
                    keysMap[ST_VK_Q] = false;
                }
                if(keysMap[ST_VK_W]) {
                    myCam.rotateZ(1.0f);
                    keysMap[ST_VK_W] = false;
                }

                // call fit all
                if(keysMap[ST_VK_F]) {
                    doFitALL();
                    keysMap[ST_VK_F] = false;
                }

                // show normals
                if(keysMap[ST_VK_N]) {
                    params.toShowNormals->reverse();
                    keysMap[ST_VK_N] = false;
                }

                // playlist navigation
                if(keysMap[ST_VK_PRIOR]) {
                    doListPrev();
                    keysMap[ST_VK_PRIOR] = false;
                }
                if(keysMap[ST_VK_NEXT]) {
                    doListNext();
                    keysMap[ST_VK_NEXT] = false;
                }
                if(keysMap[ST_VK_HOME]) {
                    doListFirst();
                    keysMap[ST_VK_HOME] = false;
                }
                if(keysMap[ST_VK_END]) {
                    doListLast();
                    keysMap[ST_VK_END] = false;
                }

                // shading mode
                if(keysMap[ST_VK_1]) {
                    params.fillMode->setValue(ST_FILL_MESH);
                    keysMap[ST_VK_1] = false;
                }
                if(keysMap[ST_VK_2]) {
                    params.fillMode->setValue(ST_FILL_SHADING);
                    keysMap[ST_VK_2] = false;
                }
                if(keysMap[ST_VK_3]) {
                    params.fillMode->setValue(ST_FILL_SHADED_MESH);
                    keysMap[ST_VK_3] = false;
                }

                break;
            }
            case StMessageList::MSG_MOUSE_DOWN: {
                StPointD_t pt;
                int mouseBtn = myWin->getMouseDown(&pt);
                myGUI->tryClick(pt, mouseBtn);

                if(mouseBtn == ST_MOUSE_LEFT) {
                    myIsLeftHold = true; ///
                    myPrevMouse = pt;
                } else if(mouseBtn == ST_MOUSE_RIGHT) {
                    myIsRightHold = true; ///
                    myPrevMouse = pt;
                } else if(mouseBtn == ST_MOUSE_MIDDLE) {
                    myIsMiddleHold = true; ///
                    myPrevMouse = pt;
                }

                break;
            }
            case StMessageList::MSG_MOUSE_UP: {
                StPointD_t pt;
                int mouseBtn = myWin->getMouseUp(&pt);
                switch(mouseBtn) {
                    case ST_MOUSE_LEFT: {
                        myIsLeftHold = false;
                        break;
                    }
                    case ST_MOUSE_RIGHT: {
                        if(myIsRightHold && myIsCtrlPressed) {
                            // rotate
                            StPointD_t aPt = myWin->getMousePos();
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
                }
                myGUI->tryUnClick(pt, mouseBtn);
                break;
            }
        }
    }

    if(myIsMiddleHold && myIsCtrlPressed) {
        // move
        StPointD_t aPt = myWin->getMousePos();
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
            myModel = aNewMesh;
            ST_DEBUG_LOG("GL memory Before\n" + StGLMemInfo().toString()); ///
            myModel->initVBOs();
            ST_DEBUG_LOG("GL memory After\n" + StGLMemInfo().toString()); ///

            // update normals representation
            if(params.toShowNormals->getValue()) {
                if(myNormalsMesh == NULL) {
                    myNormalsMesh = new StGLNormalsMesh();
                }
                myNormalsMesh->init(*myModel);
            }
            // call fit all
            doFitALL();
        }
        doUpdateStateLoaded(!aNewMesh.isNull());
    }

    myGUI->setVisibility(myWin->getMousePos(), true);
}

void StCADViewer::stglDraw(unsigned int view) {
    myGUI->getCamera()->setView(view);
    myProjection.setView(myWin->isStereoOutput() ? view : ST_DRAW_MONO);
    if(view == ST_DRAW_LEFT) {
        myGUI->stglUpdate(myWin->getMousePos());
    }

    // clear the screen and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/// setup the projection matrix
    myProjection.updateFrustum(); ///
    myProjection.setupFixed();

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
        StPointD_t aPt = myWin->getMousePos();
        StGLVec2 aFlatMove( 2.0f * GLfloat(aPt.x() - myPrevMouse.x()),
                           -2.0f * GLfloat(aPt.y() - myPrevMouse.y()));

        aCam.rotateX(-aFlatMove.x() * 90.0f);
        aCam.rotateY(-aFlatMove.y() * 90.0f);

        if(myIsCamIterative) {
            myCam = aCam;
            myPrevMouse = aPt;
        }
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(aCam);

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
            myModel->drawFixed();
        }
        if(params.fillMode->getValue() == ST_FILL_MESH
        || params.fillMode->getValue() == ST_FILL_WIREFRAME) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            myModel->drawFixed();
        }
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);

        if(params.fillMode->getValue() == ST_FILL_SHADED_MESH) {
            glColor3f(1.0f, 1.0f, 1.0f); // white
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            myModel->drawFixed();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if(myNormalsMesh != NULL) {
            glColor3f(1.0f, 1.0f, 1.0f); // white
            myNormalsMesh->drawFixed();
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
    glDisable(GL_DEPTH_TEST);
    if(params.toShowTrihedron->getValue()) {
        StRectD_t aCurrSect; myProjection.getZParams(myProjection.getZScreen(), aCurrSect);
        GLfloat aLineLen = std::abs(aCurrSect.top()) * 0.2f;
        StGLVec3 aTrihCenter = aCam.getCenter();
        aTrihCenter -= aCam.getSide() * (std::abs(aCurrSect.left()) * 1.0f - aLineLen); // move to left
        aTrihCenter -= aCam.getUp()   * (std::abs(aCurrSect.top())  * 1.0f - aLineLen); // move to bottom
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
    glDisable(GL_DEPTH_TEST);
    myGUI->stglDraw(view);
}

void StCADViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myCADLoader->getPlayList().getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWin->setTitle("sView - CAD Viewer");
    } else {
        myWin->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StCADViewer::doUpdateStateLoaded(bool isSuccess) {
    const StString aFileLoaded = myCADLoader->getPlayList().getCurrentTitle();
    if(aFileLoaded.isEmpty()) {
        myWin->setTitle("sView - CAD Viewer");
    } else {
        myWin->setTitle(aFileLoaded + (isSuccess ? StString() : StString(" FAIL to open")) + " - sView");
    }
}

void StCADViewer::doFullscreen(const bool theIsFullscreen) {
    if(!myWin.isNull()) {
        myWin->setFullScreen(theIsFullscreen);
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
    aCamBox.getPrism(myPrism);

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
        myNormalsMesh->init(*myModel);
    } else if(!toShow && myNormalsMesh != NULL) {
        delete myNormalsMesh;
        myNormalsMesh = NULL;
    }
}

void StCADViewer::doChangeProjection(const int32_t theProj) {
    switch(theProj) {
        case ST_PROJ_ORTHO: {
            myWin->setStereoOutput(false);
            myProjection.setPerspective(false);
            break;
        }
        case ST_PROJ_PERSP: {
            myWin->setStereoOutput(false);
            myProjection.setPerspective(true);
            break;
        }
        case ST_PROJ_STEREO: {
            myWin->setStereoOutput(true);
            myProjection.setPerspective(true);
            break;
        }
    }
}

// exports
ST_EXPORT StDrawerInterface* StDrawer_new() {
    return new StCADViewer(); }
ST_EXPORT void StDrawer_del(StDrawerInterface* inst) {
    delete (StCADViewer* )inst; }
ST_EXPORT stBool_t StDrawer_init(StDrawerInterface* inst, StWindowInterface* stWin) {
    return ((StCADViewer* )inst)->init(stWin); }
ST_EXPORT stBool_t StDrawer_open(StDrawerInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StCADViewer* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StDrawer_parseCallback(StDrawerInterface* inst, StMessage_t* stMessages) {
    ((StCADViewer* )inst)->parseCallback(stMessages); }
ST_EXPORT void StDrawer_stglDraw(StDrawerInterface* inst, unsigned int view) {
    ((StCADViewer* )inst)->stglDraw(view); }

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const stUtf8_t* getMIMEDescription() {
    return StCADLoader::ST_CAD_MIME_STRING.toCString();
}
