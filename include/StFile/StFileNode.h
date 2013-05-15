/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFileNode_H__
#define __StFileNode_H__

#include <StFile/StNode.h>
#include <StFile/StMIME.h>
#include <StTemplates/StHandle.h>

class StMIMEList;
class StFileNode : public StNode {

        private:

    /// TODO (Kirill Gavrilov#4) this is probably more effective to store handle here to save memory
    ///                          because MIME types are generally constant.
    StMIME stMIMEType;

        public:

    /// TODO (Kirill Gavrilov#9) resolve situation (remove enumeration or use it)
    enum {
        NODE_TYPE_FOLDER   = 0,
        NODE_TYPE_METAFILE = 1,
        NODE_TYPE_FILE     = 2,
    };

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StFileNode();

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StFileNode(const StString& theSubPath,
                            StNode*         theParentNode = NULL,
                            int             theNodeType   = NODE_TYPE_FILE);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StFileNode();

    /**
     * Method to access child file node by index.
     */
    const StFileNode* getValue(const size_t& index) const {
        return (StFileNode* )StNode::getValue(index);
    }

    /**
     * Method to access child file node by index.
     */
    StFileNode* changeValue(const size_t& index) {
        return (StFileNode* )StNode::changeValue(index);
    }

    /**
     * Fast flag to determine StFolder instance.
     */
    ST_CPPEXPORT virtual bool isFolder() const;

    /**
     * Perform detach operation for this file node.
     * As result new handle will be returned targets to the same path
     * but without back-referencies to top elementes in the tree (parent path).
     * @return new handle to detached file node copy.
     */
    ST_CPPEXPORT virtual StHandle<StFileNode> detach() const;

    /**
     * Returns MIME information assigned to this file node.
     */
    const StMIME& getMIME() const {
        return stMIMEType;
    }

    /**
     * Assign MIME information to this file node.
     */
    void setMIME(const StMIME& mime) {
        stMIMEType = mime;
    }

    /**
     * Returns folder path for this file node.
     */
    StString getFolderPath() const {
        StString aTitleString;
        StString aFolder;
        StFileNode::getFolderAndFile(getPath(), aFolder, aTitleString);
        return aFolder;
    }

        public: //!< static methods

    /**
     * Detect absolute DOS-path also used in Windows.
     * The total path length is limited to 256 chatacters.
     * Sample path:
     *   C:\folder\file
     * @return true if DOS path syntax detected.
     */
    static bool isDosPath(const StString& thePath) {
        return thePath[0] != stUtf32_t('\0') && thePath[1] == stUtf32_t(':');
    }

    /**
     * Detect extended-length NT path (can be only absolute).
     * Approximate maximum path is 32767 characters.
     * Sample path:
     *   \\?\D:\very long path
     * @return true if extended-length NT path syntax detected.
     */
    static bool isNtExtendedPath(const StString& thePath) {
        return thePath.isStartsWith(stCString("\\\\?\\"));
    }

    /**
     * UNC is a naming convention used primarily to specify and map network drives in Microsoft Windows.
     * Sample path:
     *   \\sever\share\file
     * @return true if UNC path syntax detected.
     */
    static bool isUncPath(const StString& thePath) {
        return thePath.isStartsWith(stCString("\\\\"));
    }

    /**
     * Detect extended-length UNC path.
     * Sample path:
     *   \\?\UNC\server\share
     * @return true if extended-length UNC path syntax detected.
     */
    static bool isUncExtendedPath(const StString& thePath) {
        return thePath.isStartsWith(stCString("\\\\?\\UNC\\"));
    }

    /**
     * Detect absolute UNIX-path.
     * Sample path:
     *   /media/cdrom/file
     * @return true if UNIX path syntax detected.
     */
    static bool isUnixPath(const StString& thePath) {
        return thePath[0] == stUtf32_t('/') && thePath[1] != stUtf32_t('/');
    }

    /**
     * Detect remote protocol path (http / ftp / ...).
     * Actually shouldn't be remote...
     * Sample path:
     *   http://domain/path/file
     * @return true if remote protocol path syntax detected.
     */
    ST_CPPEXPORT static bool isRemoteProtocolPath(const StString& thePath);

    /**
     * Method to recognize path is absolute or not.
     * Detection is based on path syntax - no any filesystem / network access performed.
     * @return true if path is incomplete (relative).
     */
    static bool isRelativePath(const StString& thePath) {
        return !isUncPath(thePath)
            && !isDosPath(thePath)
            && !isUnixPath(thePath)
            && !isRemoteProtocolPath(thePath);
    }

    /**
     * Method to recognize path is absolute or not.
     * Detection is based on path syntax - no any filesystem / network access performed.
     * @return true if path is complete (absolute).
     */
    static bool isAbsolutePath(const StString& thePath) {
        return !isRelativePath(thePath);
    }

    /**
     * @param path (const StString& ) - file path;
     * @return true if file/folder exists.
     */
    ST_CPPEXPORT static bool isFileExists(const StString& path);

    /**
     * Tries to remove file from filesystem.
     * @return true on success.
     */
    ST_CPPEXPORT static bool removeFile(const StString& path);

    /**
     * Tries to move/rename file.
     * @return true on success.
     */
    ST_CPPEXPORT static bool moveFile(const StString& thePathFrom,
                                      const StString& thePathTo);

    /**
     * Extract the file extension using general rules.
     * For filename "theFile.ext" extension string "ext" will be returned.
     * If filename has no extension than empty string will be returned.
     * @return file extension.
     */
    ST_CPPEXPORT static StString getExtension(const StString& theFileName);

    /**
     * Split filename to extension and name.
     * Example: IN  theFileName  ='im.age.jps'
     *          OUT theName      ='im.age'
     *          OUT theExtension ='jps'
     * @param theFileName  file path
     * @param theName      file name
     * @param theExtension file extension
     */
    ST_CPPEXPORT static void getNameAndExtension(const StString& theFileName,
                                                 StString& theName,
                                                 StString& theExtension);

    /**
     * Divide absolute filepath into folder path and file name.
     * Example: IN  theFilePath ='/media/cdrom/image.jps'
     *          OUT theFolder   ='/media/cdrom'
     *          OUT theFileName ='image.jps'
     * @param theFilePath file path
     * @param theFolder   folder path
     * @param theFileName file name
     */
    ST_CPPEXPORT static void getFolderAndFile(const StString& theFilePath,
                                              StString&       theFolder,
                                              StString&       theFileName);

    /**
     * Open native system open file dialog.
     * @param theFolder      path to open
     * @param theTitle       dialog title
     * @param theFilter      files filter
     * @param theOutFilePath file selected by user
     * @param toSave         flag this dialog to open or save file
     * @return
     */
    ST_CPPEXPORT static bool openFileDialog(const StString&   theFolder,
                                            const StString&   theTitle,
                                            const StMIMEList& theFilter,
                                            StString&         theOutFilePath,
                                            bool              toSave);

    /**
     * Function used where not possible use native Unicode paths.
     * @param theFileName original filename
     * @return compatible filename
     */
    ST_CPPEXPORT static StString getCompatibleName(const StString& theFileName);

};

#endif //__StFileNode_H__
