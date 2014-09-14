/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StResourceManager_h_
#define __StResourceManager_h_

#include <StStrings/StString.h>
#include <StFile/StRawFile.h>

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
    const StString& getName() const { return myName; }

    /**
     * Absolute file path to access resource.
     * Might be invalid if resource is packed to archive.
     * @sa isFile()
     */
    const StString& getPath() const { return myPath; }

        public: //! @name methods to access resource content

    /**
     * Read resource.
     */
    virtual bool read() = 0;

    /**
     * Access data by pointer, should be called after read().
     */
    const uint8_t* getData() const { return myData; }

    /**
     * Return resource size in bytes, should be called after read().
     */
    int getSize() const { return mySize; }

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
     * Main constructor.
     */
    ST_CPPEXPORT StResourceManager();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StResourceManager();

    /**
     * Check if resource with specified name exists.
     */
    ST_CPPEXPORT bool isResourceExist(const StString& theName) const;

    /**
     * Access resource with specified name.
     */
    ST_CPPEXPORT StHandle<StResource> getResource(const StString& theName) const;

        protected:

    StString myRoot;

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
    virtual bool isFile() const { return true; }

    /**
     * Read file content.
     */
    ST_CPPEXPORT virtual bool read();

        protected:

    StRawFile myFile; //!< file reader

};

#endif // __StResourceManager_h_
