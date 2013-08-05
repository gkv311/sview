/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StVec2_h_
#define __StVec2_h_

#include <stTypes.h>

#ifdef __cplusplus

#define ST_VEC_COMPONENTS_2D(theX, theY) \
    const StVec2<Element_t> theX##theY() const { return StVec2<Element_t>(theX(), theY()); } \
    const StVec2<Element_t> theY##theX() const { return StVec2<Element_t>(theY(), theX()); }

/**
 * Defines the 2D-vector template.
 */
template<typename Element_t>
class StVec2 {

        private:

    Element_t v[2];

        public:

    /**
     * Returns the number of components.
     */
    static size_t length() {
        return 2;
    }

    /**
     * Empty constructor. Construct the zero vector.
     */
    StVec2() {
        v[0] = v[1] = Element_t(0);
    }

    /**
     * Initialize ALL components of vector within specified value.
     */
    explicit StVec2(Element_t xy) {
        v[0] = v[1] = xy;
    }

    /**
     * Per-component constructor.
     */
    explicit StVec2(Element_t x, Element_t y) {
        v[0] = x; v[1] = y;
    }

    /**
     * Copy constructor.
     */
    StVec2(const StVec2& vec2) {
        v[0] = vec2[0]; v[1] = vec2[1];
    }

    /**
     * Assignment operator.
     */
    const StVec2& operator=(const StVec2& vec2) {
        v[0] = vec2[0]; v[1] = vec2[1];
        return *this;
    }

    /**
     * Per-component access.
     */
    Element_t x() const { return v[0]; }
    Element_t y() const { return v[1]; }

    ST_VEC_COMPONENTS_2D(x, y);

    /**
     * Per-component access (for modifications).
     */
    Element_t& x() { return v[0]; }
    Element_t& y() { return v[1]; }

    /**
     * Row access to the data (to simplify OpenGL exchange).
     */
    const Element_t* getData() const { return v; }
    operator const Element_t*() const { return v; }
    operator Element_t*() { return v; }

    /**
     * Compute per-component summary.
     */
    StVec2& operator+=(const StVec2& theAdd) {
        v[0] += theAdd.v[0];
        v[1] += theAdd.v[1];
        return *this;
    }

    /**
     * Compute per-component summary.
     */
    friend StVec2 operator+(const StVec2& theLeft, const StVec2& theRight) {
        return StVec2(theLeft.v[0] + theRight.v[0],
                      theLeft.v[1] + theRight.v[1]);
    }

    /**
     * Compute per-component subtraction.
     */
    StVec2& operator-=(const StVec2& theDec) {
        v[0] -= theDec.v[0];
        v[1] -= theDec.v[1];
        return *this;
    }

    /**
     * Compute per-component subtraction.
     */
    friend StVec2 operator-(const StVec2& theLeft, const StVec2& theRight) {
        return StVec2(theLeft.v[0] - theRight.v[0],
                      theLeft.v[1] - theRight.v[1]);
    }

    /**
     * Unary -.
     */
    StVec2 operator-() const {
        return StVec2(-x(), -y());
    }

    /**
     * Compute per-component multiplication.
     */
    StVec2& operator*=(const StVec2& theRight) {
        v[0] *= theRight.v[0];
        v[1] *= theRight.v[1];
        return *this;
    }

    /**
     * Compute per-component multiplication.
     */
    friend StVec2 operator*(const StVec2& theLeft, const StVec2& theRight) {
        return StVec2(theLeft.v[0] * theRight.v[0],
                      theLeft.v[1] * theRight.v[1]);
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    void multiply(Element_t theFactor) {
        v[0] *= theFactor;
        v[1] *= theFactor;
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec2 multiplied(Element_t theFactor) const {
        return StVec2(v[0] * theFactor,
                      v[1] * theFactor);
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec2& operator*=(Element_t theFactor) {
        multiply(theFactor);
        return *this;
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec2& operator/=(Element_t theInvFactor) {
        v[0] /= theInvFactor;
        v[1] /= theInvFactor;
        return *this;
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec2 operator*(Element_t theFactor) const {
        return multiplied(theFactor);
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec2 operator/(Element_t theInvFactor) const {
        return StVec2(v[0] / theInvFactor,
                      v[1] / theInvFactor);
    }

};

// help structures
typedef StVec2<int> StPointI_t;
typedef StVec2<double> StPointD_t;
#else
typedef struct tagStPointI {
    int xy[2]; // x, y
} StPointI_t;

typedef struct tagStPointD {
    double xy[2]; // x, y
} StPointD_t;
#endif

#endif //__StVec2_h_
