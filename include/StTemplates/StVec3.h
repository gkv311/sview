/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StVec3_h_
#define __StVec3_h_

#include <StTemplates/StVec2.h>
#include <StStrings/StString.h>

/**
 * Access to the components as vector methods.
 */
#define ST_VEC_COMPONENTS_3D(theX, theY, theZ) \
    const StVec3<Element_t> theX##theY##theZ() const { return StVec3<Element_t>(theX(), theY(), theZ()); } \
    const StVec3<Element_t> theX##theZ##theY() const { return StVec3<Element_t>(theX(), theZ(), theY()); } \
    const StVec3<Element_t> theY##theX##theZ() const { return StVec3<Element_t>(theY(), theX(), theZ()); } \
    const StVec3<Element_t> theY##theZ##theX() const { return StVec3<Element_t>(theY(), theZ(), theX()); } \
    const StVec3<Element_t> theZ##theY##theX() const { return StVec3<Element_t>(theZ(), theY(), theX()); } \
    const StVec3<Element_t> theZ##theX##theY() const { return StVec3<Element_t>(theZ(), theX(), theY()); }

/**
 * Generic 3-components vector.
 * To be used as RGB color pixel or XYZ 3D-point.
 */
template<typename Element_t>
class StVec3 {

        private:

    Element_t v[3]; //!< define the vector as array to avoid alignment

        public:

    /**
     * Returns the number of components.
     */
    static size_t length() {
        return 3;
    }

    /**
     * Empty constructor. Construct the zero vector.
     */
    StVec3() {
        stMemSet(this, 0, sizeof(StVec3));
    }

    /**
     * Initialize ALL components of vector within specified value.
     */
    explicit StVec3(Element_t xyz) {
        v[0] = v[1] = v[2] = xyz;
    }

    /**
     * Per-component constructor.
     */
    explicit StVec3(Element_t x, Element_t y, Element_t z) {
        v[0] = x; v[1] = y; v[2] = z;
    }

    /**
     * Constructor from 2-components vector.
     */
    explicit StVec3(const StVec2<Element_t>& vec2) {
        v[0] = vec2[0]; v[1] = vec2[1]; v[2] = 0.0f;
    }

    /**
     * Copy constructor.
     */
    StVec3(const StVec3& vec3) {
        stMemCpy(this, &vec3, sizeof(StVec3));
    }

    /**
     * Assignment operator.
     */
    const StVec3& operator=(const StVec3& vec3) {
        stMemCpy(this, &vec3, sizeof(StVec3));
        return *this;
    }

    Element_t x() const { return v[0]; }
    Element_t r() const { return v[0]; } // Red color

    Element_t y() const { return v[1]; }
    Element_t g() const { return v[1]; } // Green color

    Element_t z() const { return v[2]; }
    Element_t b() const { return v[2]; } // Blue color

    ST_VEC_COMPONENTS_2D(x, y);
    ST_VEC_COMPONENTS_2D(x, z);
    ST_VEC_COMPONENTS_2D(y, z);
    ST_VEC_COMPONENTS_3D(x, y, z);
    StVec2<Element_t>& xy() { return *((StVec2<Element_t>* )&v[0]); }
    StVec2<Element_t>& yz() { return *((StVec2<Element_t>* )&v[1]); }

    Element_t& x() { return v[0]; }
    Element_t& r() { return v[0]; } // Red color

    Element_t& y() { return v[1]; }
    Element_t& g() { return v[1]; } // Green color

    Element_t& z() { return v[2]; }
    Element_t& b() { return v[2]; } // Blue color

    /**
     * Row access to the data (for OpenGL exchange).
     */
    const Element_t* getData() const { return v; }
    operator const Element_t*() const { return v; }
    operator Element_t*() { return v; }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool isEqual(const StVec3& theOther) const {
        return v[0] == theOther.v[0]
            && v[1] == theOther.v[1]
            && v[2] == theOther.v[2];
    }

    /**
     * Check this vector with another vector for equality (without tolerance!).
     */
    bool operator==(const StVec3& theOther)       { return isEqual(theOther); }
    bool operator==(const StVec3& theOther) const { return isEqual(theOther); }

    /**
     * Check this vector with another vector for non-equality (without tolerance!).
     */
    bool operator!=(const StVec3& theOther)       { return !isEqual(theOther); }
    bool operator!=(const StVec3& theOther) const { return !isEqual(theOther); }

    /**
     * Compute per-component summary.
     */
    StVec3& operator+=(const StVec3& add) {
        v[0] += add.v[0];
        v[1] += add.v[1];
        v[2] += add.v[2];
        return *this;
    }

    /**
     * Compute per-component summary.
     */
    friend StVec3 operator+(const StVec3& left, const StVec3& right) {
        StVec3 summ = StVec3(left);
        return summ += right;
    }

    /**
     * Unary -.
     */
    StVec3 operator-() const {
        return StVec3(-x(), -y(), -z());
    }

    /**
     * Compute per-component subtraction.
     */
    StVec3& operator-=(const StVec3& dec) {
        v[0] -= dec.v[0];
        v[1] -= dec.v[1];
        v[2] -= dec.v[2];
        return *this;
    }

