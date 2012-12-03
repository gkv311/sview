/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StFileNode : public StNode {

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
     * Main constructor.
     */
    StFileNode(const StString& subPath = StString(), StNode* parentNode = NULL, int nodeType = NODE_TYPE_FILE)
    : StNode(subPath, parentNode, nodeType),
      stMIMEType() {
        //
    }

    /**
     * Destructor.
     */
    virtual ~StFileNode();

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
    virtual bool isFolder() const {
        return false;
    }

    /**
     * Perform detach operation for this file node.
     * As result new handle will be returned targets to the same path
     * but without back-referencies to top elementes in the tree (parent path).
     * @return new handle to detached file node copy.
     */
    virtual StHandle<StFileNode> detach() const;

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
        return thePath.isStartsWith("\\\\?\\");
    }

    /**
     * UNC is a naming convention used primarily to specify and map network drives in Microsoft Windows.
     * Sample path:
     *   \\sever\share\file
     * @return true if UNC path syntax detected.
     */
    static bool isUncPath(const StString& thePath) {
        return thePath.isStartsWith("\\\\");
    }

    /**
     * Detect extended-length UNC path.
     * Sample path:
     *   \\?\UNC\server\share
     * @return true if extended-length UNC path syntax detected.
     */
    static bool isUncExtendedPath(const StString& thePath) {
        return thePath.isStartsWith("\\\\?\\UNC\\");
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
    static bool isRemoteProtocolPath(const StString& thePath) {
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
    static bool isFileExists(const StString& path);

    /**
     * Tries to remove file from filesystem.
     * @return true on success.
     */
    static bool removeFile(const StString& path);

    /**
     * Tries to move/rename file.
     * @return true on success.
     */
    static bool moveFile(const StString& thePathFrom,
                         const StString& thePathTo);

    /**
     * Extract the file extension using general rules.
     * For filename "theFile.ext" extension string "ext" will be returned.
     * If filename has no extension than empty string will be returned.
     * @return file extension.
     */
    static StString getExtension(const StString& theFileName);

    /**
     * Split filename to extension and name.
     * Example: IN  theFileName  ='im.age.jps'
     *          OUT theName      ='im.age'
     *          OUT theExtension ='jps'
     * @param theFileName (const StString& ) - IN file name
     * @param theFolder    (StString& ) - OUT file name
     * @param theExtension (StString& ) - OUT file extension
     */
    static void getNameAndExtension(const StString& theFileName,
                                    StString& theName,
                                    StString& theExtension);

    /**
     * Divide absolute filepath into folder path and file name.
     * Example: IN  theFilePath ='/media/cdrom/image.jps'
     *          OUT theFolder   ='/media/cdrom'
     *          OUT theFileName ='image.jps'
     * @param theFilePath (const StString& ) - IN file path
     * @param theFolder   (StString& ) - OUT folder path
     * @param theFileName (StString& ) - OUT file name
     */
    static void getFolderAndFile(const StString& theFilePath,
                                 StString& theFolder,
                                 StString& theFileName);

    /**
     * Open native system open file dialog.
     * @param theFolder (const StString& ) - path to open;
     * @param theTitle  (const StString& ) - dialog title;
     * @param theFilter (const StMIMEList& ) - files filter;
     * @param theOutFilePath (StString& ) - file selected by user;
     * @param toSave (bool ) - flag this dialog to open or save file;
     * @return
     */
    static bool openFileDialog(const StString& theFolder,
                               const StString& theTitle,
                               const StMIMEList& theFilter,
                               StString& theOutFilePath,
                               bool toSave);

    /**
     * Function used where not possible use native Unicode paths.
     * @param fileName (const StString& ) - original filename;
     * @return (StString ) - compatible filename.
     */
    static StString getCompatibleName(const StString& fileName);

};

#endif //__StFileNode_H__
