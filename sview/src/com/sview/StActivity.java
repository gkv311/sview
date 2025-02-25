/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014-2017
 */
package com.sview;

import java.io.FileNotFoundException;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.widget.Toast;

/**
 * Customize NativeActivity
 */
public class StActivity extends NativeActivity implements SensorEventListener {

//region Native libraries loading routines

    /**
     * Auxiliary method to close activity on critical error
     */
    public static void exitWithError(final Activity theActivity,
                                     String         theError) {
        AlertDialog.Builder aBuilder = new AlertDialog.Builder(theActivity);
        aBuilder.setMessage(theError).setNegativeButton("Exit", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface theDialog, int theId) {
                Intent anIntent = new Intent(Intent.ACTION_MAIN);
                anIntent.addCategory(Intent.CATEGORY_HOME);
                anIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                anIntent.putExtra("EXIT", true);
                theActivity.startActivity(anIntent);
            }
        });
        AlertDialog aDialog = aBuilder.create();
        aDialog.show();

        Intent anIntent = new Intent(theActivity, CrashReportActivity.class);
        anIntent.putExtra(CrashReportActivity.THE_ERROR_REPORT_ID, theError);
        anIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        theActivity.startActivity(anIntent);
    }

    /**
     * Auxiliary method to show temporary info messages
     */
    private static boolean loadLibVerbose(String        theLibName,
                                          StringBuilder theErrors) {
        try {
            System.loadLibrary(theLibName);
            theErrors.append("Info:  native library \"");
            theErrors.append(theLibName);
            theErrors.append("\" has been loaded\n");
            return true;
        } catch(java.lang.UnsatisfiedLinkError theError) {
            theErrors.append("Error: native library \"");
            theErrors.append(theLibName);
            theErrors.append("\" is unavailable:\n  " + theError.getMessage() + "\n");
            return false;
        } catch(SecurityException theError) {
            theErrors.append("Error: native library \"");
            theErrors.append(theLibName);
            theErrors.append("\" can not be loaded for security reasons:\n  " + theError.getMessage() + "\n");
            return false;
        }
    }

    private static boolean wasNativesLoadCalled = false;
    private static boolean areNativeLoaded = false;

    /**
     * Auxiliary method to load native libraries.
     */
    public static boolean loadNatives(Activity      theActivity,
                                      StringBuilder theInfo) {
        if(wasNativesLoadCalled) {
           return areNativeLoaded;
        }

        wasNativesLoadCalled = true;

        // libraries to load in the opposite order of their dependencies
        String[] aLibs = {
            "freetype",
            //"config++", // linked statically
            "avutil", "swresample", "avcodec", "avformat", "swscale",
            "openal",
            "StShared", "StGLWidgets", "StCore",
            "StOutAnaglyph", "StOutDistorted", "StOutInterlace",
            //"StOutDual", "StOutIZ3D", "StOutPageFlip", // useless on Android
            "StImageViewer", "StMoviePlayer", "sview"
        };

        // load C++ runtime first
        boolean hasErrors = !(loadLibVerbose("c++_shared", theInfo)
                           || loadLibVerbose("gnustl_shared", theInfo));
        for (String aLibIter : aLibs) {
            hasErrors = !loadLibVerbose(aLibIter, theInfo) || hasErrors;
        }

        areNativeLoaded = !hasErrors;
        if (hasErrors) {
            StActivity.exitWithError(theActivity, "Broken apk?\n" + theInfo);
            return false;
        }
        return true;
    }

    /**
     * Define device Left/Right eyes swap flag.
     * Considering this flag depends on device output implementation.
     */
    public void setSwapEyes(boolean theToSwap) {
        if(myCppGlue != 0) {
            cppSetSwapEyes(myCppGlue, theToSwap);
        }
    }

//endregion

