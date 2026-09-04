/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StVec4_h_
#define __StVec4_h_

#include <StTemplates/StVec3.h>

/**
 * Generic 4-components vector.
 * To be used as RGBA color vector or XYZW 3D-point with special W-component
 * for operations with projection / model view matrices.
 * Use this class for 3D-points carefully because declared W-component may
 * results in strange results if used without matrices.
 */
template<typename Element_t>
class StVec4 {

        private:

    Element_t v[4];

        public:

    static constexpr size_t length() noexcept {
        return 4;
    }

    /**
     * Empty constructor. Construct the zero vector.
     */
    constexpr StVec4() noexcept : v{} {
        //
    }

    /**
     * Initialize ALL components of vector within specified value.
     */
    explicit constexpr StVec4(Element_t xyzw) noexcept : v{xyzw, xyzw, xyzw, xyzw} {
        //
    }

    /**
     * Per-component constructor.
     */
    explicit constexpr StVec4(Element_t x, Element_t y, Element_t z, Element_t w) noexcept : v{x, y, z, w} {
        //
    }

    /**
     * Constructor from 2-components vector.
     */
    explicit constexpr StVec4(const StVec2<Element_t>& vec2) noexcept : v{vec2.x(), vec2.y(), Element_t(), Element_t()} {
        //
    }

    /**
     * Constructor from 3-components vector.
     */
    explicit constexpr StVec4(const StVec3<Element_t>& vec3) noexcept : v{vec3.x(), vec3.y(), vec3.z(), Element_t()} {
        //
    }

    /**
     * Constructor from 3-components vector + alpha value.
     */
    explicit constexpr StVec4(const StVec3<Element_t>& vec3, const Element_t alpha) noexcept
    : v{vec3.x(), vec3.y(), vec3.z(), alpha}  {
        //
    }

    /**
     * Copy constructor.
     */
    constexpr StVec4(const StVec4& vec4) noexcept : v{vec4.v[0], vec4.v[1], vec4.v[2], vec4.v[3]} {
        //
    }

    /**
     * Assignment operator.
     */
    const StVec4& operator=(const StVec4& vec4) {
        stMemCpy(this, &vec4, sizeof(StVec4));
        return *this;
    }

    constexpr Element_t x() const noexcept { return v[0]; }
    constexpr Element_t r() const noexcept { return v[0]; } // Red color

    constexpr Element_t y() const noexcept { return v[1]; }
    constexpr Element_t g() const noexcept { return v[1]; } // Green color

    constexpr Element_t z() const noexcept { return v[2]; }
    constexpr Element_t b() const noexcept { return v[2]; } // Blue color

    constexpr Element_t w() const noexcept { return v[3]; }
    constexpr Element_t a() const noexcept { return v[3]; } // Alpha color

    ST_VEC_COMPONENTS_2D(x, y);
    ST_VEC_COMPONENTS_2D(x, z);
    ST_VEC_COMPONENTS_2D(x, w);
    ST_VEC_COMPONENTS_2D(y, z);
    ST_VEC_COMPONENTS_2D(y, w);
    ST_VEC_COMPONENTS_2D(z, w);

    ST_VEC_COMPONENTS_3D(x, y, z);
    ST_VEC_COMPONENTS_3D(x, y, w);
    ST_VEC_COMPONENTS_3D(x, z, w);
    ST_VEC_COMPONENTS_3D(y, z, w);

    ST_VEC_COMPONENTS_3D(r, g, b);

    Element_t& x() { return v[0]; }
    Element_t& r() { return v[0]; } // Red color

    Element_t& y() { return v[1]; }
    Element_t& g() { return v[1]; } // Green color

    Element_t& z() { return v[2]; }
    Element_t& b() { return v[2]; } // Blue color

    Element_t& w() { return v[3]; }
    Element_t& a() { return v[3]; } // Alpha color

    StVec2<Element_t>& xy()  { return *((StVec2<Element_t>* )&v[0]); }
    StVec2<Element_t>& yz()  { return *((StVec2<Element_t>* )&v[1]); }
    StVec2<Element_t>& zw()  { return *((StVec2<Element_t>* )&v[2]); }
    StVec3<Element_t>& xyz() { return *((StVec3<Element_t>* )&v[0]); }
    StVec3<Element_t>& yzw() { return *((StVec3<Element_t>* )&v[1]); }
    StVec3<Element_t>& rgb() { return *((StVec3<Element_t>* )&v[0]); }


#if defined(_MSC_VER) && (_MSC_VER < 1930)
    //! Access element by index within [0,3] range - workaround for old msvc.
    const Element_t& operator[](int theIndex) const { return v[theIndex]; }
          Element_t& operator[](int theIndex)       { return v[theIndex]; }
#endif

    typedef Element_t CArray_t[4];

    /**
     * Row access to the data (to simplify OpenGL exchange).
     */
    const CArray_t&  getData() const { return v; }
    operator const CArray_t&() const { return v; }
    operator       CArray_t&()       { return v; }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool isEqual(const StVec4& theOther) const {
        return v[0] == theOther.v[0]
            && v[1] == theOther.v[1]
            && v[2] == theOther.v[2]
            && v[3] == theOther.v[3];
    }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool operator==(const StVec4& theOther)       { return isEqual(theOther); }
    bool operator==(const StVec4& theOther) const { return isEqual(theOther); }

