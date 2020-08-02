/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
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

#include "StVideoQueue.h"

#if defined(_WIN32)

#include <StAV/StAVPacket.h>
#include <StAV/StAVFrame.h>
#include <StAV/StAVBufferPool.h>
#include <StLibrary.h>

#include <vector>

#ifdef _MSC_VER
    // nonstandard extension used : nameless struct/union
    #pragma warning(disable : 4201)
#endif

#include <d3d9.h>
#include <dxva2api.h>

#ifdef _MSC_VER
    #pragma warning(default : 4201)
#endif

extern "C" {

  #include <libavcodec/dxva2.h>
  #include <libavutil/buffer.h>
  #include <libavutil/imgutils.h>

};

// define all the GUIDs used directly here,
// to avoid problems with inconsistent dxva2api.h versions in mingw-w64 and different MSVC version
#include <initguid.h>

DEFINE_GUID(IID_IDirectXVideoDecoderService, 0xfc51a551,0xd5e7,0x11d9,0xaf,0x55,0x00,0x05,0x4e,0x43,0xff,0x02);

DEFINE_GUID(DXVA2_ModeMPEG2_VLD,      0xee27417f, 0x5e28,0x4e65,0xbe,0xea,0x1d,0x26,0xb5,0x08,0xad,0xc9);
DEFINE_GUID(DXVA2_ModeMPEG2and1_VLD,  0x86695f12, 0x340e,0x4f04,0x9f,0xd3,0x92,0x53,0xdd,0x32,0x74,0x60);
DEFINE_GUID(DXVA2_ModeH264_E,         0x1b81be68, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeH264_F,         0x1b81be69, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVADDI_Intel_ModeH264_E, 0x604F8E68, 0x4951,0x4C54,0x88,0xFE,0xAB,0xD2,0x5C,0x15,0xB3,0xD6);
DEFINE_GUID(DXVA2_ModeVC1_D,          0x1b81beA3, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeVC1_D2010,      0x1b81beA4, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeHEVC_VLD_Main,  0x5b11d51b, 0x2f4c,0x4452,0xbc,0xc3,0x09,0xf2,0xa1,0x16,0x0c,0xc0);
DEFINE_GUID(DXVA2_NoEncrypt,          0x1b81beD0, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(GUID_NULL,                0x00000000, 0x0000,0x0000,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);

struct StDxva2Mode {
    const GUID* guid;
    AVCodecID   codec;

    StDxva2Mode(const GUID&     theGuid,
                const AVCodecID theCodec)
    : guid(&theGuid),
      codec(theCodec) {}
};

struct StDxva2SurfaceInfo {
    int      used;
    uint64_t age;
};

/**
 * DXVA2 HWAccel context.
 */
class StDxva2Context : public StHWAccelContext {

        public:

    /**
     * Empty constructor.
     */
    StDxva2Context()
    : myDeviceHandle(INVALID_HANDLE_VALUE),
      myD3d9(NULL),
      myD3d9Device(NULL),
      myD3d9DevMgr(NULL),
      myDecoderService(NULL),
      myDxvaDecoder(NULL),
      myDecoderGuid(GUID_NULL),
      myD3dSurfaces(NULL),
      mySurfInfos(NULL),
      myNbSurfaces(0),
      mySurfaceAge(0),
      myFrameTmp(NULL) {
        myD3dLib  .loadSimple("d3d9.dll");
        myDxva2Lib.loadSimple("dxva2.dll");
    }

    /**
     * Destructor.
     */
    virtual ~StDxva2Context() {
        release();
    }

    /**
     * Release context.
     */
    void release();

    /**
     * Return true if DXVA2 has been successfully initialized.
     */
    virtual bool isValid() const ST_ATTR_OVERRIDE { return myDeviceHandle != INVALID_HANDLE_VALUE; }

    /**
     * Create context.
     */
    virtual bool create(StVideoQueue& theVideo) ST_ATTR_OVERRIDE;

    /**
     * Destroy decoder.
     */
    virtual void decoderDestroy(AVCodecContext* ) ST_ATTR_OVERRIDE {
        myPoolsTmp[0].release();
        myPoolsTmp[1].release();
        myPoolsTmp[2].release();

        if(myD3dSurfaces != NULL) {
            for(uint32_t aSurfIter = 0; aSurfIter < myNbSurfaces; ++aSurfIter) {
                if(myD3dSurfaces[aSurfIter] != NULL) {
                    myD3dSurfaces[aSurfIter]->Release();
                }
            }
        }
        av_freep(&myD3dSurfaces);
        av_freep(&mySurfInfos);
        myNbSurfaces = 0;
        mySurfaceAge = 0;

        if(myDxvaDecoder != NULL) {
            myDxvaDecoder->Release();
            myDxvaDecoder = NULL;
        }
    }

    /**
     * Create decoder.
     */
    virtual bool decoderCreate(StVideoQueue&   theVideo,
                               AVCodecContext* theCodecCtx) ST_ATTR_OVERRIDE;

    /**
     * AVFrame initialization callback.
     */
    virtual int getFrameBuffer(StVideoQueue& theVideo,
                               AVFrame*      theFrame) ST_ATTR_OVERRIDE;

    /**
     * Fetch decoded results into specified frame.
     */
    virtual bool retrieveFrame(StVideoQueue& theVideo,
                               AVFrame*      theFrame) ST_ATTR_OVERRIDE;

        private:

    struct StDxva2SurfaceWrapper {
        StDxva2Context*       ctx;
        LPDIRECT3DSURFACE9    surface;
        IDirectXVideoDecoder* decoder;
    };

    /**
     * Buffer release callback.
     */
    static void stReleaseBufferDxva2(void*    theOpaque,
                                     uint8_t* theData) {
        (void )theData;
        StDxva2SurfaceWrapper* aWrapper = (StDxva2SurfaceWrapper* )theOpaque;
        StDxva2Context*        aDxvaCtx = aWrapper->ctx;
        for(uint32_t aSurfIter = 0; aSurfIter < aDxvaCtx->myNbSurfaces; ++aSurfIter) {
            if(aDxvaCtx->myD3dSurfaces[aSurfIter] == aWrapper->surface) {
                aDxvaCtx->mySurfInfos[aSurfIter].used = 0;
                break;
            }
        }
        aWrapper->surface->Release();
        aWrapper->decoder->Release();
        av_free(aWrapper);
    }

        private:

    StLibrary myD3dLib;
    StLibrary myDxva2Lib;

    HANDLE    myDeviceHandle;

    IDirect3D9*                  myD3d9;
    IDirect3DDevice9*            myD3d9Device;
    IDirect3DDeviceManager9*     myD3d9DevMgr;
    IDirectXVideoDecoderService* myDecoderService;
    IDirectXVideoDecoder*        myDxvaDecoder;

    GUID                         myDecoderGuid;
    DXVA2_ConfigPictureDecode    myDecoderConfig;

    IDirect3DSurface9**          myD3dSurfaces;
    StDxva2SurfaceInfo*          mySurfInfos;
    uint32_t                     myNbSurfaces;
    uint64_t                     mySurfaceAge;

    StAVBufferPool               myPoolsTmp[3];
    AVFrame*                     myFrameTmp;
    dxva_context                 myDxvaCtxAV;

    std::vector<StDxva2Mode>     myDxvaModes;

};

void StDxva2Context::release() {
    decoderDestroy(NULL);
    if(myDecoderService != NULL) {
        myDecoderService->Release();
        myDecoderService = NULL;
    }
    if(myD3d9DevMgr   != NULL
    && myDeviceHandle != INVALID_HANDLE_VALUE) {
        myD3d9DevMgr->CloseDeviceHandle(myDeviceHandle);
        myDeviceHandle = INVALID_HANDLE_VALUE;
    }
    if(myD3d9DevMgr != NULL) {
        myD3d9DevMgr->Release();
        myD3d9DevMgr = NULL;
    }
    if(myD3d9Device != NULL) {
        myD3d9Device->Release();
        myD3d9Device = NULL;
    }
    if(myD3d9 != NULL) {
        myD3d9->Release();
        myD3d9 = NULL;
    }
    av_frame_free(&myFrameTmp);
}

bool StDxva2Context::create(StVideoQueue& theVideo) {
    const StSignal<void (const StCString& )>& onError = theVideo.signals.onError;
    if(!myD3dLib.isOpened()) {
        onError(stCString("StVideoQueue: Failed to load D3D9 library"));
        release();
        return false;
    } else if(!myDxva2Lib.isOpened()) {
        onError(stCString("StVideoQueue: Failed to load DXVA2 library"));
        release();
        return false;
    }

    typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT);
    typedef HRESULT (WINAPI *DXVA2CreateDirect3DDeviceManager9_t)(UINT* , IDirect3DDeviceManager9** );
    Direct3DCreate9_t aCreateD3dProc = NULL;
    DXVA2CreateDirect3DDeviceManager9_t aCreateDevMgrProc = NULL;
    if(!myD3dLib.find("Direct3DCreate9", aCreateD3dProc)) {
        onError(stCString("StVideoQueue: Failed to locate Direct3DCreate9"));
        release();
        return false;
    }
    if(!myDxva2Lib.find("DXVA2CreateDirect3DDeviceManager9", aCreateDevMgrProc)) {
        onError(stCString("StVideoQueue: Failed to locate DXVA2CreateDirect3DDeviceManager9"));
        release();
        return false;
    }

    myD3d9 = aCreateD3dProc(D3D_SDK_VERSION);
    if(myD3d9 == NULL) {
        onError(stCString("StVideoQueue: Failed to create IDirect3D object"));
        release();
        return false;
    }

    D3DDISPLAYMODE aD3dDispMode;
    UINT anAdapter = D3DADAPTER_DEFAULT;
    myD3d9->GetAdapterDisplayMode(anAdapter, &aD3dDispMode);
    D3DPRESENT_PARAMETERS aD3dParams = {0};
    aD3dParams.Windowed         = TRUE;
    aD3dParams.BackBufferWidth  = 2;
    aD3dParams.BackBufferHeight = 2;
    aD3dParams.BackBufferCount  = 0;
    aD3dParams.BackBufferFormat = aD3dDispMode.Format;
    aD3dParams.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    aD3dParams.Flags            = D3DPRESENTFLAG_VIDEO;

    HRESULT aHRes = myD3d9->CreateDevice(anAdapter, D3DDEVTYPE_HAL, GetDesktopWindow(),
                                         D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                         &aD3dParams, &myD3d9Device);
    if(aHRes != D3D_OK) {
        onError(stCString("StVideoQueue: Failed to create Direct3D device"));
        release();
        return false;
    }

    unsigned aResetToken = 0;
    aHRes = aCreateDevMgrProc(&aResetToken, &myD3d9DevMgr);
    if(aHRes != S_OK) {
        onError(stCString("StVideoQueue: Failed to create Direct3D device manager"));
        release();
        return false;
    }

    aHRes = myD3d9DevMgr->ResetDevice(myD3d9Device, aResetToken);
    if(aHRes != S_OK) {
        onError(stCString("StVideoQueue: Failed to bind Direct3D device to device manager"));
        release();
        return false;
    }

    aHRes = myD3d9DevMgr->OpenDeviceHandle(&myDeviceHandle);
    if(aHRes != S_OK) {
        onError(stCString("StVideoQueue: Failed to open device handle"));
        release();
        return false;
    }

    aHRes = myD3d9DevMgr->GetVideoService(myDeviceHandle, IID_IDirectXVideoDecoderService, (void** )&myDecoderService);
    if(aHRes != S_OK) {
        onError(stCString("StVideoQueue: Failed to create IDirectXVideoDecoderService"));
        release();
        return false;
    }

    myFrameTmp = av_frame_alloc();
    if(myFrameTmp == NULL) {
        release();
        return false;
    }

    // MPEG-2
    myDxvaModes.clear();
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeMPEG2_VLD,      theVideo.CodecIdMPEG2));
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeMPEG2and1_VLD,  theVideo.CodecIdMPEG2));
    // H.264
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeH264_F,         theVideo.CodecIdH264));
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeH264_E,         theVideo.CodecIdH264));
    // Intel specific H.264 mode
    myDxvaModes.push_back(StDxva2Mode(DXVADDI_Intel_ModeH264_E, theVideo.CodecIdH264));
    // VC-1 / WMV3
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeVC1_D2010,      theVideo.CodecIdVC1));
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeVC1_D2010,      theVideo.CodecIdWMV3));
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeVC1_D,          theVideo.CodecIdVC1));
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeVC1_D,          theVideo.CodecIdWMV3));
    // HEVC/H.265
    myDxvaModes.push_back(StDxva2Mode(DXVA2_ModeHEVC_VLD_Main,  theVideo.CodecIdHEVC));
    return true;
}

