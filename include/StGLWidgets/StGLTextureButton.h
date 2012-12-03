/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextureButton_h_
#define __StGLTextureButton_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>

#include <StTemplates/StArray.h>

template<> inline void StArray<StGLTexture>::sort() {}

class ST_LOCAL StGLTextureButton : public StGLWidget {

        public:

    StGLTextureButton(StGLWidget*      theParent,
                      const int        theLeft = 32,
                      const int        theTop = 32,
                      const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                      const size_t     theFacesCount = 1);

    virtual ~StGLTextureButton();

    virtual const StString& getClassName();

    size_t getFaceId() {
        return myFaceId;
    }

    void setFaceId(const size_t theId) {
        myFaceId = theId;
    }

    void setTexturePath(const StString* theTexturesPaths,
                        const size_t    theCount = 1);

    virtual void stglResize(const StRectI_t& winRectPx);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int view);

    virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

        public:    //!< Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private:   //!< callback Slots (private overriders)

    void doMouseUnclick(const int theBtnId);

        private:

    void glWaveTimerControl();

        private:

    StGLVertexBuffer           myVertBuf;
    StGLVertexBuffer           myTCrdBuf;
    size_t                     myFaceId;
    size_t                     myFacesCount;
    StArray<StGLTexture>       myTextures;
    StArray<StString>          myTexturesPaths;

    class StButtonProgram;
    StGLShare<StButtonProgram> myProgram;

    StTimer                    myWaveTimer;

};

#endif //__StGLTextureButton_h_
