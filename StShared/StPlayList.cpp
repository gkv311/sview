/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StPlayList.h>

#include <StFile/StRawFile.h>
#include <StThreads/StProcess.h>

namespace {
    static size_t THE_UNDO_LIMIT = 1024;
};

StPlayItem::StPlayItem(StFileNode* theFileNode,
                       const StStereoParams& theDefParams)
: myPrevItem(NULL),
  myNextItem(NULL),
  myPosition(0),
  myFileNode(theFileNode),
  myStParams(new StStereoParams(theDefParams)),
  myPlayFlag(false) {
    //
}

StPlayItem::~StPlayItem() {
    if(myPrevItem != NULL) {
        myPrevItem->setNext(myNextItem);
    } else if(myNextItem != NULL) {
        myNextItem->setPrev(NULL);
    }
}

void StPlayItem::setPrev(StPlayItem* thePrev) {
    if(myPrevItem == thePrev) {
        return;
    }
    if(myPrevItem != NULL) {
        // silently remove self
        myPrevItem->myNextItem = NULL;
    }
    myPrevItem = thePrev;
    if(thePrev != NULL) {
        // set self as next item
        thePrev->setNext(this);
    }
}

void StPlayItem::setNext(StPlayItem* theNext) {
    if(myNextItem == theNext) {
        return;
    }
    if(myNextItem != NULL) {
        // silently remove self
        myNextItem->myPrevItem = NULL;
    }
    myNextItem = theNext;
    if(theNext != NULL) {
        // set self as previous item
        theNext->setPrev(this);
    }
}

StString StPlayItem::getPath() const {
    if(myFileNode == NULL) {
        return StString();
    } else if(!myFileNode->isEmpty()) {
        // for metafile return path to the first (master) file
        return myFileNode->getValue(0)->getPath();
    } else {
        return myFileNode->getPath();
    }
}

StString StPlayItem::getFolderPath() const {
    StString aTitleString;
    StString aFolder;
    StFileNode::getFolderAndFile(getPath(), aFolder, aTitleString);
    return aFolder;
}

StString StPlayItem::getTitle() const {
    if(!myTitle.isEmpty()) {
        return myTitle;
    }

    StString aTitleString;
    StString aFolder;
    StFileNode::getFolderAndFile(getPath(), aFolder, aTitleString);
    return aTitleString;
}

void StPlayItem::setTitle(const StString& theTitle) {
    myTitle = theTitle;
}

void StPlayList::addPlayItem(StPlayItem* theNewItem) {
    if(myFirst == NULL) {
        myFirst = myLast = myCurrent = theNewItem;
    } else {
        myLast->setNext(theNewItem);
        myLast = theNewItem;
    }
    theNewItem->setPosition(myItemsCount++);
}

void StPlayList::delPlayItem(StPlayItem* theRemItem) {
    if(myFirst == NULL || theRemItem == NULL) {
        // item does not exists in the list
        return;
    } else if(myFirst == theRemItem) {
        // removed first item from the list
        myFirst = myFirst->hasNext() ? myFirst->getNext() : NULL;
    }

    if(theRemItem->hasPrev()) {
        // connect previous and next items
        theRemItem->getPrev()->setNext(theRemItem->getNext());
    } else if(theRemItem->hasNext()) {
        theRemItem->getNext()->setPrev(NULL);
    }

    myStackPrev.clear();
    myStackNext.clear();

    --myItemsCount;
}

void StPlayList::addToPlayList(StFileNode* theFileNode) {
    for(size_t aNodeId = 0; aNodeId < theFileNode->size(); ++aNodeId) {
        StFileNode* aSubFileNode = theFileNode->changeValue(aNodeId);
        if(aSubFileNode->isFolder()) {
            addToPlayList(aSubFileNode);
        } else {
            addPlayItem(new StPlayItem(aSubFileNode, myDefStParams));
        }
    }
}

StPlayList::StPlayList(const int  theRecursionDeep,
                       const bool theIsLoop)
: myFirst(NULL),
  myLast(NULL),
  myCurrent(NULL),
  myItemsCount(0),
  myDefStParams(StStereoParams::FLAT_IMAGE),
  myPlayedCount(0),
  myRecursionDeep(theRecursionDeep),
  myIsShuffle(false),
  myIsLoopFlag(theIsLoop),
  myRecentLimit(10),
  myIsNewRecent(false) {
    //
}

