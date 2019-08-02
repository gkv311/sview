/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */
package com.sview;

import android.os.Bundle;

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
        super.onCreate(theSavedInstanceState);
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
            // this service (but the very existance) prevents system closing/suspending sView working threads playing audio in background
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
