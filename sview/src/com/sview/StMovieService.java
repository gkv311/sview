/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2019
 */
package com.sview;

import android.os.Bundle;

/**
 * Dummy foreground service for playing audio in background.
 * Does nothing but shows a notification.
 */
public class StMovieService extends android.app.Service  {

    /**
     * Action to start service.
     */
    public static final String THE_ACTION_START_SERVICE = "ACTION_START_SERVICE";

    /**
     * Action to stop service.
     */
    public static final String THE_ACTION_STOP_SERVICE = "ACTION_STOP_SERVICE";

    /**
     * Custom channel ID.
     */
    private final String THE_SVIEW_AUDIO_CHANNEL_ID = "com.sview.audio_channel";

    /**
     * Empty constructor.
     */
    public StMovieService() {
        //
    }

    /**
     * Create service.
     */
    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        // should be started within several seconds, or will be killed by system
        startForegroundService();
    }

    /**
     * Destroy service.
     */
    @Override
    public void onDestroy() {
        //
    }

    /**
     * Create binding.
     */
    @Override
    public android.os.IBinder onBind(android.content.Intent theIntent) {
        // we don't provide binding, so return null
        return null;
    }

    /**
     * Start a command.
     */
    @Override
    public int onStartCommand(android.content.Intent theIntent, int theFlags, int theStartId) {
        String anAction = theIntent != null ? theIntent.getAction() : THE_ACTION_STOP_SERVICE;
        //android.widget.Toast.makeText(this, " @@ " + anAction, android.widget.Toast.LENGTH_LONG).show();
        switch(anAction) {
            case THE_ACTION_START_SERVICE: {
                // if we get killed, after returning from here, restart
                startForegroundService();
                return START_STICKY;
            }
            case THE_ACTION_STOP_SERVICE: {
                stopForegroundService();
                return START_NOT_STICKY;
            }
            case StActivity.THE_ACTION_PLAY_PREV:
            case StActivity.THE_ACTION_PLAY_NEXT:
            case StActivity.THE_ACTION_PLAY_PAUSE: {
                //android.widget.Toast.makeText(this, " $$ " + anAction, android.widget.Toast.LENGTH_LONG).show();
                final StMovieActivity aMainActivity = StMovieActivity.backgroundActivity;
                if(aMainActivity != null) {
                    aMainActivity.setPlaybackAction(theIntent);
                }
                return START_NOT_STICKY;
            }
        }
        return START_NOT_STICKY;
    }

    /**
     * Start foreground service - create notification.
     */
    private void startForegroundService() {
        final StMovieActivity aMainActivity = StMovieActivity.backgroundActivity;
        String anActiveItem = "Audio playback";
        if(aMainActivity != null) {
            anActiveItem = aMainActivity.getCurrentTitle();
        }

        android.content.Intent anIntent = new android.content.Intent(this, StMovieActivity.class);
        android.app.PendingIntent aPendingIntent = android.app.PendingIntent.getActivity(this, 0, anIntent, 0);

        android.app.Notification.Builder aNoti;
        if(android.os.Build.VERSION.SDK_INT >= 26) {
            aNoti = new android.app.Notification.Builder(this, THE_SVIEW_AUDIO_CHANNEL_ID);
        } else {
            aNoti = new android.app.Notification.Builder(this);
        }
        aNoti.setPriority(android.app.Notification.PRIORITY_LOW); // Android 8.0+ will use Notification Channel importance instead
        //aNoti.setAutoCancel(true);
        aNoti.setContentIntent(aPendingIntent); // intent on tapping notification
        aNoti.setContentTitle(anActiveItem);
        //aNoti.setContentText("");
        aNoti.setSmallIcon(com.sview.R.drawable.ic_media_play);
        //aNoti.setLargeIcon(aBitmap);

        // add buttons (note that Android 7.0+ will NOT show action icons!)
        {
            android.content.Intent aPlayIntent = new android.content.Intent(this, StMovieService.class);
            aPlayIntent.setAction(StActivity.THE_ACTION_PLAY_PREV);
            android.app.PendingIntent aPendingPlayIntent = android.app.PendingIntent.getService(this, 0, aPlayIntent, 0);
            aNoti.addAction(new android.app.Notification.Action(android.R.drawable.ic_media_previous,
                                                                android.os.Build.VERSION.SDK_INT >= 24
                                                              ? "    \u23EE    "
                                                              : "PREV",
                                                                aPendingPlayIntent));
        }
        {
            android.content.Intent aPlayIntent = new android.content.Intent(this, StMovieService.class);
            aPlayIntent.setAction(StActivity.THE_ACTION_PLAY_PAUSE);
            android.app.PendingIntent aPendingPlayIntent = android.app.PendingIntent.getService(this, 0, aPlayIntent, 0);
            aNoti.addAction(new android.app.Notification.Action(android.R.drawable.ic_media_pause,
                                                                android.os.Build.VERSION.SDK_INT >= 24
                                                              ? "    \u23F9    " // "    \u23F8    "
                                                              : "PAUSE", aPendingPlayIntent));
        }
        {
            android.content.Intent aPlayIntent = new android.content.Intent(this, StMovieService.class);
            aPlayIntent.setAction(StActivity.THE_ACTION_PLAY_NEXT);
            android.app.PendingIntent aPendingPlayIntent = android.app.PendingIntent.getService(this, 0, aPlayIntent, 0);
            aNoti.addAction(new android.app.Notification.Action(android.R.drawable.ic_media_next,
                                                                android.os.Build.VERSION.SDK_INT >= 24
                                                              ? "    \u23ED    "
                                                              : "NEXT",
                                                                aPendingPlayIntent));
        }
        final int aNotifId = 1; // unique ID, should not be 0
        startForeground(aNotifId, aNoti.build());
    }

    /**
     * Stop foreground service and remove the notification.
     */
    private void stopForegroundService() {
        stopForeground(true);
        stopSelf();
    }

    /**
     * Create the NotificationChannel (adjust settings of the channel).
     */
    private void createNotificationChannel() {
        if(android.os.Build.VERSION.SDK_INT < 26) {
            return;
        }

        android.app.NotificationChannel aChannel = new android.app.NotificationChannel(THE_SVIEW_AUDIO_CHANNEL_ID,
                                                                                       THE_SVIEW_AUDIO_CHANNEL_ID,
                                                                                       android.app.NotificationManager.IMPORTANCE_LOW);
        aChannel.enableLights(false);
        aChannel.enableVibration(false);
        aChannel.setSound(null, null);
        android.app.NotificationManager aNotifMgr = getSystemService(android.app.NotificationManager.class);
        aNotifMgr.createNotificationChannel(aChannel);
    }

}
