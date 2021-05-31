/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLWidgets/StGLOpenFile.h>

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLMenuCheckbox.h>
#include <StGLWidgets/StGLCheckbox.h>
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
     * Init list.
     */
    void init() {
        const DWORD aLogicalDrivesMask = ::GetLogicalDrives();
        for(int aLetterIter = 'A'; aLetterIter <= 'Z'; ++aLetterIter) {
            const int aBit = 1 << (aLetterIter - 'A');
            if((aLogicalDrivesMask & aBit) != 0) {
                Drives.push_back(StString()         + char(aLetterIter) + ":\\");
                Labels.push_back(StString("Drive ") + char(aLetterIter) + ":\\");
            }
        }

        // enumerate all physical drives seen by Windows
        const GUID PARTITION_BASIC_DATA_GUID = { 0xEBD0A0A2L, 0xB9E5, 0x4433, {0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7} };
        static const DWORD aPartInfoSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + 128 * sizeof(PARTITION_INFORMATION_EX); // up to 128 partitions
        char aPartInfoBuffer[aPartInfoSize] = {};
        DRIVE_LAYOUT_INFORMATION_EX* aPartitions = (DRIVE_LAYOUT_INFORMATION_EX* )aPartInfoBuffer;
        for(int aDriveIter = 0; aDriveIter < 1000; ++aDriveIter) {
            const StStringUtfWide aDriveId = StStringUtfWide(L"\\\\.\\PhysicalDrive") + aDriveIter;
            HANDLE aDriveHandle = CreateFileW(aDriveId.toCString(), 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
            if(aDriveHandle == INVALID_HANDLE_VALUE) {
                break;
            }

            // list partitions
            DWORD aDummyIor = 0;
            if(!DeviceIoControl(aDriveHandle, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, aPartitions, aPartInfoSize, &aDummyIor, NULL)) {
                CloseHandle(aDriveHandle);
                continue;
            }

            for(DWORD aPartIter = 0; aPartIter < aPartitions->PartitionCount; ++aPartIter) {
                const PARTITION_INFORMATION_EX& aPart = aPartitions->PartitionEntry[aPartIter];
                switch(aPart.PartitionStyle) {
                    case PARTITION_STYLE_MBR: {
                        if(aPart.Mbr.PartitionType == PARTITION_ENTRY_UNUSED || !aPart.Mbr.RecognizedPartition) {
                            continue; // ignore unused
                        }
                        break;
                    }
                    case PARTITION_STYLE_GPT: {
                        if(aPart.Gpt.PartitionType != PARTITION_BASIC_DATA_GUID) {
                            continue; // ignore unused
                        }
                        break;
                    }
                    default: {
                        continue; // ignore unknown
                    }
                }

                const StStringUtfWide aPartVolume = findPartitionVolume(aDriveIter, aPart);
                if(aPartVolume.isEmpty()) {
                    continue;
                }

                // find mount points
                DWORD aMntPointLen = 0;
                std::vector<wchar_t> aMntPoints(1);
                GetVolumePathNamesForVolumeNameW(aPartVolume.toCString(), aMntPoints.data(), 0, &aMntPointLen);
                if(aMntPointLen > 0) {
                    aMntPoints.resize (aMntPointLen + 1, '\0');
                    if(GetVolumePathNamesForVolumeNameW(aPartVolume.toCString(), aMntPoints.data(), aMntPointLen, &aMntPointLen) != FALSE) {
                        StString aMntPointRes(aMntPoints.data());
                        for(const wchar_t* aMntPointIter = aMntPoints.data(); aMntPointIter[0] != '\0'; aMntPointIter += wcslen(aMntPointIter)) {
                            const StString aMntPoint(aMntPointIter);
                            if(aMntPoint.getLength() == 3
                            && aMntPoint.toCString()[1] == ':'
                            && aMntPoint.toCString()[2] == '\\') {
                                // filter common logical drive letter
                                aMntPointRes.clear();
                                break;
                            }
                        }
                        if(!aMntPointRes.isEmpty()) {
                            if(aMntPointRes.getLength() > 3
                            && aMntPointRes.isEndsWith('\\')) {
                                aMntPointRes = aMntPointRes.subString(0, aMntPointRes.getLength() - 1);
                            }
                            Drives.push_back(aMntPointRes);
                            Labels.push_back(StString("Drive ") + aMntPointRes);
                        }
                    }
                }
            }
            CloseHandle(aDriveHandle);
        }
    }

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
            StStringUtfWide aDrivePathW = aDrivePath.toUtfWide();
            if(!aDrivePath.isEndsWith('\\')) {
                aDrivePathW += '\\';
            }
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

        private:

    /**
     * Find volume name for partition.
     */
    static StStringUtfWide findPartitionVolume(DWORD theDiskNumber, const PARTITION_INFORMATION_EX& thePart) {
        // handle up-to 256 extents
        static const DWORD THE_EXTENTS_SIZE = sizeof(VOLUME_DISK_EXTENTS) + 256 * sizeof(DISK_EXTENT);
        char aBuffer[THE_EXTENTS_SIZE] = {};
        VOLUME_DISK_EXTENTS* aVolDiskExtents = (VOLUME_DISK_EXTENTS* )aBuffer;

        std::vector<wchar_t> aVolName(4096, '\0');
        HANDLE aVolIter = FindFirstVolumeW(aVolName.data(), (DWORD )aVolName.size());
        for(bool hasMoreVolumes = aVolIter != INVALID_HANDLE_VALUE; hasMoreVolumes; hasMoreVolumes = FindNextVolumeW (aVolIter, aVolName.data(), (DWORD )aVolName.size()) != FALSE) {
            aVolName[wcslen(aVolName.data()) - 1] = '\0'; // volume must be without trailing backslash here
            HANDLE aVolHandle = CreateFileW(aVolName.data(), 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
            aVolName[wcslen(aVolName.data())] = '\\';
            if(aVolHandle == INVALID_HANDLE_VALUE) {
                continue;
            }

            DWORD anExtentsSize = 0;
            if(DeviceIoControl(aVolHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, (void* )aVolDiskExtents, THE_EXTENTS_SIZE, &anExtentsSize, NULL) == FALSE) {
                CloseHandle(aVolHandle);
                continue;
            }

            for(DWORD anExtentIter = 0; anExtentIter < aVolDiskExtents->NumberOfDiskExtents; ++anExtentIter) {
                const DISK_EXTENT& anExtent = aVolDiskExtents->Extents[anExtentIter];
                if(anExtent.DiskNumber == theDiskNumber
                && anExtent.StartingOffset.QuadPart == thePart.StartingOffset.QuadPart
                && anExtent.ExtentLength.QuadPart == thePart.PartitionLength.QuadPart) {
                    CloseHandle(aVolHandle);
                    FindVolumeClose(aVolIter);
                    return StStringUtfWide(aVolName.data());
                }
            }
            CloseHandle(aVolHandle);
        }
        FindVolumeClose(aVolIter);
        return StStringUtfWide();
    }

};
#endif

/**
 * Dummy sub-class overriding scrollable behavior.
 */
class StGLOpenFileMenu : public StGLMenu {

        public:

    ST_LOCAL StGLOpenFileMenu(StGLWidget* theParent,
                              int  theLeft,
                              int  theTop,
                              int  theOrient = MENU_VERTICAL,
                              bool theIsRootMenu = false)
    : StGLMenu(theParent, theLeft, theTop, theOrient, theIsRootMenu) {}

    ST_LOCAL virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE {
        return StGLWidget::doScroll(theEvent); // skip StGLMenu
    }

};

StGLOpenFile::StGLOpenFile(StGLWidget*     theParent,
                           const StString& theTitle,
                           const StString& theCloseText)
: StGLMessageBox(theParent, theTitle, "",
                 theParent->getRoot()->scale(512), theParent->getRoot()->scale(400)),
  myCurrentPath(NULL),
  myHotListContent(NULL),
  myHotList(NULL),
  myList(NULL),
  myMainFilterCheck(NULL),
  myExtraFilterCheck(NULL),
  myToShowMainFilter(new StBoolParam(true)),
  myToShowExtraFilter(new StBoolParam(false)),
  myHighlightColor(0.5f, 0.5f, 0.5f, 1.0f),
  myItemColor     (1.0f, 1.0f, 1.0f, 1.0f),
  myFileColor     (0.7f, 0.7f, 0.7f, 1.0f),
  myHotColor      (1.0f, 1.0f, 1.0f, 1.0f),
  myHotSizeX (theParent->getRoot()->scale(10)),
  myMarginX  (theParent->getRoot()->scale(8)),
  myIconSizeX(theParent->getRoot()->scale(16)) {
    myToAdjustY = false;

    myToShowMainFilter ->signals.onChanged = stSlot(this, &StGLOpenFile::doFilterCheck);
    myToShowExtraFilter->signals.onChanged = stSlot(this, &StGLOpenFile::doFilterCheck);

    int aMarginTop = myMarginTop + myRoot->scale(30);
    myCurrentPath = new StGLTextArea(this, myMarginLeft, myMarginTop, StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                     myContent->getRectPx().width(), myContent->getRectPx().height());
    myCurrentPath->setupAlignment(StGLTextFormatter::ST_ALIGN_X_LEFT,
                                  StGLTextFormatter::ST_ALIGN_Y_TOP);
    myCurrentPath->setTextColor(myRoot->getColorForElement(StGLRootWidget::Color_MessageText));

    myContent->changeRectPx().top()  = aMarginTop;
    myContent->changeRectPx().left() = myMarginLeft + myHotSizeX;

    myHotListContent = new StGLScrollArea(this, myMarginLeft, aMarginTop,
                                          StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                          myHotSizeX, myContent->getRectPx().height());

    myHotList = new StGLOpenFileMenu(myHotListContent, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
    myHotList->setOpacity(1.0f, true);
    myHotList->setItemWidth(myHotSizeX);
    myHotList->setColor(StGLVec4(0.0f, 0.0f, 0.0f, 0.0f));

    myList = new StGLOpenFileMenu(myContent, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
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
    StHandle<StDriveNameResolver> aResolver = new StDriveNameResolver();
    aResolver->init();
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

void StGLOpenFile::setMimeList(const StMIMEList& theFilter,
                               const StString& theName,
                               const bool theIsExtra) {
    if(theIsExtra) {
        myExtraFilter = theFilter;
    } else {
        myFilter = theFilter;
    }

    StGLMenuCheckbox*& aFilterWidget = theIsExtra ? myExtraFilterCheck : myMainFilterCheck;
    if(!theName.isEmpty() && aFilterWidget == NULL) {
        StHandle<StBoolParam>& aFilterParam = theIsExtra ? myToShowExtraFilter : myToShowMainFilter;
        aFilterWidget = addHotCheckbox(aFilterParam, theName);
    }
    if(aFilterWidget != NULL) {
        aFilterWidget->setText(theName);
    }
    initExtensions();
}

StGLMenuCheckbox* StGLOpenFile::addHotCheckbox(const StHandle<StBoolParam>& theParam,
                                               const StString& theName) {
    StGLMenuCheckbox* aFilterWidget = new StGLMenuCheckbox(myHotList, theParam);

    StGLCheckbox* aCheckBox = aFilterWidget->getCheckbox();
    aCheckBox->setColor(myHotColor);
    aCheckBox->setCorner(StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_RIGHT));
    aCheckBox->changeRectPx().moveLeftTo(-aCheckBox->getRectPx().left());

    aFilterWidget->changeMargins().left  = -(myMarginX + myIconSizeX + myMarginX); // TODO weird logic
    aFilterWidget->setupStyle(StFTFont::Style_Italic);
    aFilterWidget->setText(theName);
    aFilterWidget->setupAlignment(StGLTextFormatter::ST_ALIGN_X_RIGHT, StGLTextFormatter::ST_ALIGN_Y_CENTER);
    aFilterWidget->setTextColor(myHotColor);
    aFilterWidget->setHilightColor(myHighlightColor);
    return aFilterWidget;
}

void StGLOpenFile::initExtensions() {
    myExtensions.clear();
    StArrayList<StString> aList1 = myToShowMainFilter ->getValue() ? myFilter.getExtensionsList()      : StArrayList<StString>(1);
    StArrayList<StString> aList2 = myToShowExtraFilter->getValue() ? myExtraFilter.getExtensionsList() : StArrayList<StString>(1);
    size_t anExtent = aList1.size() + aList2.size();
    myExtensions.initList(anExtent);
    for(size_t anExtIter = 0; anExtIter < aList1.size(); ++anExtIter) {
        myExtensions.add(aList1[anExtIter]);
    }
    for(size_t anExtIter = 0; anExtIter < aList2.size(); ++anExtIter) {
        myExtensions.add(aList2[anExtIter]);
    }
}

void StGLOpenFile::doHotItemClick(const size_t theItemId) {
    myItemToLoad = myHotPaths[theItemId];
}

void StGLOpenFile::doFilterCheck(const bool ) {
    initExtensions();
    if(!myFolder.isNull()) {
        StString aPath = myFolder->getPath();
        openFolder(aPath);
    }
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
    anItem->setupStyle(StFTFont::Style_Bold);
    anItem->setText(aName);
    anItem->setTextColor(myHotColor);
    anItem->setHilightColor(myHighlightColor);
    anItem->setUserData(myHotPaths.size() - 1);
    anItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doHotItemClick);

    int aSizeX = anItem->getMargins().left + anItem->computeTextWidth() + anItem->getMargins().right;
    myHotSizeX = stMax(myHotSizeX, aSizeX);

    myHotListContent->changeRectPx().right() = myHotListContent->getRectPx().left() + myHotSizeX;
    if(myMainFilterCheck != NULL) {
        myMainFilterCheck->changeRectPx().right() = myHotSizeX;
    }
    myContent->changeRectPx().left() = myHotListContent->getRectPx().right();
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
    myCurrentPath->setText(StString("<b>Location:*</b>") + aPath
                         + (!aPath.isEmpty() && !aPath.isEndsWith(SYS_FS_SPLITTER) ? ST_FILE_SPLITTER : ""));

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
