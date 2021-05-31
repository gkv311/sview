/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLSubtitles_h_
#define __StGLSubtitles_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StSubQueue.h>

#include <StSettings/StEnumParam.h>
#include <StSettings/StFloat32Param.h>

// dummy
template<>
inline void StArray<StHandle <StSubItem> >::sort() {}

class StGLImageRegion;

/**
 * Subtitles widget.
 */
class StGLSubtitles : public StGLTextArea {

        private:

    /**
     * This class groups active subtitle items (with interleaved show time).
     * In most cases will contain just one item.
     */
    class StSubShowItems : public StArrayList<StHandle <StSubItem> > {

            public:

        StString     Text;  //!< active string representation
        StImagePlane Image; //!< active image  representation
        float        Scale; //!< image scale factor

            public:

        /**
         * Default constructor.
         */
        ST_LOCAL StSubShowItems();

        /**
         * Remove subtitle items with outdated PTS.
         * @param thePTS current presentation timestamp
         * @return true if active representation was changed (items were removed)
         */
        ST_LOCAL bool pop(const double thePTS);

        /**
         * Append subtitle item.
         */
        ST_LOCAL void add(const StHandle<StSubItem>& theItem);

    };

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLSubtitles(StGLImageRegion* theParent,
                               const StHandle<StSubQueue>&     theSubQueue,
                               const StHandle<StInt32Param>&   thePlace,
                               const StHandle<StFloat32Param>& theFontSize);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLSubtitles();
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;

    /**
     * Retrieve handle to the queue.
     */
    ST_CPPEXPORT const StHandle<StSubQueue>& getQueue() const;

    /**
     * Update PTS.
     */
    ST_CPPEXPORT void setPTS(const double thePTS);

        public: //! @name Properties

    struct {

        StHandle<StInt32Param>     Place;         //!< placement
        StHandle<StFloat32Param>   TopDY;         //!< displacement
        StHandle<StFloat32Param>   BottomDY;      //!< displacement
        StHandle<StFloat32Param>   FontSize;      //!< font size parameter
        StHandle<StFloat32Param>   Parallax;      //!< text parallax
        StHandle<StEnumParam>      Parser;        //!< text parser option
        StHandle<StBoolParamNamed> ToApplyStereo; //!< apply stereoscopic format of video to image subtitles

    } params;

        private:

    StGLTexture              myTexture;   //!< texture for image-based subtitles
    StGLVertexBuffer         myVertBuf;   //!< vertex buffer for image-based subtitles
    StGLVertexBuffer         myTCrdBuf;   //!< texture coordinates buffer for image-based subtitles
    StHandle<StSubQueue>     myQueue;     //!< thread-safe subtitles queue
    StSubShowItems           myShowItems; //!< active (shown) subtitle items
    double                   myPTS;       //!< active PTS

    class StImgProgram;
    StGLShare<StImgProgram>  myImgProgram;

};

#endif // __StGLSubtitles_h_
