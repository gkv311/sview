/**
 * Copyright © 2014-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StResourceManager_h_
#define __StResourceManager_h_

#include <StStrings/StString.h>
#include <StFile/StRawFile.h>

#if defined(__ANDROID__)
struct AAssetManager;
#endif

/**
 * Resource interface.
 */
class StResource {

        public:

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StResource();

    /**
     * Flag indicating that this resource can be accessed using normal file API
     * (e.g. not packed into archive or binary).
     * @sa getPath()
     */
    virtual bool isFile() const = 0;

    /**
     * Resource name - file path relative to resources root.
     */
    ST_LOCAL const StString& getName() const { return myName; }

    /**
     * Absolute file path to access resource.
     * Might be invalid if resource is packed to archive.
     * @sa isFile()
     */
    ST_LOCAL const StString& getPath() const { return myPath; }

        public: //! @name methods to access resource content

    /**
     * Read resource.
     */
    virtual bool read() = 0;

    /**
     * Access data by pointer, should be called after read().
     */
    ST_LOCAL const uint8_t* getData() const { return myData; }

    /**
     * Return resource size in bytes, should be called after read().
     */
    ST_LOCAL int getSize() const { return mySize; }

        protected:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StResource(const StString& theName,
                            const StString& thePath);

        protected:

    StString       myName; //!< file path relative to resources root
    StString       myPath; //!< absolute file path
    const uint8_t* myData; //!< pointer to the data
    int            mySize; //!< data size

};

/**
 * Resource manager.
 */
class StResourceManager {

        public:

    /**
     * The list of standard folders.
     */
    enum FolderId {
        FolderId_SdCard,
        FolderId_Documents,
        FolderId_Downloads,
        FolderId_Pictures,
        FolderId_Photos,
        FolderId_Music,
        FolderId_Videos,
        FolderId_NB
    };

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StResourceManager(const StString& theAppName = "sview");

#if defined(__ANDROID__)
    /**
     * Main constructor.
     */
    ST_CPPEXPORT StResourceManager(AAssetManager*  theAssetMgr,
                                   const StString& theAppName = "com.sview");
#endif

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StResourceManager();

    /**
     * Return folder for specified task.
     */
    ST_LOCAL const StString& getFolder(const FolderId theId) const {
        return myFolders[theId];
    }

    /**
     * Assign folder.
     */
    ST_LOCAL void setFolder(const FolderId  theId,
                            const StString& thePath) {
        myFolders[theId] = thePath;
    }

    /**
     * Folder containing user-specific application data.
     */
    ST_LOCAL const StString& getUserDataFolder() const {
        return myUserDataFolder;
    }

    /**
     * Folder containing user-specific application settings.
     */
    ST_LOCAL const StString& getSettingsFolder() const {
        return mySettingsFolder;
    }

    /**
     * Folder containing application cache.
     */
    ST_LOCAL const StString& getCacheFolder() const {
        return myCacheFolder;
    }

    /**
     * 2-letters system language code.
     */
    ST_LOCAL const StString& getSystemLanguage() const {
        return myLang;
    }

    /**
     * Check if resource with specified name exists.
     */
    ST_CPPEXPORT bool isResourceExist(const StString& theName) const;

    /**
     * Access resource with specified name.
     */
    ST_CPPEXPORT StHandle<StResource> getResource(const StString& theName) const;

    /**
     * Read content of specified folder.
     */
    ST_CPPEXPORT void listSubFolders(const StString&        theFolder,
                                     StArrayList<StString>& theSubFolder) const;

    /**
     * Access resource with specified name.
     */
    ST_CPPEXPORT virtual int openFileDescriptor(const StString& thePath) const;

        protected:

    StString       myFolders[FolderId_NB];
    StString       myAppName;        //!< application name - "sview" by default
    StString       myUserHomeFolder; //!< user home folder
    StString       myUserDataFolder; //!< folder for saving user-specific application data
    StString       mySettingsFolder; //!< folder for saving user-specific application settings
    StString       myCacheFolder;    //!< folder for saving application cache
    StString       myResFolder;      //!< folder for immutable resources
    StString       myLang;           //!< system language code
#if defined(__ANDROID__)
    AAssetManager* myAssetMgr;       //!< assets manger to retrieve resources from apk archive
#endif

};

/**
 * File resource.
 */
class StFileResource : public StResource {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StFileResource(const StString& theName,
                                const StString& thePath);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StFileResource();

    /**
     * Flag indicating that this resource can be accessed using normal file API
     * (e.g. not packed into archive or binary).
     * @sa getPath()
     */
    ST_LOCAL virtual bool isFile() const { return true; }

    /**
     * Read file content.
     */
    ST_CPPEXPORT virtual bool read();

        protected:

    StRawFile myFile; //!< file reader

};

#endif // __StResourceManager_h_
