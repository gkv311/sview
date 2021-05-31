/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StRect_h_
#define __StRect_h_

#include <StTemplates/StVec2.h>
#include <StStrings/StString.h>

/**
 * Simple rectangle class template.
 */
template<typename Element_t>
class StRect {

        private:

    Element_t tblr[4]; // top, bottom, left, right

        public:

    StRect() {
        top()    = 0;
        bottom() = 0;
        left()   = 0;
        right()  = 0;
    }

    StRect(const Element_t theTop,  const Element_t theBottom,
           const Element_t theLeft, const Element_t theRight) {
        top()    = theTop;
        bottom() = theBottom;
        left()   = theLeft;
        right()  = theRight;
    }

    StRect(const StRect& theCopy) {
        stMemCpy(tblr, theCopy.tblr, sizeof(tblr));
    }

    const StRect& operator=(const StRect& theCopy) {
        stMemCpy(tblr, theCopy.tblr, sizeof(tblr));
        return (*this);
    }

    void setValues(const Element_t theTop,  const Element_t theBottom,
                   const Element_t theLeft, const Element_t theRight) {
        top()    = theTop;
        bottom() = theBottom;
        left()   = theLeft;
        right()  = theRight;
    }

    Element_t top()    const { return tblr[0]; }
    Element_t bottom() const { return tblr[1]; }
    Element_t left()   const { return tblr[2]; }
    Element_t right()  const { return tblr[3]; }

    Element_t& top()    { return tblr[0]; }
    Element_t& bottom() { return tblr[1]; }
    Element_t& left()   { return tblr[2]; }
    Element_t& right()  { return tblr[3]; }

    /**
     * @return 2D vector with top-left rectangle corner coordinates.
     */
    StVec2<Element_t> topLeft() const {
        return StVec2<Element_t>(left(), top());
    }

    /**
     * @return 2D vector with top-right rectangle corner coordinates.
     */
    StVec2<Element_t> topRight() const {
        return StVec2<Element_t>(right(), top());
    }

    /**
     * @return 2D vector with bottom-left rectangle corner coordinates.
     */
    StVec2<Element_t> bottomLeft() const {
        return StVec2<Element_t>(left(), bottom());
    }

    /**
     * @return 2D vector with bottom-right rectangle corner coordinates.
     */
    StVec2<Element_t> bottomRight() const {
        return StVec2<Element_t>(right(), bottom());
    }

    /**
     * Fill input 2D vector with top-left rectangle corner coordinates.
     * @param  theVec - vector to change (by reference!)
     * @return reference modified vector
     */
    StVec2<Element_t>& getTopLeft(StVec2<Element_t>& theVec) const {
        theVec.x() = left();
        theVec.y() = top();
        return theVec;
    }

    /**
     * Fill input 2D vector with top-right rectangle corner coordinates.
     * @param  theVec - vector to change (by reference!)
     * @return reference modified vector
     */
    StVec2<Element_t>& getTopRight(StVec2<Element_t>& theVec) const {
        theVec.x() = right();
        theVec.y() = top();
        return theVec;
    }

    /**
     * Fill input 2D vector with bottom-left rectangle corner coordinates.
     * @param  theVec - vector to change (by reference!)
     * @return reference modified vector
     */
    StVec2<Element_t>& getBottomLeft(StVec2<Element_t>& theVec) const {
        theVec.x() = left();
        theVec.y() = bottom();
        return theVec;
    }

    /**
     * Fill input 2D vector with bottom-right rectangle corner coordinates.
     * @param  theVec - vector to change (by reference!)
     * @return reference modified vector
     */
    StVec2<Element_t>& getBottomRight(StVec2<Element_t>& theVec) const {
        theVec.x() = right();
        theVec.y() = bottom();
        return theVec;
    }

    /**
     * @return width (Element_t ) - computed rectangle width.
     */
    Element_t width() const {
        return right() - left();
    }

    /**
     * @return height (Element_t ) - computed rectangle height.
     */
    Element_t height() const {
        return bottom() - top();
    }

    /**
     * @return ratio (double ) - computed rectangle ratio (width/height).
     */
    double ratio() const {
        Element_t anH = height();
        return (anH > Element_t(0.0)) ? double(width()) / double(height()) : 1.0;
    }

