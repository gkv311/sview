/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StPlayList.h>

#include <StFile/StRawFile.h>
#include <StThreads/StProcess.h>

#include <sstream>

namespace {
    static size_t THE_UNDO_LIMIT = 1024;
}

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
    }

    // update first/last items
    if(theRemItem == myLast) {
        myLast  = myLast->hasPrev()  ? myLast->getPrev()  : NULL;
    }
    if(myFirst == theRemItem) {
        myFirst = myFirst->hasNext() ? myFirst->getNext() : NULL;
    }

    // reset enumeration
    size_t aPosId = theRemItem->getPosition();
    for(StPlayItem* anIter = theRemItem->getNext(); anIter != NULL; ++aPosId, anIter = anIter->getNext()) {
        anIter->setPosition(aPosId);
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
  myDefStParams(),
  myPlayedCount(0),
  myRecursionDeep(theRecursionDeep),
  myIsShuffle(false),
  myToLoopSingle(false),
  myIsLoopFlag(theIsLoop),
  myRecentLimit(10),
  myIsNewRecent(false),
  myWasCleared(false) {
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

void StPlayList::setLoop(const bool theLoop) {
    StMutexAuto anAutoLock(myMutex);
    myIsLoopFlag = theLoop;
}

void StPlayList::setLoopSingle(const bool theValue) {
    StMutexAuto anAutoLock(myMutex);
    myToLoopSingle = theValue;
}

bool StPlayList::isShuffle() const {
    StMutexAuto anAutoLock(myMutex);
    return myIsShuffle;
}

void StPlayList::setShuffle(bool theShuffle) {
    StMutexAuto anAutoLock(myMutex);
    myIsShuffle = theShuffle;
}

int32_t StPlayList::getSerial() {
    StMutexAuto anAutoLock(myMutex);
    if(myWasCleared
    && myFirst != NULL) {
        myWasCleared = false;
        mySerial.increment();
    }
    return mySerial.getValue();
}

void StPlayList::clear() {
    StMutexAuto anAutoLock(myMutex);
    if(myFirst != NULL) {
        myWasCleared = true;
        mySerial.increment();
    }

    if(!myPlsFile.isNull()
    && myCurrent != NULL) {
        if(myPlsFile->File->isEmpty()) {
            myPlsFile->File->add(new StFileNode(myCurrent->getPath(), myPlsFile->File.access()));
        } else {
            myPlsFile->File->changeValue(0)->setSubPath(myCurrent->getPath());
        }
    }
    myPlsFile.nullify();

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

StPlayList::CurrentPosition StPlayList::getCurrentPosition() const {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        return CurrentPosition_NONE;
    } else if(myCurrent == myFirst) {
        if(myCurrent == myLast) {
            return CurrentPosition_Single;
        }
        return CurrentPosition_First;
    } else if(myCurrent == myLast) {
        return CurrentPosition_Last;
    }
    return CurrentPosition_Middle;
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

bool StPlayList::walkToNext(const bool theToForce) {
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL
    || (myToLoopSingle && !theToForce)) {
        return false;
    } else if(myIsShuffle && myItemsCount >= 3) {
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

            // determine next random position
            const size_t aCurrPos  = myCurrent->getPosition();
            bool         aCurrFlag = myCurrent->getPlayedFlag();
            StPlayItem*  aNextItem = myCurrent;
            const size_t aNextPos  = stMin(size_t(myRandGen.next() * myItemsCount), myItemsCount - 1);
            if(aNextPos > aCurrPos) {
                // forward direction
                for(size_t aNextDiff = aNextPos - aCurrPos; aNextItem != NULL && aNextDiff != 0; --aNextDiff) {
                    aNextItem = aNextItem->getNext();
                }
            } else {
                // backward direction
                for(size_t aNextDiff = aCurrPos - aNextPos; aNextItem != NULL && aNextDiff != 0; --aNextDiff) {
                    aNextItem = aNextItem->getPrev();
                }
            }
            if(aCurrFlag == aNextItem->getPlayedFlag()) {
                // find nearest position not yet played - prefer item farther from current one
                StPlayItem* aNextItem1 = aNextPos > aCurrPos ? aNextItem->getNext() : aNextItem->getPrev();
                StPlayItem* aNextItem2 = aNextPos > aCurrPos ? aNextItem->getPrev() : aNextItem->getNext();
                for(; aNextItem1 != NULL || aNextItem2 != NULL;) {
                    if(aNextItem1 != NULL) {
                        if(aCurrFlag != aNextItem1->getPlayedFlag()) {
                            aNextItem = aNextItem1;
                            break;
                        }
                        aNextItem1 = aNextPos > aCurrPos ? aNextItem1->getNext() : aNextItem1->getPrev();
                    }
                    if(aNextItem2 != NULL) {
                        if(aCurrFlag != aNextItem2->getPlayedFlag()) {
                            aNextItem = aNextItem2;
                            break;
                        }
                        aNextItem2 = aNextPos > aCurrPos ? aNextItem2->getPrev() : aNextItem2->getNext();
                    }
                }
                if(aCurrFlag == aNextItem->getPlayedFlag()) {
                    // something wrong!
                    ST_DEBUG_LOG("Disaster - next shuffle position not found!");
                    aCurrFlag     = !aCurrFlag;
                    myPlayedCount = 0;
                }
            }

            ST_DEBUG_LOG(aCurrPos + " -> " + aNextItem->getPosition());
            ++myPlayedCount;

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

bool StPlayList::getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                StHandle<StStereoParams>& theParams,
                                StHandle<StFileNode>&     thePlsFile) {
    theFileNode.nullify();
    theParams.nullify();
    thePlsFile.nullify();
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
    theParams   = myCurrent->getParams();
    if(!myPlsFile.isNull()) {
        thePlsFile = myPlsFile->File;
    }
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

bool StPlayList::remove(const StString& thePath,
                        const bool      theToRemovePhysically) {
    StString    aPath    = thePath;
    StPlayItem* aRemItem = NULL;
    StMutexAuto anAutoLock(myMutex);
    if(myCurrent == NULL) {
        // empty playlist
        return false;
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
        const bool aPlayedFlag = aRemItem->getPlayedFlag();
        if(myCurrent->hasNext()) {
            myCurrent = myCurrent->getNext();
        } else if(myCurrent->hasPrev()) {
            myCurrent = myCurrent->getPrev();
        } else {
            myCurrent     = NULL;
            myPlayedCount = 0;
        }

        if(myCurrent != NULL) {
            if(myCurrent->getPlayedFlag() != aPlayedFlag) {
                // the item has not been played yet - mark it as such
                myCurrent->setPlayedFlag(aPlayedFlag);
            } else {
                // one played item has been removed
                --myPlayedCount;
            }
        }
    }

    // remove item itself
    const bool isDeleted = aRemItem != NULL
                        && (!theToRemovePhysically || StFileNode::removeFile(aPath));
    if(isDeleted) {
        delPlayItem(aRemItem);
        delete aRemItem;
    }

    anAutoLock.unlock();
    signals.onPlaylistChange();
    return isDeleted;
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
                               StFolder* theFolder,
                               StString& theTitle) {
    if(*theIter == '\0') {
        return NULL;
    }

    char* aNextLine = nextLine(theIter);
    if(aNextLine > theIter + 1) {
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
        StString    anItemPath = theIter;
        StFolder*   aFolder    = theFolder != NULL
                              && StFileNode::isRelativePath(anItemPath)
                               ? theFolder
                               : &myFoldersRoot;
        StFileNode* aFileNode  = new StFileNode(anItemPath, aFolder);
        aFolder->add(aFileNode);

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

bool StPlayList::saveM3U(const StCString& thePath) {
    StRawFile aFile;
    if(thePath.isEmpty()
    || !aFile.openFile(StRawFile::WRITE, thePath)) {
        return false;
    }

    StMutexAuto anAutoLock(myMutex);
    aFile.write(stCString("#EXTM3U"));

    for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
        const StFileNode* aNode = anItem->getFileNode();
        if(aNode == NULL) {
            continue;
        } else if(aNode->size() < 2) {
            aFile.write(stCString("\n#EXTINF:0,"));
            if(anItem->hasCustomTitle()) {
                aFile.write(anItem->getTitle());
            }
            aFile.write(stCString("\n"));
            aFile.write(aNode->getPath());
        }
    }
    aFile.write(stCString("\n"));
    return true;
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

namespace {
    ST_LOCAL bool stAreSameRecent(const StFileNode& theA,
                                  const StFileNode& theB) {
        if(theB.size() < 2) {
            return theB.getPath() == theA.getPath();
        } else if(theB.size() != theA.size()) {
            return false;
        }

        bool areSame = true;
        for(size_t aChildIter = 0; aChildIter < theB.size(); ++aChildIter) {
            areSame = areSame && (theB.getValue(aChildIter)->getPath() == theA.getValue(aChildIter)->getPath());
        }
        return areSame;
    }
};

bool StPlayList::isRecentChanged() const {
    const bool aValue = myIsNewRecent;
    myIsNewRecent = false;
    return aValue;
}

size_t StPlayList::findRecent(const StString thePathL,
                              const StString thePathR) const {
    StFileNode aNode;
    if(thePathR.isEmpty()) {
        aNode.setSubPath(thePathL);
    } else {
        aNode.add(new StFileNode(thePathL, &aNode));
        aNode.add(new StFileNode(thePathR, &aNode));
    }

    StMutexAuto anAutoLock(myMutex);
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StRecentItem>& aRecent = myRecent[anIter];
        if(stAreSameRecent(aNode, *aRecent->File)) {
            return anIter;
        }
    }
    return size_t(-1);
}

StHandle<StStereoParams> StPlayList::openRecent(const size_t theItemId) {
    StMutexAuto anAutoLock(myMutex);
    if(theItemId >= myRecent.size()) {
        return StHandle<StStereoParams>();
    }

    const StHandle<StRecentItem> aRecent = myRecent[theItemId];
    const StHandle<StFileNode>   aFile   = aRecent->File;
    if(aFile->size() == 2) {
        // stereo pair from two files
        clear();
        addOneFile(aFile->getValue(0)->getPath(),
                   aFile->getValue(1)->getPath());
    } else if(aFile->size() == 1) {
        // playlist
        open(aFile->getPath(), aFile->getValue(0)->getSubPath());
    } else {
        // single file
        open(aFile->getPath());
    }
    return aRecent->Params;
}

void StPlayList::updateRecent(const StHandle<StFileNode>&     theFile,
                              const StHandle<StStereoParams>& theParams) {
    if(theFile.isNull()) {
        return;
    }

    StMutexAuto anAutoLock(myMutex);
    if(!myPlsFile.isNull()
     && myPlsFile->File == theFile) {
        // remember properties of last played file
        myPlsFile->Params = theParams;
        return;
    }

    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        StHandle<StRecentItem>& aRecent = myRecent[anIter];
        if(stAreSameRecent(*theFile, *aRecent->File)) {
            aRecent->Params = theParams;
            return;
        }
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
        const StHandle<StRecentItem>& aRecent = myRecent[anIter];
        const StHandle<StFileNode>&   aFile   = aRecent->File;

        const StString aPath = aFile->size() == 2 ? aFile->getValue(0)->getPath() : aFile->getPath();
        StString aTitleString;
        StString aFolder;
        StFileNode::getFolderAndFile(aPath, aFolder, aTitleString);
        theList.add(aTitleString);
    }
}

void StPlayList::currentToRecent() {
    StMutexAuto anAutoLock(myMutex);
    if( myCurrent == NULL
    || !myPlsFile.isNull()) {
        return;
    }

    StHandle<StPlayList::StRecentItem> aRecent = addRecentFile(*myCurrent->getFileNode());
    aRecent->Params = myCurrent->getParams();
}

const StHandle<StPlayList::StRecentItem>& StPlayList::addRecentFile(const StFileNode& theFile,
                                                                    const bool        theToFront) {
    // remove duplicates
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StRecentItem>& aRecent = myRecent[anIter];
        if(stAreSameRecent(theFile, *aRecent->File)) {
            myRecent.erase(myRecent.begin() + anIter);
            break;
        }
    }

    if(myRecent.size() > myRecentLimit) {
        myRecent.pop_back();
    }

    StHandle<StRecentItem> aNewRecent = new StRecentItem();
    if(StFileNode::isContentProtocolPath(theFile.getPath())) {
        // ignore temporary URLs
        aNewRecent->File = new StFileNode();
    } else {
        aNewRecent->File = theFile.detach();
    }
    if(theToFront) {
        myRecent.push_front(aNewRecent);
    } else {
        myRecent.push_back(aNewRecent);
    }
    myIsNewRecent = true;
    return theToFront ? myRecent.front() : myRecent.back();
}

StString StPlayList::dumpRecentList() const {
    StMutexAuto anAutoLock(myMutex);
    StArgumentsMap aMap;
    for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
        const StHandle<StRecentItem>&   aRecent = myRecent[anIter];
        const StHandle<StFileNode>&     aFile   = aRecent->File;
        const StHandle<StStereoParams>& aParams = aRecent->Params;
        if(!myPlsFile.isNull()
         && aFile == myPlsFile->File
         && myCurrent != NULL) {
            StArgument anArgFile(StString("file")  + anIter, aFile->getPath());
            StArgument anArgPos (StString("pos")   + anIter, myCurrent->getPath());
            aMap.add(anArgFile);
            aMap.add(anArgPos);
        } else if(aFile->isEmpty()) {
            StArgument anArgFile(StString("file")  + anIter, aFile->getPath());
            aMap.add(anArgFile);
        } else if(aFile->size() == 1) {
            StArgument anArgFile(StString("file")  + anIter, aFile->getPath());
            StArgument anArgPos (StString("pos")   + anIter, aFile->getValue(0)->getSubPath());
            aMap.add(anArgFile);
            aMap.add(anArgPos);
        } else if(aFile->size() == 2) {
            StArgument anArgLeft (StString("left")  + anIter, aFile->getValue(0)->getPath());
            StArgument anArgRight(StString("right") + anIter, aFile->getValue(1)->getPath());
            aMap.add(anArgLeft);
            aMap.add(anArgRight);
        }
        if(!aParams.isNull()) {
            if(aParams->Timestamp > 360.0) {
                std::stringstream aStream;
                aStream.imbue(std::locale("C"));
                aStream << aParams->Timestamp;
                const StString aStrValue = aStream.str().c_str();
                StArgument anArgTime(StString("time")  + anIter, aStrValue);
                aMap.add(anArgTime);
            }
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
        const StArgument anArgFile  = aMap[StString("file")  + anIter];
        const StArgument anArgLeft  = aMap[StString("left")  + anIter];
        const StArgument anArgRight = aMap[StString("right") + anIter];
        const StArgument anArgTime  = aMap[StString("time")  + anIter];
        StHandle<StRecentItem> aRecent;
        if(anArgLeft.isValid() && anArgRight.isValid()) {
            StHandle<StFileNode> aFileNode = new StFileNode(StString());
            aFileNode->add(new StFileNode(anArgLeft.getValue(),  aFileNode.access()));
            aFileNode->add(new StFileNode(anArgRight.getValue(), aFileNode.access()));
            aRecent = addRecentFile(*aFileNode, false);
        } else if(anArgFile.isValid()) {
            StHandle<StFileNode> aFileNode = new StFileNode(anArgFile.getValue());
            const StArgument anArgPos = aMap[StString("pos") + anIter];
            if(anArgPos.isValid()) {
                aFileNode->add(new StFileNode(anArgPos.getValue(), aFileNode.access()));
            }
            aRecent = addRecentFile(*aFileNode, false);
        }
        if(aRecent.isNull()) {
            continue;
        }

        if(anArgTime.isValid()) {
            if(aRecent->Params.isNull()) {
                aRecent->Params = new StStereoParams();
            }
            std::stringstream aStream;
            aStream.imbue(std::locale("C"));
            aStream << anArgTime.getValue().toCString();
            aStream >> aRecent->Params->Timestamp;
        }
    }
}

/**
 * Skip BOM for UTF8 written by some weird programs
 */
static inline char* skipBOM(char* thePos) {
    if(thePos[0] == '\xEF'
    && thePos[1] == '\xBB'
    && thePos[2] == '\xBF') {
        return thePos + 3;
    }
    return thePos;
}

void StPlayList::open(const StCString& thePath,
                      const StCString& theItem) {
    StMutexAuto anAutoLock(myMutex);

    // check if it is recently played playlist
    bool hasTarget = !theItem.isEmpty();
    StString aTarget = hasTarget ? theItem : thePath;
    if(!hasTarget) {
        for(size_t anIter = 0; anIter < myRecent.size(); ++anIter) {
            const StHandle<StRecentItem>& aRecent = myRecent[anIter];
            const StHandle<StFileNode>&   aFile   = aRecent->File;
            if(aFile->size() != 1) {
                continue;
            }

            if(thePath.isEquals(aFile->getPath())) {
                hasTarget = true;
                aTarget = aFile->getValue(0)->getSubPath();
                break;
            }
        }
    }

    clear();
    int aSearchDeep = myRecursionDeep;
    StString aFolderPath;
    StString aFileName;
    if(StFolder::isFolder(thePath)) {
        // add all files from the folder and subfolders
        aFolderPath = thePath;
        aSearchDeep = myRecursionDeep;
        myPlsFile   = addRecentFile(StFileNode(thePath)); // append to recent files list
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
        if(anExt.isEqualsIgnoreCase(stCString("m3u"))
        || anExt.isEqualsIgnoreCase(stCString("m3u8"))) {
            StHandle<StRawFile> aRawFile = new StRawFile(thePath);
            if(aRawFile->readFile()) {
                StFolder* aPlsFolder = new StFolder(aFolderPath, &myFoldersRoot);
                myFoldersRoot.add(aPlsFolder);

                StString aTitle;
                for(char* anIter = skipBOM((char* )aRawFile->getBuffer()); anIter != NULL;) {
                    anIter = parseM3UIter(anIter, aPlsFolder, aTitle);
                }
                aRawFile.nullify();

                if(myFirst != nullptr
                && myFirst->getNext() == nullptr) {
                    const StString aFirstPath = myFirst->getPath();
                    StString anItemExt = StFileNode::getExtension(aFirstPath);
                    if(anItemExt.isEqualsIgnoreCase(stCString("m3u"))
                    || anItemExt.isEqualsIgnoreCase(stCString("m3u8"))) {
                        aRawFile = new StRawFile(aFirstPath);
                        if(aRawFile->readFile()) {
                            for(char* anIter = skipBOM((char* )aRawFile->getBuffer()); anIter != NULL;) {
                                anIter = parseM3UIter(anIter, NULL, aTitle);
                            }
                            remove(aFirstPath, false);
                        }
                        aRawFile.nullify();
                    }
                }

                myPlsFile = addRecentFile(StFileNode(thePath)); // append to recent files list
                if(hasTarget) {
                    // set current item
                    for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
                        if(anItem->getPath() == aTarget) {
                            myCurrent = anItem;
                            break;
                        }
                    }
                }

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
        StFolder* aSubFolder = new StFolder(stCString(""), &myFoldersRoot);
        myFoldersRoot.add(aSubFolder);

        StFileNode* aFileNode = new StFileNode(thePath, aSubFolder);
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
    if(hasTarget || !aFileName.isEmpty()) {
        // set current item
        for(StPlayItem* anItem = myFirst; anItem != NULL; anItem = anItem->getNext()) {
            if(anItem->getPath() == aTarget) {
                myCurrent = anItem;
                if(myPlsFile.isNull()) {
                    addRecentFile(*anItem->getFileNode()); // append to recent files list
                }
                break;
            }
        }
    }

    anAutoLock.unlock();
    signals.onPlaylistChange();
}
