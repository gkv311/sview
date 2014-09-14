/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StThreads/StResourceManager.h>

#include <StThreads/StProcess.h>

#include <limits>
#ifdef max
    #undef max
#endif

StResourceManager::StResourceManager()
: myRoot(StProcess::getStShareFolder()) {
    //
}

StResourceManager::~StResourceManager() {
    //
}

bool StResourceManager::isResourceExist(const StString& theName) const {
    return StFileNode::isFileExists(myRoot + theName);
}

StHandle<StResource> StResourceManager::getResource(const StString& theName) const {
    const StString aPath = myRoot + theName;
    if(StFileNode::isFileExists(aPath)) {
        return new StFileResource(theName, aPath);
    }
    return StHandle<StResource>();
}

StResource::StResource(const StString& theName,
                       const StString& thePath)
: myName(theName),
  myPath(thePath),
  myData(NULL),
  mySize(0) {
    //
}

StResource::~StResource() {
    //
}

StFileResource::StFileResource(const StString& theName,
                               const StString& thePath)
: StResource(theName, thePath) {
    //
}

StFileResource::~StFileResource() {
    //
}

bool StFileResource::read() {
    if(myData != NULL) {
        return true;
    } else if(!myFile.readFile(myPath)) {
        return false;
    } else if(myFile.getSize() > size_t(std::numeric_limits<int>::max())) {
        return false;
    }

    myData = myFile.getBuffer();
    mySize = (int )myFile.getSize();
    return true;
}