int StDxva2Context::getFrameBuffer(StVideoQueue& theVideo,
                                   AVFrame*      theFrame) {
    if( theFrame->format != stAV::PIX_FMT::DXVA2_VLD
    || !isValid()) {
        return -1;
    }

    uint32_t anOldUnused = uint32_t(-1);
    for(uint32_t aSurfIter = 0; aSurfIter < myNbSurfaces; ++aSurfIter) {
        StDxva2SurfaceInfo* info = &mySurfInfos[aSurfIter];
        if(!info->used
        && (anOldUnused == uint32_t(-1) || info->age < mySurfInfos[anOldUnused].age)) {
            anOldUnused = aSurfIter;
        }
    }
    if(anOldUnused == -1) {
        theVideo.signals.onError(stCString("StVideoQueue: No free DXVA2 surface!"));
        return AVERROR(ENOMEM);
    }
    const uint32_t     aSurfId  = anOldUnused;
    IDirect3DSurface9* aSurface = myD3dSurfaces[aSurfId];

    StDxva2SurfaceWrapper* aWrapper = (StDxva2SurfaceWrapper* )av_mallocz(sizeof(StDxva2SurfaceWrapper));
    if(aWrapper == NULL) {
        return AVERROR(ENOMEM);
    }

    theFrame->buf[0] = av_buffer_create((uint8_t* )aSurface, 0,
                                        stReleaseBufferDxva2, aWrapper,
                                        AV_BUFFER_FLAG_READONLY);
    if(!theFrame->buf[0]) {
        av_free(aWrapper);
        return AVERROR(ENOMEM);
    }

    aWrapper->ctx     = this;
    aWrapper->surface = aSurface;
    aWrapper->surface->AddRef();
    aWrapper->decoder = myDxvaDecoder;
    aWrapper->decoder->AddRef();

    mySurfInfos[aSurfId].used = 1;
    mySurfInfos[aSurfId].age  = mySurfaceAge++;

    theFrame->data[3] = (uint8_t* )aSurface;
    return 0;
}

