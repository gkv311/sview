/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
