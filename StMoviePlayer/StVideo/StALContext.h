/**
 * Copyright © 2009-2025 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StALContext_h_
#define __StALContext_h_

#include <StStrings/StDictionary.h>

// OpenAL headers
#if (defined(__APPLE__))
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
    //#include <OpenAL/MacOSX_OALExtensions.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
    #include <AL/alext.h>
#endif

namespace {
    // ALC_SOFT_output_mode extension
    #define ALC_OUTPUT_MODE_SOFT                     0x19AC
    #define ALC_ANY_SOFT                             0x19AD
    #define ALC_MONO_SOFT                            0x1500
    #define ALC_STEREO_SOFT                          0x1501
    #define ALC_STEREO_BASIC_SOFT                    0x19AE
    #define ALC_STEREO_UHJ_SOFT                      0x19AF
    #define ALC_STEREO_HRTF_SOFT                     0x19B2
    #define ALC_QUAD_SOFT                            0x1503
    #define ALC_SURROUND_5_1_SOFT                    0x1504
    #define ALC_SURROUND_6_1_SOFT                    0x1505
    #define ALC_SURROUND_7_1_SOFT                    0x1506

    // Accepted as part of the <attrList> parameter of alcCreateContext and alcDeviceResetSOFT(), and as the <paramName> parameter of alcGetIntegerv()
    #define ALC_HRTF_SOFT                            0x1992

    // Accepted as part of the <attrList> parameter of alcCreateContext() and alcDeviceResetSOFT()
    #define ALC_HRTF_ID_SOFT                         0x1996

    // Accepted as part of the <attrList> parameter of alcCreateContext() and alcDeviceResetSOFT(), for the ALC_HRTF_SOFT attribute.
    #define ALC_DONT_CARE_SOFT                       0x0002

    // Accepted as the <paramName> parameter of alcGetIntegerv()
    #define ALC_HRTF_STATUS_SOFT                     0x1993
    #define ALC_NUM_HRTF_SPECIFIERS_SOFT             0x1994

    // Accepted as the <paramName> parameter of alcGetString() and alcGetStringiSOFT()
    #define ALC_HRTF_SPECIFIER_SOFT                  0x1995

    // Possible results from a ALC_HRTF_STATUS_SOFT query
    #define ALC_HRTF_DISABLED_SOFT                   0x0000

    #define ALC_HRTF_ENABLED_SOFT                    0x0001
    #define ALC_HRTF_DENIED_SOFT                     0x0002
    #define ALC_HRTF_REQUIRED_SOFT                   0x0003
    #define ALC_HRTF_HEADPHONES_DETECTED_SOFT        0x0004
    #define ALC_HRTF_UNSUPPORTED_FORMAT_SOFT         0x0005

    typedef const ALCchar* (ALC_APIENTRY* alcGetStringiSOFT_t )(ALCdevice* device, ALCenum paramName, ALCsizei index);
    typedef ALCboolean     (ALC_APIENTRY* alcResetDeviceSOFT_t)(ALCdevice* device, const ALCint* attrList);
}

/**
 * Wrapper over C-interface to OpenAL library.
 * Represent the device context.
 */
class StALContext {

        public:  //!< OpenAL extensions

    bool hasExtEAX2;         //!< EAX2.0 support
    bool hasExtFloat32;      //!< has 32bit float formats
    bool hasExtFloat64;      //!< has 64bit float formats
    bool hasExtMultiChannel; //!< has multichannel formats
    bool hasExtBFormat;      //!< AL_EXT_BFORMAT
    bool hasExtDisconnect;   //!< ALC_EXT_disconnect
    bool hasExtSoftOutMode;  //!< ALC_SOFT_output_mode
    bool hasExtSoftHrtf;     //!< ALC_SOFT_HRTF

    alcGetStringiSOFT_t  alcGetStringiSOFT;
    alcResetDeviceSOFT_t alcResetDeviceSOFT;

        public:

    /**
     * Empty constructor (doesn't initialize any AL device).
     */
    ST_CPPEXPORT StALContext();

    /**
     * Destructor, will release the AL device if any.
     */
    ST_CPPEXPORT ~StALContext();

    /**
     * @return human-readable list of available extensions.
     */
    ST_CPPEXPORT StString toStringExtensions() const;

    /**
     * Retrieve info from OpenAL context.
     */
    ST_CPPEXPORT void fullInfo(StDictionary& theMap) const;

    /**
     * Creates the AL device with specified name.
     */
    ST_CPPEXPORT bool create(const std::string& theDeviceName);

    /**
     * Release the AL device.
     */
    ST_CPPEXPORT void destroy();

    /**
     * Make AL device active in current thread.
     */
    ST_CPPEXPORT bool makeCurrent();

    /**
     * Notice that this method will always return true if extensions ALC_EXT_disconnect is not available.
     * @return true if device is in connected state.
     */
    ST_CPPEXPORT bool isConnected() const;

    /**
     * Return OpenAL device.
     */
    ST_LOCAL ALCdevice* getAlDevice() { return myAlDevice; }

    /**
     * Return OpenAL context.
     */
    ST_LOCAL ALCcontext* getAlContext() { return myAlContext; }

        private: //!< private fields

    ALCdevice*  myAlDevice;
    ALCcontext* myAlContext;

};

#endif //__StALContext_h_