void StPlayList::setExtensions(const StArrayList<StString>& theExtensions) {
    myExtensions = theExtensions;
    for(size_t anExtId = 0; anExtId < myExtensions.size(); ++anExtId) {
        if(myExtensions[anExtId].isEqualsIgnoreCase(stCString("m3u"))) {
            myExtensions.remove(anExtId); // playlist files are treated in special way
            --anExtId;
        }
    }
}

StPlayList::~StPlayList() {
    signals.onTitleChange.disconnect();
    signals.onPositionChange.disconnect();
    signals.onPlaylistChange.disconnect();
    clear();
}

bool StPlayList::isLoop() const {
    StMutexAuto anAutoLock(myMutex);
    return myIsLoopFlag;
}

void StPlayList::setLoop(bool theLoop) {
    StMutexAuto anAutoLock(myMutex);
    myIsLoopFlag = theLoop;
}

bool StPlayList::isShuffle() const {
    StMutexAuto anAutoLock(myMutex);
    return myIsShuffle;
}

void StPlayList::setShuffle(bool theShuffle) {
    StMutexAuto anAutoLock(myMutex);
    myIsShuffle = theShuffle;
}

void StPlayList::clear() {
    StMutexAuto anAutoLock(myMutex);
    // destroy double-linked list content
    for(StPlayItem *anItem(myFirst), *anItemToDel(NULL); anItem != NULL;) {
        anItemToDel = anItem;
        anItem = anItem->getNext();
        delete anItemToDel;
    }
    myStackPrev.clear();
    myStackNext.clear();
    myFirst = myLast = myCurrent = NULL;
    myItemsCount = myPlayedCount = 0;

    anAutoLock.unlock();
    signals.onPlaylistChange();
}

size_t StPlayList::getCurrentId() const {
    StMutexAuto anAutoLock(myMutex);
    return (myCurrent != NULL) ? myCurrent->getPosition() : 0;
}

StString StPlayList::getCurrentTitle() const {
    StMutexAuto anAutoLock(myMutex);
    return (myCurrent != NULL) ? myCurrent->getTitle() : StString();
}

bool StPlayList::walkToPosition(const size_t theId) {
    StMutexAuto anAutoLock(myMutex);

    size_t anIter = 0;
    for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext(), ++anIter) {
        if(anIter == theId) {
            if(myCurrent == anItem) {
                return false;
            }

            StPlayItem* aPrev = myCurrent;
            if(aPrev != NULL) {
                myStackPrev.push_back(aPrev);
                if(myStackPrev.size() > THE_UNDO_LIMIT) {
                    myStackPrev.pop_front();
                }
            }

            myCurrent = anItem;
            anAutoLock.unlock();
            signals.onPositionChange(theId);
            return true;
        }
    }

    return false;
}

bool StPlayList::walkToFirst() {
    StMutexAuto anAutoLock(myMutex);
    bool wasntFirst = (myCurrent != myFirst);
    myCurrent = myFirst;
    if(wasntFirst) {
        myStackPrev.clear();
        myStackNext.clear();
        anAutoLock.unlock();
        signals.onPositionChange(0);
    }
    return wasntFirst;
}

bool StPlayList::walkToLast() {
    StMutexAuto anAutoLock(myMutex);
    bool wasntLast = (myCurrent != myLast);
    myCurrent = myLast;
    if(wasntLast) {
        myStackPrev.clear();
        myStackNext.clear();
        const size_t anItemId = myCurrent != NULL ? myCurrent->getPosition() : 0;
        anAutoLock.unlock();
        signals.onPositionChange(anItemId);
    }
    return wasntLast;
}

bool StPlayList::walkToPrev() {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        return false;
    } else if(myIsShuffle && myItemsCount >= 3) {
        StPlayItem* aNext = myCurrent;
        if(!myStackPrev.empty()) {
            myCurrent = myStackPrev.back();
            myStackPrev.pop_back();
        } else if(myCurrent != myFirst) {
            myCurrent = myCurrent->getPrev();
        } else {
            aNext = NULL;
        }

        if(aNext != myCurrent
        && aNext != NULL) {
            myStackNext.push_front(aNext);
            if(myStackNext.size() > THE_UNDO_LIMIT) {
                myStackNext.pop_back();
            }
            const size_t anItemId = myCurrent->getPosition();
            anAutoLock.unlock();
            signals.onPositionChange(anItemId);
            return true;
        }
        return false;
    } else if(myCurrent != myFirst) {
        myCurrent = myCurrent->getPrev();
        const size_t anItemId = myCurrent->getPosition();
        anAutoLock.unlock();
        signals.onPositionChange(anItemId);
        return true;
    } else if(myIsLoopFlag) {
        return walkToLast();
    }
    return false;
}