//region Overridden methods of NativeActivity class

    /**
     * Create activity.
     */
    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        StringBuilder anInfo = new StringBuilder();
        loadNatives(this, anInfo);

        // create folder for external storage
        myContext = new ContextWrapper(this);
        myContext.getExternalFilesDir(null);

        askUserPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE, null);

        android.os.PowerManager aPowerManager = (android.os.PowerManager)getSystemService(Context.POWER_SERVICE);
        myWakeLock = aPowerManager.newWakeLock(android.os.PowerManager.PARTIAL_WAKE_LOCK, "sView.PartialWakeLock");

        mySensorMgr = (SensorManager )getSystemService(Context.SENSOR_SERVICE);
        mySensorOri = mySensorMgr.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
        if(mySensorOri == null) {
            myIsPoorOri = true;
            mySensorOri = mySensorMgr.getDefaultSensor(Sensor_TYPE_ORIENTATION_DEPRECATED);
        } else {
            myIsPoorOri = mySensorMgr.getDefaultSensor(Sensor.TYPE_GYROSCOPE) == null;
        }
        myS3dvSurf = new StS3dvSurface();

        try {
            super.onCreate(theSavedInstanceState);
        } catch(java.lang.UnsatisfiedLinkError theError) {
            anInfo.append("Error: native library is unavailable:\n  " + theError.getMessage());
            StActivity.exitWithError(this, "Broken apk?\n" + anInfo);
        } catch(SecurityException theError) {
            anInfo.append("Error: native library can not be loaded for security reasons:\n  " + theError.getMessage());
            StActivity.exitWithError(this, "Broken apk?\n" + anInfo);
        } catch(Exception theError) {
            java.io.StringWriter aStringWriter = new java.io.StringWriter();
            theError.printStackTrace (new java.io.PrintWriter (aStringWriter));
            anInfo.append("Error: unhandled exception:\n  " + theError.getMessage()
                        + "\nCall stack:\n" + aStringWriter.toString());
            StActivity.exitWithError(this, "Broken apk?\n" + anInfo);
        }

        updateHideSystemBars(myToHideStatusBar, myToHideNavBar);

        // readOpenPath(false) should be called - NULLify intent afterwards
        setIntent(null);
    }

    /**
     * Request user permission.
     */
    protected void askUserPermission(String thePermission, String theRationale) {
        // Dynamically load methods introduced by API level 23.
        // On older system this permission is granted by user during application installation.
        java.lang.reflect.Method aMetPtrCheckSelfPermission, aMetPtrRequestPermissions, aMetPtrShouldShowRequestPermissionRationale;
        try {
            aMetPtrCheckSelfPermission = myContext.getClass().getMethod("checkSelfPermission", String.class);
            aMetPtrRequestPermissions = getClass().getMethod("requestPermissions", String[].class, int.class);
            aMetPtrShouldShowRequestPermissionRationale = getClass().getMethod("shouldShowRequestPermissionRationale", String.class);
        } catch(SecurityException theError) {
            //postMessage("Unable to find permission methods:\n" + theError.getMessage());
            return;
        } catch(NoSuchMethodException theError) {
            //postMessage("Unable to find permission methods:\n" + theError.getMessage());
            return;
        }

        try {
            //int isAlreadyGranted = myContext.checkSelfPermission(thePermission);
            int isAlreadyGranted = (Integer )aMetPtrCheckSelfPermission.invoke(myContext, thePermission);
            if(isAlreadyGranted == android.content.pm.PackageManager.PERMISSION_GRANTED) {
                return;
            }

            //boolean toShowInfo = shouldShowRequestPermissionRationale(thePermission);
            boolean toShowInfo = theRationale != null && (Boolean )aMetPtrShouldShowRequestPermissionRationale.invoke(this, thePermission);
            if(toShowInfo) {
                postMessage(theRationale);
            }

            // show dialog to user
            //requestPermissions (new String[]{thePermission}, 0);
            aMetPtrRequestPermissions.invoke(this, new String[]{thePermission}, 0);
        } catch(IllegalArgumentException theError) {
            postMessage("Internal error: Unable to call permission method:\n" + theError.getMessage());
            return;
        } catch(IllegalAccessException theError) {
            postMessage("Internal error: Unable to call permission method:\n" + theError.getMessage());
            return;
        } catch(java.lang.reflect.InvocationTargetException theError) {
            postMessage("Internal error: Unable to call permission method:\n" + theError.getMessage());
            return;
        }
    }

    /**
     * Handle new open file event.
     */
    @Override
    protected void onNewIntent(Intent theIntent) {
        super.onNewIntent(theIntent);
        setIntent(theIntent);
    }

    /**
     * Handle resume event.
     */
    @Override
    protected void onResume() {
        super.onResume();
        if(myToTrackOrient) {
            updateTrackOrientation(true);
        }
        updateHideSystemBars(myToHideStatusBar, myToHideNavBar);

        android.os.Handler aHandler = new android.os.Handler();
        aHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                updateHideSystemBars(myToHideStatusBar, myToHideNavBar);
            }
        }, 500);
    }

    /**
     * Handle pause event.
     */
    @Override
    protected void onPause() {
       super.onPause();
       updateTrackOrientation(false);
    }

    /**
     * Redirect back button to C++ level.
     */
    @Override
    public void onBackPressed() {
        if(myCppGlue != 0) {
            cppOnBackPressed(myCppGlue);
        } else {
            super.onBackPressed();
        }
    }

    /**
     * Redirect key event to C++ level.
     */
    @Override
    public boolean dispatchKeyEvent(android.view.KeyEvent theEvent) {
        if(super.dispatchKeyEvent(theEvent)) {
            return true;
        }
        return myCppGlue != 0 && cppIsKeyOverridden(myCppGlue, theEvent.getKeyCode());
    }

    @Override
    public void surfaceCreated(SurfaceHolder theHolder) {
        super.surfaceCreated(theHolder);
        myS3dvSurf.setSurface(theHolder);
        myS3dvSurf.setStereo(myToEnableStereoHW);
    }

    @Override
    public void surfaceChanged(SurfaceHolder theHolder, int theFormat, int theWidth, int theHeight) {
        if(myCppGlue != 0) {
            cppOnBeforeSurfaceChanged(myCppGlue, true);
        }
        super.surfaceChanged(theHolder, theFormat, theWidth, theHeight);
        if(myCppGlue != 0) {
            cppOnBeforeSurfaceChanged(myCppGlue, false);
        }
        myS3dvSurf.setSurface(theHolder);
        myS3dvSurf.setStereo(myToEnableStereoHW);
    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder theHolder) {
        super.surfaceRedrawNeeded(theHolder);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder theHolder) {
        super.surfaceDestroyed(theHolder);
    }

