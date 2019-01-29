/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLOpenFile.h>

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLTextureButton.h>

#include <StThreads/StThread.h>

#include <fstream>

#if !defined(_WIN32)
    #include <unistd.h>
#endif

#ifdef _WIN32
/**
 * Auxiliary tool resolving drive labels.
 */
struct StDriveNameResolver {

    StMutex Mutex;
    std::vector<StString> Drives;
    std::vector<StString> Labels;
    volatile int NbProcessed;
    volatile bool ToAbort;

public:
    /**
     * Empty constructor.
     */
    StDriveNameResolver() : NbProcessed(0), ToAbort(false) {}

    /**
     * Working thread callback.
     */
    void resolve() {
        wchar_t aVolumeName[256];
        wchar_t aFileSystemName[256];
        StString aDriveLabel;
        const StString aDriveA = "A:";
        const StString aDriveB = "B:";
        for(; NbProcessed < (int )Drives.size(); StAtomicOp::Increment(NbProcessed)) {
            const StString& aDrivePath = Drives[NbProcessed];
            const StStringUtfWide aDrivePathW = aDrivePath.toUtfWide();
            DWORD aSerialNumber = 0, aMaxFileNameLength = 0, aFileSystemFlags = 0;
            if(!aDrivePath.isStartsWithIgnoreCase(aDriveA)
            && !aDrivePath.isStartsWithIgnoreCase(aDriveB)
            && ::GetVolumeInformationW(aDrivePathW.toCString(),
                                       aVolumeName, sizeof(aVolumeName) / sizeof(wchar_t),
                                       &aSerialNumber, &aMaxFileNameLength, &aFileSystemFlags,
                                       aFileSystemName, sizeof(aFileSystemName) / sizeof(wchar_t))) {
                aDriveLabel.fromUnicode(aVolumeName);
                if(!aDriveLabel.isEmpty() && !ToAbort) {
                    StMutexAuto aLock(Mutex);
                    Labels[NbProcessed] = aDrivePath + " [" + aDriveLabel + "]";
                }
            }

            if(ToAbort) {
                return;
            }
        }
    }

    /**
     * Working thread callback.
     */
    static SV_THREAD_FUNCTION resolveThread(void* thePtr) {
        StHandle<StDriveNameResolver> aThis = *static_cast<StHandle<StDriveNameResolver>* >(thePtr);
        aThis->resolve();
        return SV_THREAD_RETURN 0;
    }
};
#endif

StGLOpenFile::StGLOpenFile(StGLWidget*     theParent,
                           const StString& theTitle,
                           const StString& theCloseText)
: StGLMessageBox(theParent, theTitle, "",
                 theParent->getRoot()->scale(512), theParent->getRoot()->scale(400)),
  myCurrentPath(NULL),
  myHotList(NULL),
  myList(NULL),
  myHighlightColor(0.5f, 0.5f, 0.5f, 1.0f),
  myItemColor     (1.0f, 1.0f, 1.0f, 1.0f),
  myFileColor     (0.7f, 0.7f, 0.7f, 1.0f),
  myHotColor      (1.0f, 1.0f, 1.0f, 1.0f),
  myHotSizeX (theParent->getRoot()->scale(10)),
  myMarginX  (theParent->getRoot()->scale(8)),
  myIconSizeX(theParent->getRoot()->scale(16)) {
    myToAdjustY = false;

    int aMarginTop = myMarginTop + myRoot->scale(30);
    myCurrentPath = new StGLTextArea(this, myMarginLeft, myMarginTop, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                     myContent->getRectPx().width(), myContent->getRectPx().height());
    myCurrentPath->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                                  StGLTextFormatter::ST_ALIGN_Y_TOP);
    myCurrentPath->setTextColor(myRoot->getColorForElement(StGLRootWidget::Color_MessageText));

    myHotList = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
    myHotList->setOpacity(1.0f, true);
    myHotList->setItemWidth(myHotSizeX);
    myHotList->setColor(StGLVec4(0.0f, 0.0f, 0.0f, 0.0f));
    myHotList->changeRectPx().top()   = aMarginTop;
    myHotList->changeRectPx().left()  = myMarginLeft;
    myHotList->changeRectPx().right() = myMarginLeft + myHotSizeX;
    myContent->changeRectPx().top()   = aMarginTop;
    myContent->changeRectPx().left()  = myMarginLeft + myHotSizeX;

    myList = new StGLMenu(myContent, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
    myList->setOpacity(1.0f, true);
    myList->setColor(StGLVec4(0.0f, 0.0f, 0.0f, 0.0f));
    myList->setItemWidthMin(myContent->getRectPx().width());

    //if(!myRoot->isMobile()) {
        addButton(theCloseText);
    //}

    addSystemDrives();
}

