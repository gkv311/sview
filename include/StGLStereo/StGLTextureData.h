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
    ST_CPPEXPORT StGLTextureData(const std::shared_ptr<StGLTextureUploadParams>& theUploadParams);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StGLTextureData();

    /**
     * @return stereo parameters for current data
     */
    ST_LOCAL const std::shared_ptr<StStereoParams>& getSource() const {
        return myStParams;
    }

    ST_LOCAL void resetStParams() {
        myStParams.reset();
        if (myDataPair.getBufferCounter().get() != nullptr
         || myDataL.getBufferCounter().get() != nullptr
         || myDataR.getBufferCounter().get() != nullptr) {
            reset();
        }
    }

    /**
     * @return presentation timestamp
     */
    ST_LOCAL double getPTS() {
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
    ST_LOCAL StGLTextureData* getPrev() {
        return myPrev;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted previous item
     */
    ST_LOCAL void setPrev(StGLTextureData* theTextureData) {
        myPrev = theTextureData;
        if(theTextureData != NULL) {
            theTextureData->myNext = this;
        }
    }

    /**
     * Iterator's function.
     * @return next item
     */
    ST_LOCAL StGLTextureData* getNext() {
        return myNext;
    }

    /**
     * Iterator's function, communicate queue.
     * @param textureData setted next item
     */
    ST_LOCAL void setNext(StGLTextureData* theTextureData) {
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
    ST_CPPEXPORT void updateData(const StGLDeviceCaps& theDevCaps,
                                 const StImage& theDataL,
                                 const StImage& theDataR,
                                 const std::shared_ptr<StStereoParams>& theStParams,
                                 const StFormat theFormat,
                                 const StCubemap theCubemap,
                                 const double thePts);

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

    StGLTextureData* myPrev = nullptr; //!< pointer to previous item in the list
    StGLTextureData* myNext = nullptr; //!< pointer to next item

    GLubyte* myDataPtr = nullptr; //!< data for left and right views
    size_t   myDataSizeBytes = 0; //!< allocated data size in bytes
    StImage  myDataPair;
    StImage  myDataL;
    StImage  myDataR;

    std::shared_ptr<StStereoParams> myStParams;
    double                   myPts = 0.0; //!< presentation timestamp
    StFormat                 mySrcFormat = StFormat_AUTO;
    StCubemap                myCubemapFormat = StCubemap_OFF;

    std::shared_ptr<StGLTextureUploadParams> myUploadParams; //!< texture streaming parameters
    GLsizei myFillFromRow = 0;
    GLsizei myFillRows = 0;

};

#endif // __StGLTextureData_h_