//endregion

//region Implementation of SensorEventListener interface

    @Override
    public void onAccuracyChanged(Sensor theSensor, int theAccuracy) {}

    /**
     * Fetch new orientation quaternion.
     */
    @Override
    public void onSensorChanged(SensorEvent theEvent) {
        if(myCppGlue == 0) {
            return;
        }

        float aScreenRot = 0.0f;
        switch(getWindowManager().getDefaultDisplay().getRotation()) {
            case android.view.Surface.ROTATION_0:   aScreenRot = 0.0f;   break;
            case android.view.Surface.ROTATION_90:  aScreenRot = 90.0f;  break;
            case android.view.Surface.ROTATION_180: aScreenRot = 180.0f; break;
            case android.view.Surface.ROTATION_270: aScreenRot = 270.0f; break;
        }

        switch(theEvent.sensor.getType()) {
            case Sensor_TYPE_ORIENTATION_DEPRECATED: {
                cppSetOrientation(myCppGlue, theEvent.values[0], theEvent.values[1], theEvent.values[2], aScreenRot);
                return;
            }
            case Sensor.TYPE_ROTATION_VECTOR: {
                SensorManager.getQuaternionFromVector(myQuat, theEvent.values);
                cppSetQuaternion(myCppGlue, myQuat[0], myQuat[1], myQuat[2], myQuat[3], aScreenRot);
                return;
            }
        }
    }

//endregion

