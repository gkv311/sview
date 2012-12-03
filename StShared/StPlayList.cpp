/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StPlayList.h>

#include <StFile/StRawFile.h>

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
    StString aTitleString;
    StString aFolder;
    StFileNode::getFolderAndFile(getPath(), aFolder, aTitleString);
    return aTitleString;
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

StPlayList::StPlayList(const StArrayList<StString>& theExtensions,
                       int theRecursionDeep,
                       bool theIsLoop)
: myMutex(),
  myFoldersRoot(),
  myFirst(NULL),
  myLast(NULL),
  myCurrent(NULL),
  myItemsCount(0),
  myExtensions(theExtensions),
  myDefStParams(StStereoParams::FLAT_IMAGE),
  myRandGen(),
  myPlayedCount(0),
  myRecursionDeep(theRecursionDeep),
  myIsShuffle(false),
  myIsLoopFlag(theIsLoop) {
    //
}

StPlayList::~StPlayList() {
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
    myFirst = myLast = myCurrent = NULL;
    myItemsCount = myPlayedCount = 0;
}

StString StPlayList::getCurrentTitle() const {
    StMutexAuto anAutoLock(myMutex);
    return (myCurrent != NULL) ? myCurrent->getTitle() : StString();
}

bool StPlayList::walkToFirst() {
    StMutexAuto anAutoLock(myMutex);
    bool wasntFirst = (myCurrent != myFirst);
    myCurrent = myFirst;
    if(wasntFirst) {
        signals.onPositionChange();
    }
    return wasntFirst;
}

bool StPlayList::walkToLast() {
    StMutexAuto anAutoLock(myMutex);
    bool wasntLast = (myCurrent != myLast);
    myCurrent = myLast;
    if(wasntLast) {
        signals.onPositionChange();
    }
    return wasntLast;
}

bool StPlayList::walkToPrev() {
    StMutexAuto anAutoLock(myMutex);

    /// TODO (Kirill Gavrilov#5) walk in history stack
    ///if(myIsShuffle) {

    if((myCurrent != myFirst) && myCurrent != NULL) {
        myCurrent = myCurrent->getPrev();
        signals.onPositionChange();
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

        if((myPlayedCount >= (myItemsCount - 1)) || (myPlayedCount == 0)) {
            // reset the playback counter
            /// TODO (Kirill Gavrilov#5) use external seed
            ///myRandGen.setSeed();
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

        signals.onPositionChange();
        return true;
    } else if(myCurrent != myLast) {
        myCurrent = myCurrent->getNext();
        signals.onPositionChange();
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
    return false;
}

void StPlayList::addOneFile(const StString& theFilePath,
                            const StMIME&   theFileMIME) {
    StMutexAuto anAutoLock(myMutex);
    StFileNode* aFileNode = new StFileNode(theFilePath, &myFoldersRoot);
    aFileNode->setMIME(theFileMIME);
    myFoldersRoot.add(aFileNode);
    addPlayItem(new StPlayItem(aFileNode, myDefStParams));
}

void StPlayList::addOneFile(const StString& theFilePathLeft,
                            const StString& theFilePathRight) {
    StMutexAuto anAutoLock(myMutex);
    StFileNode* aFileNode = new StFileNode(StString(), &myFoldersRoot);
    aFileNode->add(new StFileNode(theFilePathLeft,  aFileNode));
    aFileNode->add(new StFileNode(theFilePathRight, aFileNode));
    myFoldersRoot.add(aFileNode);
    addPlayItem(new StPlayItem(aFileNode, myDefStParams));
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

char* StPlayList::parseM3UIter(char* theIter) {
    if(*theIter == '\0') {
        return NULL;
    }

    char* aNextLine = nextLine(theIter);
    if(*aNextLine != '\0') {
        // replace CRLF with '\0'
        *(aNextLine - 1) = '\0';
        if(*(aNextLine - 2) == '\x0D') {
            *(aNextLine - 2) = '\0';
        }
    }
    if(*theIter != '#') {
        StFileNode* aFileNode = new StFileNode(theIter, &myFoldersRoot);
        myFoldersRoot.add(aFileNode);
        addPlayItem(new StPlayItem(aFileNode, myDefStParams));
    }
    return aNextLine;
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
        if(anExt.isEqualsIgnoreCase("m3u")) {
            StRawFile aRawFile(thePath);
            if(aRawFile.readFile()) {
                char* anIter = (char* )aRawFile.getBuffer();
                if(anIter[0] == '\xEF' && anIter[1] == '\xBB' && anIter[2] == '\xBF') {
                    // skip BOM for UTF8 written by some stupid programs
                    anIter += 3;
                }
                while(anIter != NULL) {
                    anIter = parseM3UIter(anIter);
                }
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
        addPlayItem(new StPlayItem(aFileNode, myDefStParams));
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
                break;
            }
        }
    }
}
