/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StVideoPluginInfo_h_
#define __StVideoPluginInfo_h_

/**
 *.mkv  - Matroska Video (http://www.bunkus.org/videotools/mkvtoolnix/index.html)
 */
#define ST_MKV_MIME "video/x-matroska" // "video/x-mkv"
#define ST_MKV_EXT  "mkv"
#define ST_MKV_DESC "MKV - Matroska Video"
#define ST_MKV_MIME_STRING ST_MKV_MIME ":" ST_MKV_EXT ":" ST_MKV_DESC

/**
 *.webm - MKV subset for internet
 */
#define ST_WEBM_MIME "video/webm"
#define ST_WEBM_EXT  "webm"
#define ST_WEBM_MIME_STRING ST_WEBM_MIME ":" ST_WEBM_EXT ":" ST_MKV_DESC

/**
 *.mka - Matroska container for audio only (just another file extension)
 */
#define ST_MKA_MIME "audio/x-matroska" // "audio/x-mka"
#define ST_MKA_EXT  "mka"
#define ST_MKA_MIME_STRING ST_MKA_MIME ":" ST_MKA_EXT ":" ST_MKV_DESC

/**
 *.mk3d - Matroska container for stereoscopic video (just another file extension)
 */
#define ST_MK3D_MIME "video/x-matroska-3d"
#define ST_MK3D_EXT  "mk3d"
#define ST_MK3D_MIME_STRING ST_MK3D_MIME ":" ST_MK3D_EXT ":" ST_MKV_DESC

/**
 *.avs  - Avisynth Script (http://avisynth.org)
 */
#define ST_AVS_MIME "video/x-avs"
#define ST_AVS_EXT  "avs"
#define ST_AVS_DESC "AVS - Avisynth Script"
#define ST_AVS_MIME_STRING ST_AVS_MIME ":" ST_AVS_EXT ":" ST_AVS_DESC

/**
 *.ogm  - OGG Movie
 */
#define ST_OGM_MIME "video/x-ogm"
#define ST_OGM_EXT  "ogm"
#define ST_OGM_DESC "OGM - OGG Movie"
#define ST_OGM_MIME_STRING ST_OGM_MIME ":" ST_OGM_EXT ":" ST_OGM_DESC

/**
 *.ogv  - OGG Video
 */
#define ST_OGV_MIME "video/ogg"
#define ST_OGV_EXT  "ogv"
#define ST_OGV_DESC "OGV - OGG Video"
#define ST_OGV_MIME_STRING ST_OGV_MIME ":" ST_OGV_EXT ":" ST_OGV_DESC

/**
 *.avi  - Audio/Video interleaved
 */
#define ST_AVI_MIME "video/msvideo" // "video/avi", "video/x-msvideo"
#define ST_AVI_EXT  "avi"
#define ST_AVI_DESC "AVI - Audio/Video interleaved"
#define ST_AVI_MIME_STRING ST_AVI_MIME ":" ST_AVI_EXT ":" ST_AVI_DESC

/**
 *.asf, *.asx  - MS Advanced Streaming Format, Active Streaming Format
 */
#define ST_ASF_MIME "video/x-ms-asf"
#define ST_ASF_EXT  "asf"
#define ST_ASF_DESC "ASF - Advanced Streaming Format (video)"
#define ST_ASF_MIME_STRING ST_ASF_MIME ":" ST_ASF_EXT ":" ST_ASF_DESC
#define ST_ASX_EXT  "asx"
#define ST_ASX_MIME_STRING ST_ASF_MIME ":" ST_ASX_EXT ":" ST_ASF_DESC

/**
 *.wmv  - Windows Media Video
 */
#define ST_WMV_MIME "video/x-ms-wmv" // "video/wmv"
#define ST_WMV_EXT  "wmv"
#define ST_WMV_DESC "WMV - Windows Media Video"
#define ST_WMV_MIME_STRING ST_WMV_MIME ":" ST_WMV_EXT ":" ST_WMV_DESC

/**
 *.mp4;*.m4v;*.m4a;*.mpg;*.mp2;*.m2v;*.mpa;*.mpe;*.mpeg;*.mpv2 - MPEG Video family
 */
