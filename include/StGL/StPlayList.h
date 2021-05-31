/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    ST_CPPEXPORT StPlayItem(StFileNode*           theFileNode,
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

    ST_CPPEXPORT void setTitle(const StString& theTitle);

    ST_LOCAL inline bool hasCustomTitle() const {
        return !myTitle.isEmpty();
    }

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
    StString    myTitle;    //!< item title
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
     * Enumeration identifying position of current item in the playlist.
     */
    enum CurrentPosition {
        CurrentPosition_NONE,   //!< empty list
        CurrentPosition_Single, //!< single item
        CurrentPosition_First,  //!< first     in the list
        CurrentPosition_Last,   //!< last      in the list
        CurrentPosition_Middle, //!< somewhere in the list
    };

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StPlayList(const int  theRecursionDeep,
                            const bool theIsLoop = false);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StPlayList();

    /**
     * Set extensions list.
     */
    ST_CPPEXPORT void setExtensions(const StArrayList<StString>& theExtensions);

    /**
     * Clear playlist.
     */
    ST_CPPEXPORT void clear();

    /**
     * @return serial number of playlist content (how many times playlist has been cleared)
     */
    ST_CPPEXPORT int32_t getSerial();

    inline StStereoParams& changeDefParams() {
        return myDefStParams;
    }

    /**
     * @return item index for the current position in playlist (starting from 0)
     */
    ST_CPPEXPORT size_t getCurrentId() const;

    /**
     * Return item index for the current position in playlist
     * and flags if it is last or first
     */
    ST_CPPEXPORT StPlayList::CurrentPosition getCurrentPosition() const;

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
     * Set title for specified item.
     */
    ST_CPPEXPORT void setTitle(const StHandle<StStereoParams>& theKey,
                               const StString&                 theTitle);

    /**
     * Returns file node and stereo parameters for current playing position.
     * @return true if playlist is not empty.
     */
    ST_CPPEXPORT bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                     StHandle<StStereoParams>& theParams,
                                     StHandle<StFileNode>&     thePlsFile);

    /**
     * Returns file node and stereo parameters for current playing position.
     * @return true if playlist is not empty.
     */
    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams) {
        StHandle<StFileNode> aPlsFile;
        return getCurrentFile(theFileNode, theParams, aPlsFile);
    }

    ST_CPPEXPORT void addToNode(const StHandle<StFileNode>& theFileNode,
                                const StString&             thePathToAdd);

    /**
     * Remove file from playlist.
     */
    ST_CPPEXPORT bool remove(const StString& thePath,
                             const bool      theToRemovePhysically);

    /**
     * Remove file from playlist and delete it physically.
     */
    ST_LOCAL bool removePhysically(const StHandle<StFileNode>& theFileNode) {
        return remove(theFileNode->getPath(), true);
    }

    /**
     * Returns loop flag.
     */
    ST_CPPEXPORT bool isLoop() const;

    /**
     * Setup loop flag.
     */
    ST_CPPEXPORT void setLoop(const bool theLoop);

    /**
     * Setup single item loop flag.
     */
    ST_CPPEXPORT void setLoopSingle(const bool theValue);

    /**
     * Returns shuffle flag.
     */
    ST_CPPEXPORT bool isShuffle() const;

    /**
     * Setup shuffle flag.
     */
    ST_CPPEXPORT void setShuffle(bool theShuffle);

    /**
     * @return current playlist size
     */
    ST_LOCAL inline size_t getItemsCount() const {
        return myItemsCount;
    }

    ST_LOCAL bool isEmpty() const {
        StMutexAuto anAutoLock(myMutex);
        return myFirst == NULL;
    }

    /**
     * Change current position in playlist to specified index.
     * @param theId new position (starting from 0)
     * @return true if position has been changed
     */
    ST_CPPEXPORT bool walkToPosition(const size_t theId);

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
     * @param theToForce ignore single item loop flag
     * @return true if current position was changed
     */
    ST_CPPEXPORT bool walkToNext(const bool theToForce = true);

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
    ST_CPPEXPORT void open(const StCString& thePath,
                           const StCString& theItem = stCString(""));

    /**
     * Fill list with playlist items (only titles).
     * @param theList  the list to fill
     * @param theStart start index (inclusive) in playlist
     * @param theEnd   end   index (exclusive) in playlist
     */
    ST_CPPEXPORT void getSubList(StArrayList<StString>& theList,
                                 const size_t           theStart,
                                 const size_t           theEnd) const;

        public: //! @name recently opened files list

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
     * Search file path in the list of recently opened items.
     * @return index of recent item or -1 if not found
     */
    ST_CPPEXPORT size_t findRecent(const StString thePathL,
                                   const StString thePathR = "") const;

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
     * Set last recent file to the currently played file.
     * Has effect only for playlist automatically generated from opened file (not folder).
     */
    ST_CPPEXPORT void currentToRecent();

    /**
     * Open recent file at specified position.
     * @param theItemId Position in recent files list
     * @return saved parameters or NULL
     */
    ST_CPPEXPORT StHandle<StStereoParams> openRecent(const size_t theItemId);

    /**
     * Reset list of recently opened files.
     */
    ST_CPPEXPORT void clearRecent();

    /**
     * Update parameters of recent item.
     */
    ST_CPPEXPORT void updateRecent(const StHandle<StFileNode>&     theFile,
                                   const StHandle<StStereoParams>& theParams);

        public: //!< Signals

    struct {
        /**
         * Emit callback Slot when current position in playlist is changed.
         * @param theItem new position in playlist
         */
        StSignal<void (const size_t )> onPositionChange;

        /**
         * Emit callback Slot when playlist content is changed.
         */
        StSignal<void (void )> onPlaylistChange;

        /**
         * Emit callback Slot when title of some item changed.
         * @param theItem index of changed item
         */
        StSignal<void (const size_t )> onTitleChange;
    } signals;

        private:

    struct StRecentItem {
        StHandle<StFileNode>     File;
        StHandle<StStereoParams> Params;
    };

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
    ST_LOCAL const StHandle<StRecentItem>& addRecentFile(const StFileNode& theFile,
                                                         const bool        theToFront = true);

    /**
     * M3U parsing stuff.
     */
    ST_LOCAL char* parseM3UIter(char*     theIter,
                                StFolder* theFolder,
                                StString& theTitle);

    /**
     * Save current playlist in m3u format.
     */
    ST_LOCAL bool saveM3U(const StCString& thePath);

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
    bool                    myToLoopSingle;  //!< play single item in loop
    bool                    myIsLoopFlag;

    StHandle<StRecentItem>  myPlsFile;       //!< current playlist file (if any)
    std::deque< StHandle<StRecentItem> > myRecent; //!< list of recently opened files
    size_t                  myRecentLimit;   //!< the maximum size of list with recently opened files
    mutable bool            myIsNewRecent;   //!< flag indicates modified state of recent files list

    StAtomic<int32_t>       mySerial;        //!< serial number of playlist content
    bool                    myWasCleared;    //!< flag to indicate that playlist was cleared recently

};

#endif // __StPlayList_h__
