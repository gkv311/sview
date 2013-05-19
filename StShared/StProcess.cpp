/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StThreads/StProcess.h>
#include <StFile/StFileNode.h>
#include <StLibrary.h>

#ifndef _WIN32
    #include <unistd.h>
#endif

#if (defined(__APPLE__))
    #include <mach-o/dyld.h>
#elif (defined(__linux__) || defined(__linux))
    #include <fstream>
#endif

namespace {
#if (defined(_WIN64) || defined(__WIN64__))\
 || (defined(_LP64)  || defined(__LP64__))
    static const StString ST_ENV_NAME_STCORE_PATH = "StCore64";
#else
    static const StString ST_ENV_NAME_STCORE_PATH = "StCore32";
#endif

#ifdef _WIN32
    static const StString STCORE_NAME = StString("StCore")    + ST_DLIB_SUFFIX;
#else
    static const StString STCORE_NAME = StString("libStCore") + ST_DLIB_SUFFIX;
    static const StString ST_DEFAULT_PATH = "/usr/share/sView/";
#endif
}

const StString StArgument::ST_ARG_ON   ("on");
const StString StArgument::ST_ARG_TRUE ("true");
const StString StArgument::ST_ARG_OFF  ("off");
const StString StArgument::ST_ARG_FALSE("false");

StArgument::StArgument()
: key(),
  val() {
    //
}

StArgument::StArgument(const StString& theKey,
                       const StString& theValue)
: key(theKey),
  val(theValue.unquoted()) {
    //
}

StArgument::~StArgument() {
    //
}

StString StArgument::toString() const {
    return key + "=\"" + val + '\"';
}

bool StArgument::isValueOn() const {
    return val.isEqualsIgnoreCase(ST_ARG_ON) || val.isEqualsIgnoreCase(ST_ARG_TRUE);
}

bool StArgument::isValueOff() const {
    return val.isEqualsIgnoreCase(ST_ARG_OFF) || val.isEqualsIgnoreCase(ST_ARG_FALSE);
}

void StArgument::parseString(const StString& theString) {
    for(StUtf8Iter anIter = theString.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t('=')) {
            key = theString.subString(0, anIter.getIndex());
            val = theString.subString(anIter.getIndex() + 1, theString.getLength()).unquoted();
            return;
        }
    }
    key = theString;
}

StArgumentsMap::StArgumentsMap()
: StArrayList<StArgument>() {
    //
}

StArgumentsMap::~StArgumentsMap() {
    //
}

StString StArgumentsMap::toString() const {
    return StArrayList<StArgument>::toString();
}

void StArgumentsMap::parseList(const StArrayList<StString>& theStringList) {
    for(size_t id = 0; id < theStringList.size(); ++id) {
        StArgument newArgument;
        newArgument.parseString(theStringList[id]);
        add(newArgument);
    }
}

void StArgumentsMap::parseString(const StString& theString) {
    size_t aStart = 0;
    bool isInQuotes1 = false;
    bool isInQuotes2 = false;
    for(StUtf8Iter anIter = theString.iterator();; ++anIter) {
        if(*anIter == stUtf32_t('\'') && !isInQuotes2) {
            isInQuotes1 = !isInQuotes1;
        } else if(*anIter == stUtf32_t('\"') && !isInQuotes1) {
            isInQuotes2 = !isInQuotes2;
        } else if((*anIter == stUtf32_t('\n')
                || *anIter == stUtf32_t('\0')) && !isInQuotes1 && !isInQuotes2) {
            StArgument aNewArgument;
            aNewArgument.parseString(theString.subString(aStart, anIter.getIndex()));
            add(aNewArgument);
            aStart = anIter.getIndex() + 1;
        }
        if(*anIter == 0) {
            return;
        }
    }
}