//region Auxiliary methods

    /**
     * Action to play/pause.
     */
    public static final String THE_ACTION_PLAY_PAUSE = "ACTION_PLAY_PAUSE";

    /**
     * Action to open previous item in playlist.
     */
    public static final String THE_ACTION_PLAY_PREV = "ACTION_PLAY_PREV";

    /**
     * Action to open next item in playlist.
     */
    public static final String THE_ACTION_PLAY_NEXT = "ACTION_PLAY_NEXT";

    /**
     * Set playback action.
     */
    public boolean setPlaybackAction(Intent theIntent) {
        if(myCppGlue == 0 || theIntent == null) {
            return false;
        }

        if(THE_ACTION_PLAY_PAUSE.equals(theIntent.getAction())) {
            cppSetOpenPath(myCppGlue, THE_ACTION_PLAY_PAUSE, "", false);
            return true;
        } else if(THE_ACTION_PLAY_PREV.equals(theIntent.getAction())) {
            cppSetOpenPath(myCppGlue, THE_ACTION_PLAY_PREV, "", false);
            return true;
        } else if(THE_ACTION_PLAY_NEXT.equals(theIntent.getAction())) {
            cppSetOpenPath(myCppGlue, THE_ACTION_PLAY_NEXT, "", false);
            return true;
        }
        return false;
    }

    /**
     * Read the open path from current intent and nullify it.
     * This method is called by StAndroidGlue from C++.
     */
    protected void readOpenPath(boolean theToNullifyIntent) {
        if(myCppGlue == 0) {
            return;
        }

        Intent anIntent = getIntent();
        if(theToNullifyIntent) {
            setIntent(null);
        }
        if(anIntent == null) {
            cppSetOpenPath(myCppGlue, "", "", false);
            return;
        }
        if(setPlaybackAction(anIntent)) {
            return;
        }

        String anOpenPath = anIntent.getDataString();
        String anOpenMime = anIntent.getType();
        boolean isLaunchedFromHistory = (anIntent.getFlags() & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY) != 0;
        android.net.Uri aContentUri = null;
        if("content".equals(anIntent.getScheme())) {
            aContentUri = anIntent.getData();
        }
        if(Intent.ACTION_SEND.equals(anIntent.getAction())) {
            aContentUri = anIntent.getParcelableExtra(Intent.EXTRA_STREAM);
            if(aContentUri != null) {
                anOpenPath = aContentUri.toString();
            }
        }

        if(aContentUri != null) {
            android.content.ContentResolver aContentResolver = getContentResolver();
            try {
                // Resolve the real file path from symlink in /proc.
                // The full path is much simpler to handle within C++ level
                // and allows to fill playlist from folder content.
                android.os.ParcelFileDescriptor aParcelFile = aContentResolver.openFileDescriptor(aContentUri, "r");
                String aFilePathInProc = "/proc/self/fd/" + aParcelFile.getFd();
                java.io.File aFileSymLink = new java.io.File(aFilePathInProc);
                String aCanonicalPath = aFileSymLink.getCanonicalPath();
                java.io.File aFileCanonical = new java.io.File(aCanonicalPath);
                if(aFileCanonical.canRead()) {
                    // take the real file when it is accessible by process
                    anOpenPath = aCanonicalPath;
                }
                // otherwise file should be read using opened file descriptor
            } catch(FileNotFoundException e) {
                //e.printStackTrace();
            } catch(IOException e) {
                //e.printStackTrace();
            } catch(SecurityException e) {
                //e.printStackTrace();
            }
        }

        cppSetOpenPath(myCppGlue, anOpenPath, anOpenMime, isLaunchedFromHistory);
    }

    /**
     * Open file descriptor for specified path, including content:// URLs.
     * This method is called by StAndroidGlue from C++.
     * @return file descriptor, which should be closed by caller, or -1 on error
     */
    protected int openFileDescriptor(String thePath) {
        if(thePath.isEmpty()) {
            return -1;
        }

        android.content.ContentResolver aContentResolver = getContentResolver();
        android.net.Uri anUri = android.net.Uri.parse(thePath);
        try {
            android.os.ParcelFileDescriptor aParcelFile = aContentResolver.openFileDescriptor(anUri, "r");
            return aParcelFile.detachFd();
        } catch(FileNotFoundException e) {
            //e.printStackTrace();
        } catch(SecurityException e) {
            //e.printStackTrace();
        }

        return -1;
    }

    /**
     * Wrapper to turn orientation sensor on/off (regardless off myToTrackOrient flag).
     */
    protected void updateTrackOrientation(boolean theToTrack) {
        if(mySensorOri == null) {
            return;
        }

        if(theToTrack) {
            mySensorMgr.registerListener(this, mySensorOri, SensorManager.SENSOR_DELAY_FASTEST);
        } else {
            mySensorMgr.unregisterListener(this);
        }
    }

    /**
     * Wrapper to show/hide navigation bar.
     */
    protected void updateHideSystemBars(boolean theToHideStatusBar,
                                        boolean theToHideNavBar) {
        if(android.os.Build.VERSION.SDK_INT < 19) {
            return;
        }

        int anOptions = 0;

        // Status bar can not be restored by these flags if
        // "@android:style/Theme.NoTitleBar.Fullscreen" is set in manifest!
        if(theToHideStatusBar) {
            anOptions = anOptions
                      | 0x00000004  // SYSTEM_UI_FLAG_FULLSCREEN
                      | 0x00001000  // SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                      | 0x00000100; // SYSTEM_UI_FLAG_LAYOUT_STABLE
        }
        if(theToHideNavBar) {
            anOptions = anOptions
                      | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                      | 0x00001000  // SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                      | 0x00000200  // SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                      | 0x00000100; // SYSTEM_UI_FLAG_LAYOUT_STABLE
        }

        final android.view.View aDecorView = getWindow().getDecorView();
        aDecorView.setSystemUiVisibility(anOptions);
    }

