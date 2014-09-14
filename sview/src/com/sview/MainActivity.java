/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */
package com.sview;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
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

        Intent anIntent  = getIntent();
        String aDataType = anIntent != null ? anIntent.getType() : "";
        Uri    aDataUrl  = anIntent != null ? anIntent.getData() : null;
        String aDataPath = aDataUrl != null ? aDataUrl.getEncodedPath() : "";

        TextView aTextView = new TextView(this);
        aTextView.setMovementMethod(new ScrollingMovementMethod());
        aTextView.setText("sView loader in progress...\n  URL: " + aDataPath + "\n  Type: " + aDataType);
        setContentView(aTextView);

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
        aTextView.append("\n\n" + anErrors);

        Intent anImgViewer = new Intent(this, StActivity.class);
        anImgViewer.setDataAndType(aDataUrl, aDataType);
        startActivity(anImgViewer);
    }

}