StArgument StArgumentsMap::operator[](const StString& theKey) const {
    for(size_t anId = 0; anId < size(); ++anId) {
        const StArgument& anArg = getValue(anId);
        if(anArg.getKey().isEqualsIgnoreCase(theKey)) {
            return anArg;
        }
    }
    return StArgument();
}

static StString GetFontsRoot() {
#ifdef _WIN32
    return StProcess::getWindowsFolder() + "fonts\\";
#elif (defined(__APPLE__))
    //return "/System/Library/Fonts/";
    return "/Library/Fonts/";
    //return "/usr/X11/lib/X11/fonts/TTF/";
#elif (defined(__linux__) || defined(__linux))
    if(StFileNode::isFileExists("/usr/share/fonts/truetype/ttf-dejavu")) {
        // Ubuntu
        return "/usr/share/fonts/truetype/ttf-dejavu/";
    } else if(StFileNode::isFileExists("/usr/share/fonts/dejavu")) {
        // Gentoo
        return "/usr/share/fonts/dejavu/";
    } else if(StFileNode::isFileExists("/usr/share/fonts/TTF/dejavu")) {
        // Mandriva
        return "/usr/share/fonts/TTF/dejavu/";
    }
    // unknown
    return "/usr/share/fonts/";
#endif
}

StString StProcess::getFontsRoot() {
    static const StString FONTS_ROOT = GetFontsRoot();
    return FONTS_ROOT;
}

StString StProcess::getProcessFullPath() {
#ifdef _WIN32
    // TODO (Kirill Gavrilov#9) - implement correct method
    stUtfWide_t aBuff[MAX_PATH];
    if(GetModuleFileNameW(NULL, aBuff, MAX_PATH) == 0) {
        return StString();
    }
    StString aProcessPath = StString(aBuff);
    if(StFileNode::isNtExtendedPath(aProcessPath)) {
        // probably got truncated path, try to retrieve full
        stUtfWide_t aBuffExt[32768]; // how we should determine enough length???
        if(GetModuleFileNameW(NULL, aBuffExt, 32768) == 0) {
            return StString();
        }
        aProcessPath = StString(aBuffExt);
    }
    return aProcessPath;
#elif (defined(__APPLE__))
    // determine buffer size
    uint32_t aBytes = 0;
    _NSGetExecutablePath(NULL, &aBytes);
    if(aBytes == 0) {
        return StString();
    }

    // retrieve path to executable (probably link)
    stUtf8_t* aBuff = new stUtf8_t[aBytes + 1];
    _NSGetExecutablePath(aBuff, &aBytes);
    aBuff[aBytes] = '\0';

    // retrieve real path to executable (resolve links and normalize)
    stUtf8_t* aResultBuf = realpath(aBuff, NULL);
    if(aResultBuf == NULL) {
        delete[] aBuff;
        return StString();
    }

    StString aProcessPath = StString(aResultBuf);
    free(aResultBuf); // according to man for realpath()
    delete[] aBuff;
    return aProcessPath;
#elif (defined(__linux__) || defined(__linux))
    // get info from /proc/PID/exe
    stUtf8_t aBuff[ST_MAX_PATH];
    stUtf8_t aSimLink[ST_MAX_PATH];
    stsprintf(aSimLink, ST_MAX_PATH, "/proc/%d/exe", getpid());
    ssize_t aBytes = readlink(aSimLink, aBuff, ST_MAX_PATH);
    if(aBytes > 0) {
        // got the full path for process
        aBuff[aBytes] = '\0';
        return StString(aBuff);
    }
    return StString();
#endif
}

StString StProcess::getProcessName() {
    StString aFullPath = getProcessFullPath();
    size_t aLastSplit = size_t(-1);
    for(StUtf8Iter anIter = aFullPath.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t(SYS_FS_SPLITTER)) {
            aLastSplit = anIter.getIndex();
        }
    }

    if(aLastSplit != size_t(-1)) {
        return aFullPath.subString(aLastSplit + 1, aFullPath.getLength());
    } else {
        return aFullPath;
    }
}

