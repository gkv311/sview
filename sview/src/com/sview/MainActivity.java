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
import android.content.ContextWrapper;
import android.content.Intent;
import android.text.method.ScrollingMovementMethod;
import android.widget.TextView;
import android.net.Uri;
import android.os.Bundle;

/**
 * sView launcher.
 */
public class MainActivity extends Activity {

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

        StringBuilder anInfo = new StringBuilder();
        if(!StActivity.loadNatives(this, anInfo)) {
            //StActivity.exitWithError(this, "Broken apk?\n" + anInfo);
            return;
        }
        myTextView.append("\n\n" + anInfo);

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
                    /*if(aLine.startsWith("ERROR !!")) {
                        myTextView.append(Html.fromHtml("<font color=\"#ff0000\">ERROR !!</font>"));
                        myTextView.append(aLine.substring(8));
                    } else if(aLine.startsWith("DEBUG --")) {
                        myTextView.append(Html.fromHtml("<font color=\"#d4aa00\">DEBUG --</font>"));
                        myTextView.append(aLine.substring(8));
                    } else*/ {
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
