/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StOpenInfo_h_
#define __StOpenInfo_h_

#include <stTypes.h>

#include <StFile/StMIME.h>
#include <StThreads/StProcess.h>

class StOpenInfo {

        private:

    // strings needed for struct
    StString mime; // MIME type, exported string
    StString path; // path to open
    StString args; // arguments string

        public:

    /**
     * Empty constructor.
     */
    StOpenInfo()
    : mime(),
      path(),
      args() {
        //
    }

    ~StOpenInfo() {
        //
    }

    bool isEmpty() const {
        return getMIME().isEmpty() && path.isEmpty();
    }

    void clear() {
        mime.clear();
        path.clear();
        args.clear();
    }

    /**
     * Parse formatted string and return MIME.
     */
    StMIME getMIME() const {
        return StMIME(mime);
    }

    /**
     * Automatically create formatted string from MIME.
     */
    void setMIME(const StMIME& theMime) {
        this->mime = theMime.toString();
    }

    /**
     * Returns path to open.
     */
    const StString getPath() const {
        return path;
    }

    /**
     * Returns true if path to open set.
     */
    bool hasPath() const {
        return !path.isEmpty();
    }

    /**
     * Set path to open.
     */
    void setPath(const StString& thePath) {
        this->path = thePath;
    }

    /**
     * Returns true if arguments not empty.
     */
    bool hasArgs() const {
        return !args.isEmpty();
    }

    /**
     * Parse string and return arguments map.
     */
    StArgumentsMap getArgumentsMap() const {
        StArgumentsMap agrsMap; agrsMap.parseString(args);
        return agrsMap;
    }

    void setArgumentsMap(const StArgumentsMap& newArgsMap) {
        args = newArgsMap.toString();
    }

};

#endif //__StOpenInfo_h_
