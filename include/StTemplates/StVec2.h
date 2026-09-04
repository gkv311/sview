/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StVec2_h_
#define __StVec2_h_

#include <stTypes.h>

#ifdef __cplusplus

#define ST_VEC_COMPONENTS_2D(theX, theY) \
    constexpr const StVec2<Element_t> theX##theY() const noexcept { return StVec2<Element_t>(theX(), theY()); } \
    constexpr const StVec2<Element_t> theY##theX() const noexcept { return StVec2<Element_t>(theY(), theX()); }

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
    static constexpr size_t length() noexcept {
        return 2;
    }

    /**
     * Empty constructor. Construct the zero vector.
     */
    constexpr StVec2() noexcept : v{} {
        //
    }

    /**
     * Initialize ALL components of vector within specified value.
     */
    explicit constexpr StVec2(Element_t xy) noexcept : v{xy, xy} {
        //
    }

    /**
     * Per-component constructor.
     */
    explicit constexpr StVec2(Element_t x, Element_t y) noexcept : v{x, y} {
        //
    }

    /**
     * Copy constructor.
     */
    constexpr StVec2(const StVec2& vec2) noexcept : v{vec2.v[0], vec2.v[1]} {
        //
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
    constexpr Element_t x() const noexcept { return v[0]; }
    constexpr Element_t y() const noexcept { return v[1]; }

    ST_VEC_COMPONENTS_2D(x, y);

    /**
     * Per-component access (for modifications).
     */
    Element_t& x() { return v[0]; }
    Element_t& y() { return v[1]; }

#if defined(_MSC_VER) && (_MSC_VER < 1930)
    //! Access element by index within [0,1] range - workaround for old msvc.
    const Element_t& operator[](int theIndex) const { return v[theIndex]; }
          Element_t& operator[](int theIndex)       { return v[theIndex]; }
#endif

    typedef Element_t CArray_t[2];

    /**
     * Row access to the data (to simplify OpenGL exchange).
     */
    const CArray_t&  getData() const { return v; }
    operator const CArray_t&() const { return v; }
    operator       CArray_t&()       { return v; }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool isEqual(const StVec2& theOther) const {
        return v[0] == theOther.v[0]
            && v[1] == theOther.v[1];
    }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool operator==(const StVec2& theOther)       { return isEqual(theOther); }
    bool operator==(const StVec2& theOther) const { return isEqual(theOther); }

    /**
     * Check this vector with another vector for non-equality (without tolerance!).
     */
    bool operator!=(const StVec2& theOther)       { return !isEqual(theOther); }
    bool operator!=(const StVec2& theOther) const { return !isEqual(theOther); }

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
     * Computes the vector modulus (magnitude, length).
     */
    Element_t modulus() const {
        return std::sqrt(x() * x() + y() * y());
    }

    /**
     * Computes the square of vector modulus (magnitude, length).
     * This method may be used for performance tricks.
     */
    Element_t squareModulus() const {
        return x() * x() + y() * y();
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

    /**
     * Return component-wise minimum of two vectors.
     */
    StVec2 cwiseMin(const StVec2& theVec) const {
        return StVec2(v[0] < theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] < theVec.v[1] ? v[1] : theVec.v[1]);
    }

    /**
     * Return component-wise maximum of two vectors.
     */
    StVec2 cwiseMax(const StVec2& theVec) const {
        return StVec2(v[0] > theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] > theVec.v[1] ? v[1] : theVec.v[1]);
    }

    /**
     * Compute component-wise modulus of the vector.
     */
    StVec2 cwiseAbs() const {
        return StVec2(std::abs(v[0]), std::abs(v[1]));
    }

    /**
     * Compute maximum component of the vector.
     */
    constexpr Element_t maxComp() const noexcept {
        return v[0] > v[1] ? v[0] : v[1];
    }

    /**
     * Compute minimum component of the vector.
     */
    constexpr Element_t minComp() const noexcept {
        return v[0] < v[1] ? v[0] : v[1];
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