    /**
     * Specify new Left without affecting dimensions.
     */
    void moveLeftTo(const Element_t theLeft) {
        Element_t aWidth = width();
        left()  = theLeft;
        right() = theLeft + aWidth;
    }

    /**
     * Specify new Right without affecting dimensions.
     */
    void moveRightTo(const Element_t theRight) {
        Element_t aWidth = width();
        right() = theRight;
        left()  = theRight - aWidth;
    }

    /**
     * Specify new Top without affecting dimensions.
     */
    void moveTopTo(const Element_t theTop) {
        Element_t anH = height();
        top()    = theTop;
        bottom() = theTop + anH;
    }

    /**
     * Specify new Bottom without affecting dimensions.
     */
    void moveBottomTo(const Element_t theBottom) {
        Element_t anH = height();
        bottom() = theBottom;
        top()    = theBottom - anH;
    }

    /**
     * Specify new Top-Left without affecting dimensions.
     */
    void moveTopLeftTo(const Element_t theLeft,
                       const Element_t theTop) {
        moveLeftTo(theLeft);
        moveTopTo(theTop);
    }

    /**
     * Move rectangle by specified vector.
     * @param theVec - translation vector.
     */
    void move(const StVec2<Element_t>& theVec) {
        left()   += theVec.x();
        right()  += theVec.x();
        top()    += theVec.y();
        bottom() += theVec.y();
    }

    /**
     * Move rectangle in horizontal direction.
     * @param theVec - X translation vector.
     */
    void moveX(const Element_t& theVec) {
        left()   += theVec;
        right()  += theVec;
    }

    /**
     * Move rectangle in vertical direction.
     * @param theVec - Y translation vector.
     */
    void moveY(const Element_t& theVec) {
        top()    += theVec;
        bottom() += theVec;
    }

    /**
     * @return center point in rectangle.
     */
    StVec2<Element_t> center() const {
        return StVec2<Element_t>(left() + width()  / Element_t(2),
                                 top()  + height() / Element_t(2));
    }

    /**
     * @param thePoint (const StVec2<Element_t>& ) - point to test.
     * @return true if point IN rectangle (or on the rectangle border line).
     */
    bool isPointIn(const StVec2<Element_t>& thePoint) const {
        return thePoint.x() >= left() && thePoint.x() <= right()
            && thePoint.y() >= top()  && thePoint.y() <= bottom();
    }

    bool isOut(const StRect& theRect) const {
        return theRect.right()  < left()
            || theRect.left()   > right()
            || theRect.bottom() < top()
            || theRect.top()    > bottom();
    }

    bool operator==(const StRect& theCompare) const {
        return top()    == theCompare.top()
            && left()   == theCompare.left()
            && bottom() == theCompare.bottom()
            && right()  == theCompare.right();
    }

    bool operator!=(const StRect& theCompare) const {
        return !operator==(theCompare);
    }

    /**
     * @return string (StString ) - rectangle string representation.
     */
    StString toString() const {
        return (StString()
            + "\n"
            + " | (" + left() + ", " + top() + ") = (L, Top)\n"
            + "-------------------------------\n"
            + " |                           | \n"
            + " | (" + width() + " x " + height() + ") = W x H \n"
            + " |                           | \n"
            + "-------------------------------\n"
            + " |                           | (" + right() + ", " + bottom() + ") = (R, Bottom)\n"
        );
    }

};

typedef StRect<int>    StRectI_t;
typedef StRect<double> StRectD_t;
typedef StRect<float>  StRectF_t;

/**
 * Simple structure to define rectangular margins.
 */
struct StMarginsI {

        public:

    int left;
    int right;
    int top;
    int bottom;

        public:

    /**
     * Setup zero-margin.
     */
    StMarginsI() : left(0), right(0), top(0), bottom(0) {}

    /**
     * Setup margin to specified value for all boundaries.
     */
    void setValues(const int theValue) {
        left = right = top = bottom = theValue;
    }

    /**
     * Extend margin to specified value for all boundaries.
     */
    void extend(const int theValue) {
        left   += theValue;
        right  += theValue;
        top    += theValue;
        bottom += theValue;
    }

    bool operator==(const StMarginsI& theCompare) const {
        return top    == theCompare.top
            && left   == theCompare.left
            && bottom == theCompare.bottom
            && right  == theCompare.right;
    }

    bool operator!=(const StMarginsI& theCompare) const {
        return !operator==(theCompare);
    }

};

#endif //__StRect_h_
