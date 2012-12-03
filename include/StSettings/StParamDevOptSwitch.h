/**
 * Copyright © 2011 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StParamDevOptSwitch_h_
#define __StParamDevOptSwitch_h_

#include <StSettings/StParam.h>

// forward declarations
class StWindow;
typedef struct tagStSDSwitch StSDSwitch_t;

/**
 * Parameter to show/control switch renderer device option.
 */
class ST_LOCAL StParamDevOptSwitch : public StInt32Param {

        private:

    StWindow*     myWindow; //!< link to the StWindow instance
    StSDSwitch_t* myOption; //!< pointer to the option

        public:

    /**
     * Main constructor.
     */
    StParamDevOptSwitch(StWindow*     theWindow,
                        StSDSwitch_t* theOptionPtr);

    /**
     * Retrieve value from shared options storage.
     */
    virtual int32_t getValue() const;

    /**
     * Change value, append update message to the window and emit connected slot.
     */
    virtual bool setValue(const int32_t theValue);

};

#endif //__StParamDevOptSwitch_h_