bool StPlayList::walkToNext() {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        return false;
    } else if(myIsShuffle && myItemsCount >= 3) {
        /// TODO (Kirill Gavrilov#5) walk to the history front before next random
        StPlayItem* aPrev = myCurrent;
        if(!myStackNext.empty()) {
            myCurrent = myStackNext.front();
            myStackNext.pop_front();
        } else {
            if((myPlayedCount >= (myItemsCount - 1)) || (myPlayedCount == 0)) {
                // reset the playback counter
            #ifdef _WIN32
                FILETIME aTime;
                GetSystemTimeAsFileTime(&aTime);
                myRandGen.setSeed(aTime.dwLowDateTime);
            #else
                timeval aTime;
                gettimeofday(&aTime, NULL);
                myRandGen.setSeed(aTime.tv_usec);
            #endif
                myPlayedCount = 0;
                myCurrent->setPlayedFlag(!myCurrent->getPlayedFlag());
                ST_DEBUG_LOG("Restart the shuffle");
            }

            const size_t aCurrPos = myCurrent->getPosition();
            bool aCurrFlag  = myCurrent->getPlayedFlag();

            StPlayItem* aNextItem;
            size_t aNextPos;
            size_t aNextDiff;
            for(size_t anIter = 0;; ++anIter) {
                aNextItem = myCurrent;
                aNextPos = size_t(myRandGen.next() * myItemsCount);
                if(aNextPos > aCurrPos) {
                    // forward direction
                    aNextDiff = aNextPos - aCurrPos;
                    for(; aNextItem != NULL && aNextDiff > 0; --aNextDiff) {
                        aNextItem = aNextItem->getNext();
                    }
                } else {
                    // backward direction
                    aNextDiff = aCurrPos - aNextPos;
                    for(; aNextItem != NULL && aNextDiff > 0; --aNextDiff) {
                        aNextItem = aNextItem->getPrev();
                    }
                }
                if(aCurrFlag != aNextItem->getPlayedFlag()) {
                    // found the item!
                    break;
                } else if(anIter >= 2 * myItemsCount) {
                    // something wrong!
                    ST_DEBUG_LOG("Next shuffle position not found within " + anIter + " iterations!");
                    aCurrFlag = !aCurrFlag;
                }
            }
            ST_DEBUG_LOG(aCurrPos + " -> " + aNextPos);
            ++myPlayedCount; ///

            aNextItem->setPlayedFlag(aCurrFlag);
            myCurrent = aNextItem;
        }

        if(aPrev != myCurrent
        && aPrev != NULL) {
            myStackPrev.push_back(aPrev);
            if(myStackPrev.size() > THE_UNDO_LIMIT) {
                myStackPrev.pop_front();
            }
        }

        const size_t anItemId = myCurrent->getPosition();
        anAutoLock.unlock();
        signals.onPositionChange(anItemId);
        return true;
    } else if(myCurrent != myLast) {
        myCurrent = myCurrent->getNext();
        const size_t anItemId = myCurrent->getPosition();
        anAutoLock.unlock();
        signals.onPositionChange(anItemId);
        return true;
    } else if(myIsLoopFlag) {
        return walkToFirst();
    }
    return false;
}

StHandle<StFileNode> StPlayList::getCurrentFile() {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        // empty list
        return StHandle<StFileNode>();
    }
    StFileNode* aFileNode = myCurrent->getFileNode();
    if(aFileNode == NULL) {
        // invalid item
        return StHandle<StFileNode>();
    }
    return new StFileNode(aFileNode->getPath());
}

void StPlayList::setTitle(const StHandle<StStereoParams>& theKey,
                          const StString&                 theTitle) {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        return;
    }

    if(theKey != myCurrent->getParams()) {
        return;
    }

    const size_t anItemId = myCurrent->getPosition();
    myCurrent->setTitle(theTitle);
    anAutoLock.unlock();
    signals.onTitleChange(anItemId);
}

