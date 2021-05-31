/**
 * Copyright © 1998-2010 Geometric Tools, LLC
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLMesh/StBndSphere.h>
#include <StTemplates/StArrayList.h>
#include <StGL/StGLMatrix.h>

#include <cfloat>

StBndSphere::StBndSphere()
: StBndContainer(),
  myRadius(0.0f) {
    //
}

StBndSphere::~StBndSphere() {
    //
}

void StBndSphere::init(const StArray<StGLVec3>& thePoints) {
    initWelzl(thePoints);
}

void StBndSphere::reset() {
    StBndContainer::reset();
    myCenter = StGLVec3();
    myRadius = 0.0f;
}

bool StBndSphere::isIn(const StGLVec3& thePnt) const {
    if(isVoid()) {
        return false;
    }
    StGLVec3 dV = thePnt - myCenter;
    return dV.modulus() <= myRadius;
}

void StBndSphere::enlarge(const GLfloat theTolerance) {
    if(!isVoid()) {
        myRadius += theTolerance;
    }
}

void StBndSphere::enlarge(const StGLVec3& theNewPnt) {
    if(isVoid()) {
        // setup the first point
        myCenter = theNewPnt;
        StBndContainer::setDefined();
    } else {
        StGLVec3 dV = theNewPnt - myCenter;
        GLfloat aDist = dV.modulus();
        if(aDist > myRadius) {
            // point outside the sphere, enlarge radius just enough
            myRadius = (myRadius + aDist) * 0.5f;
            // shift center toward point
            myCenter = myCenter + dV * ((aDist - myRadius) / aDist);
        }
    }
}

void StBndSphere::enlarge(const StArray<StGLVec3>& thePoints) {
    if(thePoints.size() == 0) {
        return;
    } else if(isVoid()) {
        // find first approximation (initial sphere center and diameter)
        init(thePoints);
    }

    // check that all points are in the sphere
    StGLVec3 dV;
    GLfloat aDistSquare, aDist;
    GLfloat aRadiusSquare = myRadius * myRadius;
    for(size_t aPntId = 0; aPntId < thePoints.size(); ++aPntId) {
        dV = thePoints[aPntId] - myCenter;
        aDistSquare = dV.squareModulus();
        if(aDistSquare <= aRadiusSquare) {
            // point is inside the sphere already
            continue;
        }
        // point not in sphere, so expand sphere to include it
        aDist = std::sqrt(aDistSquare);
        // enlarge radius just enough
        myRadius = (myRadius + aDist) * 0.5f;
        aRadiusSquare = myRadius * myRadius;
        // shift center toward point
        myCenter = myCenter + dV * ((aDist - myRadius) / aDist);
    }
    StBndContainer::setDefined();
}

void StBndSphere::initFast(const StArray<StGLVec3>& thePoints) {
    reset();
    if(thePoints.size() == 0) {
        return;
    }

    // find a large diameter to start with
    // first get the bounding box and extreme points for it
    StGLVec3 aMin = thePoints.getFirst();
    StGLVec3 aMax = thePoints.getFirst();
    size_t aPntMinX = 0;
    size_t aPntMinY = 0;
    size_t aPntMinZ = 0;
    size_t aPntMaxX = 0;
    size_t aPntMaxY = 0;
    size_t aPntMaxZ = 0;
    for(size_t aPntId = 0; aPntId < thePoints.size(); ++aPntId) {
        const StGLVec3& aPnt = thePoints[aPntId];
        if(aPnt.x() < aMin.x()) {
            aMin.x() = aPnt.x();
            aPntMinX = aPntId;
        } else if(aPnt.x() > aMax.x()) {
            aMax.x() = aPnt.x();
            aPntMaxX = aPntId;
        }
        if(aPnt.y() < aMin.y()) {
            aMin.y() = aPnt.y();
            aPntMinY = aPntId;
        } else if(aPnt.y() > aMax.y()) {
            aMax.y() = aPnt.y();
            aPntMaxY = aPntId;
        }
        if(aPnt.z() < aMin.z()) {
            aMin.z() = aPnt.z();
            aPntMinZ = aPntId;
        } else if(aPnt.z() > aMax.z()) {
            aMax.z() = aPnt.z();
            aPntMaxZ = aPntId;
        }
    }

    // select the largest extent as an initial diameter for the sphere
    StGLVec3 dVX = thePoints[aPntMaxX] - thePoints[aPntMinX];
    StGLVec3 dVY = thePoints[aPntMaxY] - thePoints[aPntMinY];
    StGLVec3 dVZ = thePoints[aPntMaxZ] - thePoints[aPntMinZ];
    GLfloat dXSquare = dVX.squareModulus();
    GLfloat dYSquare = dVY.squareModulus();
    GLfloat dZSquare = dVZ.squareModulus();
    if(dXSquare >= dYSquare && dXSquare >= dZSquare) {
        // center = midpoint of extremes
        myCenter = thePoints[aPntMinX] + (dVX * 0.5f);
        // radius squared
        myRadius = (thePoints[aPntMaxX] - myCenter).modulus();
    } else if(dYSquare >= dXSquare && dYSquare >= dZSquare) {
        // center = midpoint of extremes
        myCenter = thePoints[aPntMinY] + (dVY * 0.5f);
        // radius squared
        myRadius = (thePoints[aPntMaxY] - myCenter).modulus();
    } else {
        // center = midpoint of extremes
        myCenter = thePoints[aPntMinZ] + (dVZ * 0.5f);
        // radius squared
        myRadius = (thePoints[aPntMaxZ] - myCenter).modulus();
    }
    // now we should check that all points are in the sphere
}

/**
 * Welzl algorithm stuff.
 */
