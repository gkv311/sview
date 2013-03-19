/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StPlayList_h__
#define __StPlayList_h__

#include <StFile/StFolder.h>
#include <StGL/StParams.h>

#include <StGLStereo/StGLTextureQueue.h>
#include <StThreads/StMinGen.h>
#include <StSlots/StSignal.h>

#include <deque>

/**
 * Playlist node.
 */
class ST_LOCAL StPlayItem {

        private:

    StPlayItem* myPrevItem; //!< previous item in list
    StPlayItem* myNextItem; //!< next item in list
    size_t      myPosition; //!< position in list
    StFileNode* myFileNode; //!< link to file node
    StHandle<StStereoParams> myStParams; //!< stereo parameters
    bool        myPlayFlag; //!< flag for shuffle check

        public:

    /**
     * Default constructor.
     */
    StPlayItem(StFileNode* theFileNode,
               const StStereoParams& theDefParams);

    /**
     * Destructor.
     */
    ~StPlayItem();

    StPlayItem* getPrev() {
        return myPrevItem;
    }

    void setPrev(StPlayItem* thePrev);

    StPlayItem* getNext() {
        return myNextItem;
    }

    void setNext(StPlayItem* theNext);

    bool hasPrev() const {
        return myPrevItem != NULL;
    }

    bool hasNext() const {
        return myNextItem != NULL;
    }

    size_t getPosition() const {
        return myPosition;
    }

    void setPosition(size_t thePosition) {
        myPosition = thePosition;
    }

    StFileNode* getFileNode() {
        return myFileNode;
    }

    StString getPath() const;

    StString getFolderPath() const;

    StString getTitle() const;

    StHandle<StStereoParams> getParams() {
        return myStParams;
    }

    bool getPlayedFlag() const {
        return myPlayFlag;
    }

    void setPlayedFlag(bool theFlag) {
        myPlayFlag = theFlag;
    }

};

/**
 * This is playlist class. All items stored in double-linked list
 * and provides fast sequential (but not random) access.
 * All public methods are thread-safe, thus returns the objects copies.
 */
class ST_LOCAL StPlayList {

        public:

    /**
     * Main constructor.
     */
    StPlayList(const StArrayList<StString>& theExtensions,
               int theRecursionDeep,
               bool theIsLoop = false);

    /**
     * Destructor.
     */
    ~StPlayList();

    /**
     * Clear playlist.
     */
    void clear();

    /**const StPlayItem* getCurrent() const {
        return myCurrent;
    }*/

    StStereoParams& changeDefParams() {
        return myDefStParams;
    }

    /**
     * Return title for the current position in playlist.
     * Will be empty string if playlist is empty.
     */
    StString getCurrentTitle() const;

    /**
     * Returns file node for current playing position.
     */
    StHandle<StFileNode> getCurrentFile();

    /**
     * Returns file node and stereo parameters for current playing position.
     * @return true if playlist is not empty.
     */
    bool getCurrentFile(StHandle<StFileNode>& theFileNode,
                        StHandle<StStereoParams>& theParams);

    void addToNode(const StHandle<StFileNode>& theFileNode,
                   const StString&             thePathToAdd);

    /**
     * Remove file from playlist and delete it physically.
     */
    void removePhysically(const StHandle<StFileNode>& theFileNode);

    /**
     * Returns loop flag.
     */
    bool isLoop() const;

    /**
     * Setup loop flag.
     */
    void setLoop(bool theLoop);

    /**
     * Returns shuffle flag.
     */
    bool isShuffle() const;

    /**
     * Setup shuffle flag.
     */
    void setShuffle(bool theShuffle);

    bool isEmpty() const {
        StMutexAuto anAutoLock(myMutex);
        return myFirst == NULL;
    }

    /**bool isFirst() const {
        StMutexAuto anAutoLock(myMutex);
        return myCurrent == myFirst;
    }

    bool isLast() const {
        StMutexAuto anAutoLock(myMutex);
        return myCurrent == myLast;
    }*/

