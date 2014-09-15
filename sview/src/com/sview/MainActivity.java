/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */
package com.sview;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.text.Html;
import android.text.method.ScrollingMovementMethod;
import android.widget.TextView;
import android.widget.Toast;
import android.net.Uri;
import android.os.Bundle;

/**
 * sView launcher.
 */
public class MainActivity extends Activity {

    /**
     * Auxiliary method to close activity on critical error
     */
    public void exitWithError(String theError) {
        AlertDialog.Builder aBuilder = new AlertDialog.Builder(this);
        aBuilder.setMessage(theError).setNegativeButton("Exit", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface theDialog, int theId) {
                Intent anIntent = new Intent(Intent.ACTION_MAIN);
                anIntent.addCategory(Intent.CATEGORY_HOME);
                anIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                anIntent.putExtra("EXIT", true);
                startActivity(anIntent);
            }
        });
        AlertDialog aDialog = aBuilder.create();
        aDialog.show();
    }

    /**
     * Auxiliary method to show temporary info messages
     */
    public void printShortInfo(CharSequence theInfo) {
        Context aCtx = getApplicationContext();
        Toast aToast = Toast.makeText(aCtx, theInfo, Toast.LENGTH_SHORT);
        aToast.show();
    }

    /**
     * Auxiliary method to show temporary info messages
     */
    private boolean loadLibVerbose(String        theLibName,
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
            theErrors.append("\" is unavailable:\n  " + theError.getMessage());
            return false;
	    } catch(SecurityException theError) {
            theErrors.append("Error: native library \"");
            theErrors.append(theLibName);
            theErrors.append("\" can not be loaded for security reasons:\n  " + theError.getMessage());
            return false;
	    }
    }

    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        super.onCreate(theSavedInstanceState);

        // create folder for external storage
        myContext = new ContextWrapper(this);
        myContext.getExternalFilesDir(null);

        Intent anIntent  = getIntent();
        String aDataType = anIntent != null ? anIntent.getType() : "";
        Uri    aDataUrl  = anIntent != null ? anIntent.getData() : null;
        String aDataPath = aDataUrl != null ? aDataUrl.getEncodedPath() : "";

        myTextView = new TextView(this);
        myTextView.setMovementMethod(new ScrollingMovementMethod());
        myTextView.setText("sView loader in progress...\n  URL: " + aDataPath + "\n  Type: " + aDataType);
        setContentView(myTextView);

        StringBuilder anErrors = new StringBuilder();
        if(!loadLibVerbose("gnustl_shared",   anErrors)
        || !loadLibVerbose("freetype",        anErrors)
        || !loadLibVerbose("avutil-54",       anErrors)
        || !loadLibVerbose("swresample-1",    anErrors)
        || !loadLibVerbose("avcodec-56",      anErrors)
        || !loadLibVerbose("avformat-56",     anErrors)
        || !loadLibVerbose("swscale-3",       anErrors)
        || !loadLibVerbose("openal",          anErrors)
        || !loadLibVerbose("StShared",        anErrors)
        || !loadLibVerbose("StGLWidgets",     anErrors)
        || !loadLibVerbose("StSettings",      anErrors)
        || !loadLibVerbose("StCore",          anErrors)
        || !loadLibVerbose("StOutAnaglyph",   anErrors)
        || !loadLibVerbose("StOutDistorted",  anErrors)
        //|| !loadLibVerbose("StOutDual",       anErrors)
        || !loadLibVerbose("StOutInterlace",  anErrors)
        //|| !loadLibVerbose("StOutIZ3D",       anErrors)
        //|| !loadLibVerbose("StOutPageFlip",   anErrors)
        || !loadLibVerbose("StImageViewer",   anErrors)
        || !loadLibVerbose("StMoviePlayer",   anErrors)
        || !loadLibVerbose("sview",           anErrors)) {
	        exitWithError("Broken apk?\n" + anErrors);
	        return;
        }
        myTextView.append("\n\n" + anErrors);

        Intent anImgViewer = new Intent(this, StActivity.class);
        anImgViewer.setDataAndType(aDataUrl, aDataType);
        startActivityForResult(anImgViewer, 0);
    }

    /**
     * Handle StActivity exit.
     */
    protected void onActivityResult(int    theRequestCode,
                                    int    theResultCode,
                                    Intent theData) {
        myTextView.append("\n\nStActivity has stopped. Log:\n");
        try {
            File anExtFolder = myContext.getExternalFilesDir(null);
            File aLogFile    = null;
            if(anExtFolder != null) {
                aLogFile = new File(anExtFolder, "sview.log");
            }
            if(aLogFile != null && aLogFile.canRead()) {
                FileInputStream aLogStream = new FileInputStream(aLogFile);
                BufferedReader  aLogReader = new BufferedReader(new InputStreamReader(aLogStream));
                for(String aLine = aLogReader.readLine(); aLine != null; aLine = aLogReader.readLine()) {
                    if(aLine.startsWith("ERROR !!")) {
                        myTextView.append(Html.fromHtml("<font color=\"#ff0000\">ERROR !!</font>"));
                        myTextView.append(aLine.substring(8));
                    } else if(aLine.startsWith("DEBUG --")) {
                        myTextView.append(Html.fromHtml("<font color=\"#d4aa00\">DEBUG --</font>"));
                        myTextView.append(aLine.substring(8));
                    } else {
                        myTextView.append(aLine);
                    }
                    myTextView.append("\n");
                }
                aLogReader.close();
            }
        } catch(IOException theError) {
            theError.printStackTrace();
        }

        // scroll to the bottom
        myTextView.post(new Runnable() {
            @Override
            public void run() {
                final int aTop     = (int )myTextView.getTextSize() + myTextView.getHeight();
                final int aScrollY = myTextView.getLineBounds(myTextView.getLineCount() - 1, null) - aTop;
                myTextView.scrollTo(0, aScrollY);
            }
        });
    }

    private TextView       myTextView;
    private ContextWrapper myContext;

}