#define ST_MPEG_MIME "video/mpeg"
#define ST_MPEG_DESC "MPEG Video"
#define ST_MPA_EXT   "mpa"
#define ST_MPA_MIME_STRING ST_MPEG_MIME ":" ST_MPA_EXT ":" ST_MPEG_DESC
#define ST_MPE_EXT   "mpe"
#define ST_MPE_MIME_STRING ST_MPEG_MIME ":" ST_MPE_EXT ":" ST_MPEG_DESC
#define ST_MPG_EXT   "mpg"
#define ST_MPG_MIME_STRING ST_MPEG_MIME ":" ST_MPG_EXT ":" ST_MPEG_DESC
#define ST_MPEG_EXT  "mpeg"
#define ST_MPEG_MIME_STRING ST_MPEG_MIME ":" ST_MPEG_EXT ":" ST_MPEG_DESC

#define ST_MPEG2_MIME "video/mpeg"
#define ST_MPEG2_DESC "MPEG2 Video"
#define ST_MP2_EXT    "mp2"
#define ST_MP2_MIME_STRING ST_MPEG2_MIME ":" ST_MP2_EXT ":" ST_MPEG2_DESC
#define ST_M2V_EXT    "m2v"
#define ST_M2V_MIME_STRING ST_MPEG2_MIME ":" ST_M2V_EXT ":" ST_MPEG2_DESC
#define ST_MPV2_EXT   "mpv2"
#define ST_MPV2_MIME_STRING ST_MPEG2_MIME ":" ST_MPV2_EXT ":" ST_MPEG2_DESC

#define ST_MP4_MIME   "video/x-mp4"
#define ST_MPEG4_DESC "MPEG4 Video"
#define ST_MP4_EXT    "mp4"
#define ST_MP4_MIME_STRING ST_MP4_MIME ":" ST_MP4_EXT ":" ST_MPEG4_DESC

#define ST_M4V_MIME   "video/x-m4v"
#define ST_M4V_EXT    "m4v"
#define ST_M4V_MIME_STRING ST_M4V_MIME ":" ST_M4V_EXT ":" ST_MPEG4_DESC

#define ST_M4A_MIME   "video/x-m4a"
#define ST_M4A_DESC   "MPEG4 Audio"
#define ST_M4A_EXT    "m4a"
#define ST_M4A_MIME_STRING ST_M4A_MIME ":" ST_M4A_EXT ":" ST_M4A_DESC

/**
 *.mov;*.qt  - QuickTime Video (Apple)
 */
#define ST_MOV_MIME "video/quicktime"
#define ST_MOV_EXT  "mov"
#define ST_QT_DESC "QuickTime Video"
#define ST_MOV_MIME_STRING ST_MOV_MIME ":" ST_MOV_EXT ":" ST_QT_DESC
#define ST_QT_MIME "video/quicktime"
#define ST_QT_EXT  "qt"
#define ST_QT_MIME_STRING ST_QT_MIME ":" ST_QT_EXT ":" ST_QT_DESC

/**
 *.flv - Flash Video
 */
#define ST_FLV_MIME "video/x-flv"
#define ST_FLV_EXT  "flv"
#define ST_FLV_DESC "FLV - Flash Video"
#define ST_FLV_MIME_STRING ST_FLV_MIME ":" ST_FLV_EXT ":" ST_FLV_DESC

/**
 *.vob  - Video files on DVD
 */
#define ST_VOB_MIME "video/dvd"
#define ST_VOB_EXT  "vob"
#define ST_VOB_DESC "VOB - DVD Video object"
#define ST_VOB_MIME_STRING ST_VOB_MIME ":" ST_VOB_EXT ":" ST_VOB_DESC

/**
 *.aob  - Audio files on DVD
 */
#define ST_AOB_MIME "audio/x-aob"
#define ST_AOB_EXT  "aob"
#define ST_AOB_DESC "AOB - DVD Audio object"
#define ST_AOB_MIME_STRING ST_AOB_MIME ":" ST_AOB_EXT ":" ST_AOB_DESC

/**
 *.ts   - MPEG   transport stream
 *.mts  - MPEG   transport stream
 *.m2ts - MPEG-2 transport stream
 */
#define ST_TS_MIME "video/mpeg"
#define ST_TS_EXT  "ts"
#define ST_TS_DESC "TS - MPEG Transport Stream"
#define ST_TS_MIME_STRING ST_TS_MIME ":" ST_TS_EXT ":" ST_TS_DESC
#define ST_MTS_MIME "video/mp2t" // "video/avchd"
#define ST_MTS_EXT  "mts"
#define ST_MTS_DESC "mts - MPEG Transport Stream"
#define ST_MTS_MIME_STRING ST_MTS_MIME ":" ST_MTS_EXT ":" ST_MTS_DESC
#define ST_M2TS_MIME "video/mp2t" // "video/avchd"
#define ST_M2TS_EXT  "m2ts"
#define ST_M2TS_DESC "m2ts - MPEG-2 Transport Stream"
#define ST_M2TS_MIME_STRING ST_M2TS_MIME ":" ST_M2TS_EXT ":" ST_M2TS_DESC