void StGLOpenFile::addSystemDrives() {
#ifdef _WIN32
    const DWORD aMask = ::GetLogicalDrives();
    StHandle<StDriveNameResolver> aResolver = new StDriveNameResolver();
    for(int aLetterIter = 'A'; aLetterIter <= 'Z'; ++aLetterIter) {
        const int aBit = 1 << (aLetterIter - 'A');
        if((aMask & aBit) != 0) {
            aResolver->Drives.push_back(StString()         + char(aLetterIter) + ":\\");
            aResolver->Labels.push_back(StString("Drive ") + char(aLetterIter) + ":\\");
        }
    }
    if(!aResolver->Drives.empty()) {
        // name resolver might hang on resource unavailability
        StThread aThread(StDriveNameResolver::resolveThread, &aResolver);
        if(!aThread.wait(1000)) {
            //aThread.kill();
            aResolver->ToAbort = true;
            aThread.detach();
        }

        StMutexAuto aLock(aResolver->Mutex);
        for(size_t aDriveIter = 0; aDriveIter < aResolver->Drives.size(); ++aDriveIter) {
            addHotItem(aResolver->Drives[aDriveIter], aResolver->Labels[aDriveIter]);
        }
    }
#else
    // modern Android does not permit listing / content
    if(access("/", R_OK) == 0) {
        addHotItem("/", "Root");
    }
#endif

#if defined(__ANDROID__)
    std::ifstream aMountsFile("/proc/mounts");
    if(!aMountsFile.is_open()) {
        return;
    }
    std::string aLineStd;
    while(std::getline(aMountsFile, aLineStd)) {
        const StString aLine(aLineStd.c_str());
        StHandle<StArrayList<StString> > aMountLine = aLine.split(' ');
        if(aMountLine->size() < 4) {
            continue;
        }

        const StString& aDevice  = aMountLine->getValue(0);
        const StString& aMount   = aMountLine->getValue(1);
        const StString& aFileSys = aMountLine->getValue(2);
        const StString aMountLower = aMount.lowerCased();
        // filter by unsupported file-system
        /*if(aFileSys == stCString("tmpfs")
        || aFileSys == stCString("devpts")
        || aFileSys == stCString("sysfs")
        || aFileSys == stCString("selinuxfs")
        || aFileSys == stCString("debugfs")
        || aFileSys == stCString("configfs")
        || aFileSys == stCString("cgroup")
        || aFileSys == stCString("functionfs")) {
            continue;
        }*/
        // filter by supported file-system
        if(aFileSys != stCString("fuse")
        && aFileSys != stCString("vfat")
        && aFileSys != stCString("ntfs")
        && aFileSys != stCString("ext4")) {
            continue;
        }
        // filter special paths
        if(aMountLower.isStartsWith(stCString("/mnt/secure"))
        || aMountLower.isStartsWith(stCString("/mnt/asec"))
        || aMountLower.isStartsWith(stCString("/mnt/mapper"))
        || aMountLower.isStartsWith(stCString("/mnt/obb"))
        || aMountLower == stCString("/storage/emulated")
        || aMountLower.isStartsWith(stCString("/mnt/shell/emulated"))         // ignore symlink to /storage/emulated/0/ for current user, Android 4.2+
        || aMountLower.isStartsWith(stCString("/storage/emulated/legacy"))) { // ignore symlink to /storage/emulated/X/ for current user
            continue;
        }
        // filter bootdevice entities
        if( aDevice.isStartsWith(stCString("/dev/block"))
        && !aDevice.isStartsWith(stCString("/dev/block/vold"))) {
            continue;
        }
        addHotItem(aMount);
    }
#endif
}

StGLOpenFile::~StGLOpenFile() {
    StGLContext& aCtx = getContext();
    if(!myTextureFolder.isNull()) {
        for(size_t aTexIter = 0; aTexIter < myTextureFolder->size(); ++aTexIter) {
            myTextureFolder->changeValue(aTexIter).release(aCtx);
        }
        myTextureFolder.nullify();
    }
    if(!myTextureFile.isNull()) {
        for(size_t aTexIter = 0; aTexIter < myTextureFile->size(); ++aTexIter) {
            myTextureFile->changeValue(aTexIter).release(aCtx);
        }
        myTextureFile.nullify();
    }
}

void StGLOpenFile::setMimeList(const StMIMEList& theFilter) {
    myFilter     = theFilter;
    myExtensions = theFilter.getExtensionsList();
}

void StGLOpenFile::doHotItemClick(const size_t theItemId) {
    myItemToLoad = myHotPaths[theItemId];
}

void StGLOpenFile::doFileItemClick(const size_t theItemId) {
    const StFileNode* aNode = myFolder->getValue(theItemId);
    myItemToLoad = aNode->getPath();
}

void StGLOpenFile::doFolderUpClick(const size_t ) {
    StString aPath   = myFolder->getPath();
    StString aPathUp = StFileNode::getFolderUp(aPath);
    if(!aPathUp.isEmpty()) {
        myItemToLoad = aPathUp;
    }
}

