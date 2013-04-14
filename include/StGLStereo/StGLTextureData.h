/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextureData_h_
#define __StGLTextureData_h_

#include <StImage/StImage.h>
#include <StGLStereo/StGLQuadTexture.h>

/**
 * This is just ratio-table.
 */
namespace videoRatio {

    // TV
    static const GLfloat TV_OVERUNDER       = 4.0f  / 6.0f;  // 0.6(6):1 ~ 4:6   Over/Under
    static const GLfloat TV_NORMAL          = 4.0f  / 3.0f;  // 1.3(3):1 ~ 4:3   Mono
    static const GLfloat TV_SIDEBYSIDE      = 8.0f  / 3.0f;  // 2.6(6):1 ~ 8:3   SideBySide

    // Widescreen
    static const GLfloat WIDE_NORMAL        = 16.0f / 9.0f;  // 1.7(7):1 ~ 16:9  Mono
    static const GLfloat WIDE_PC            = 16.0f / 10.0f; // 1.6:1    ~ 16:10 Mono
    static const GLfloat WIDE_SIDEBYSIDE    = 32.0f / 9.0f;  // 3.5(5):1 ~ 32:9  SideBySide

    // Cinemascope
    static const GLfloat CINEMASCOPE        = 29.0f / 9.0f;  // 3.2(2):1 ~ 29:9  Mono

    static const GLfloat USERDEF_SIDEBYSIDE = 2.86f;

}

/**
 * This class represents stereo data for textures
 * in separate buffers.
 * Also structure store pointers for next and previous
 * StTextureData in queue.
 */
class StGLTextureData {

        public:

    /**
     * Returns name for format.
     */
    static StString GET_NAME_STRING(StFormatEnum formatEnum);

    /**
     * Return enumeration value from the string.
     */
    static StFormatEnum GET_FROM_STRING(const StString& formatString);

        public:

    ST_CPPEXPORT StGLTextureData();

    ST_CPPEXPORT ~StGLTextureData();

    /**
     * @return stereo parameters for current data
     */
    inline const StHandle<StStereoParams>& getSource() const {
        return myStParams;
    }

    inline void resetStParams() {
        myStParams.nullify();
    }

    /**
     * @return presentation timestamp
     */
    inline double getPTS() {
        return srcPTS;
    }

    /**
     * @return format of source data
     */
    inline StFormatEnum getSourceFormat() {
        return srcFormat;
    }

    /**
     * Iterator's function.
     * @return previous item
     */
    inline StGLTextureData* getPrev() {
        return prev;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted previous item
     */
    inline void setPrev(StGLTextureData* textureData) {
        prev = textureData;
        if(textureData != NULL) {
            textureData->next = this;
        }
    }

    /**
     * Iterator's function.
     * @return next item
     */
    inline StGLTextureData* getNext() {
        return next;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted next item
     */
    inline void setNext(StGLTextureData* textureData) {
        next = textureData;
        if(textureData != NULL) {
            textureData->prev = this;
        }
    }

    /**
     * Setup new data.
     * @param srcPTS (double ) - presentation timestamp.
     */
    ST_CPPEXPORT void updateData(const StImage& srcDataLeft,
                                 const StImage& srcDataRight,
                                 const StHandle<StStereoParams>& theStParams,
                                 StFormatEnum srcFormat,
                                 double srcPTS);

    /**
     * Perform texture update with current data.
     * @param stQTexture - texture to fill in.
     * @return true if texture update (all iterations) finished.
     */
    ST_CPPEXPORT bool fillTexture(StGLContext&     theCtx,
                                  StGLQuadTexture& theQTexture);

    ST_CPPEXPORT void getCopy(StImage* outDataL, StImage* outDataR) const;

    /**
     * Release memory.
     */
    ST_CPPEXPORT void reset();

        private:


    ST_LOCAL bool reAllocate(size_t newSizeBytes);

    /**
     * Fill the texture plane.
     */
    ST_LOCAL void fillTexture(StGLContext&        theCtx,
                              StGLFrameTexture&   theFrameTexture,
                              const StImagePlane& theData);

    ST_LOCAL void setupAttributes(StGLFrameTextures& stFrameTextures, const StImage& theImage);

        private:

    StGLTextureData*    prev; // pointer to previous item
    StGLTextureData*    next; // pointer to next item

    GLubyte*         dataPtr; // data for left and right views
    size_t     dataSizeBytes; // allocated data size in bytes
    StImage          myDataL;
    StImage          myDataR;

    StHandle<StStereoParams> myStParams;
    double            srcPTS; // presentation timestamp
    StFormatEnum   srcFormat;

    GLsizei      fillFromRow;
    GLsizei         fillRows;

};

#endif //__StGLTextureData_h_
