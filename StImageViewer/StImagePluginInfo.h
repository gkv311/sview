/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * StImageViewer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StImageViewer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StImagePluginInfo_h_
#define __StImagePluginInfo_h_

/**
 *.jps - jpeg stereo image file (SideBySide or CrossEye), lossy
 */
#define ST_JPS_MIME "image/jps"
#define ST_X_JPS_MIME "image/x-jps"
#define ST_JPS_EXT  "jps"
#define ST_JPS_DESC "JPS - jpeg stereo image, lossy"
#define ST_JPS_MIME_STRING ST_JPS_MIME ":" ST_JPS_EXT ":" ST_JPS_DESC
#define ST_X_JPS_MIME_STRING ST_X_JPS_MIME ":" ST_JPS_EXT ":" ST_JPS_DESC

/**
 *.mpo - Multi Picture Object, could be stereo image file (Separate), lossy
 */
#define ST_MPO_MIME "image/mpo"
#define ST_X_MPO_MIME "image/x-mpo"
#define ST_MPO_EXT  "mpo"
#define ST_MPO_DESC "MPO - Multi Picture Object, lossy"
#define ST_MPO_MIME_STRING ST_MPO_MIME ":" ST_MPO_EXT ":" ST_MPO_DESC
#define ST_X_MPO_MIME_STRING ST_X_MPO_MIME ":" ST_MPO_EXT ":" ST_MPO_DESC

/**
 *.pns - PNG stereo image file (SideBySide or CrossEye), lossless
 */
#define ST_PNS_MIME "image/pns"
#define ST_X_PNS_MIME "image/x-pns"
#define ST_PNS_EXT  "pns"
#define ST_PNS_DESC "PNS - png stereo image, lossless"
#define ST_PNS_MIME_STRING ST_PNS_MIME ":" ST_PNS_EXT ":" ST_PNS_DESC
#define ST_X_PNS_MIME_STRING ST_X_PNS_MIME ":" ST_PNS_EXT ":" ST_PNS_DESC

/**
 *.jpg, *.jpeg, *.jpe - Joint Photographic Experts Group image file, lossy
 *.jp2, *.j2k - JPEG 2000 image, lossy
 */
#define ST_JPG_MIME "image/jpg"
#define ST_JPG_EXT  "jpg"
#define ST_JPEG_DESC "JPEG/JIFF (Joint Photographic Experts Group) image, lossy"
#define ST_JPE_MIME "image/x-jpe"
#define ST_JPE_EXT  "jpe"
#define ST_JPEG_MIME "image/jpeg"
#define ST_JPEG_EXT  "jpeg"
#define ST_JP2_MIME "image/x-jp2"
#define ST_JP2_EXT  "jp2"
#define ST_JP2_DESC "J2K - JPEG 2000 image, lossy"
#define ST_J2K_MIME "image/x-j2k"
#define ST_J2K_EXT  "j2k"
#define ST_J2K_DESC "J2K - JPEG 2000 image, lossy"
#define ST_JXL_MIME "image/jxl"
#define ST_JXL_EXT  "jxl"
#define ST_JXL_DESC "JPEG XL image"

/**
 *.insp - Insta360 Image (jpeg unstitched panorama)
 */
#define ST_INSP_MIME "image/x-insp"
#define ST_INSP_EXT  "insp"
#define ST_INSP_DESC "INSP - Insta360 Image (JPEG)"

/**
 *.png - Portable Network Graphics image file, lossless
 */
#define ST_PNG_MIME "image/x-png"
#define ST_PNG_EXT  "png"
#define ST_PNG_DESC "PNG - Portable Network Graphics image, lossless"

/**
 *.bmp - BitMap image file, lossless
 */
#define ST_BMP_MIME "image/x-bmp"
#define ST_BMP_EXT  "bmp"
#define ST_BMP_DESC "BMP - bitmap image, lossless"

/**
 *.gif - Graphical Interchange Format image, lossy
 */
#define ST_GIF_MIME "image/x-gif"
#define ST_GIF_EXT  "gif"
#define ST_GIF_DESC "GIF - Graphical Interchange Format image or animation, lossy"