namespace {

    struct StSphere {
        StGLVec3 center;
        GLfloat  squareRadius;

        StSphere()
        : center(),
          squareRadius(0.0f) {}

        StSphere(const StGLVec3& theCenter, GLfloat theSquareRadius)
        : center(theCenter),
          squareRadius(theSquareRadius) {}

        bool isContains(const StGLVec3& thePnt, GLfloat& theDistDiff) {
            StGLVec3 dV = thePnt - center;
            theDistDiff = dV.squareModulus() - squareRadius;
            return theDistDiff <= 0.0f;
        }
    };

    /**
     * Indices of points that support current minimum volume sphere.
     */
    struct StSupport {
        size_t quantity;
        size_t indices[4];

        bool isContains(const StArrayList<StGLVec3*>& thePoints, size_t theIndex, GLfloat theSquareEpsilon) {
            for(size_t anId = 0; anId < quantity; ++anId) {
                StGLVec3 dV = *thePoints[theIndex] - *thePoints[indices[anId]];
                if(dV.squareModulus() < theSquareEpsilon) {
                    return true;
                }
            }
            return false;
        }
    };

    typedef StSphere (*updateFunction_t)(const StArrayList<StGLVec3*>& thePermuted,
                                         size_t thePntIndex,
                                         StSupport& theSupport);

    StSphere getExactSphere1(const StGLVec3& thePnt) {
        return StSphere(thePnt, 0.0f);
    }

    StSphere getExactSphere2(const StGLVec3& theP0, const StGLVec3& theP1) {
        StGLVec3 dV = theP1 - theP0;
        return StSphere(StGLVec3::getLERP(theP0, theP1, 0.5f),
                        0.25f * dV.squareModulus());
    }

