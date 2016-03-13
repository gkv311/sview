/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StCADWindow_h_
#define __StCADWindow_h_

#if defined(_WIN32)
    #include <WNT_Window.hxx>
#endif
#include <Aspect_Window.hxx>

#include <stTypes.h>

/**
 * Dummy window.
 */
#if defined(_WIN32)
class StCADWindow : public WNT_Window {
#else
class StCADWindow : public Aspect_Window {
#endif

        public:

    /**
     * Empty constructor.
     */
#if defined(_WIN32)
    ST_LOCAL StCADWindow(const Aspect_Handle theHandle) : WNT_Window(theHandle),
#else
    ST_LOCAL StCADWindow() :
#endif
      myX1(0), myX2(0), myY1(0), myY2(0) {
    #if defined(_WIN32)
        myX1 = aXLeft;
        myX2 = aXRight;
        myY1 = aYTop;
        myY2 = aYBottom;
    #endif
    }

#if !defined(_WIN32)
    ST_LOCAL virtual Aspect_Drawable NativeHandle()       const Standard_OVERRIDE { return 0; }
    ST_LOCAL virtual Aspect_Drawable NativeParentHandle() const Standard_OVERRIDE { return 0; }
    ST_LOCAL virtual Aspect_FBConfig NativeFBConfig()     const Standard_OVERRIDE { return 0; }
#endif
    ST_LOCAL virtual void Map()   const Standard_OVERRIDE {}
    ST_LOCAL virtual void Unmap() const Standard_OVERRIDE {}
    ST_LOCAL virtual Aspect_TypeOfResize DoResize()  const Standard_OVERRIDE { return Aspect_TOR_UNKNOWN; }
    ST_LOCAL virtual Standard_Boolean    DoMapping() const Standard_OVERRIDE { return Standard_True; }
    ST_LOCAL virtual Standard_Boolean    IsMapped()  const Standard_OVERRIDE { return Standard_True; }
    ST_LOCAL virtual Quantity_Ratio      Ratio()     const Standard_OVERRIDE { return 1.0; }

    /**
     * Return position.
     */
    ST_LOCAL virtual void Position (Standard_Integer& X1, Standard_Integer& Y1,
                                    Standard_Integer& X2, Standard_Integer& Y2) const Standard_OVERRIDE {
        X1 = myX1;
        X2 = myX2;
        Y1 = myY1;
        Y2 = myY2;
    }

    /**
     * Set position.
     */
    ST_LOCAL void SetPosition(const int X1, const int Y1,
                              const int X2, const int Y2) {
        myX1 = X1;
        myX2 = X2;
        myY1 = Y1;
        myY2 = Y2;
    }

    /**
     * Return size.
     */
    ST_LOCAL virtual void Size(Standard_Integer& theWidth, Standard_Integer& theHeight) const Standard_OVERRIDE {
        theWidth  = myX2 - myX1;
        theHeight = myY2 - myY1;
    }

    /**
     * Set new size.
     * @return true if size has been changed
     */
    ST_LOCAL bool SetSize(const int theWidth, const int theHeight) {
        int aNewX2 = myX1 + theWidth;
        int aNewY2 = myY1 + theHeight;
        if(aNewX2 == myX2
        && aNewY2 == myY2) {
            return false;
        }
        myX2 = aNewX2;
        myY2 = aNewY2;
        return true;
    }

        private:

    int myX1;
    int myX2;
    int myY1;
    int myY2;

        public:

    DEFINE_STANDARD_RTTI_INLINE(StCADWindow, Aspect_Window)

};

DEFINE_STANDARD_HANDLE(StCADWindow, Aspect_Window)

#endif // __StCADWindow_h_