    /**
     * Check this vector with another vector for non-equality (without tolerance!).
     */
    bool operator!=(const StVec4& theOther)       { return !isEqual(theOther); }
    bool operator!=(const StVec4& theOther) const { return !isEqual(theOther); }

    /**
     * Compute per-component summary.
     */
    StVec4& operator+=(const StVec4& add) {
        v[0] += add.v[0];
        v[1] += add.v[1];
        v[2] += add.v[2];
        v[3] += add.v[3];
        return *this;
    }

    /**
     * Compute per-component summary.
     */
    friend StVec4 operator+(const StVec4& left, const StVec4& right) {
        StVec4 summ = StVec4(left);
        return summ += right;
    }

    /**
     * Compute per-component subtraction.
     */
    StVec4& operator-=(const StVec4& dec) {
        v[0] -= dec.v[0];
        v[1] -= dec.v[1];
        v[2] -= dec.v[2];
        v[3] -= dec.v[3];
        return *this;
    }

    /**
     * Compute per-component subtraction.
     */
    friend StVec4 operator-(const StVec4& left, const StVec4& right) {
        StVec4 summ = StVec4(left);
        return summ -= right;
    }

    /**
     * Compute per-component multiplication.
     */
    StVec4& operator*=(const StVec4& right) {
        v[0] *= right.v[0];
        v[1] *= right.v[1];
        v[2] *= right.v[2];
        v[3] *= right.v[3];
        return *this;
    }

    /**
     * Compute per-component multiplication.
     */
    friend StVec4 operator*(const StVec4& left, const StVec4& right) {
        StVec4 result = StVec4(left);
        return result *= right;
    }

    /**
     * Compute per-component multiplication.
     */
    void multiply(Element_t factor) {
        v[0] *= factor;
        v[1] *= factor;
        v[2] *= factor;
        v[3] *= factor;
    }

    /**
     * Compute per-component multiplication.
     */
    StVec4& operator*=(Element_t factor) {
        multiply(factor);
        return *this;
    }

    /**
     * Compute per-component multiplication.
     */
    StVec4 operator*(Element_t factor) const {
        return multiplied(factor);
    }

    /**
     * Compute per-component multiplication.
     */
    StVec4 multiplied(Element_t factor) const {
        StVec4 copyVec4(*this);
        copyVec4 *= factor;
        return copyVec4;
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec4& operator/=(Element_t invFactor) {
        v[0] /= invFactor;
        v[1] /= invFactor;
        v[2] /= invFactor;
        v[3] /= invFactor;
        return *this;
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec4 operator/(Element_t invFactor) {
        StVec4 result(*this);
        return result /= invFactor;
    }

    /**
     * Computes the vector modulus (magnitude, length).
     */
    Element_t modulus() const {
        return std::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
    }

    /**
     * Computes the square of vector modulus (magnitude, length).
     */
    Element_t squareModulus() const {
        return x() * x() + y() * y() + z() * z() + w() * w();
    }

    StString toString() const {
        return StString('(') + x() + "; " + y() + "; " + z() + "; " + w() + ')';
    }

    /**
     * Return component-wise minimum of two vectors.
     */
    StVec4 cwiseMin(const StVec4& theVec) const {
        return StVec4(v[0] < theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] < theVec.v[1] ? v[1] : theVec.v[1],
                      v[2] < theVec.v[2] ? v[2] : theVec.v[2],
                      v[3] < theVec.v[3] ? v[3] : theVec.v[3]);
    }

    /**
     * Return component-wise maximum of two vectors.
     */
    StVec4 cwiseMax(const StVec4& theVec) const {
        return StVec4(v[0] > theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] > theVec.v[1] ? v[1] : theVec.v[1],
                      v[2] > theVec.v[2] ? v[2] : theVec.v[2],
                      v[3] > theVec.v[3] ? v[3] : theVec.v[3]);
    }

    /**
     * Return component-wise modulus of the vector.
     */
    StVec4 cwiseAbs() const {
        return StVec4(std::abs (v[0]), std::abs (v[1]), std::abs (v[2]), std::abs (v[3]));
    }

    /**
     * Return maximum component of the vector.
     */
    Element_t maxComp() const {
        const Element_t aMax1 = v[0] > v[1] ? v[0] : v[1];
        const Element_t aMax2 = v[2] > v[3] ? v[2] : v[3];
        return aMax1 > aMax2 ? aMax1 : aMax2;
    }

    /**
     * Return minimum component of the vector.
     */
    Element_t minComp() const {
        const Element_t aMin1 = v[0] < v[1] ? v[0] : v[1];
        const Element_t aMin2 = v[2] < v[3] ? v[2] : v[3];
        return aMin1 < aMin2 ? aMin1 : aMin2;
    }

};

/**
 * Optimized version.
 */
template<>
inline StVec4<float>& StVec4<float>::operator/=(float invFactor) {
    multiply(1.0f / invFactor);
    return *this;
}

template<>
inline StVec4<double>& StVec4<double>::operator/=(double invFactor) {
    multiply(1.0 / invFactor);
    return *this;
}

#endif //__StVec4_h_
