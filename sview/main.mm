/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#if (defined(__APPLE__))

#include "StAppResponder.h"

#include <StCore/StApplication.h>
#include <StCore/StCore.h>
#include <StCocoa/StCocoaLocalPool.h>

namespace {

    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("sView");

    static StAppResponder* TheAppResponder = NULL;
    static StOpenInfo TheOpenInfo;
    static volatile bool TheToQuit = false;

    static SV_THREAD_FUNCTION anAppThreadFunc(void* theResponder) {
        StCocoaLocalPool aPool;
        StAppResponder* anAppResponder = (StAppResponder* )theResponder;
        if(![anAppResponder startStApp: NULL]) {
            return SV_THREAD_RETURN 2;
        }
        while([anAppResponder loopIter: NULL]) {}

        // will never happens
        return SV_THREAD_RETURN 0;
    }

};

@implementation StAppResponder

    - (id ) init {
        if(TheAppResponder != NULL) {
            if(self != TheAppResponder) {
                // should never happens
                [self release];
            }
            return TheAppResponder;
        }

        self = [super init];
        if(self == NULL) {
            return NULL;
        }
        myStApp = NULL;
        myTimer = NULL;
        myIsThreaded = true;
        myIsStarted = false;
        return self;
    }

    + (StAppResponder* ) sharedInstance {
        if(TheAppResponder == NULL) {
            TheAppResponder = [[super allocWithZone: NULL] init];
        }
        return TheAppResponder;
    }

    /**
     * Singletone implementation.
     */
    + (id ) allocWithZone: (NSZone* ) theZone {
        return [[self sharedInstance] retain];

    }

    /**
     * Singletone implementation.
     */
    - (id ) copyWithZone: (NSZone* ) theZone {
        return self;
    }

    /**
     * Singletone implementation.
     */
    - (id ) retain {
        return self;
    }

    /**
     * Singletone implementation.
     */
    - (NSUInteger ) retainCount {
        return NSUIntegerMax;  //denotes an object that cannot be released
    }

    /**
     * Singletone implementation.
     */
    - (void ) release {
        //do nothing
    }

    /**
     * Singletone implementation.
     */
    - (id ) autorelease {
        return self;
    }

    /**
     * Cocoa file opening (does NOT passed as argument to application).
     * Notice that this method called before applicationDidFinishLaunching(),
     * so we should not do here anything useful.
     */
    - (BOOL ) application: (NSApplication* ) theApplication
                 openFile: (NSString* ) theFilename {
        StString aFilePath([[theFilename precomposedStringWithCanonicalMapping] UTF8String]);
        if(aFilePath.isEmpty()) {
            return NO;
        } else if(!myIsStarted) {
            // application just started
            TheOpenInfo.setPath(aFilePath);
            return YES;
        }

        if(myStApp == NULL) {
            TheOpenInfo.setPath(aFilePath);
            [self launchSelf: NULL];
            return YES;
        }

        StArrayList<StString> anArguments(2);
        anArguments.add("-");
        anArguments.add(aFilePath);
        StProcess::execProcess(StProcess::getProcessFullPath(), anArguments);
        return YES;
    }

    - (void ) applicationDidFinishLaunching: (NSNotification* ) theNotification {
        StArrayList<StString> anArgs = StProcess::getArguments();
        size_t anArgsNb = anArgs.size();
        // open path set by Cocoa mechanisms
        const StString aCocoaOpenPath = TheOpenInfo.getPath();
        if(anArgsNb > 1) {
            // search for special flags
            const StString ST_COCOA_IS_THREADED = "--cocoa-threaded";
            const StString ST_COCOA_PSN = "-psn";
            for(size_t anArgId = 0; anArgId < anArgs.size(); ++anArgId) {
                const StString& aParam = anArgs[anArgId];
                if(aParam.isStartsWith(ST_COCOA_IS_THREADED)) {
                    StString aVal = aParam.subString(ST_COCOA_IS_THREADED.getLength() + 1,
                                                     aParam.getLength());
                    myIsThreaded = !aVal.isEqualsIgnoreCase("no")
                                && !aVal.isEqualsIgnoreCase("off")
                                && !aVal.isEqualsIgnoreCase("false");
                    --anArgsNb;
                } else if(aParam.isStartsWith(ST_COCOA_PSN)) {
                    --anArgsNb;
                } else if(aParam == aCocoaOpenPath) {
                    // Cocoa just duplicated normal command-line parameter.
                    // Ignore it and leave arguments parsing to sView itself.
                    TheOpenInfo.setPath("");
                    break;
                }
            }
        }
        if(TheOpenInfo.hasPath()
        || anArgsNb > 1) {
            // create StApplication instance only if we know which drawer plugin to launch
            [self launchSelf: NULL];
        }
        myIsStarted = true;
    }

    - (void ) applicationWillTerminate: (NSNotification* ) theNotification {
        TheToQuit = true; // needed if sView closed by Cocoa call rather than from StDrawer
        if(!myThread.isNull()) {
            myThread->wait();
            myThread.nullify();
        } else {
            StOpenInfo aCloseInfo;
            aCloseInfo.setMIME(StDrawerInfo::CLOSE_MIME());
            if(myStApp != NULL && myStApp->isOpened()) {
                myStApp->open(aCloseInfo);
            }
            delete myStApp;
            myStApp = NULL;
        }
        StCore::FREE();
    }

    - (void ) launchSelf: (id ) theSender {
        if(myStApp != NULL
        || !myThread.isNull()) {
            return; // already launched - invalid call
        }

        if(myIsThreaded) {
            // start StApplication instance in dedicated thread
            ST_DEBUG_LOG("StAppResponder, application started in dedicated thread");
            myThread = new StThread(anAppThreadFunc, self);
        } else {
            // process StApplication rendering iterations by timer
            ST_DEBUG_LOG("StAppResponder, application started in main thread!");
            if(![self startStApp: NULL]) {
                return;
            }
            myTimer = [[NSTimer scheduledTimerWithTimeInterval: 0.001 ///0.017
                                                        target: self
                                                      selector: @selector(loopIter:)
                                                      userInfo: NULL
                                                       repeats: YES] retain];
        }
    }

    - (bool ) startStApp: (id ) theSender {
        if(myStApp != NULL) {
            return false;
        }
        myStApp = new StApplication();
        if(!myStApp->create()) {
            delete myStApp;
            myStApp = NULL;
            return false;
        }
        if(!myStApp->open(TheOpenInfo)) {
            delete myStApp;
            myStApp = NULL;
            return false;
        }
        return true;
    }

    - (bool ) loopIter: (id ) theSender {
        if(myStApp == NULL) {
            return false;
        }

        if(TheToQuit) {
            StOpenInfo aCloseInfo;
            aCloseInfo.setMIME(StDrawerInfo::CLOSE_MIME());
            if(myStApp->isOpened()) {
                myStApp->open(aCloseInfo);
            }
        }

        if(!myStApp->isOpened()) {
            delete myStApp;
            myStApp = NULL;

            // this will call exit(), so code below has no effect
            [NSApp terminate: nil];
            return false;
        }
        myStApp->callback();
        return true;
    }

    - (void ) launchImageViewer: (id ) theSender {
        StString aDrawerPath(StProcess::getStCoreFolder() + StCore::getDrawersDir()
                             + SYS_FS_SPLITTER + "StImageViewer" + ST_DLIB_SUFFIX);
        if(myStApp == NULL) {
            TheOpenInfo.setMIME(StDrawerInfo::DRAWER_MIME().toString());
            TheOpenInfo.setPath(aDrawerPath);
            [self launchSelf: NULL];
            return;
        }

        StArrayList<StString> anArguments(1);
        anArguments.add("--in=image");
        StProcess::execProcess(StProcess::getProcessFullPath(), anArguments);
    }

    - (void ) launchMoviePlayer: (id ) theSender {
        StString aDrawerPath(StProcess::getStCoreFolder() + StCore::getDrawersDir()
                             + SYS_FS_SPLITTER + "StMoviePlayer" + ST_DLIB_SUFFIX);
        if(myStApp == NULL) {
            TheOpenInfo.setMIME(StDrawerInfo::DRAWER_MIME().toString());
            TheOpenInfo.setPath(aDrawerPath);
            [self launchSelf: NULL];
            return;
        }

        StArrayList<StString> anArguments(1);
        anArguments.add("--in=video");
        StProcess::execProcess(StProcess::getProcessFullPath(), anArguments);
    }

    - (void ) launchDiagnostics: (id ) theSender {
        StString aDrawerPath(StProcess::getStCoreFolder() + StCore::getDrawersDir()
                             + SYS_FS_SPLITTER + "StDiagnostics" + ST_DLIB_SUFFIX);
        if(myStApp == NULL) {
            TheOpenInfo.setMIME(StDrawerInfo::DRAWER_MIME().toString());
            TheOpenInfo.setPath(aDrawerPath);
            [self launchSelf: NULL];
            return;
        }

        StArrayList<StString> anArguments(1);
        anArguments.add("--in=StDiagnostics");
        StProcess::execProcess(StProcess::getProcessFullPath(), anArguments);
    }

    - (id ) valueForKey: (NSString* ) theKey {
        StString aKey([theKey UTF8String]);
        if(aKey == "sViewVersion") {
            StString aValueSt = StVersionInfo::getSDKVersionString();
            NSString* aValueNs = [NSString stringWithUTF8String: aValueSt.toCString()];
            return aValueNs;
        }
        return @"Wrong Key";
    }

    + (void ) doDummyThread: (id ) theParam {}