bool StGLOpenFile::tryUnClick(const StClickEvent& theEvent,
                              bool&               theIsItemUnclicked) {
    bool aRes = StGLMessageBox::tryUnClick(theEvent, theIsItemUnclicked);
    if(!myItemToLoad.isEmpty()) {
        StString aPath = myItemToLoad;
        myItemToLoad.clear();
        if(StFolder::isFolder(aPath)) {
            openFolder(aPath);
        } else if(StFileNode::isFileExists(aPath)) {
            StHandle<StString> aPathHandle = new StString(aPath);
            signals.onFileSelected.emit(aPathHandle);
            myRoot->destroyWithDelay(this);
        } else {
            StGLMessageBox* aMsgBox = new StGLMessageBox(myRoot, "Error", StString("Path is inaccessible!\n") + aPath);
            aMsgBox->addButton("Close");
            aMsgBox->stglInit();
        }
    }
    return aRes;
}

void StGLOpenFile::addHotItem(const StString& theTarget,
                              const StString& theName) {
    StString aName = theName;
    if(aName.isEmpty()) {
        StString aFoler;
        StFileNode::getFolderAndFile(theTarget, aFoler, aName);
    }
    if(aName.isEmpty()) {
        aName = theTarget;
    }
    if(aName.isEmpty()
    || theTarget.isEmpty()
    || !StFileNode::isFileExists(theTarget)) {
        return;
    }
    myHotPaths.add(theTarget);

    StGLMenuItem* anItem = new StGLPassiveMenuItem(myHotList);
    setItemIcon(anItem, myHotColor, true);
    anItem->setText(aName);
    anItem->setTextColor(myHotColor);
    anItem->setHilightColor(myHighlightColor);
    anItem->setUserData(myHotPaths.size() - 1);
    anItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doHotItemClick);

    int aSizeX = anItem->getMargins().left + anItem->computeTextWidth() + anItem->getMargins().right;
    myHotSizeX = stMax(myHotSizeX, aSizeX);

    myContent->changeRectPx().left() = myMarginLeft + myHotSizeX;
    myList->setItemWidthMin(myContent->getRectPx().width());
}

void StGLOpenFile::setItemIcon(StGLMenuItem*   theItem,
                               const StGLVec4& theColor,
                               const bool      theisFolder) {
    if(theItem == NULL) {
        return;
    }

    theItem->changeMargins().left = myMarginX + myIconSizeX + myMarginX;
    if(myTextureFolder.isNull()) {
        const StString& anIcon0 = myRoot->getIcon(StGLRootWidget::IconImage_Folder);
        const StString& anIcon1 = myRoot->getIcon(StGLRootWidget::IconImage_File);
        if(!anIcon0.isEmpty()
        && !anIcon1.isEmpty()) {
            myTextureFolder = new StGLTextureArray(1);
            myTextureFile   = new StGLTextureArray(1);
            myTextureFolder->changeValue(0).setName(anIcon0);
            myTextureFile  ->changeValue(0).setName(anIcon1);
        } else {
            return;
        }
    }

    StGLIcon* anIcon = new StGLIcon(theItem, myMarginX, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), 0);
    anIcon->setColor(theColor);
    if(theisFolder) {
        anIcon->setExternalTextures(myTextureFolder);
    } else {
        anIcon->setExternalTextures(myTextureFile);
    }
    theItem->setIcon(anIcon);
}

void StGLOpenFile::openFolder(const StString& theFolder) {
    myItemToLoad.clear();
    myList->destroyChildren();

    StString aFolder = theFolder;
    if(aFolder.isEmpty()) {
    #ifdef _WIN32
        aFolder = "C:\\";
    #else
        aFolder = "//";
    #endif
    }

    myFolder = new StFolder(aFolder);
    myFolder->init(myExtensions, 1, true);
    StString aPath = myFolder->getPath();
    myCurrentPath->setText(StString("<b>Location:*</b>") + aPath + (!aPath.isEmpty() ? ST_FILE_SPLITTER : ""));

    StString aPathUp = StFileNode::getFolderUp(aPath);
    if(!aPathUp.isEmpty()) {
        StGLMenuItem* anUpItem = new StGLPassiveMenuItem(myList);
        anUpItem->setText("..");
        anUpItem->setTextColor(myItemColor);
        anUpItem->setHilightColor(myHighlightColor);
        anUpItem->changeMargins().left = myMarginX + myIconSizeX + myMarginX;
        anUpItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doFolderUpClick);
    }

    const size_t aNbItems = myFolder->size();
    for(size_t anItemIter = 0; anItemIter < aNbItems; ++anItemIter) {
        const StFileNode* aNode = myFolder->getValue(anItemIter);
        StString aName = aNode->getSubPath();
        StGLMenuItem* anItem = new StGLPassiveMenuItem(myList);
        setItemIcon(anItem, aNode->isFolder() ? myItemColor : myFileColor, aNode->isFolder());
        anItem->setText(aName);
        anItem->setTextColor(myItemColor);
        anItem->setHilightColor(myHighlightColor);
        anItem->setUserData(anItemIter);
        anItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doFileItemClick);
    }
    myList->stglInit();
    stglInit();
}