bool StDxva2Context::retrieveFrame(StVideoQueue& theVideo,
                                   AVFrame*      theFrame) {
    if(!isValid()
     || theFrame->format != stAV::PIX_FMT::DXVA2_VLD) {
        return false;
    }

    IDirect3DSurface9* aSurface = (IDirect3DSurface9* )theFrame->data[3];
    D3DSURFACE_DESC    aSurfDesc;
    aSurface->GetDesc(&aSurfDesc);

    av_frame_unref(myFrameTmp);
    myFrameTmp->width  = theFrame->width;
    myFrameTmp->height = theFrame->height;
    //myFrameTmp->format = stAV::PIX_FMT::NV12;
    myFrameTmp->format = stAV::PIX_FMT::YUV420P;
    const int aHeightUV   = theFrame->height / 2;
    const int aWidthUV    = theFrame->width  / 2;

    myFrameTmp->linesize[0] = (int )getAligned(theFrame->width, 32);
    myFrameTmp->linesize[1] = (int )getAligned(aWidthUV,        32);
    myFrameTmp->linesize[2] = (int )getAligned(aWidthUV,        32);

    const int aBufSizeY   = myFrameTmp->linesize[0] * theFrame->height;
    const int aBufSizeU   = myFrameTmp->linesize[1] * aHeightUV;
    if(myPoolsTmp[0].init(aBufSizeY)
    && myPoolsTmp[1].init(aBufSizeU)
    && myPoolsTmp[2].init(aBufSizeU)) {
        myFrameTmp->buf[0] = myPoolsTmp[0].getBuffer();
        myFrameTmp->buf[1] = myPoolsTmp[1].getBuffer();
        myFrameTmp->buf[2] = myPoolsTmp[2].getBuffer();
        if(myFrameTmp->buf[0] == NULL
        || myFrameTmp->buf[1] == NULL
        || myFrameTmp->buf[2] == NULL) {
            ST_ERROR_LOG("StDxva2Context: unable to allocate YUV420P frame buffers");
            return false;
        }

        myFrameTmp->data[0] = myFrameTmp->buf[0]->data;
        myFrameTmp->data[1] = myFrameTmp->buf[1]->data;
        myFrameTmp->data[2] = myFrameTmp->buf[2]->data;
    } else if(av_frame_get_buffer(myFrameTmp, 32) < 0) {
        ST_ERROR_LOG("StDxva2Context: unable to allocate YUV420P frame buffers");
        return false;
    }

    D3DLOCKED_RECT aLockRect;
    if(aSurface->LockRect(&aLockRect, NULL, D3DLOCK_READONLY) != D3D_OK) {
        theVideo.signals.onError(stCString("StVideoQueue: Unable to lock DXVA2 surface"));
        return false;
    }

    av_image_copy_plane(myFrameTmp->data[0], myFrameTmp->linesize[0],
                        (uint8_t* )aLockRect.pBits,
                        aLockRect.Pitch, theFrame->width, theFrame->height);
    //av_image_copy_plane(myFrameTmp->data[1], myFrameTmp->linesize[1],
    //                    (uint8_t* )aLockRect.pBits + aLockRect.Pitch * aSurfDesc.Height,
    //                    aLockRect.Pitch, theFrame->width, theFrame->height / 2);
    const uint8_t* aSrcPlaneUV = (const uint8_t* )aLockRect.pBits + aLockRect.Pitch * aSurfDesc.Height;
    for(int aRow = 0; aRow < aHeightUV; ++aRow) {
        const uint8_t* aSrcRowUV = aSrcPlaneUV + aRow * aLockRect.Pitch;
        uint8_t*       aDstRowU  = myFrameTmp->data[1] + myFrameTmp->linesize[1] * aRow;
        uint8_t*       aDstRowV  = myFrameTmp->data[2] + myFrameTmp->linesize[2] * aRow;
        for(int aCol = 0; aCol < aWidthUV; ++aCol) {
            const uint8_t* aSrcUV = aSrcRowUV + aCol * 2;
            aDstRowU[aCol] = aSrcUV[0];
            aDstRowV[aCol] = aSrcUV[1];
        }
    }

    aSurface->UnlockRect();

    int aRes = av_frame_copy_props(myFrameTmp, theFrame);
    if(aRes < 0) {
        av_frame_unref(myFrameTmp);
        return false;
    }

    av_frame_unref   (theFrame);
    av_frame_move_ref(theFrame, myFrameTmp);
    return true;
}