    /**
       Compute the circle (in 3D) containing p0, p1, and p2.
       The Center in barycentric coordinates is K = u0*p0+u1*p1+u2*p2 where u0+u1+u2=1.
       The Center is equidistant from the three points, so
         |K-p0| = |K-p1| = |K-p2| = R
       where R is the radius of the circle.

       From these conditions,
         K-p0 = u0*A + u1*B - A
         K-p1 = u0*A + u1*B - B
         K-p2 = u0*A + u1*B
       where A = p0-p2 and B = p1-p2, which leads to
         r^2 = |u0*A+u1*B|^2 - 2*Dot(A,u0*A+u1*B) + |A|^2
         r^2 = |u0*A+u1*B|^2 - 2*Dot(B,u0*A+u1*B) + |B|^2
         r^2 = |u0*A+u1*B|^2
       Subtracting the last equation from the first two and writing
       the equations as a linear system,

       +-                 -++   -+       +-        -+
       | Dot(A,A) Dot(A,B) || u0 | = 0.5 | Dot(A,A) |
       | Dot(B,A) Dot(B,B) || u1 |       | Dot(B,B) |
       +-                 -++   -+       +-        -+

       The following code solves this system for u0 and u1, then
       evaluates the third equation in r^2 to obtain r.
     */
    StSphere getExactSphere3(const StGLVec3& theP0,
                             const StGLVec3& theP1,
                             const StGLVec3& theP2) {
        StGLVec3 aVecA = theP0 - theP2;
        StGLVec3 aVecB = theP1 - theP2;
        GLfloat AdA = aVecA.dot(aVecA);
        GLfloat AdB = aVecA.dot(aVecB);
        GLfloat BdB = aVecB.dot(aVecB);
        GLfloat aDet = AdA * BdB - AdB * AdB;
        if(std::abs(aDet) > 0.0f) {
            GLfloat m00, m01, m10, m11, d0, d1;
            if(AdA >= BdB) {
                m00 = 1.0f;
                m01 = AdB / AdA;
                m10 = m01;
                m11 = BdB / AdA;
                d0 = 0.5f;
                d1 = 0.5f * m11;
            } else {
                m00 = AdA / BdB;
                m01 = AdB / BdB;
                m10 = m01;
                m11 = 1.0f;
                d0 = 0.5f * m00;
                d1 = 0.5f;
            }
            GLfloat invDet = 1.0f / (m00 * m11 - m01 * m10);
            GLfloat u0 = invDet * (m11 * d0 - m01 * d1);
            GLfloat u1 = invDet * (m00 * d1 - m10 * d0);
            GLfloat u2 = 1.0f - u0 - u1;

            StGLVec3 aTmp = aVecA * u0 + aVecB * u1;
            return StSphere(theP0 * u0 + theP1 * u1 + theP2 * u2,
                            aTmp.squareModulus());
        } else {
            // infinite sphere
            return StSphere(StGLVec3(0.0f, 0.0f, 0.0f), FLT_MAX);
        }
    }