bool StPlayList::getCurrentFile(StHandle<StFileNode>& theFileNode,
                                StHandle<StStereoParams>& theParams) {
    theFileNode.nullify();
    theParams.nullify();
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        // empty list
        return false;
    }
    StFileNode* aFileNode = myCurrent->getFileNode();
    if(aFileNode == NULL) {
        // invalid item
        return false;
    }

    theFileNode = aFileNode->detach();
    theParams = myCurrent->getParams();
    return true;
}

void StPlayList::addToNode(const StHandle<StFileNode>& theFileNode,
                           const StString&             thePathToAdd) {
    StString aPath = theFileNode->getPath();
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        return;
    } else if(aPath != myCurrent->getPath()) {
        for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
            if(aPath == anItem->getPath()) {
                myCurrent = anItem;
                break;
            }
        }
    }

    StFileNode* aFileNode = myCurrent->getFileNode();
    if(aFileNode->getParent() != &myFoldersRoot) {
        // convert filenode to metafile with empty root
        aFileNode->reParent(&myFoldersRoot);
        aFileNode->setSubPath(StString());
        aFileNode->add(new StFileNode(aPath, aFileNode));
    }
    aFileNode->add(new StFileNode(thePathToAdd, aFileNode));
}

void StPlayList::removePhysically(const StHandle<StFileNode>& theFileNode) {
    StString aPath = theFileNode->getPath();
    StPlayItem* aRemItem = NULL;
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        // empty playlist
        return;
    } else if(aPath != myCurrent->getPath()) {
        // search play item
        for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
            if(aPath == anItem->getPath()) {
                aRemItem = anItem;
                break;
            }
        }
    } else {
        // walk to another playlist position
        aRemItem = myCurrent;
        if(myCurrent->hasNext()) {
            myCurrent = myCurrent->getNext();
        } else if(myCurrent->hasPrev()) {
            myCurrent = myCurrent->getPrev();
        } else {
            myCurrent = NULL;
        }
    }

    // remove item itself
    if(aRemItem != NULL && StFileNode::removeFile(aPath)) {
        delPlayItem(aRemItem);
        delete aRemItem;
    }

    anAutoLock.unlock();
    signals.onPlaylistChange();
}

bool StPlayList::checkExtension(const StString& thePath) {
    if(StFolder::isFolder(thePath)) {
        // just folder
        return true;
    }
    StString anExtension = StFileNode::getExtension(thePath);
    for(size_t anExtId = 0; anExtId < myExtensions.size(); ++anExtId) {
        if(anExtension.isEqualsIgnoreCase(myExtensions[anExtId])) {
            return true;
        }
    }
    if(anExtension.isEqualsIgnoreCase(stCString("m3u"))) {
        return true;
    }
    return false;
}

void StPlayList::addOneFile(const StString& theFilePath,
                            const StMIME&   theFileMIME) {
    StMutexAuto anAutoLock(myMutex);
    StFileNode* aFileNode = new StFileNode(theFilePath, &myFoldersRoot);
    aFileNode->setMIME(theFileMIME);
    myFoldersRoot.add(aFileNode);

    addRecentFile(*aFileNode); // append to recent files list
    addPlayItem(new StPlayItem(aFileNode, myDefStParams));

    anAutoLock.unlock();
    signals.onPlaylistChange();
}

void StPlayList::addOneFile(const StString& theFilePathLeft,
                            const StString& theFilePathRight) {
    StMutexAuto anAutoLock(myMutex);
    StFileNode* aFileNode = new StFileNode(StString(), &myFoldersRoot);
    aFileNode->add(new StFileNode(theFilePathLeft,  aFileNode));
    aFileNode->add(new StFileNode(theFilePathRight, aFileNode));
    myFoldersRoot.add(aFileNode);

    addRecentFile(*aFileNode); // append to recent files list
    addPlayItem(new StPlayItem(aFileNode, myDefStParams));

    anAutoLock.unlock();
    signals.onPlaylistChange();
}

static char* nextLine(char* theLine) {
    for(char* anIter = theLine;; ++anIter) {
        if(*anIter == '\0') {
            return anIter;
        } else if(*anIter == '\n') {
            return ++anIter;
        }
    }
}

