/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#ifndef __StCADPluginInfo_h_
#define __StCADPluginInfo_h_

/**
 *.iges, *.igs - IGES model file
 */
#define ST_IGS_MIME  "model/iges"
#define ST_IGS_EXT  "igs"
#define ST_IGS_DESC "IGES - CAD model file"
#define ST_IGS_MIME_STRING  ST_IGS_MIME  ":" ST_IGS_EXT  ":" ST_IGS_DESC
#define ST_IGES_MIME "model/x-iges"
#define ST_IGES_EXT  "iges"
#define ST_IGES_MIME_STRING ST_IGES_MIME ":" ST_IGES_EXT ":" ST_IGS_DESC

/**
 *.step, *.stp - STEP model file
 */
#define ST_STP_MIME "model/step"
#define ST_STP_EXT  "stp"
#define ST_STP_DESC "STEP - CAD model file"
#define ST_STP_MIME_STRING  ST_STP_MIME  ":" ST_STP_EXT  ":" ST_STP_DESC
#define ST_STEP_MIME "model/x-step"
#define ST_STEP_EXT  "step"
#define ST_STEP_MIME_STRING ST_STEP_MIME ":" ST_STEP_EXT ":" ST_STP_DESC

/**
 *.brep, *.rle - BREP model file
 */
#define ST_BREP_MIME "model/brep"
#define ST_BREP_EXT  "brep"
#define ST_BREP_DESC "BREP - CAD model file (OCCT framework)"
#define ST_BREP_MIME_STRING ST_BREP_MIME ":" ST_BREP_EXT ":" ST_BREP_DESC
#define ST_RLE_MIME  "model/rle"
#define ST_RLE_EXT   "rle"
#define ST_RLE_MIME_STRING  ST_RLE_MIME  ":" ST_RLE_EXT  ":" ST_BREP_DESC

/**
 *.obj - simplest triangulation exchange format
 */
#define ST_OBJ_MIME "model/obj"
#define ST_OBJ_EXT  "obj"
#define ST_OBJ_DESC "OBJ - triangulation exchange format"
#define ST_OBJ_MIME_STRING ST_OBJ_MIME ":" ST_OBJ_EXT  ":" ST_OBJ_DESC

/**
 *.ply - triangulation file
 */
/*#define ST_PLY_MIME "model/ply"
#define ST_PLY_EXT  "ply"
#define ST_PLY_DESC "PLY - triangulation file"
#define ST_PLY_MIME_STRING ST_PLY_MIME ":" ST_PLY_EXT  ":" ST_PLY_DESC*/

/**
 *.3ds
 */
/*#define ST_3DS_MIME "model/3ds"
#define ST_3DS_EXT  "3ds"
#define ST_3DS_DESC "3DS"
#define ST_3DS_MIME_STRING ST_3DS_MIME ":" ST_3DS_EXT  ":" ST_3DS_DESC*/

/**
 * Define total supported MIME list.
 */
#define ST_CAD_PLUGIN_MIME_CHAR ST_IGS_MIME_STRING ";" ST_IGES_MIME_STRING ";" \
ST_STP_MIME_STRING  ";" ST_STEP_MIME_STRING ";" \
ST_BREP_MIME_STRING ";" ST_RLE_MIME_STRING  ";" \
ST_OBJ_MIME_STRING \
"\000"

#endif //__StCADPluginInfo_h_