    /**
       Compute the sphere containing p0, p1, p2, and p3.
       The Center in barycentric coordinates is K = u0*p0+u1*p1+u2*p2+u3*p3 where u0+u1+u2+u3=1.
       The Center is equidistant from the three points, so
         |K-p0| = |K-p1| = |K-p2| = |K-p3| = R
       where R is the radius of the sphere.

       From these conditions,
         K-p0 = u0*A + u1*B + u2*C - A
         K-p1 = u0*A + u1*B + u2*C - B
         K-p2 = u0*A + u1*B + u2*C - C
         K-p3 = u0*A + u1*B + u2*C
       where A = p0-p3, B = p1-p3, and C = p2-p3 which leads to
         r^2 = |u0*A+u1*B+u2*C|^2 - 2*Dot(A,u0*A+u1*B+u2*C) + |A|^2
         r^2 = |u0*A+u1*B+u2*C|^2 - 2*Dot(B,u0*A+u1*B+u2*C) + |B|^2
         r^2 = |u0*A+u1*B+u2*C|^2 - 2*Dot(C,u0*A+u1*B+u2*C) + |C|^2
         r^2 = |u0*A+u1*B+u2*C|^2
       Subtracting the last equation from the first three and writing
       the equations as a linear system,

       +-                          -++   -+       +-        -+
       | Dot(A,A) Dot(A,B) Dot(A,C) || u0 | = 0.5 | Dot(A,A) |
       | Dot(B,A) Dot(B,B) Dot(B,C) || u1 |       | Dot(B,B) |
       | Dot(C,A) Dot(C,B) Dot(C,C) || u2 |       | Dot(C,C) |
       +-                          -++   -+       +-        -+

       If M = [A B C] is the matrix whose columns are the vectors A, B, and C;
       if D is the 3x1 column 0.5*(Dot(A,A),Dot(B,B),Dot(C,C)); and if U is
       the 3x1 column (u0,u1,u2), then the system is M^T*M*U = D.
       The system is solved in two steps:
         V = M*U = M^{-T}*D,
         U = M^{-1}*V.
       After solving the system, r^2 is computed from the fourth equation listed previously.
     */
    StSphere getExactSphere4(const StGLVec3& thePnt0,
                             const StGLVec3& thePnt1,
                             const StGLVec3& thePnt2,
                             const StGLVec3& thePnt3) {
        StGLVec3 aVecA = thePnt0 - thePnt3;
        StGLVec3 aVecB = thePnt1 - thePnt3;
        StGLVec3 aVecC = thePnt2 - thePnt3;
        StGLVec3 aVecD(aVecA.dot(aVecA), aVecB.dot(aVecB), aVecC.dot(aVecC));
        aVecD *= 0.5f;
        StGLMatrix aMat;
        aMat.setColumn(StGLVec4(aVecA, 0.0f), 0);
        aMat.setColumn(StGLVec4(aVecB, 0.0f), 1);
        aMat.setColumn(StGLVec4(aVecC, 0.0f), 2);
        StGLMatrix anInvMat;
        if(aMat.inverted(anInvMat)) {
            StGLVec4 aVecV = StGLVec4(aVecD, 0.0f) * anInvMat;
            StGLVec4 aVecU = anInvMat * aVecV;
            GLfloat anU3 = 1.0f - aVecU.x() - aVecU.y() - aVecU.z();
            StGLVec3 aTmp = aVecA * aVecU.x() + aVecB * aVecU.y() + aVecC * aVecU.z();
            return StSphere(thePnt0 * aVecU.x() + thePnt1 * aVecU.y() + thePnt2 * aVecU.z() + thePnt3 * anU3,
                            aTmp.squareModulus());
        } else {
            // infinite sphere
            return StSphere(StGLVec3(0.0f, 0.0f, 0.0f), FLT_MAX);
        }
    }

    StSphere updateSupport1(const StArrayList<StGLVec3*>& thePermuted,
                            size_t thePntIndex,
                            StSupport& theSupport) {
        const StGLVec3& aPnt0 = *thePermuted[theSupport.indices[0]];
        const StGLVec3& aPnt1 = *thePermuted[thePntIndex];
        StSphere aMinSphere = getExactSphere2(aPnt0, aPnt1);
        theSupport.quantity = 2;
        theSupport.indices[1] = thePntIndex;
        return aMinSphere;
    }

