/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
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
    static StString GET_NAME_STRING(StFormat formatEnum);

    /**
     * Return enumeration value from the string.
     */
    static StFormat GET_FROM_STRING(const StString& formatString);

        public:

    /**
     * Default constructor
     */
    ST_CPPEXPORT StGLTextureData();

    /**
     * Destructor.
     */
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
        return myPts;
    }

    /**
     * @return format of source data
     */
    inline StFormat getSourceFormat() const {
        return mySrcFormat;
    }

    /**
     * Iterator's function.
     * @return previous item
     */
    inline StGLTextureData* getPrev() {
        return myPrev;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted previous item
     */
    inline void setPrev(StGLTextureData* theTextureData) {
        myPrev = theTextureData;
        if(theTextureData != NULL) {
            theTextureData->myNext = this;
        }
    }

    /**
     * Iterator's function.
     * @return next item
     */
    inline StGLTextureData* getNext() {
        return myNext;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted next item
     */
    inline void setNext(StGLTextureData* theTextureData) {
        myNext = theTextureData;
        if(theTextureData != NULL) {
            theTextureData->myPrev = this;
        }
    }

    /**
     * Setup new data.
     * @param theDataL    frame which contains left view or left+right views
     * @param theDataR    frame which contains right view (optional)
     * @param theStParams handle to associated data
     * @param theFormat   stereo layout in data
     * @param thePts      presentation timestamp
     */
    ST_CPPEXPORT void updateData(const StImage&                  theDataL,
                                 const StImage&                  theDataR,
                                 const StHandle<StStereoParams>& theStParams,
                                 const StFormat                  theFormat,
                                 const double                    thePts);

    /**
     * Perform texture update with current data.
     * @param theCtx      OpenGL context
     * @param theQTexture texture to fill in
     * @return true if texture update (all iterations) finished
     */
    ST_CPPEXPORT bool fillTexture(StGLContext&     theCtx,
                                  StGLQuadTexture& theQTexture);

    ST_CPPEXPORT void getCopy(StImage* outDataL, StImage* outDataR) const;

    /**
     * Release memory.
     */
    ST_CPPEXPORT void reset();

        private:


    ST_LOCAL bool reAllocate(const size_t theSizeBytes);

    /**
     * Fill the texture plane.
     */
    ST_LOCAL void fillTexture(StGLContext&        theCtx,
                              StGLFrameTexture&   theFrameTexture,
                              const StImagePlane& theData);

    ST_LOCAL void setupAttributes(StGLFrameTextures& stFrameTextures, const StImage& theImage);

        private:

    StGLTextureData*         myPrev;          //!< pointer to previous item in the list
    StGLTextureData*         myNext;          //!< pointer to next item

    GLubyte*                 myDataPtr;       //!< data for left and right views
    size_t                   myDataSizeBytes; //!< allocated data size in bytes
    StImage                  myDataL;
    StImage                  myDataR;

    StHandle<StStereoParams> myStParams;
    double                   myPts;           //!< presentation timestamp
    StFormat                 mySrcFormat;

    GLsizei                  myFillFromRow;
    GLsizei                  myFillRows;

};

#endif // __StGLTextureData_h_