@end

int main(int , char** ) {
#ifdef __ST_DEBUG__
    // TODO (Kirill Gavrilov#9) debug environment
    #if (defined(_LP64) || defined(__LP64__))
        const StString ST_ENV_NAME_STCORE_PATH = "StCore64";
    #else
        const StString ST_ENV_NAME_STCORE_PATH = "StCore32";
    #endif
    StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, StProcess::getProcessFolder());
#endif

    // autorelease pool is required for Cocoa
    StCocoaLocalPool aPool;

    // create dummy NSThread to ensure Cocoa thread-safety
    [NSThread detachNewThreadSelector: @selector(doDummyThread: )
                             toTarget: [StAppResponder class]
                           withObject: NULL];

    // initialize Cocoa application
    NSApplication* anAppNs = [NSApplication sharedApplication];
    StAppResponder* anAppResp = [StAppResponder sharedInstance];
    [anAppNs setDelegate: anAppResp];
    [NSBundle loadNibNamed: @"MainMenu"
                     owner: anAppResp];

    // create dummy hidden window - workaround against strange bug
    // when right half of GL window title doesn't allow to be moved
    NSWindow* aWin = [NSWindow alloc];
    [aWin initWithContentRect: NSMakeRect(16, 16, 16, 16)
                    styleMask: NSBorderlessWindowMask
                      backing: NSBackingStoreBuffered
                        defer: NO];
    [aWin release];

    // allow our application to steal inout focus (when needed)
    [anAppNs activateIgnoringOtherApps: YES];

    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("StCore Library initialization FAILED!");
        return 1;
    }

    // Cocoa event loop can be started ONLY in main thread
    [anAppNs run];
    // will never come here - we should use applicationWillTerminate if necessary
    return 0;
}

#endif // __APPLE__