//endregion

//region Methods to be called from C++ level

    /**
     * Virtual method defining StApplication class.
     */
    public String getStAppClass() {
        return null;
    }

    /**
     * Method returning identifier of stereo API available on Android device.
     */
    public String getStereoApiInfo() {
        if(StS3dvSurface.isApiAvailable()) {
            return "S3DV";
        }
        return null;
    }

    /**
     * Set new activity title.
     */
    public void setWindowTitle(String theTitle) {
        if(android.os.Build.VERSION.SDK_INT < 21) {
            return;
        }

        final String aTitle = theTitle;
        this.runOnUiThread (new Runnable() { public void run() {
            //setTitle(aTitle); // sets window title, which we don't have...
            setTaskDescription(new android.app.ActivityManager.TaskDescription(aTitle));
        }});
    }

    /**
     * Keep CPU on.
     */
    public void setPartialWakeLockOn(String theTitle, boolean theToLock) {
        if(myWakeLock == null) {
            return;
        }

        if(theToLock) {
            myWakeLock.acquire();
        } else if(myWakeLock.isHeld()) {
            myWakeLock.release();
        }
    }

    /**
     * Method to turn stereo output on or off.
     */
    public void setHardwareStereoOn(boolean theToEnable) {
        final boolean toEnable = theToEnable;
        this.runOnUiThread (new Runnable() { public void run() {
            if(myToEnableStereoHW == toEnable) {
                return;
            }
            myToEnableStereoHW = toEnable;
            myS3dvSurf.setStereo(toEnable);
        }});
    }

    /**
     * Method is called when StAndroidGlue has been created (BEFORE starting application thread!)
     * or during destruction.
     */
    public void setCppInstance(long theCppInstance) {
        myCppGlue = theCppInstance;
        if(myCppGlue != 0) {
            cppDefineOrientationSensor(myCppGlue, mySensorOri != null, myIsPoorOri);
            if(myToTrackOrient) {
                updateTrackOrientation(true);
            }
        } else {
            updateTrackOrientation(false);
        }
    }

    /**
     * Method to turn orientation sensor on/off.
     */
    public void setTrackOrientation(boolean theToTrack) {
        final boolean toTrack = theToTrack;
        this.runOnUiThread (new Runnable() { public void run() {
            if(myToTrackOrient == toTrack
            || mySensorOri     == null) {
                myToTrackOrient = toTrack;
                return;
            }

            myToTrackOrient = toTrack;
            updateTrackOrientation(toTrack);
        }});
    }

    /**
     * Method to show/hide navigation bar.
     */
    public void setHideSystemBars(boolean theToHideStatusBar,
                                  boolean theToHideNavBar) {
        if(android.os.Build.VERSION.SDK_INT < 19) {
            return;
        }

        final boolean toHideStatusBar = theToHideStatusBar;
        final boolean toHideNavBar    = theToHideNavBar;
        this.runOnUiThread (new Runnable() { public void run() {
            if(myToHideStatusBar == toHideStatusBar
            && myToHideNavBar    == toHideNavBar) {
                return;
            }

            myToHideStatusBar = toHideStatusBar;
            myToHideNavBar    = toHideNavBar;
            updateHideSystemBars(toHideStatusBar, toHideNavBar);
        }});
    }

    /**
     * Auxiliary method to show temporary info message.
     */
    public void postToast(String theInfo) {
        final String aText = theInfo;
        this.runOnUiThread (new Runnable() { public void run() {
            Context aCtx   = getApplicationContext();
            Toast   aToast = Toast.makeText(aCtx, aText, Toast.LENGTH_SHORT);
            aToast.show();
        }});
    }

    /**
     * Auxiliary method to show info message.
     */
    public void postMessage(String theMessage) {
        final String  aText = theMessage;
        final Context aCtx  = this;
        this.runOnUiThread (new Runnable() { public void run() {
            AlertDialog.Builder aBuilder = new AlertDialog.Builder(aCtx);
            aBuilder.setMessage(aText).setNegativeButton("OK", null);
            AlertDialog aDialog = aBuilder.create();
            aDialog.show();
        }});
    }

    /**
     * Auxiliary method to close the activity.
     */
    public void postExit() {
        final Activity aCtx = this;
        this.runOnUiThread (new Runnable() { public void run() {
            aCtx.finish();
        }});
    }

