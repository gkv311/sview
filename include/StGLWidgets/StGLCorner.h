/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009, 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLCorner_h_
#define __StGLCorner_h_

#include <StTemplates/StRect.h>

#if defined(_MSC_VER)
  // strongly typed enumerations is part of C++0x
  // however old MSVC detects this as extension for CLR layer
  // and show this warning
  #pragma warning(disable : 4480)
#endif

#ifndef __APPLE__
typedef enum tagStGLVCorner : unsigned char {
#else
typedef enum tagStGLVCorner {
#endif
    ST_VCORNER_TOP,    //!< top    position, default
    ST_VCORNER_CENTER, //!< center position
    ST_VCORNER_BOTTOM, //!< bottom position
} StGLVCorner;

#ifndef __APPLE__
typedef enum tagStGLHCorner : unsigned char {
#else
typedef enum tagStGLHCorner {
#endif
    ST_HCORNER_LEFT,   //!< left   position, default
    ST_HCORNER_CENTER, //!< center position
    ST_HCORNER_RIGHT,  //!< right  position
} StGLHCorner;

/**
 * Struct to store alignment rules.
 */
struct StGLCorner {

    unsigned char v; //!< vertical   alignment
    unsigned char h; //!< horizontal alignment

    StGLCorner(const StGLVCorner theVCorner,
               const StGLHCorner theHCorner)
    : v(theVCorner),
      h(theHCorner) {}

};

#endif //__StGLCorner_h_