    StSphere updateSupport2(const StArrayList<StGLVec3*>& thePermuted,
                            size_t thePntIndex,
                            StSupport& theSupport) {
        const StGLVec3* point[2] = {
            thePermuted[theSupport.indices[0]], // aPnt0
            thePermuted[theSupport.indices[1]]  // aPnt1
        };
        const StGLVec3& aPnt2 = *thePermuted[thePntIndex];

        // permutations of type 2, used for calling getExactSphere2()
        const int numType2 = 2;
        const int type2[numType2][2] = {
            {0, /*2*/ 1},
            {1, /*2*/ 0}
        };

        // permutations of type 3, used for calling getExactSphere3()
        const int numType3 = 1;  // {0, 1, 2}

        StSphere aSpheres[numType2 + numType3];
        int aSphere = 0;
        int aSphereMinR = -1;
        int aSphereMinDistDiff = 0;
        GLfloat aMinRSqr = FLT_MAX;
        GLfloat aDistDiff = FLT_MAX;
        GLfloat aMinDistDiff = FLT_MAX;

        // Permutations of type 2.
        for(int aPnt = 0; aPnt < numType2; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere2(*point[type2[aPnt][0]], aPnt2);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type2[aPnt][1]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // permutations of type 3.
        aSpheres[aSphere] = getExactSphere3(*point[0], *point[1], aPnt2);
        if(aSpheres[aSphere].squareRadius < aMinRSqr) {
            aMinRSqr = aSpheres[aSphere].squareRadius;
            aSphereMinR = aSphere;
        }

        // theoretically, aSphereMinR >= 0, but floating-point round-off errors
        // can lead to aSphereMinR == -1. When this happens, the minimal sphere
        // is chosen to be the one that has the minimum absolute errors between
        // the sphere and points (barely) outside the sphere.
        if(aSphereMinR == -1) {
            aSphereMinR = aSphereMinDistDiff;
        }

        switch(aSphereMinR) {
            case 0:
                theSupport.indices[1] = thePntIndex;
                break;
            case 1:
                theSupport.indices[0] = thePntIndex;
                break;
            case 2:
                theSupport.quantity = 3;
                theSupport.indices[2] = thePntIndex;
                break;
        }
        return aSpheres[aSphereMinR];
    }

    StSphere updateSupport3(const StArrayList<StGLVec3*>& thePermuted,
                            size_t thePntIndex,
                            StSupport& theSupport) {
        const StGLVec3* point[3] = {
            thePermuted[theSupport.indices[0]], // aPnt0
            thePermuted[theSupport.indices[1]], // aPnt1
            thePermuted[theSupport.indices[2]]  // aPnt2
        };
        const StGLVec3& aPnt3 = *thePermuted[thePntIndex];

        // permutations of type 2, used for calling getExactSphere2()
        const int numType2 = 3;
        const int type2[numType2][3] = {
            {0, /*3*/ 1, 2},
            {1, /*3*/ 0, 2},
            {2, /*3*/ 0, 1}
        };

        // permutations of type 3, used for calling getExactSphere3()
        const int numType3 = 3;
        const int type3[numType3][3] = {
            {0, 1, /*3*/ 2},
            {0, 2, /*3*/ 1},
            {1, 2, /*3*/ 0}
        };

        // permutations of type 4, used for calling getExactSphere4()
        const int numType4 = 1;  // {0, 1, 2, 3}

        StSphere aSpheres[numType2 + numType3 + numType4];
        int aSphere = 0;
        int aSphereMinR = -1;
        int aSphereMinDistDiff = 0;
        GLfloat aMinRSqr = FLT_MAX;
        GLfloat aDistDiff = FLT_MAX;
        GLfloat aMinDistDiff = FLT_MAX;

        // permutations of type 2
        for(int aPnt = 0; aPnt < numType2; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere2(*point[type2[aPnt][0]],
                                                 aPnt3);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type2[aPnt][1]], aDistDiff)
                && aSpheres[aSphere].isContains(*point[type2[aPnt][2]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // permutations of type 3
        for(int aPnt = 0; aPnt < numType3; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere3(*point[type3[aPnt][0]],
                                                *point[type3[aPnt][1]],
                                                 aPnt3);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type3[aPnt][2]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // permutations of type 4
        aSpheres[aSphere] = getExactSphere4(*point[0],
                                            *point[1],
                                            *point[2],
                                             aPnt3);
        if(aSpheres[aSphere].squareRadius < aMinRSqr) {
            aMinRSqr = aSpheres[aSphere].squareRadius;
            aSphereMinR = aSphere;
        }

        // theoretically, aSphereMinR >= 0, but floating-point round-off errors
        // can lead to aSphereMinR == -1.  When this happens, the minimal sphere
        // is chosen to be the one that has the minimum absolute errors between
        // the sphere and points (barely) outside the sphere.
        if(aSphereMinR == -1) {
            aSphereMinR = aSphereMinDistDiff;
        }

        switch(aSphereMinR) {
            case 0:
                theSupport.quantity = 2;
                theSupport.indices[1] = thePntIndex;
                break;
            case 1:
                theSupport.quantity = 2;
                theSupport.indices[0] = thePntIndex;
                break;
            case 2:
                theSupport.quantity = 2;
                theSupport.indices[0] = theSupport.indices[2];
                theSupport.indices[1] = thePntIndex;
                break;
            case 3:
                theSupport.indices[2] = thePntIndex;
                break;
            case 4:
                theSupport.indices[1] = thePntIndex;
                break;
            case 5:
                theSupport.indices[0] = thePntIndex;
                break;
            case 6:
                theSupport.quantity = 4;
                theSupport.indices[3] = thePntIndex;
                break;
        }
        return aSpheres[aSphereMinR];
    }

    StSphere updateSupport4(const StArrayList<StGLVec3*>& thePermuted,
                            size_t thePntIndex,
                            StSupport& theSupport) {
        const StGLVec3* point[4] = {
            thePermuted[theSupport.indices[0]],  // aPnt0
            thePermuted[theSupport.indices[1]],  // aPnt1
            thePermuted[theSupport.indices[2]],  // aPnt2
            thePermuted[theSupport.indices[3]]   // aPnt3
        };
        const StGLVec3& aPnt4 = *thePermuted[thePntIndex];

        // Permutations of type 2, used for calling ExactSphere2(...).
        const int numType2 = 4;
        const int type2[numType2][4] = {
            {0, /*4*/ 1, 2, 3},
            {1, /*4*/ 0, 2, 3},
            {2, /*4*/ 0, 1, 3},
            {3, /*4*/ 0, 1, 2}
        };

        // Permutations of type 3, used for calling ExactSphere3(...).
        const int numType3 = 6;
        const int type3[numType3][4] = {
            {0, 1, /*4*/ 2, 3},
            {0, 2, /*4*/ 1, 3},
            {0, 3, /*4*/ 1, 2},
            {1, 2, /*4*/ 0, 3},
            {1, 3, /*4*/ 0, 2},
            {2, 3, /*4*/ 0, 1}
        };

        // Permutations of type 4, used for calling ExactSphere4(...).
        const int numType4 = 4;
        const int type4[numType4][4] = {
            {0, 1, 2, /*4*/ 3},
            {0, 1, 3, /*4*/ 2},
            {0, 2, 3, /*4*/ 1},
            {1, 2, 3, /*4*/ 0}
        };

        StSphere aSpheres[numType2 + numType3 + numType4];
        int aSphere = 0;
        int aSphereMinR = -1;
        int aSphereMinDistDiff = -1;
        GLfloat aMinRSqr = FLT_MAX;
        GLfloat aDistDiff = FLT_MAX;
        GLfloat aMinDistDiff = FLT_MAX;

        // permutations of type 2
        for(int aPnt = 0; aPnt < numType2; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere2(*point[type2[aPnt][0]],
                                                 aPnt4);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type2[aPnt][1]], aDistDiff)
                && aSpheres[aSphere].isContains(*point[type2[aPnt][2]], aDistDiff)
                && aSpheres[aSphere].isContains(*point[type2[aPnt][3]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // permutations of type 3
        for(int aPnt = 0; aPnt < numType3; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere3(*point[type3[aPnt][0]],
                                                *point[type3[aPnt][1]],
                                                 aPnt4);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type3[aPnt][2]], aDistDiff)
                && aSpheres[aSphere].isContains(*point[type3[aPnt][3]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // permutations of type 4
        for(int aPnt = 0; aPnt < numType4; ++aPnt, ++aSphere) {
            aSpheres[aSphere] = getExactSphere4(*point[type4[aPnt][0]],
                                                *point[type4[aPnt][1]],
                                                *point[type4[aPnt][2]],
                                                 aPnt4);
            if(aSpheres[aSphere].squareRadius < aMinRSqr) {
                if(aSpheres[aSphere].isContains(*point[type4[aPnt][3]], aDistDiff)) {
                    aMinRSqr = aSpheres[aSphere].squareRadius;
                    aSphereMinR = aSphere;
                } else if(aDistDiff < aMinDistDiff) {
                    aMinDistDiff = aDistDiff;
                    aSphereMinDistDiff = aSphere;
                }
            }
        }

        // theoretically, aSphereMinR >= 0, but floating-point round-off errors
        // can lead to aSphereMinR == -1.  When this happens, the minimal sphere
        // is chosen to be the one that has the minimum absolute errors between
        // the sphere and points (barely) outside the sphere.
        if(aSphereMinR == -1) {
            aSphereMinR = aSphereMinDistDiff;
        }

        switch(aSphereMinR) {
            case 0:
                theSupport.quantity = 2;
                theSupport.indices[1] = thePntIndex;
                break;
            case 1:
                theSupport.quantity = 2;
                theSupport.indices[0] = thePntIndex;
                break;
            case 2:
                theSupport.quantity = 2;
                theSupport.indices[0] = theSupport.indices[2];
                theSupport.indices[1] = thePntIndex;
                break;
            case 3:
                theSupport.quantity = 2;
                theSupport.indices[0] = theSupport.indices[3];
                theSupport.indices[1] = thePntIndex;
                break;
            case 4:
                theSupport.quantity = 3;
                theSupport.indices[2] = thePntIndex;
                break;
            case 5:
                theSupport.quantity = 3;
                theSupport.indices[1] = thePntIndex;
                break;
            case 6:
                theSupport.quantity = 3;
                theSupport.indices[1] = theSupport.indices[3];
                theSupport.indices[2] = thePntIndex;
                break;
            case 7:
                theSupport.quantity = 3;
                theSupport.indices[0] = thePntIndex;
                break;
            case 8:
                theSupport.quantity = 3;
                theSupport.indices[0] = theSupport.indices[3];
                theSupport.indices[2] = thePntIndex;
                break;
            case 9:
                theSupport.quantity = 3;
                theSupport.indices[0] = theSupport.indices[3];
                theSupport.indices[1] = thePntIndex;
                break;
            case 10:
                theSupport.indices[3] = thePntIndex;
                break;
            case 11:
                theSupport.indices[2] = thePntIndex;
                break;
            case 12:
                theSupport.indices[1] = thePntIndex;
                break;
            case 13:
                theSupport.indices[0] = thePntIndex;
                break;
        }
        return aSpheres[aSphereMinR];
    }

};

void StBndSphere::initWelzl(const StArray<StGLVec3>& thePoints) {
    const GLfloat anEpsilon = 1e-03f;
    reset();
    if(thePoints.size() == 0) {
        return;
    }

    updateFunction_t anUpdatesList[5];
    anUpdatesList[0] = NULL;
    anUpdatesList[1] = updateSupport1;
    anUpdatesList[2] = updateSupport2;
    anUpdatesList[3] = updateSupport3;
    anUpdatesList[4] = updateSupport4;

    // create identity permutation (0, 1,..., thePoints.size() - 1)
    StArrayList<StGLVec3*> aPermuted(thePoints.size());
    for(size_t aPntId = 0; aPntId < thePoints.size(); ++aPntId) {
        // actually we will not change the input values
        aPermuted.add((StGLVec3* )&thePoints[aPntId]);
    }

    // generate random permutation, this needed for algorithm stability
    for(size_t aPntId = aPermuted.size() - 1; aPntId < size_t(-1); --aPntId) {
        size_t aRndId = rand() % (aPntId + 1);
        if(aRndId != aPntId) {
            // swap pointers
            StGLVec3* aTmp = aPermuted[aPntId];
            aPermuted[aPntId] = aPermuted[aRndId];
            aPermuted[aRndId] = aTmp;
        }
    }

    StSphere aMinSphere = getExactSphere1(*aPermuted[0]);
    StSupport aSupport;
    aSupport.quantity = 1;
    aSupport.indices[0] = 0;
    GLfloat dummy;
    for(size_t aPntId = 1; aPntId < aPermuted.size(); ) {
        if(!aSupport.isContains(aPermuted, aPntId, anEpsilon)
        && !aMinSphere.isContains(*aPermuted[aPntId], dummy)) {
            updateFunction_t anUpdateFunc = anUpdatesList[aSupport.quantity];
            StSphere aSphere = anUpdateFunc(aPermuted, aPntId, aSupport);
            if(aSphere.squareRadius > aMinSphere.squareRadius) {
                aMinSphere = aSphere;
                aPntId = 0;
                continue;
            }
        }
        ++aPntId;
    }

    myCenter = aMinSphere.center;
    GLfloat aWelzlRadius = std::sqrt(aMinSphere.squareRadius);
    myRadius = aWelzlRadius;
    StBndContainer::setDefined();

    /// TODO (Kirill Gavrilov#9) check the algorithm implementation for errors
    // now we should check that all points are in the sphere
}