/**
 *.tiff - Tagged Image File Format, lossy or lossless, could be multipaged
 */
#define ST_TIF_MIME "image/x-tif"
#define ST_TIF_EXT  "tif"
#define ST_TIFF_DESC "TIFF - Tagged Image File Format, lossy or lossless"
#define ST_TIFF_MIME "image/x-tiff"
#define ST_TIFF_EXT  "tiff"

/**
 *.tga - Truevision Targa Graphic image, lossless
 */
#define ST_TGA_MIME "image/x-tga"
#define ST_TGA_EXT  "tga"
#define ST_TGA_DESC "TGA - Truevision Targa Graphic image, lossless"

/**
 *.dds - Microsoft DirectDraw Surface
 */
#define ST_DDS_MIME "image/x-dds"
#define ST_DDS_EXT  "dds"
#define ST_DDS_DESC "DDS - Microsoft DirectDraw Surface"

/**
 *.pgm - Portable Gray Map image (transparency mask)
 *.pnm - Portable aNy Map image
 *.pbm - Portable Bit Map image
 */
#define ST_PGM_MIME "image/x-pgm"
#define ST_PGM_EXT  "pgm"
#define ST_PGM_DESC "PGM - Portable Gray Map image (transparency mask)"
#define ST_PNM_MIME "image/x-pnm"
#define ST_PNM_EXT  "pnm"
#define ST_PNM_DESC "PNM - Portable Any Map image"
#define ST_PBM_MIME "image/x-pbm"
#define ST_PBM_EXT  "pbm"
#define ST_PBM_DESC "PBM - Portable Bit Map image"

/**
 *.pcx - PCExchange image
 */
#define ST_PCX_MIME "image/x-pcx"
#define ST_PCX_EXT  "pcx"
#define ST_PCX_DESC "PCX - PCExchange image"

/**
 *.pcd - Photo CD image (was developed by Kodak)
 */
#define ST_PCD_MIME "image/x-pcd"
#define ST_PCD_EXT  "pcd"
#define ST_PCD_DESC "PCD - Photo CD image"

/**
 *.psd - Photoshop Document image
 */
#define ST_PSD_MIME "image/x-psd"
#define ST_PSD_EXT  "psd"
#define ST_PSD_DESC "PSD - Photoshop Document image"

/**
 *.iwi - Ignite image optimiser file
 */
#define ST_IWI_MIME "image/x-iwi"
#define ST_IWI_EXT  "iwi"
#define ST_IWI_DESC "IWI - Ignite image optimiser file"

/**
 *.iff - image
 */
#define ST_IFF_MIME "image/iff"
#define ST_IFF_EXT  "iff"
#define ST_IFF_DESC "IFF - image"

/**
 *.exr - OpenEXR, high dynamic-range (HDR) image file format
 */
#define ST_EXR_MIME "image/x-exr"
#define ST_EXR_EXT  "exr"
#define ST_EXR_DESC "EXR - OpenEXR, high dynamic-range (HDR) image"

/**
 *.hdr - high dynamic-range (HDR) image file
 */
#define ST_HDR_MIME "image/x-hdr"
#define ST_HDR_EXT  "hdr"
#define ST_HDR_DESC "HDR - high dynamic-range (HDR) image"

/**
 *.dpx - Digital Picture Exchange
 */
#define ST_DPX_MIME "image/x-dpx"
#define ST_DPX_EXT  "dpx"
#define ST_DPX_DESC "DPX - Digital Picture Exchange"

/**
 *.ico - Icon image (on Windows' systems)
 *.cur - Cursor image (on Windows' systems)
 *.icns - Apple Mac OS X Icon image
 */
#define ST_ICO_MIME "image/x-ico"
#define ST_ICO_EXT  "ico"
#define ST_ICO_DESC "ICO - icon image"
#define ST_CUR_MIME "image/x-cur"
#define ST_CUR_EXT  "cur"
#define ST_CUR_DESC "CUR - cursor image"
#define ST_ICNS_MIME "image/x-icns"
#define ST_ICNS_EXT  "icns"
#define ST_ICNS_DESC "ICNS - icon image"