/**
 *.bik - BINK video (games)
 */
#define ST_BIK_MIME "video/x-bik"
#define ST_BIK_EXT  "bik"
#define ST_BIK_DESC "BIK - BINK video (games)"
#define ST_BIK_MIME_STRING ST_BIK_MIME ":" ST_BIK_EXT ":" ST_BIK_DESC

/**
 *.flac - FLAC Audio, lossless
 */
#define ST_FLAC_MIME "audio/x-flac"
#define ST_FLAC_EXT  "flac"
#define ST_FLAC_DESC "FLAC - FLAC Audio, lossless"
#define ST_FLAC_MIME_STRING ST_FLAC_MIME ":" ST_FLAC_EXT ":" ST_FLAC_DESC

/**
 *.ape - Monkey Audio, lossless
 */
#define ST_APE_MIME "audio/x-ape"
#define ST_APE_EXT  "ape"
#define ST_APE_DESC "APE - Monkey Audio, lossless"
#define ST_APE_MIME_STRING ST_APE_MIME ":" ST_APE_EXT ":" ST_APE_DESC

/**
 *.mp3 - MPEG Layer3 Audio, lossy
 */
#define ST_MP3_MIME "audio/mpeg"
#define ST_MP3_EXT  "mp3"
#define ST_MP3_DESC "MP3 - MPEG Layer3 Audio, lossy"
#define ST_MP3_MIME_STRING ST_MP3_MIME ":" ST_MP3_EXT ":" ST_MP3_DESC

/**
 *.ogg - OGG Vorbis Audio, lossy
 */
#define ST_OGG_MIME "audio/x-ogg"
#define ST_OGG_EXT  "ogg"
#define ST_OGG_DESC "OGG - OGG Vorbis Audio, lossy"
#define ST_OGG_MIME_STRING ST_OGG_MIME ":" ST_OGG_EXT ":" ST_OGG_DESC

/**
 *.ac3 - AC3 Audio, lossy
 */
#define ST_AC3_MIME "audio/x-ac3"
#define ST_AC3_EXT  "ac3"
#define ST_AC3_DESC "AC3 - AC3 Audio, lossy"
#define ST_AC3_MIME_STRING ST_AC3_MIME ":" ST_AC3_EXT ":" ST_AC3_DESC

/**
 *.wma - Windows Media Audio, lossy or lossless
 */
#define ST_WMA_MIME "audio/x-wma"
#define ST_WMA_EXT  "wma"
#define ST_WMA_DESC "WMA - Windows Media Audio"
#define ST_WMA_MIME_STRING ST_WMA_MIME ":" ST_WMA_EXT ":" ST_WMA_DESC

/**
 *.dts - DTS Audio
 */
#define ST_DTS_MIME "audio/vnd.dts"
#define ST_DTS_EXT  "dts"
#define ST_DTS_DESC "DTS Audio"
#define ST_DTS_MIME_STRING ST_DTS_MIME ":" ST_DTS_EXT ":" ST_DTS_DESC
#define ST_DTSHD_MIME "audio/vnd.dts.hd"
#define ST_DTSHD_EXT  "dtshd"
#define ST_DTSHD_DESC "DTS High Definition Audio"
#define ST_DTSHD_MIME_STRING ST_DTSHD_MIME ":" ST_DTSHD_EXT ":" ST_DTSHD_DESC
#define ST_DTSMA_MIME "audio/x-dts-ma"
#define ST_DTSMA_EXT  "dtsma"
#define ST_DTSMA_DESC "DTS HD Master Audio"
#define ST_DTSMA_MIME_STRING ST_DTSMA_MIME ":" ST_DTSMA_EXT ":" ST_DTSMA_DESC

/**
 *.wav - PCM Audio, uncompressed
 */
#define ST_WAV_MIME "audio/x-wav"
#define ST_WAV_EXT  "wav"
#define ST_WAV_DESC "WAV - PCM Audio, uncompressed"
#define ST_WAV_MIME_STRING ST_WAV_MIME ":" ST_WAV_EXT ":" ST_WAV_DESC

/**
 *.m3u - Playlist
 */
