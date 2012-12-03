/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StALContext {

        private: //!< private fields

    ALCdevice*   hDevice;
    ALCcontext* hContext;

        public:  //!< OpenAL extensions

    bool hasExtEAX2;         //!< EAX2.0 support
    bool hasExtFloat32;      //!< has 32bit float formats
    bool hasExtFloat64;      //!< has 64bit float formats
    bool hasExtMultiChannel; //!< has multichannel formats

        public:

    /**
     * Empty constructor (doesn't initialize any AL device).
     */
    StALContext();

    /**
     * Destructor, will release the AL device if any.
     */
    ~StALContext();

    /**
     * @return human-readable list of available extensions.
     */
    StString toStringExtensions() const;

    /**
     * Creates the AL device with specified name.
     */
    bool create(const StString& theDeviceName = StString());

    /**
     * Release the AL device.
     */
    void destroy();

    /**
     * Make AL device active in current thread.
     */
    bool makeCurrent();

};

#endif //__StALContext_h_
