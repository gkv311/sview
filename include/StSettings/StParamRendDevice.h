/**
 * Copyright © 2011-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StParamRendDevice_h_
#define __StParamRendDevice_h_

#include <StSettings/StParam.h>

#include <StCore/StCore.h>

// forward declarations
class StWindow;
typedef struct tagStSDOptionsList StSDOptionsList_t;

/**
 * Parameter indicates/controls active renderer device.
 */
class StParamRendDevice : public StInt32Param {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StParamRendDevice(StWindow* theWindow);

    /**
     * Overridden method that retrieve value from StWindow instance.
     */
    ST_CPPEXPORT virtual int32_t getValue() const;

    /**
     * Append change device message to StWindow instance
     * and emit connected slots.
     */
    ST_CPPEXPORT virtual bool setValue(const int32_t theValue);

    /**
     * @return renderers list.
     */
    ST_CPPEXPORT const StArrayList<StRendererInfo>& getRenderers() const;

        private:

    /**
     * Access to the shared option structure.
     */
    ST_LOCAL StSDOptionsList_t* getSharedInfo() const;

        private:

    StWindow*                   myWindow;     //!< link to the StWindow instance
    StArrayList<StRendererInfo> myRenderers;  //!< renderers list
    size_t                      myPluginBase; //!< base device id
    int32_t                     myPluginLim;  //!< devices number for active plugin

};

#endif //__StParamRendDevice_h_