#define ST_M3U_MIME "video/x-m3u"
#define ST_M3U_EXT  "m3u"
#define ST_M3U_DESC "M3U - Plain text Playlist"
#define ST_M3U_MIME_STRING ST_M3U_MIME ":" ST_M3U_EXT ":" ST_M3U_DESC

/**
 * Define total supported MIME list.
 */
#define ST_VIDEO_PLUGIN_MIME_CHAR ST_MKV_MIME_STRING ";" \
ST_MK3D_MIME_STRING ";" \
ST_WEBM_MIME_STRING ";" \
ST_MKA_MIME_STRING ";" \
ST_AVS_MIME_STRING ";" \
ST_OGM_MIME_STRING ";" \
ST_OGV_MIME_STRING ";" \
ST_AVI_MIME_STRING ";" \
ST_ASF_MIME_STRING ";" \
ST_ASX_MIME_STRING ";" \
ST_WMV_MIME_STRING ";" \
ST_MPA_MIME_STRING ";" \
ST_MPE_MIME_STRING ";" \
ST_MPG_MIME_STRING ";" \
ST_MPEG_MIME_STRING ";" \
ST_MPV2_MIME_STRING ";" \
ST_MP2_MIME_STRING ";" \
ST_M2V_MIME_STRING ";" \
ST_MP4_MIME_STRING ";" \
ST_M4V_MIME_STRING ";" \
ST_M4A_MIME_STRING ";" \
ST_MOV_MIME_STRING ";" \
ST_QT_MIME_STRING ";" \
ST_FLV_MIME_STRING ";" \
ST_VOB_MIME_STRING ";" \
ST_AOB_MIME_STRING ";" \
ST_TS_MIME_STRING ";" \
ST_MTS_MIME_STRING ";" \
ST_M2TS_MIME_STRING ";" \
ST_BIK_MIME_STRING ";" \
ST_FLAC_MIME_STRING ";" \
ST_APE_MIME_STRING ";" \
ST_MP3_MIME_STRING ";" \
ST_OGG_MIME_STRING ";" \
ST_AC3_MIME_STRING ";" \
ST_WMA_MIME_STRING ";" \
ST_DTS_MIME_STRING ";" \
ST_DTSHD_MIME_STRING ";" \
ST_DTSMA_MIME_STRING ";" \
ST_WAV_MIME_STRING ";" \
ST_M3U_MIME_STRING ";" \
"\000"

/**
 * Define Audio MIME list.
 */
#define ST_VIDEO_PLUGIN_AUDIO_MIME_CHAR ST_MKA_MIME_STRING ";" \
ST_FLAC_MIME_STRING ";" \
ST_APE_MIME_STRING ";" \
ST_AOB_MIME_STRING ";" \
ST_MP3_MIME_STRING ";" \
ST_OGG_MIME_STRING ";" \
ST_AC3_MIME_STRING ";" \
ST_WMA_MIME_STRING ";" \
ST_DTS_MIME_STRING ";" \
ST_DTSHD_MIME_STRING ";" \
ST_DTSMA_MIME_STRING ";" \
ST_WAV_MIME_STRING ";" \
"\000"

/**
 *.srt
 */
#define ST_SRT_MIME "subtitles/x-srt"
#define ST_SRT_EXT  "srt"
#define ST_SRT_DESC "SRT - Utf-8 subtitles"
#define ST_SRT_MIME_STRING ST_SRT_MIME ":" ST_SRT_EXT ":" ST_SRT_DESC

/**
 *.smi
 */
#define ST_SMI_MIME "subtitles/x-smi"
#define ST_SMI_EXT  "smi"
#define ST_SMI_DESC "SAMI - Utf-8 subtitles"
#define ST_SMI_MIME_STRING ST_SMI_MIME ":" ST_SMI_EXT ":" ST_SMI_DESC

/**
 *.sub
 */
#define ST_SUB_MIME "subtitles/x-sub"
#define ST_SUB_EXT  "sub"
#define ST_SUB_DESC "SUB - Image-based subtitles"
#define ST_SUB_MIME_STRING ST_SUB_MIME ":" ST_SUB_EXT ":" ST_SUB_DESC

/**
 * Define Subtitles MIME list.
 */
#define ST_VIDEO_PLUGIN_SUBTIT_MIME_CHAR ST_SRT_MIME_STRING ";" \
ST_SMI_MIME_STRING ";" \
ST_SUB_MIME_STRING ";" \
"\000"

#endif // __StVideoPluginInfo_h_