char* StPlayList::parseM3UIter(char*     theIter,
                               StString& theTitle) {
    if(*theIter == '\0') {
        return NULL;
    }

    char* aNextLine = nextLine(theIter);
    if(*aNextLine != '\0') {
        // replace LF or CRLF with '\0'
        *(aNextLine - 1) = '\0';
        char* aTail = aNextLine - 2;
        if(*(aNextLine - 2) == '\x0D') {
            *(aNextLine - 2) = '\0';
            --aTail;
        }

        // skip trailing spaces
        for(; *aTail == ' ' && aTail >= theIter; --aTail) {
            *aTail = '\0';
        }
    }
    if(*theIter == '\0') {
        return aNextLine; // skip empty lines
    }

    if(*theIter != '#') {
        StFileNode* aFileNode = new StFileNode(theIter, &myFoldersRoot);
        myFoldersRoot.add(aFileNode);

        StPlayItem* anItem = new StPlayItem(aFileNode, myDefStParams);
        anItem->setTitle(theTitle);
        addPlayItem(anItem);
        theTitle = "";
    } else if(stAreEqual(theIter, "#EXTINF:", 8)) {
        theIter += 8;
        for(; *theIter != '\0'; ++theIter) {
            if(*theIter == ',') {
                for(; *theIter == ' '; ++theIter) {
                    // skip spaces in the beginning
                }

                theTitle = ++theIter;
                break;
            }
        }
    }
    return aNextLine;
}

void StPlayList::getSubList(StArrayList<StString>& theList,
                            const size_t           theStart,
                            const size_t           theEnd) const {
    theList.clear();
    StMutexAuto anAutoLock(myMutex);

    size_t anIter = 0;
    StPlayItem* anItem = myFirst;
    for(; anItem != NULL; anItem = anItem->getNext(), ++anIter) {
        if(anIter == theStart) {
            break;
        }
    }

    if(anIter != theStart) {
        return;
    }

    for(; anItem != NULL; anItem = anItem->getNext(), ++anIter) {
        if(anIter == theEnd) {
            break;
        }

        theList.add(anItem->getTitle());
    }
}

bool StPlayList::isRecentChanged() const {
    const bool aValue = myIsNewRecent;
    myIsNewRecent = false;
    return aValue;
}

void StPlayList::openRecent(const size_t theItemId) {
    StMutexAuto anAutoLock(myMutex);
    if(theItemId >= myRecent.size()) {
        return;
    }

    const StHandle<StFileNode>& aFile = myRecent[theItemId];
    if(aFile->size() == 2) {
        clear();
        addOneFile(aFile->getValue(0)->getPath(),
                   aFile->getValue(1)->getPath());
    } else {
        open(aFile->getPath());
    }
}

void StPlayList::clearRecent() {
    StMutexAuto anAutoLock(myMutex);
    myRecent.clear();
    myIsNewRecent = true;
}

void StPlayList::getRecentList(StArrayList<StString>& theList) const {
    theList.clear();
    StMutexAuto anAutoLock(myMutex);
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StFileNode>& aFile = myRecent[anIter];
        const StString aPath = aFile->isEmpty() ? aFile->getPath() : aFile->getValue(0)->getPath();
        StString aTitleString;
        StString aFolder;
        StFileNode::getFolderAndFile(aPath, aFolder, aTitleString);
        theList.add(aTitleString);
    }
}

void StPlayList::addRecentFile(const StFileNode& theFile,
                               const bool        theToFront) {
    // remove duplicates
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StFileNode>& aFile = myRecent[anIter];
        if(aFile->size() != theFile.size()) {
            continue;
        }
        bool areSame = true;
        for(size_t aChildIter = 0; aChildIter < aFile->size(); ++aChildIter) {
            areSame = areSame && (aFile->getValue(aChildIter)->getPath() == theFile.getValue(aChildIter)->getPath());
        }
        if(aFile->isEmpty()) {
            areSame = (aFile->getPath() == theFile.getPath());
        }
        if(areSame) {
            myRecent.erase(myRecent.begin() + anIter);
            break;
        }
    }

    if(myRecent.size() > myRecentLimit) {
        myRecent.pop_back();
    }
    if(theToFront) {
        myRecent.push_front(theFile.detach());
    } else {
        myRecent.push_back(theFile.detach());
    }
    myIsNewRecent = true;
}

StString StPlayList::dumpRecentList() const {
    StMutexAuto anAutoLock(myMutex);
    StArgumentsMap aMap;
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StFileNode>& aFile = myRecent[anIter];
        if(aFile->isEmpty()) {
            StArgument anArgFile (StString("file")  + anIter, aFile->getPath());
            aMap.add(anArgFile);
        } else if(aFile->size() == 2) {
            StArgument anArgLeft (StString("left")  + anIter, aFile->getValue(0)->getPath());
            StArgument anArgRight(StString("right") + anIter, aFile->getValue(1)->getPath());
            aMap.add(anArgLeft);
            aMap.add(anArgRight);
        }
    }
    return aMap.toString();
}

