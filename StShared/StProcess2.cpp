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
StArrayList<StString> StProcess::getArguments() {
    StArrayList<StString> aList;
#if (defined(_WIN32) || defined(__WIN32__))
    int argc = 0;
    stUtfWide_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for(int aParamId = 0; aParamId < argc; ++aParamId) {
        aList.add(StString(argv[aParamId]));
    }
    // free memory allocated for CommandLineToArgvW arguments.
    LocalFree(argv);
#elif (defined(__APPLE__))
    if(_NSGetArgc() == NULL || _NSGetArgv() == NULL) {
        return aList; // is it possible?
    }
    int anArgsNb = *_NSGetArgc();
    char** anArgVec = *_NSGetArgv();
    for(int aParamId = 0; aParamId < anArgsNb; ++aParamId) {
        // automatically convert filenames from decomposed form used by Mac OS X file systems
        aList.add(stFromUtf8Mac(anArgVec[aParamId]));
    }
#elif (defined(__linux__) || defined(__linux))
    stUtf8_t aCmdlineInfoFile[4096];
    sprintf(aCmdlineInfoFile, "/proc/%d/cmdline", getpid());
    std::ifstream iFile;
    iFile.open(aCmdlineInfoFile);
    if(iFile.is_open()) {
        char aCmdlineInfoBuff[4096];
        while(!iFile.eof()) {
            stMemSet(aCmdlineInfoBuff, 0, sizeof(aCmdlineInfoBuff));
            iFile.getline(aCmdlineInfoBuff, 4096, '\0');
            if(aCmdlineInfoBuff[0] != '\0') {
                aList.add(StString(aCmdlineInfoBuff));
            }
        }
        iFile.close();
    }
#endif
    return aList;
}