    /**
     * Compute per-component subtraction.
     */
    friend StVec3 operator-(const StVec3& left, const StVec3& right) {
        StVec3 summ = StVec3(left);
        return summ -= right;
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    void multiply(Element_t factor) {
        v[0] *= factor;
        v[1] *= factor;
        v[2] *= factor;
    }

    /**
     * Compute per-component multiplication.
     */
    StVec3& operator*=(const StVec3& right) {
        v[0] *= right.v[0];
        v[1] *= right.v[1];
        v[2] *= right.v[2];
        return *this;
    }

    /**
     * Compute per-component multiplication.
     */
    friend StVec3 operator*(const StVec3& left, const StVec3& right) {
        StVec3 result = StVec3(left);
        return result *= right;
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec3& operator*=(Element_t factor) {
        multiply(factor);
        return *this;
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec3 operator*(Element_t factor) const {
        return multiplied(factor);
    }

    /**
     * Compute per-component multiplication by scale factor.
     */
    StVec3 multiplied(Element_t factor) const {
        StVec3 copyVec3(*this);
        copyVec3 *= factor;
        return copyVec3;
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec3& operator/=(Element_t invFactor) {
        v[0] /= invFactor;
        v[1] /= invFactor;
        v[2] /= invFactor;
        return *this;
    }

    /**
     * Compute per-component division by scale factor.
     */
    StVec3 operator/(Element_t invFactor) {
        StVec3 result(*this);
        return result /= invFactor;
    }

    /**
     * Computes the dot product.
     */
    Element_t dot(const StVec3& theOther) const {
        return x() * theOther.x() + y() * theOther.y() + z() * theOther.z();
    }

    /**
     * Computes the vector modulus (magnitude, length).
     */
    Element_t modulus() const {
        return std::sqrt(x() * x() + y() * y() + z() * z());
    }

    /**
     * Computes the square of vector modulus (magnitude, length).
     * This method may be used for performance tricks.
     */
    Element_t squareModulus() const {
        return x() * x() + y() * y() + z() * z();
    }

    /**
     * Normalize the vector.
     */
    void normalize() {
        Element_t aModulus = modulus();
        if(aModulus != Element_t(0)) { // just avoid divide by zero
            x() = x() / aModulus;  y() = y() / aModulus;  z() = z() / aModulus;
        }
    }

    /**
     * Normalize the vector.
     */
    StVec3 normalized() const {
        StVec3 aCopy(*this);
        aCopy.normalize();
        return aCopy;
    }

    /**
     * Computes the cross product.
     */
    static StVec3 cross(const StVec3& theVec1, const StVec3& theVec2) {
        return StVec3(theVec1.y() * theVec2.z() - theVec1.z() * theVec2.y(),
                      theVec1.z() * theVec2.x() - theVec1.x() * theVec2.z(),
                      theVec1.x() * theVec2.y() - theVec1.y() * theVec2.x());
    }

    StString toString() const {
        return StString('(') + x() + "; " + y() + "; " + z() + ')';
    }

    /**
     * Return component-wise minimum of two vectors.
     */
    StVec3 cwiseMin(const StVec3& theVec) const {
        return StVec3(v[0] < theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] < theVec.v[1] ? v[1] : theVec.v[1],
                      v[2] < theVec.v[2] ? v[2] : theVec.v[2]);
    }

    /**
     * Return component-wise maximum of two vectors.
     */
    StVec3 cwiseMax(const StVec3& theVec) const {
        return StVec3(v[0] > theVec.v[0] ? v[0] : theVec.v[0],
                      v[1] > theVec.v[1] ? v[1] : theVec.v[1],
                      v[2] > theVec.v[2] ? v[2] : theVec.v[2]);
    }

    /**
     * Return component-wise modulus of the vector.
     */
    StVec3 cwiseAbs() const {
        return StVec3(std::abs (v[0]), std::abs (v[1]), std::abs (v[2]));
    }

    /**
     * Return maximum component of the vector.
     */
    Element_t maxComp() const {
        return v[0] > v[1] ? (v[0] > v[2] ? v[0] : v[2])
                           : (v[1] > v[2] ? v[1] : v[2]);
    }

    /**
     * Return minimum component of the vector.
     */
    Element_t minComp() const {
        return v[0] < v[1] ? (v[0] < v[2] ? v[0] : v[2])
                           : (v[1] < v[2] ? v[1] : v[2]);
    }

    /**
     * Compute linear interpolation between to vectors.
     * @param theT (const Element_t ) - interpolation coefficient 0..1;
     * @return interpolation result.
     */
    static StVec3 getLERP(const StVec3& theFrom, const StVec3& theTo, const Element_t theT) {
        return theFrom * (Element_t(1) - theT) + theTo * theT;
    }

    static StVec3 DX() {
        return StVec3(Element_t(1), Element_t(0), Element_t(0));
    }

    static StVec3 DY() {
        return StVec3(Element_t(0), Element_t(1), Element_t(0));
    }

    static StVec3 DZ() {
        return StVec3(Element_t(0), Element_t(0), Element_t(1));
    }

};

/**
 * Optimized version.
 */
template<>
inline StVec3<float>& StVec3<float>::operator/=(float invFactor) {
    multiply(1.0f / invFactor);
    return *this;
}

template<>
inline StVec3<double>& StVec3<double>::operator/=(double invFactor) {
    multiply(1.0 / invFactor);
    return *this;
}

#endif //__StVec3_h_
