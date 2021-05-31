/**
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextureData_h_
#define __StGLTextureData_h_

#include <StImage/StImage.h>
#include <StGLStereo/StGLTextureUploadParams.h>
#include <StGLStereo/StGLQuadTexture.h>
#include <StGL/StGLDeviceCaps.h>

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
    ST_CPPEXPORT StGLTextureData(const StHandle<StGLTextureUploadParams>& theUploadParams);

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

    ST_LOCAL void resetStParams() {
        myStParams.nullify();
        if(!myDataPair.getBufferCounter().isNull()
        || !myDataL.getBufferCounter().isNull()
        || !myDataR.getBufferCounter().isNull()) {
            reset();
        }
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
    ST_LOCAL StFormat getSourceFormat() const {
        return mySrcFormat;
    }

    /**
     * @return format of source data
     */
    ST_LOCAL StCubemap getCubemapFormat() const {
        return myCubemapFormat;
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
     * @param theDevCaps  device capabilities
     * @param theDataL    frame which contains left view or left+right views
     * @param theDataR    frame which contains right view (optional)
     * @param theStParams handle to associated data
     * @param theFormat   stereo layout in data
     * @param theCubemap  cubemap format
     * @param thePts      presentation timestamp
     */
    ST_CPPEXPORT void updateData(const StGLDeviceCaps&           theDevCaps,
                                 const StImage&                  theDataL,
                                 const StImage&                  theDataR,
                                 const StHandle<StStereoParams>& theStParams,
                                 const StFormat                  theFormat,
                                 const StCubemap                 theCubemap,
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

    ST_LOCAL void copyProps(const StImage& theDataL,
                            const StImage& theDataR);

    /**
     * Validate and setup packed cubemap flag.
     */
    ST_LOCAL void validateCubemap(const StCubemap theCubemap);

    /**
     * Fill the texture plane.
     */
    ST_LOCAL void fillTexture(StGLContext&        theCtx,
                              StGLFrameTexture&   theFrameTexture,
                              const StImagePlane& theData);

    ST_LOCAL void setupAttributes(StGLFrameTextures& stFrameTextures, const StImage& theImage);

    ST_LOCAL void setupDataRectangle(const StImagePlane& theImagePlane,
                                     const GLfloat       thePixelRatio,
                                     StGLFrameTexture&   theTextureFrame);

        private:

    StGLTextureData*         myPrev;          //!< pointer to previous item in the list
    StGLTextureData*         myNext;          //!< pointer to next item

    GLubyte*                 myDataPtr;       //!< data for left and right views
    size_t                   myDataSizeBytes; //!< allocated data size in bytes
    StImage                  myDataPair;
    StImage                  myDataL;
    StImage                  myDataR;

    StHandle<StStereoParams> myStParams;
    double                   myPts;           //!< presentation timestamp
    StFormat                 mySrcFormat;
    StCubemap                myCubemapFormat;

    StHandle<StGLTextureUploadParams> myUploadParams; //!< texture streaming parameters
    GLsizei                  myFillFromRow;
    GLsizei                  myFillRows;

};

#endif // __StGLTextureData_h_
