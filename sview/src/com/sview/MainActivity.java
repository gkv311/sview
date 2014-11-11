/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */
package com.sview;

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

    private TextView       myTextView;
    private ContextWrapper myContext;

}
