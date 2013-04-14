/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include <StStrings/StString.h>

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
    bool hasExtDisconnect;   //!< ALC_EXT_disconnect

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
     * Creates the AL device with specified name.
     */
    ST_CPPEXPORT bool create(const StString& theDeviceName = StString());

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

        private: //!< private fields

    ALCdevice*  hDevice;
    ALCcontext* hContext;

};

#endif //__StALContext_h_
