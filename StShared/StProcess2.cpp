/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StThreads/StProcess.h>

#if (defined(__APPLE__))
    #include <crt_externs.h>
#elif (defined(__linux__) || defined(__linux))
    #include <fstream>
    #include <unistd.h>
#endif

// we move this function to another object file
// to optimize static linkage (avoid unnecessary dependencies)
std::vector<StString> StProcess::getArguments() {
    std::vector<StString> aList;
#if defined(_WIN32)
    int argc = 0;
    stUtfWide_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int aParamId = 0; aParamId < argc; ++aParamId) {
        aList.push_back(StString(argv[aParamId]));
    }
    // free memory allocated for CommandLineToArgvW arguments.
    LocalFree(argv);
#elif defined(__APPLE__)
    if (_NSGetArgc() == nullptr || _NSGetArgv() == nullptr) {
        return aList; // is it possible?
    }
    int anArgsNb = *_NSGetArgc();
    char** anArgVec = *_NSGetArgv();
    for (int aParamId = 0; aParamId < anArgsNb; ++aParamId) {
        // automatically convert filenames from decomposed form used by Mac OS X file systems
        aList.push_back(stFromUtf8Mac(anArgVec[aParamId]));
    }
#elif (defined(__linux__) || defined(__linux))
    stUtf8_t aCmdlineInfoFile[4096];
    stsprintf(aCmdlineInfoFile, 4096, "/proc/%d/cmdline", getpid());
    std::ifstream iFile;
    iFile.open(aCmdlineInfoFile);
    if (iFile.is_open()) {
        char aCmdlineInfoBuff[4096];
        while (!iFile.eof()) {
            stMemSet(aCmdlineInfoBuff, 0, sizeof(aCmdlineInfoBuff));
            iFile.getline(aCmdlineInfoBuff, 4096, '\0');
            if (aCmdlineInfoBuff[0] != '\0') {
                aList.push_back(StString(aCmdlineInfoBuff));
            }
        }
        iFile.close();
    }
#endif
    return aList;
}