bool StDxva2Context::decoderCreate(StVideoQueue&   theVideo,
                                   AVCodecContext* theCodecCtx) {
    const StSignal<void (const StCString& )>& onError = theVideo.signals.onError;
    decoderDestroy(theCodecCtx);
    theCodecCtx->hwaccel_context = NULL;

    uint32_t aNbGuids  = 0;
    GUID*    aGuidList = NULL;
    if(myDecoderService->GetDecoderDeviceGuids(&aNbGuids, &aGuidList) != S_OK) {
        onError(stCString("StVideoQueue: Failed to retrieve decoder DXVA2 device GUIDs"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    GUID      aDeviceGuid      = GUID_NULL;
    D3DFORMAT aTargetD3dFormat = D3DFMT_UNKNOWN;
    for(std::vector<StDxva2Mode>::const_iterator aModeIter = myDxvaModes.begin();
        aModeIter != myDxvaModes.end(); ++aModeIter) {
        const StDxva2Mode& aMode = *aModeIter;
        if(aMode.codec != theCodecCtx->codec_id) {
            continue;
        }

        uint32_t aGuidIter = 0;
        for(; aGuidIter < aNbGuids; ++aGuidIter) {
            if(::IsEqualGUID(*aMode.guid, aGuidList[aGuidIter])) {
                break;
            }
        }
        if(aGuidIter == aNbGuids) {
            continue;
        }

        uint32_t   aNbD3dTargets  = 0;
        D3DFORMAT* aD3dTargetList = NULL;
        if(myDecoderService->GetDecoderRenderTargets(*aMode.guid, &aNbD3dTargets, &aD3dTargetList) != S_OK) {
            continue;
        }

        for(uint32_t aTargetIter = 0; aTargetIter < aNbD3dTargets; ++aTargetIter) {
            const D3DFORMAT aD3dFormat = aD3dTargetList[aTargetIter];
            if(aD3dFormat == MKTAG('N','V','1','2')) {
                aTargetD3dFormat = aD3dFormat;
                break;
            }
        }
        ::CoTaskMemFree(aD3dTargetList);
        if(aTargetD3dFormat != D3DFMT_UNKNOWN) {
            aDeviceGuid = *aMode.guid;
            break;
        }
    }
    ::CoTaskMemFree(aGuidList);

    if(::IsEqualGUID(aDeviceGuid, GUID_NULL)) {
        onError(stCString("StVideoQueue: No DXVA2 decoder device for codec found"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    DXVA2_VideoDesc aDesc = { 0 };
    aDesc.SampleWidth  = theCodecCtx->coded_width;
    aDesc.SampleHeight = theCodecCtx->coded_height;
    aDesc.Format       = aTargetD3dFormat;

    uint32_t aNbConfigs = 0;
    uint32_t aBestScore = 0;
    DXVA2_ConfigPictureDecode* aCfgList = NULL;
    DXVA2_ConfigPictureDecode  aBestCfg = {{0}};
    if(myDecoderService->GetDecoderConfigurations(aDeviceGuid, &aDesc, NULL, &aNbConfigs, &aCfgList) != S_OK) {
        onError(stCString("StVideoQueue: Unable to retrieve DXVA2 decoder configurations"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    for(uint32_t aCfgIter = 0; aCfgIter < aNbConfigs; ++aCfgIter) {
        const DXVA2_ConfigPictureDecode& aCfg = aCfgList[aCfgIter];
        uint32_t aScore = 0;
        if(aCfg.ConfigBitstreamRaw == 1) {
            aScore = 1;
        } else if(theCodecCtx->codec_id == theVideo.CodecIdH264
               && aCfg.ConfigBitstreamRaw == 2) {
            aScore = 2;
        } else {
            continue;
        }

        if(::IsEqualGUID(aCfg.guidConfigBitstreamEncryption, DXVA2_NoEncrypt)) {
            aScore += 16;
        }
        if(aScore > aBestScore) {
            aBestScore = aScore;
            aBestCfg   = aCfg;
        }
    }
    ::CoTaskMemFree(aCfgList);
    if(aBestScore == 0) {
        onError(stCString("StVideoQueue: No valid DXVA2 decoder configuration available"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    // decoding MPEG-2 requires additional alignment on some Intel GPUs,
    // but it causes issues for H.264 on certain AMD GPUs...
    int aSurfaceAlignment = 16;
    if(theCodecCtx->codec_id == theVideo.CodecIdMPEG2) {
        aSurfaceAlignment = 32;
    // the HEVC DXVA2 spec asks for 128 pixel aligned surfaces to ensure
    // all coding features have enough room to work with
    } else if(theCodecCtx->codec_id == theVideo.CodecIdHEVC) {
        aSurfaceAlignment = 128;
    }

    // 4 base work surfaces
    myNbSurfaces = 4;

    // add surfaces based on number of possible refs
    if((theCodecCtx->codec_id == theVideo.CodecIdH264)
    || (theCodecCtx->codec_id == theVideo.CodecIdHEVC)) {
        myNbSurfaces += 16;
    } else {
        myNbSurfaces += 2;
    }

    // add extra surfaces for frame threading
    if(theCodecCtx->active_thread_type & FF_THREAD_FRAME) {
        myNbSurfaces += theCodecCtx->thread_count;
    }

    myD3dSurfaces = (IDirect3DSurface9** )av_mallocz(myNbSurfaces * sizeof(*myD3dSurfaces));
    mySurfInfos   = (StDxva2SurfaceInfo* )av_mallocz(myNbSurfaces * sizeof(*mySurfInfos));
    if(myD3dSurfaces == NULL
    || mySurfInfos   == NULL) {
        ST_ERROR_LOG(stCString("StVideoQueue: Unable to allocate surface arrays"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    HRESULT anHRes = myDecoderService->CreateSurface(FFALIGN(theCodecCtx->coded_width,  aSurfaceAlignment),
                                                     FFALIGN(theCodecCtx->coded_height, aSurfaceAlignment),
                                                     myNbSurfaces - 1,
                                                     aTargetD3dFormat, D3DPOOL_DEFAULT, 0,
                                                     DXVA2_VideoDecoderRenderTarget,
                                                     myD3dSurfaces, NULL);
    if(anHRes != S_OK) {
        onError(StString("StVideoQueue: Failed to create ") + myNbSurfaces + " video surfaces");
        decoderDestroy(theCodecCtx);
        return false;
    }

    anHRes = myDecoderService->CreateVideoDecoder(aDeviceGuid,
                                                  &aDesc, &aBestCfg, myD3dSurfaces,
                                                  myNbSurfaces, &myDxvaDecoder);
    if(anHRes != S_OK) {
        onError(stCString("StVideoQueue: Failed to create DXVA2 video decoder"));
        decoderDestroy(theCodecCtx);
        return false;
    }

    myDecoderGuid   = aDeviceGuid;
    myDecoderConfig = aBestCfg;

    theCodecCtx->hwaccel_context = &myDxvaCtxAV;
    stMemZero(&myDxvaCtxAV, sizeof(myDxvaCtxAV));
    myDxvaCtxAV.cfg           = &myDecoderConfig;
    myDxvaCtxAV.decoder       = myDxvaDecoder;
    myDxvaCtxAV.surface       = myD3dSurfaces;
    myDxvaCtxAV.surface_count = myNbSurfaces;
    myDxvaCtxAV.workaround    = 0;
    if(::IsEqualGUID(myDecoderGuid, DXVADDI_Intel_ModeH264_E)) {
        myDxvaCtxAV.workaround |= FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO;
    }

    return true;
}

bool StVideoQueue::hwaccelInit() {
    if(!myHWAccelCtx.isNull()
    && !myHWAccelCtx->isValid()) {
        return false;
    } else if(myCodecCtx->codec_id == AV_CODEC_ID_NONE) {
        return false;
    }

    if(myCodecCtx->codec_id == CodecIdH264
    && (myCodecCtx->profile & ~FF_PROFILE_H264_CONSTRAINED) > FF_PROFILE_H264_HIGH) {
        signals.onError(StString("StVideoQueue: Unsupported H.264 profile for DXVA2 HWAccel: ") + myCodecCtx->profile);
        return false;
    }

    if(myHWAccelCtx.isNull()) {
        myHWAccelCtx = new StDxva2Context();
        if(!myHWAccelCtx->create(*this)) {
            return false;
        }
    }

    if(!myHWAccelCtx->decoderCreate(*this, myCodecCtx)) {
        return false;
    }
    fillCodecInfo(myCodecCtx->codec, " (DXVA2)");
    return true;
}

#endif // _WIN32