/**
 *.webp   - Web Picture (based on VP8)
 *.webpll - Web Picture LossLess (experimental)
 */
#define ST_WEBP_MIME "image/webp"
#define ST_WEBP_EXT  "webp"
#define ST_WEBP_DESC "WebP - Web Picture (encoded with VP8)"
#define ST_WEBPLL_MIME "image/webpll"
#define ST_WEBPLL_EXT  "webpll"
#define ST_WEBPLL_DESC "WebPLL - Web LossLess Picture"

/**
 * Define total supported MIME list.
 */
#define ST_IMAGE_PLUGIN_MIME_CHAR ST_JPS_MIME_STRING ";" ST_X_JPS_MIME_STRING ";" \
ST_PNS_MIME_STRING ";" ST_X_PNS_MIME_STRING ";" \
ST_MPO_MIME_STRING ";" ST_X_MPO_MIME_STRING ";" \
ST_JPG_MIME ":" ST_JPG_EXT ":" ST_JPEG_DESC ";" \
ST_JPE_MIME ":" ST_JPE_EXT ":" ST_JPEG_DESC ";" \
ST_JPEG_MIME ":" ST_JPEG_EXT ":" ST_JPEG_DESC ";" \
ST_INSP_MIME ":" ST_INSP_EXT ":" ST_INSP_DESC ";" \
ST_JP2_MIME ":" ST_JP2_EXT ":" ST_JP2_DESC ";" \
ST_J2K_MIME ":" ST_J2K_EXT ":" ST_J2K_DESC ";" \
ST_JXL_MIME ":" ST_JXL_EXT ":" ST_JXL_DESC ";" \
ST_PNG_MIME ":" ST_PNG_EXT ":" ST_PNG_DESC ";" \
ST_BMP_MIME ":" ST_BMP_EXT ":" ST_BMP_DESC ";" \
ST_GIF_MIME ":" ST_GIF_EXT ":" ST_GIF_DESC ";" \
ST_TIF_MIME ":" ST_TIF_EXT ":" ST_TIFF_DESC ";" \
ST_TIFF_MIME ":" ST_TIFF_EXT ":" ST_TIFF_DESC ";" \
ST_TGA_MIME ":" ST_TGA_EXT ":" ST_TGA_DESC ";" \
ST_DDS_MIME ":" ST_DDS_EXT ":" ST_DDS_DESC ";" \
ST_PGM_MIME ":" ST_PGM_EXT ":" ST_PGM_DESC ";" \
ST_PNM_MIME ":" ST_PNM_EXT ":" ST_PNM_DESC ";" \
ST_PBM_MIME ":" ST_PBM_EXT ":" ST_PBM_DESC ";" \
ST_PCX_MIME ":" ST_PCX_EXT ":" ST_PCX_DESC ";" \
ST_PCD_MIME ":" ST_PCD_EXT ":" ST_PCD_DESC ";" \
ST_PSD_MIME ":" ST_PSD_EXT ":" ST_PSD_DESC ";" \
ST_IWI_MIME ":" ST_IWI_EXT ":" ST_IWI_DESC ";" \
ST_IFF_MIME ":" ST_IFF_EXT ":" ST_IFF_DESC ";" \
ST_EXR_MIME ":" ST_EXR_EXT ":" ST_EXR_DESC ";" \
ST_HDR_MIME ":" ST_HDR_EXT ":" ST_HDR_DESC ";" \
ST_DPX_MIME ":" ST_DPX_EXT ":" ST_DPX_DESC ";" \
ST_ICO_MIME ":" ST_ICO_EXT ":" ST_ICO_DESC ";" \
ST_CUR_MIME ":" ST_CUR_EXT ":" ST_CUR_DESC ";" \
ST_WEBP_MIME   ":" ST_WEBP_EXT   ":" ST_WEBP_DESC   ";" \
ST_WEBPLL_MIME ":" ST_WEBPLL_EXT ":" ST_WEBPLL_DESC ";" \
ST_ICNS_MIME ":" ST_ICNS_EXT ":" ST_ICNS_DESC \
"\000"

#endif //__StImagePluginInfo_h_
