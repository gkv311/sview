/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StQuaternion_h_
#define __StQuaternion_h_

#include <StTemplates/StVec4.h>

template<typename Element_t>
class StQuaternion {

        public:

    /**
     * Construct identity quaternion.
     */
    StQuaternion() : myV((Element_t )0, (Element_t )0, (Element_t )0, (Element_t )1) {}

    /**
     * Construct quaternion from component values.
     */
    StQuaternion(const Element_t theX, const Element_t theY, const Element_t theZ, const Element_t theW)
    : myV(theX, theY, theZ, theW) {}

    /**
     * Construct orientation as rotation around axis.
     */
    StQuaternion(const StVec3<Element_t>& theAxis,
                 const Element_t          theAngle) {
        setVectorAndAngle(theAxis, theAngle);
    }

    /**
     * Setup orientation as rotation around axis.
     */
    void setVectorAndAngle(const StVec3<Element_t>& theAxis,
                           const Element_t          theAngle) {
        StVec3<Element_t> anAxis = theAxis.normalized();
        Element_t anAngleHalf = Element_t(0.5) * theAngle;
        Element_t aSin        = std::sin(anAngleHalf);
        myV.x() = anAxis.x() * aSin;
        myV.y() = anAxis.y() * aSin;
        myV.z() = anAxis.z() * aSin;
        myV.w() = std::cos(anAngleHalf);
    }

    /**
     * Return quaternion components.
     */
    const StVec4<Element_t>& vec4() const { return myV; }

    /**
     * Get x component.
     */
    Element_t x() const { return myV.x(); }

    /**
     * Get y component.
     */
    Element_t y() const { return myV.y(); }

    /**
     * Get z component.
     */
    Element_t z() const { return myV.z(); }

    /**
     * Get w component.
     */
    Element_t w() const { return myV.w(); }

    /**
     * Multiply two quaternions (e.g. rotate object by theQ2 and then by theQ1 - order is important!).
     */
    static StQuaternion multiply(const StQuaternion& theQ1,
                                 const StQuaternion& theQ2) {
        return StQuaternion(theQ1.w() * theQ2.x() + theQ1.x() * theQ2.w() + theQ1.y() * theQ2.z() - theQ1.z() * theQ2.y(),
                            theQ1.w() * theQ2.y() + theQ1.y() * theQ2.w() + theQ1.z() * theQ2.x() - theQ1.x() * theQ2.z(),
                            theQ1.w() * theQ2.z() + theQ1.z() * theQ2.w() + theQ1.x() * theQ2.y() - theQ1.y() * theQ2.x(),
                            theQ1.w() * theQ2.w() - theQ1.x() * theQ2.x() - theQ1.y() * theQ2.y() - theQ1.z() * theQ2.z());
    }

        private:

    StVec4<Element_t> myV;

};

#endif //__StQuaternion_h_
