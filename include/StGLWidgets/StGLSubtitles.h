/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLSubtitles_h_
#define __StGLSubtitles_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGLWidgets/StSubQueue.h>

// dummy
template<>
inline void StArray<StHandle <StSubItem> >::sort() {}

/**
 * Subtitles widget.
 */
class ST_LOCAL StGLSubtitles : public StGLTextArea {

        private:

    /**
     * This class groups active subtitle items (with interleaved show time).
     * In most cases will contain just one item.
     */
    class StSubShowItems : public StArrayList<StHandle <StSubItem> > {

            public:

        StString myText; //!< active string representation

            public:

        /**
         * Default constructor.
         */
        StSubShowItems();

        /**
         * Remove subtitle items with outdated PTS.
         * @param thePTS (const double ) - current presentation timestamp;
         * @return true if active representation was changed (items were removed).
         */
        bool pop(const double thePTS);

        /**
         * Append subtitle item.
         */
        void add(const StHandle<StSubItem>& theItem);

    };

        private:

    StHandle<StSubQueue> myQueue; //!< thread-safe subtitles queue
    StSubShowItems myShowItems;   //!< active (shown) subtitle items
    double myPTS;                 //!< active PTS

        public:

    StGLSubtitles(StGLWidget* theParent);
    virtual ~StGLSubtitles();
    virtual const StString& getClassName();
    virtual void stglUpdate(const StPointD_t& thePointZo);
    virtual void stglResize(const StRectI_t& theWinRectPx);

    /**
     * Retrieve handle to the queue.
     */
    const StHandle<StSubQueue>& getQueue() const;

    /**
     * Update PTS.
     */
    void setPTS(const double thePTS);

};

#endif //__StGLSubtitles_h_
