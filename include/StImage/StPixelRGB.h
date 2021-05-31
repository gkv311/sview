/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StPixelRGB_h_
#define __StPixelRGB_h_

#include <stTypes.h>
#include <StTemplates/StVec3.h>
#include <StTemplates/StVec4.h>

/**
 * Class defines packed 3-bytes RGB pixel.
 */
typedef StVec3<GLubyte> StPixelRGB;

/**
 * Class defines packed 4-bytes RGBA pixel.
 */
typedef StVec4<GLubyte> StPixelRGBA;

#endif //__StPixelRGB_h_
