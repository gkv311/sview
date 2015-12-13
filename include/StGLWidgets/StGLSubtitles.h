/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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

    ST_CPPEXPORT StGLSubtitles(StGLWidget*                     theParent,
                               const StHandle<StSubQueue>&     theSubQueue,
                               const StHandle<StInt32Param>&   thePlace,
                               const StHandle<StFloat32Param>& theTopDY,
                               const StHandle<StFloat32Param>& theBottomDY,
                               const StHandle<StFloat32Param>& theFontSize,
                               const StHandle<StFloat32Param>& theParallax,
                               const StHandle<StEnumParam>&    theParser);
    ST_CPPEXPORT virtual ~StGLSubtitles();
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& thePointZo) ST_ATTR_OVERRIDE;
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

        private:

    StHandle<StInt32Param>   myPlace;     //!< placement
    StHandle<StFloat32Param> myTopDY;     //!< displacement
    StHandle<StFloat32Param> myBottomDY;  //!< displacement
    StHandle<StFloat32Param> myFontSize;  //!< font size parameter
    StHandle<StFloat32Param> myParallax;  //!< text parallax
    StHandle<StEnumParam>    myParser;    //!< text parser option
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
