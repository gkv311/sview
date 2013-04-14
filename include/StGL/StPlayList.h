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
class StPlayItem {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StPlayItem(StFileNode* theFileNode,
                            const StStereoParams& theDefParams);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StPlayItem();

    inline StPlayItem* getPrev() {
        return myPrevItem;
    }

    ST_CPPEXPORT void setPrev(StPlayItem* thePrev);

    inline StPlayItem* getNext() {
        return myNextItem;
    }

    ST_CPPEXPORT void setNext(StPlayItem* theNext);

    inline bool hasPrev() const {
        return myPrevItem != NULL;
    }

    inline bool hasNext() const {
        return myNextItem != NULL;
    }

    inline size_t getPosition() const {
        return myPosition;
    }

    inline void setPosition(size_t thePosition) {
        myPosition = thePosition;
    }

    inline StFileNode* getFileNode() {
        return myFileNode;
    }

    ST_CPPEXPORT StString getPath() const;

    ST_CPPEXPORT StString getFolderPath() const;

    ST_CPPEXPORT StString getTitle() const;

    inline StHandle<StStereoParams> getParams() {
        return myStParams;
    }

    inline bool getPlayedFlag() const {
        return myPlayFlag;
    }

    inline void setPlayedFlag(bool theFlag) {
        myPlayFlag = theFlag;
    }

        private:

    StPlayItem* myPrevItem; //!< previous item in list
    StPlayItem* myNextItem; //!< next item in list
    size_t      myPosition; //!< position in list
    StFileNode* myFileNode; //!< link to file node
    StHandle<StStereoParams> myStParams; //!< stereo parameters
    bool        myPlayFlag; //!< flag for shuffle check

};

/**
 * This is playlist class. All items stored in double-linked list
 * and provides fast sequential (but not random) access.
 * All public methods are thread-safe, thus returns the objects copies.
 */
class StPlayList {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StPlayList(const StArrayList<StString>& theExtensions,
                            int theRecursionDeep,
                            bool theIsLoop = false);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StPlayList();

    /**
     * Clear playlist.
     */
    ST_CPPEXPORT void clear();

    /**const StPlayItem* getCurrent() const {
        return myCurrent;
    }*/

    inline StStereoParams& changeDefParams() {
        return myDefStParams;
    }

    /**
     * Return title for the current position in playlist.
     * Will be empty string if playlist is empty.
     */
    ST_CPPEXPORT StString getCurrentTitle() const;

    /**
     * Returns file node for current playing position.
     */
    ST_CPPEXPORT StHandle<StFileNode> getCurrentFile();

    /**
     * Returns file node and stereo parameters for current playing position.
     * @return true if playlist is not empty.
     */
    ST_CPPEXPORT bool getCurrentFile(StHandle<StFileNode>& theFileNode,
                                     StHandle<StStereoParams>& theParams);

    ST_CPPEXPORT void addToNode(const StHandle<StFileNode>& theFileNode,
                                const StString&             thePathToAdd);

    /**
     * Remove file from playlist and delete it physically.
     */
    ST_CPPEXPORT void removePhysically(const StHandle<StFileNode>& theFileNode);

    /**
     * Returns loop flag.
     */
    ST_CPPEXPORT bool isLoop() const;

    /**
     * Setup loop flag.
     */
    ST_CPPEXPORT void setLoop(bool theLoop);

    /**
     * Returns shuffle flag.
     */
    ST_CPPEXPORT bool isShuffle() const;

    /**
     * Setup shuffle flag.
     */
    ST_CPPEXPORT void setShuffle(bool theShuffle);

    inline bool isEmpty() const {
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
    ST_CPPEXPORT bool walkToFirst();

    /**
     * Change current position in playlist to the last item.
     * @return true if current position was changed.
     */
    ST_CPPEXPORT bool walkToLast();

    /**
     * Change current position in playlist to the previous item.
     * @return true if current position was changed.
     */
    ST_CPPEXPORT bool walkToPrev();

    /**
     * Change current position in playlist to the next item.
     * @return true if current position was changed.
     */
    ST_CPPEXPORT bool walkToNext();

    /**
     * Verify the filename extension is in supported list.
     */
    ST_CPPEXPORT bool checkExtension(const StString& thePath);

    /**
     * Just add one file to the playlist.
     */
    ST_CPPEXPORT void addOneFile(const StString& theFilePath,
                                 const StMIME&   theFileMIME);

    /**
     * Add meta file to the playlist.
     */
    ST_CPPEXPORT void addOneFile(const StString& theFilePathLeft,
                                 const StString& theFilePathRight);

    /**
     * Clean up current playlist and create new one.
     * If given path is a folder than it content will be added to list.
     * If given path is a file than playlist will be fill with folder content
     * and playlist position will be set to this file.
     */
    ST_CPPEXPORT void open(const StString& thePath);

    /**
     * Check and reset modification flag.
     * @return true if recent files list was modified since last call
     */
    ST_CPPEXPORT bool isRecentChanged() const;

    /**
     * Fill list with recently opened files (only titles).
     * @param theList List to fill
     */
    ST_CPPEXPORT void getRecentList(StArrayList<StString>& theList) const;

    /**
     * Restore list of recent files from the string serialized by dumpRecentList() method.
     * @param theString String with list of files
     */
    ST_CPPEXPORT void loadRecentList(const StString theString);

    /**
     * Dump list of recently opened files into string.
     * @return String with special format
     */
    ST_CPPEXPORT StString dumpRecentList() const;

    /**
     * Open recent file at specified position.
     * @param theItemId Position in recent files list
     */
    ST_CPPEXPORT void openRecent(const size_t theItemId);

    /**
     * Reset list of recently opened files.
     */
    ST_CPPEXPORT void clearRecent();

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
    ST_LOCAL void addPlayItem(StPlayItem* theNewItem);

    /**
     * Remove the item from double-linked list but NOT destroy it.
     */
    ST_LOCAL void delPlayItem(StPlayItem* theRemItem);

    /**
     * Recursively add all file nodes to playlist.
     */
    ST_LOCAL void addToPlayList(StFileNode* theFileNode);

    /**
     * Add file to list of recent files.
     */
    ST_LOCAL void addRecentFile(const StFileNode& theFile);

    /**
     * M3U parsing stuff.
     */
    ST_LOCAL char* parseM3UIter(char* theIter);

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
