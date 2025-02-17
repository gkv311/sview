/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */
package com.sview;

import android.os.Bundle;
import android.system.Os;

/**
 * Customize StActivity
 */
public class StMovieActivity extends StActivity {

    /**
     * Global reference to activity for managing by background service.
     */
    public static StMovieActivity backgroundActivity;

    /**
     * Virtual method defining StApplication class.
     */
    @Override
    public String getStAppClass() {
        return "video";
    }

    /**
     * Return current title.
     */
    public String getCurrentTitle() {
        return myCurrentTitle;
    }

    /**
     * Create activity.
     */
    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        configureAlSoft();
        super.onCreate(theSavedInstanceState);
    }

    /** Configure OpenAL soft. */
    private void configureAlSoft() {
        if (android.os.Build.VERSION.SDK_INT < 21) {
            return;
        }

        // Configure OpenAL Soft environment variables
        // at Java level before loading native libraries.
        //setenv("ALSOFT_LOGLEVEL", "3");

        String aRootInt = "/data/data/";
        String aRootExt = "/sdcard/Android/data/";
        java.io.File aRootExtF = new java.io.File(aRootExt);
        String aRoot = aRootExtF.isDirectory() ? aRootExt : aRootInt;

        String aPkgName = getApplicationContext().getPackageName();
        String aPkgRoot = aRoot + aPkgName;
        String aConfDirs = aPkgRoot + "/shared_prefs";
        // > $XDG_CONFIG_DIRS/alsoft.conf
        setenv("XDG_CONFIG_DIRS", aConfDirs);
        {
            java.io.File aDir = new java.io.File(aConfDirs);
            if (!aDir.exists()) {
                aDir.mkdir();
            }
        }

        String anAlConf = aConfDirs + "/alsoft.conf";
        // $ALSOFT_CONF could be set instead of $XDG_CONFIG_DIRS
        //setenv("ALSOFT_CONF", anAlConf);

        String aDefCfg =
            "# Comma-separated list of folders containing HRTF data sets (.mhr files)\n"
           + "hrtf-paths = " + aPkgRoot + "/files\n"
           + "# Default HRTF to use\n"
           + "#default-hrtf = Built-In HRTF\n";
        writeTextFile(anAlConf, aDefCfg, false);

        // $XDG_DATA_DIRS could be set instead of 'hrtf-paths' in 'alsoft.conf'.
        // > $XDG_DATA_DIRS/openal/hrtf
        //setenv("XDG_DATA_DIRS", aConfDirs);
    }

    /** Setup environment variable. */
    private static boolean setenv(String theName, String theVal) {
        try {
            android.system.Os.setenv(theName, theVal, true);
            return true;
        } catch(android.system.ErrnoException theError) {
            return false;
        }
    }

    /** Write text file. */
    private static boolean writeTextFile(String thePath, String theText, boolean theToOverwrite) {
        try {
            java.io.File aFile = new java.io.File(thePath);
            if (!theToOverwrite && aFile.exists()) {
                return false;
            }
            java.io.FileOutputStream   aStream = new java.io.FileOutputStream(aFile);
            java.io.OutputStreamWriter aWriter = new java.io.OutputStreamWriter(aStream, "UTF-8");
            aWriter.write(theText);
            aWriter.close();
        } catch (java.io.IOException theExc) {
            return false;
        }
        return true;
    }

    /**
     * Keep CPU on.
     */
    @Override
    public void setPartialWakeLockOn(String theTitle, boolean theToLock) {
        if(myWakeLock == null) {
            return;
        }

        super.setPartialWakeLockOn(theTitle, theToLock);
        if(android.os.Build.VERSION.SDK_INT < 24) {
            return;
        }

        final String  aTitle = theTitle;
        final boolean toLock = theToLock;
        final StMovieActivity aThis = this;
        this.runOnUiThread(new Runnable() { public void run() {
            // start a dummy foreground service, which actually does nothing but shows a system notification;
            // this service (but the very existence) prevents system closing/suspending sView working threads playing audio in background
            myCurrentTitle = aTitle;
            android.content.Intent anIntent = new android.content.Intent(aThis, StMovieService.class);
            if(toLock) {
                backgroundActivity = aThis;
                anIntent.setAction(toLock ? StMovieService.THE_ACTION_START_SERVICE : StMovieService.THE_ACTION_STOP_SERVICE);
                if(android.os.Build.VERSION.SDK_INT >= 26) {
                    startForegroundService(anIntent);
                } else {
                    startService(anIntent);
                }
            } else {
                backgroundActivity = null;
                stopService(anIntent);
            }
        }});
    }

    private String myCurrentTitle;

}