void StPlayList::loadRecentList(const StString theString) {
    StMutexAuto anAutoLock(myMutex);
    StArgumentsMap aMap;
    aMap.parseString(theString);
    myRecent.clear();

    for(size_t anIter = 0; anIter < myRecentLimit; ++anIter) {
        StArgument anArgFile  = aMap[StString("file")  + anIter];
        StArgument anArgLeft  = aMap[StString("left")  + anIter];
        StArgument anArgRight = aMap[StString("right") + anIter];
        if(anArgLeft.isValid() && anArgRight.isValid()) {
            StHandle<StFileNode> aFileNode = new StFileNode(StString());
            aFileNode->add(new StFileNode(anArgLeft.getValue(),  aFileNode.access()));
            aFileNode->add(new StFileNode(anArgRight.getValue(), aFileNode.access()));
            addRecentFile(*aFileNode, false);
        } else if(anArgFile.isValid()) {
            StFileNode aFileNode(anArgFile.getValue());
            addRecentFile(aFileNode, false);
        }
    }
}

void StPlayList::open(const StString& thePath) {
    StMutexAuto anAutoLock(myMutex);
    /// TODO (Kirill Gavrilov#2) do not scan folders again
    clear();
    int aSearchDeep = myRecursionDeep;
    StString aFolderPath;
    StString aFileName;
    if(StFolder::isFolder(thePath)) {
        // add all files from the folder and subfolders
        aFolderPath = thePath;
        aSearchDeep = myRecursionDeep;
    } else if(StFileNode::isFileExists(thePath)) {
        // search only current folder
        StFileNode::getFolderAndFile(thePath, aFolderPath, aFileName);
        aSearchDeep = 1;
        bool hasSupportedExt = false;
        StString anExt = StFileNode::getExtension(aFileName);
        for(size_t anExtId = 0; anExtId < myExtensions.size() && !hasSupportedExt; ++anExtId) {
            hasSupportedExt = anExt.isEqualsIgnoreCase(myExtensions[anExtId]);
        }

        // parse m3u playlist
        if(anExt.isEqualsIgnoreCase(stCString("m3u"))) {
            StRawFile aRawFile(thePath);
            if(aRawFile.readFile()) {
                StString aTitle;
                char* anIter = (char* )aRawFile.getBuffer();
                if(anIter[0] == '\xEF' && anIter[1] == '\xBB' && anIter[2] == '\xBF') {
                    // skip BOM for UTF8 written by some stupid programs
                    anIter += 3;
                }
                while(anIter != NULL) {
                    anIter = parseM3UIter(anIter, aTitle);
                }

                addRecentFile(StFileNode(thePath)); // append to recent files list

                anAutoLock.unlock();
                signals.onPlaylistChange();
                return;
            }
        }

        if(!hasSupportedExt) {
            // file with unsupported extension?
            StFileNode* aFileNode = new StFileNode(thePath, &myFoldersRoot);
            myFoldersRoot.add(aFileNode);
            addPlayItem(new StPlayItem(aFileNode, myDefStParams));
        }
    } else {
        // not a filesystem element - probably url or invalid path
        StFileNode* aFileNode = new StFileNode(thePath, &myFoldersRoot);
        myFoldersRoot.add(aFileNode);
        addRecentFile(*aFileNode); // append to recent files list
        addPlayItem(new StPlayItem(aFileNode, myDefStParams));

        anAutoLock.unlock();
        signals.onPlaylistChange();
        return;
    }
    StFolder* aSubFolder = new StFolder(aFolderPath, &myFoldersRoot);
    aSubFolder->init(myExtensions, aSearchDeep);
    myFoldersRoot.add(aSubFolder);

    addToPlayList(aSubFolder);

    myCurrent = myFirst;
    if(!aFileName.isEmpty()) {
        // set current item
        for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
            if(anItem->getPath() == thePath) {
                myCurrent = anItem;
                addRecentFile(*anItem->getFileNode()); // append to recent files list
                break;
            }
        }
    }

    anAutoLock.unlock();
    signals.onPlaylistChange();
}
