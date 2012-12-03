/**
 * Copyright © 2011 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StParamDevOptOnOff_h_
#define __StParamDevOptOnOff_h_

#include <StSettings/StParam.h>

// forward declarations
class StWindow;
typedef struct tagStSDOnOff StSDOnOff_t;

/**
 * Parameter to show/control on/off renderer device option.
 */
class ST_LOCAL StParamDevOptOnOff : public StBoolParam {

        private:

    StWindow*     myWindow; //!< link to the StWindow instance
    StSDOnOff_t*  myOption; //!< pointer to the option

        public:

    /**
     * Main constructor.
     */
    StParamDevOptOnOff(StWindow*    theWindow,
                       StSDOnOff_t* theOptionPtr);

    /**
     * Retrieve value from shared options storage.
     */
    virtual bool getValue() const;

    /**
     * Change value, append update message to the window and emit connected slot.
     */
    virtual bool setValue(const bool theValue);

};

#endif //__StParamDevOptOnOff_h_