//endregion

//region Methods to call C++ code

    /**
     * Setup open path.
     */
    private native void cppSetOpenPath(long theCppPtr,
                                       String theOpenPath,
                                       String theMimeType,
                                       boolean theIsLaunchedFromHistory);

    /**
     * Redirect back button to C++ level.
     */
    private native void cppOnBackPressed(long theCppPtr);

    /**
     * Called within surfaceChanged() call before passing to NativeActivity.
     */
    private native void cppOnBeforeSurfaceChanged(long theCppPtr,
                                                  boolean theIsBefore);

    /**
     * Define device orientation sensor.
     */
    private native void cppDefineOrientationSensor(long theCppPtr,
                                                   boolean theHasOri,
                                                   boolean theIsPoorOri);

    /**
     * Define device orientation by quaternion.
     */
    private native void cppSetQuaternion(long theCppPtr,
                                         float theX, float theY, float theZ, float theW,
                                         float theScreenRotDeg);

    /**
     * Define device orientation using deprecated Android API.
     */
    private native void cppSetOrientation(long theCppPtr,
                                          float theAzimuthDeg, float thePitchDeg, float theRollDeg,
                                          float theScreenRotDeg);

    /**
     * Define device Left/Right eyes swap flag.
     */
    private native void cppSetSwapEyes(long theCppPtr,
                                       boolean theToSwap);

    /**
     * Return TRUE if key is processed by application.
     */
    private native boolean cppIsKeyOverridden(long theCppPtr,
                                              int theKeyCode);

//endregion

    @SuppressWarnings("deprecation")
    protected static final int Sensor_TYPE_ORIENTATION_DEPRECATED = Sensor.TYPE_ORIENTATION;

//region class fields

    protected ContextWrapper myContext;
    protected android.os.PowerManager.WakeLock myWakeLock;

    protected SensorManager  mySensorMgr;
    protected Sensor         mySensorOri;
    protected float          myQuat[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    protected boolean        myIsPoorOri       = false;
    protected boolean        myToTrackOrient   = false;
    protected boolean        myToHideStatusBar = true;
    protected boolean        myToHideNavBar    = true;
    protected long           myCppGlue = 0; //!< pointer to c++ class StAndroidGlue instance

    protected StS3dvSurface  myS3dvSurf = null;
    protected boolean        myToEnableStereoHW = false;

//endregion

}
