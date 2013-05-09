/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2013
 */

#ifdef __APPLE__

#ifndef __StAppResponder_h_
#define __StAppResponder_h_

#include <StTemplates/StHandle.h>
#include <StThreads/StThread.h>

#import <Cocoa/Cocoa.h>

// forward declarations
class StApplication;

/**
 * Main Cocoa application responder.
 */
@interface StAppResponder : NSObject <NSApplicationDelegate>
    {
        @private
        StHandle<StThread> myThread; //!< handle to StApplication thread (if started)

        @private
        StHandle<StApplication> myStApp; //!< StApplication instance

        @private
        NSTimer*           myTimer;  //!< timer may be used instead of dedicated thread

        @private
        bool          myIsThreaded;  //!< switch between threaded / rendered by timer style

        @private
        bool           myIsStarted;  //!< flag indicates that application is completely started
    }

        /// public interface

    + (StAppResponder* ) sharedInstance;

    /**
     * Default constructor.
     */
    - (id ) init;

        /// private methods

    /**
     * Dummy method for thread-safety Cocoa initialization.
     */
    + (void ) doDummyThread: (id ) theParam;

    /**
     * Action to launch Image Viewer drawer plugin.
     * Will start in another process if current is already busy.
     */
    - (void ) launchImageViewer: (id ) theSender;

    /**
     * Action to launch Movie Player drawer plugin.
     * Will start in another process if current is already busy.
     */
    - (void ) launchMoviePlayer: (id ) theSender;

    /**
     * Action to launch Diagnostics drawer plugin.
     * Will start in another process if current is already busy.
     */
    - (void ) launchDiagnostics: (id ) theSender;

    /**
     * Creates an instance of StApplication.
     */
    - (bool ) startStApp: (id ) theSender;

    /**
     * Process the rendering iteration for created StApplication.
     */
    - (bool ) loopIter: (id ) theSender;

    /**
     * Action to launch sView drawer in current process.
     */
    - (void ) launchSelf: (id ) theSender;

    /**
     * Access some values.
     */
    - (id ) valueForKey: (NSString* ) theKey;

@end

#endif // __StAppResponder_h_
#endif // __APPLE__