StString StProcess::getProcessFolder() {
    StString aFullPath = getProcessFullPath();
    size_t aLastSplit = size_t(-1);
    for(StUtf8Iter anIter = aFullPath.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t(SYS_FS_SPLITTER)) {
            aLastSplit = anIter.getIndex();
        }
    }

    if(aLastSplit != size_t(-1)) {
        return aFullPath.subString(0, aLastSplit + 1); // including trailing separator!
    } else {
        return StString();
    }
}

StString StProcess::getWorkingFolder() {
    StString aWorkingFolder;
#ifdef _WIN32
    // determine buffer length (in characters, including NULL-terminated symbol)
    DWORD aBuffLen = GetCurrentDirectoryW(0, NULL);
    stUtfWide_t* aBuff = new stUtfWide_t[size_t(aBuffLen + 1)];
    // take current directory
    GetCurrentDirectoryW(aBuffLen, aBuff);
    aBuff[aBuffLen - 1] = (aBuff[aBuffLen - 2] == L'\\') ? L'\0' : L'\\';
    aBuff[aBuffLen]     = L'\0';
    aWorkingFolder = StString(aBuff);
    delete[] aBuff;
#else
    char* aPath = getcwd(NULL, 0);
    if(aPath == NULL) {
        // allocation error - should never happens
        return StString();
    }
    aWorkingFolder = StString(aPath) + SYS_FS_SPLITTER;
    free(aPath); // free alien buffer
#endif
    return aWorkingFolder;
}

StString StProcess::getTempFolder() {
    StString aTempFolder;
#ifdef _WIN32
    // determine buffer length (in characters, including NULL-terminated symbol)
    DWORD aBuffLen = GetTempPathW(0, NULL);
    stUtfWide_t* aBuff = new stUtfWide_t[size_t(aBuffLen + 1)];
    GetTempPathW(aBuffLen, aBuff);
    aBuff[aBuffLen - 1] = (aBuff[aBuffLen - 2] == L'\\') ? L'\0' : L'\\';
    aBuff[aBuffLen]     = L'\0';
    aTempFolder = StString(aBuff);
    delete[] aBuff;
#else
    aTempFolder = StString("/tmp/");
#endif
    return aTempFolder;
}

StString StProcess::getAbsolutePath(const StString& thePath) {
    if(StFileNode::isAbsolutePath(thePath)) {
        return thePath;
    }
    return StProcess::getWorkingFolder() + thePath; // do absolute path
}

#ifdef _WIN32
StString StProcess::getWindowsFolder() {
    StString aWinFolder;
    stUtfWide_t aWndFldr[MAX_PATH];
    GetWindowsDirectory(aWndFldr, MAX_PATH);
    aWndFldr[MAX_PATH - 1] = L'\0';
    aWinFolder = StString(aWndFldr) + SYS_FS_SPLITTER;
    return aWinFolder;
}

bool loadStringFromRegister(const StString& theRegisterPath, const StString& theParamPath, StString& theOutValue) {
    // TODO (Kirill Gavrilov) parse ERROR_MORE_DATA error (increase buffer)
    stUtfWide_t* aDataOut = new stUtfWide_t[4096U];
    HKEY hKey = NULL;
    DWORD aValueSize = sizeof(stUtfWide_t) * 4096U;
    RegOpenKeyExW(HKEY_CURRENT_USER, theRegisterPath.toUtfWide().toCString(), 0, KEY_READ, &hKey);
    if(RegQueryValueExW(hKey, theParamPath.toUtfWide().toCString(), NULL, NULL, (LPBYTE )aDataOut, &aValueSize) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        theOutValue = StString(aDataOut);
        delete[] aDataOut;
        return true;
    }
    RegCloseKey(hKey);
    delete[] aDataOut;
    return false;
}
#endif

bool isValidStCorePath(const StString& thePath) {
    return !thePath.isEmpty()
        && StFileNode::isFileExists(thePath + STCORE_NAME);
}

