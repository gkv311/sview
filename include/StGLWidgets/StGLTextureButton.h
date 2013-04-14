/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

class StGLTextureButton : public StGLWidget {

        public:

    ST_CPPEXPORT StGLTextureButton(StGLWidget*      theParent,
                                   const int        theLeft = 32,
                                   const int        theTop = 32,
                                   const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   const size_t     theFacesCount = 1);

    ST_CPPEXPORT virtual ~StGLTextureButton();

    ST_CPPEXPORT virtual const StString& getClassName();

    inline size_t getFaceId() {
        return myFaceId;
    }

    inline void setFaceId(const size_t theId) {
        myFaceId = theId;
    }

    ST_CPPEXPORT void setTexturePath(const StString* theTexturesPaths,
                                     const size_t    theCount = 1);

    ST_CPPEXPORT virtual void stglResize(const StRectI_t& winRectPx);
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int view);

    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

        public:    //!< Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private:   //!< callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    ST_LOCAL void glWaveTimerControl();

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
