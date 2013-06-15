/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLResources_h_
#define __StGLResources_h_

#include <StThreads/StProcess.h>
#include <StStrings/StString.h>

class StGLResources {

        public:

    static StString getShadersRoot(const StString& theProjectName) {
        return StProcess::getStShareFolder() + "shaders" + SYS_FS_SPLITTER + theProjectName + SYS_FS_SPLITTER;
    }

    static StString getShaderFile(const StString& theProjectName,
                                  const StString& theShaderName) {
        StGLResources stResources(theProjectName);
        return stResources.getShadersRoot(theShaderName);
    }

        public:

    StGLResources(const StString& theProjectName) : myShadersRoot(getShadersRoot(theProjectName)) {}
    ~StGLResources() {}

    StString getShaderFile(const StString& theShaderName) const {
        return myShadersRoot + theShaderName;
    }

        private:

    StString myShadersRoot;

};

#endif // __StGLResources_h_