    /**
     * Change current position in playlist to the first item.
     * @return true if position was changed.
     */
    bool walkToFirst();

    /**
     * Change current position in playlist to the last item.
     * @return true if current position was changed.
     */
    bool walkToLast();

    /**
     * Change current position in playlist to the previous item.
     * @return true if current position was changed.
     */
    bool walkToPrev();

    /**
     * Change current position in playlist to the next item.
     * @return true if current position was changed.
     */
    bool walkToNext();

    /**
     * Verify the filename extension is in supported list.
     */
    bool checkExtension(const StString& thePath);

    /**
     * Just add one file to the playlist.
     */
    void addOneFile(const StString& theFilePath,
                    const StMIME&   theFileMIME);

    /**
     * Add meta file to the playlist.
     */
    void addOneFile(const StString& theFilePathLeft,
                    const StString& theFilePathRight);

    /**
     * Clean up current playlist and create new one.
     * If given path is a folder than it content will be added to list.
     * If given path is a file than playlist will be fill with folder content
     * and playlist position will be set to this file.
     */
    void open(const StString& thePath);

    /**
     * Check and reset modification flag.
     * @return true if recent files list was modified since last call
     */
    bool isRecentChanged() const;

    /**
     * Fill list with recently opened files (only titles).
     * @param theList List to fill
     */
    void getRecentList(StArrayList<StString>& theList) const;

    /**
     * Restore list of recent files from the string serialized by dumpRecentList() method.
     * @param theString String with list of files
     */
    void loadRecentList(const StString theString);

    /**
     * Dump list of recently opened files into string.
     * @return String with special format
     */
    StString dumpRecentList() const;

    /**
     * Open recent file at specified position.
     * @param theItemId Position in recent files list
     */
    void openRecent(const size_t theItemId);

    /**
     * Reset list of recently opened files.
     */
    void clearRecent();

        public: //!< Signals

    struct {
        /**
         * Emit callback Slot when current position in playlist is changed.
         */
        StSignal<void (void )> onPositionChange;

        /**
         * Emit callback Slot when playlist content is changed.
         */
        StSignal<void (void )> onPlaylistChange;
    } signals;

        private:

    /**
     * Add new item to double-linked list.
     */
    void addPlayItem(StPlayItem* theNewItem);

    /**
     * Remove the item from double-linked list but NOT destroy it.
     */
    void delPlayItem(StPlayItem* theRemItem);

    /**
     * Recursively add all file nodes to playlist.
     */
    void addToPlayList(StFileNode* theFileNode);

    /**
     * Add file to list of recent files.
     */
    void addRecentFile(const StFileNode& theFile);

    /**
     * M3U parsing stuff.
     */
    char* parseM3UIter(char* theIter);

        private:

    mutable StMutex         myMutex;         //!< mutex for thread-safe access
    StFolder                myFoldersRoot;   //!< common root for all file nodes
    StPlayItem*             myFirst;         //!< double-linked list, start node
    StPlayItem*             myLast;          //!< double-linked list, last node
    StPlayItem*             myCurrent;       //!< current playback node
    std::deque<StPlayItem*> myStackPrev;     //!< stack of previous items (for shuffle playback)
    std::deque<StPlayItem*> myStackNext;     //!< stack of next     items (for shuffle playback)
    size_t                  myItemsCount;    //!< current playlist size
    StArrayList<StString>   myExtensions;    //!< extensions list
    StStereoParams          myDefStParams;   //!< default stereo parameters
    StMinGen                myRandGen;       //!< random number generator for shuffle playback
    size_t                  myPlayedCount;   //!< played items in current iteration (< myItemsCount)
    int                     myRecursionDeep;
    bool                    myIsShuffle;
    bool                    myIsLoopFlag;

    std::deque< StHandle<StFileNode> > myRecent; //!< list of recently opened files
    size_t                  myRecentLimit;   //!< the maximum size of list with recently opened files
    mutable bool            myIsNewRecent;   //!< flag indicates modified state of recent files list

};

#endif //__StPlayList_h__
