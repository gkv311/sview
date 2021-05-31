/**
 * Copyright © 2010-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))
    // stat64 deprecated since Mac OS X 10.6
    #define _DARWIN_USE_64_BIT_INODE
#endif

#include <StFile/StFileNode.h>
#include <StFile/StMIMEList.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

StFileNode::StFileNode()
: StNode(stCString(""), NULL, NODE_TYPE_FILE) {
    //
}

StFileNode::StFileNode(const StCString& theSubPath,
                       StNode*          theParentNode,
                       int              theNodeType)
: StNode(theSubPath, theParentNode, theNodeType) {
    //
}

StFileNode::~StFileNode() {
    //
}

bool StFileNode::isFolder() const {
    return false;
}

const StFileNode* StFileNode::findValue(const StCString& theName) const {
    for(size_t aChildIter = 0; aChildIter < size(); ++aChildIter) {
        const StFileNode* aChildNode = getValue(aChildIter);
        if(aChildNode->getSubPath() == theName) {
            return aChildNode;
        }

        const StFileNode* aRecFindNode = aChildNode->findValue(theName);
        if(aRecFindNode != NULL) {
            return aRecFindNode;
        }
    }
    return NULL;
}

StHandle<StFileNode> StFileNode::detach() const {
    StHandle<StFileNode> aCopy = new StFileNode(getPath());
    aCopy->setMIME(getMIME());
    for(size_t aSubId = 0; aSubId < size(); ++aSubId) {
        const StFileNode* aSubNode = getValue(aSubId);
        aCopy->add(new StFileNode(aSubNode->getSubPath(), aCopy.access()));
    }
    return aCopy;
}

bool StFileNode::isFileExists(const StCString& thePath) {
#ifdef _WIN32
    StStringUtfWide aPath;
    aPath.fromUnicode(thePath);
    struct __stat64 aStatBuffer;
    return _wstat64(aPath.toCString(), &aStatBuffer) == 0;
#elif (defined(__APPLE__))
    struct stat aStatBuffer;
    return stat(thePath.toCString(), &aStatBuffer) == 0;
#else
    struct stat64 aStatBuffer;
    return stat64(thePath.toCString(), &aStatBuffer) == 0;
#endif
}

bool StFileNode::isFileReadOnly(const StCString& thePath) {
#ifdef _WIN32
    StStringUtfWide aPath;
    aPath.fromUnicode(thePath);
    DWORD anAttribs = ::GetFileAttributesW(aPath.toCString());
    return (anAttribs & FILE_ATTRIBUTE_READONLY) != 0;
#else
    // not implemented
    return false;
#endif
}

bool StFileNode::removeReadOnlyFlag(const StCString& thePath) {
#ifdef _WIN32
    StStringUtfWide aPath;
    aPath.fromUnicode(thePath);
    DWORD anAttribs = ::GetFileAttributesW(aPath.toCString());
    if(anAttribs == INVALID_FILE_ATTRIBUTES) {
        return false;
    } else if((anAttribs & FILE_ATTRIBUTE_READONLY) == 0) {
        return true;
    }

    anAttribs = (anAttribs & ~FILE_ATTRIBUTE_READONLY);
    return ::SetFileAttributesW(aPath.toCString(), anAttribs) != FALSE;
#else
    // not implemented
    return false;
#endif
}

bool StFileNode::removeFile(const StCString& thePath) {
#ifdef _WIN32
    StStringUtfWide aPath;
    aPath.fromUnicode(thePath);
    return DeleteFileW(aPath.toCString()) != 0;
#else
    return ::remove(thePath.toCString()) == 0;
#endif
}

/**bool StFileNode::copyFile(const StString& thePathFrom,
                          const StString& thePathTo) {
#if(defined(_WIN32) || defined(__WIN32__))
    return CopyFileW(thePathFrom.toUtfWide().toCString(),
                     thePathTo.toUtfWide().toCString(), TRUE) != 0;
#else

#endif
}*/

bool StFileNode::moveFile(const StCString& thePathFrom,
                          const StCString& thePathTo) {
#ifdef _WIN32
    StStringUtfWide aPathFrom; aPathFrom.fromUnicode(thePathFrom);
    StStringUtfWide aPathTo;   aPathTo  .fromUnicode(thePathTo);
    return MoveFileW(aPathFrom.toCString(),
                     aPathTo.toCString()) != 0;
#else
    return ::rename(thePathFrom.toCString(),
                    thePathTo.toCString()) == 0;
#endif
}

StString StFileNode::getCompatibleName(const StString& theFileName) {
#ifdef _WIN32
    stUtfWide_t aShortNameWide[MAX_PATH];
    GetShortPathNameW(theFileName.toUtfWide().toCString(), aShortNameWide, MAX_PATH);
    return *aShortNameWide != L'\0' ? StString(aShortNameWide) : theFileName;
#else
    return theFileName;
#endif
}

StString StFileNode::getExtension(const StString& theFileName) {
    size_t aPntId = size_t(-1);
    for(StUtf8Iter anIter = theFileName.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t('.')) {
            aPntId = anIter.getIndex();
        }
    }
    return (aPntId != size_t(-1)) ? theFileName.subString(aPntId + 1, theFileName.getLength()) : StString();
}

void StFileNode::getNameAndExtension(const StString& theFileName,
                                     StString& theName,
                                     StString& theExtension) {
    size_t aLastPnt = size_t(-1);
    for(StUtf8Iter anIter = theFileName.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t('.')) {
            aLastPnt = anIter.getIndex();
        }
    }
    if(aLastPnt != size_t(-1)) {
        theName      = theFileName.subString(0, aLastPnt);
        theExtension = theFileName.subString(aLastPnt + 1, theFileName.getLength());
    } else {
        theName      = theFileName;
        theExtension = StString();
    }
}

void StFileNode::getFolderAndFile(const StString& theFilePath,
                                  StString& theFolder,
                                  StString& theFileName) {
    size_t aLastSplit = size_t(-1);
    for(StUtf8Iter anIter = theFilePath.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t(SYS_FS_SPLITTER)) {
            aLastSplit = anIter.getIndex();
        }
    }

    if(aLastSplit != size_t(-1)) {
        theFolder   = theFilePath.subString(0, aLastSplit);
        theFileName = theFilePath.subString(aLastSplit + 1, theFilePath.getLength());
    } else {
        theFolder   = StString();
        theFileName = theFilePath;
    }
}

StString StFileNode::getFolderUp(const StString& thePath) {
    size_t aLastSplit = size_t(-1);
    size_t aPreSplit  = size_t(-1);
    for(StUtf8Iter anIter = thePath.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t(SYS_FS_SPLITTER)) {
            aPreSplit  = aLastSplit;
            aLastSplit = anIter.getIndex();
        }
    }

    if(aLastSplit != size_t(-1)
    && aLastSplit + 1 == thePath.getLength()) {
        aLastSplit = aPreSplit;
    }
    if(aLastSplit == size_t(-1)) {
        return StString();
    }

    return thePath.subString(0, aLastSplit);
}

bool StFileNode::isRemoteProtocolPath(const StCString& thePath) {
    StUtf8Iter anIter = thePath.iterator();
    if(*anIter == stUtf32_t(':')) {
        return false;
    }
    for(; *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t(':')) {
            return *(++anIter) == stUtf32_t('/')
                && *(++anIter) == stUtf32_t('/');
        }
    }
    return false;
}