StString StProcess::getStCoreFolder() {
    StString aCoreEnvValue = getEnv(ST_ENV_NAME_STCORE_PATH);
#ifdef _WIN32
    if(aCoreEnvValue.isEmpty()) {
        // read env. value directly from registry (before first log off / log in)
        const StString aRegisterPath = "Environment";
        loadStringFromRegister(aRegisterPath, ST_ENV_NAME_STCORE_PATH, aCoreEnvValue);
    }
#endif

    // repair filesystem splitter
    if(!aCoreEnvValue.isEmpty() && !aCoreEnvValue.isEndsWith(SYS_FS_SPLITTER)) {
        aCoreEnvValue += StString(SYS_FS_SPLITTER);
    }

    if(isValidStCorePath(aCoreEnvValue)) {
        // environment variable is correctly set
        return aCoreEnvValue;
    }

    const StString aProcessPath = getProcessFolder();
    if(isValidStCorePath(aProcessPath)) {
        return aProcessPath;
    }
#ifndef _WIN32
    if(isValidStCorePath(ST_DEFAULT_PATH)) {
        return ST_DEFAULT_PATH;
    }
#endif
    return StString();
}

bool StProcess::execProcess(const StString&          theExecutablePath,
                            const StArray<StString>& theArguments) {
    if(!StFileNode::isFileExists(theExecutablePath)) {
        return false;
    }
#ifdef _WIN32
    // convert to wide strings
    StStringUtfWide anExecutablePathW = theExecutablePath.toUtfWide();
    StArrayList<StStringUtfWide> anArgumentsW(theArguments.size());
    StStringUtfWide aSplitter = ' ';
    StStringUtfWide aCmdLineW = StStringUtfWide('\"') + anExecutablePathW + StStringUtfWide("\" ");
    for(size_t anElem = 0;;) {
        // TODO (Kirill Gavrilov#9) we should probably quote arguments with spaces...
        // how to correctly deal this in the same way for UNIX / Windows?
        aCmdLineW += theArguments[anElem++].toUtfWide();
        if(anElem >= theArguments.size()) {
            break;
        }
        aCmdLineW += aSplitter;
    }

    STARTUPINFO aStartInfo;
    PROCESS_INFORMATION aProcessInfo;
    stMemSet(&aStartInfo, 0, sizeof(aStartInfo));
    aStartInfo.cb = sizeof(aStartInfo);
    stMemSet(&aProcessInfo, 0, sizeof(aProcessInfo));

    // start the process
    if(!CreateProcessW(anExecutablePathW.toCString(), (wchar_t* )aCmdLineW.toCString(),
        NULL, NULL, FALSE, 0, NULL, NULL, &aStartInfo, &aProcessInfo)) {
        return false;
    }

    // close process and thread handles
    CloseHandle(aProcessInfo.hProcess);
    CloseHandle(aProcessInfo.hThread);
    return true;
#else
    char** anArgList = new char*[theArguments.size() + 2];
    anArgList[0] = (char* )theExecutablePath.toCString();
    for(size_t anArgId = 0; anArgId < theArguments.size(); ++anArgId) {
        anArgList[anArgId + 1] = (char* )theArguments.getValue(anArgId).toCString();
    }
    anArgList[theArguments.size() + 1] = NULL;

    pid_t aChildPid = vfork();
    if(aChildPid == -1) {
        // fork fail
        delete[] anArgList;
        return false;
    } else if(aChildPid != 0) {
        // parent process give the control only after child
        // calls exit() or exec() functions
        delete[] anArgList;
        return true;
    }

    // child process
    execv(theExecutablePath.toCString(), anArgList);
    // fail
    _exit(1);
#endif
}

size_t StProcess::getPID() {
#ifdef _WIN32
    return (size_t )GetCurrentProcessId();
#else
    return (size_t )getpid();
#endif
}
