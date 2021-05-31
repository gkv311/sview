/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextureUploadParams_h_
#define __StGLTextureUploadParams_h_

/**
 * Structure holding parameters for texture uploading.
 * Uploading big amounts of data onto GPU leads to frame-rate lags (specifically in case when uploading done within rendering thread);
 * to smooth this effect, upload process can be split into smaller chunks.
 *
 * There are 4 use cases, which are sensitive to texture uploading lags:
 * - GUI rendering; will be noticeable only for very large uploads.
 * - 360 panorama with device orientation tracking; will be noticeable due lag on orientation update.
 * - HMD output with head orientation tracking; same as panorama, but will be noticeable all time (no option disabling head tracking) and can lead to headaches.
 * - Software PageFlipping (texture uploading lag might lead to missing VSync and swapping Left/Right frames).
 *
 * Most optimal case would be defining amount of chunks equal to amount of frames to be rendered before new (uploaded) texture should appear,
 * in this case average FPS will be close to constant, which is in particular good to preserve GUI interactive.
 * At the same time, uploading data within more than 1 rendering frame would add a constant time shift to video stream presented to user,
 * which ideally should be also considered.
 * At last, the real maximum rendering FPS naturally limited by monitor frequency, can be not achievable due to slow GPU.
 */
struct StGLTextureUploadParams {

    int MaxUploadIterations; //!< maximum number of texture upload iterations (frames); 1 means texture should be uploaded immediately
    int MaxUploadChunkMiB;   //!< maximum number of data in MiB to be uploaded within single iteration; 0 means no limit;
                             //!  MaxUploadIterations is stronger limit

    StGLTextureUploadParams() : MaxUploadIterations(1), MaxUploadChunkMiB(0) {}

};

#endif // __StGLTextureUploadParams_h_
