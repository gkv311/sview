/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StSeekBar_h_
#define __StSeekBar_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGL/StGLVertexBuffer.h>

/**
 * Simple seeking bar widget.
 */
class ST_LOCAL StSeekBar : public StGLWidget {

        public: //! @name public methods

    /**
     * Default constructor.
     */
    StSeekBar(StGLWidget* theParent,
              const int   theTop,
              const int   theMargin);

    /**
     * Destructor.
     */
    virtual ~StSeekBar();

    /**
     * @param theProgress - current progress from 0.0f to 1.0f;
     */
    void setProgress(const GLfloat theProgress) {
        myProgress = theProgress;
    }

    void setMoveTolerance(const int theTolerPx) {
        myMoveTolerPx = theTolerPx;
    }

    virtual void stglResize() ST_ATTR_OVERRIDE;
    virtual bool stglInit() ST_ATTR_OVERRIDE;
    virtual void stglUpdate(const StPointD_t& theCursor) ST_ATTR_OVERRIDE;
    virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on mouse click.
         * @param theMouseBtnId (const int ) - mouse button id;
         * @param theProgress (const double ) - current progress value.
         */
        StSignal<void (const int , const double )> onSeekClick;
    } signals;

        private: //! @name callback Slots (private overriders)

    void doMouseClick  (const int );
    void doMouseUnclick(const int );

        private: //! @name private methods

    void stglUpdateVertices();
    double getPointInEx(const StPointD_t& thePointZo) const;

        private:

    class StProgramSB;
    StHandle<StProgramSB> myProgram;    //!< GLSL program

    StGLVertexBuffer      myVertices;   //!< vertices VBO
    StGLVertexBuffer      myColors;     //!< colors   VBO
    GLfloat               myProgress;   //!< current progress 0..1
    int                   myProgressPx; //!< current progress - width in pixels
    int                   myClickPos;
    int                   myMoveTolerPx;

};

#endif //__StSeekBar_h_
