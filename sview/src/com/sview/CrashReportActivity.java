/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2017
 */
package com.sview;

import android.app.Activity;
import android.content.ContextWrapper;
import android.content.Intent;
import android.text.method.ScrollingMovementMethod;
import android.widget.TextView;
import android.os.Bundle;

/**
 * sView Crash Report activity.
 */
public class CrashReportActivity extends Activity {

	public static final String THE_ERROR_REPORT_ID = "ERROR_REPORT";

    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        super.onCreate(theSavedInstanceState);

        // create folder for external storage
        myContext = new ContextWrapper(this);
        myContext.getExternalFilesDir(null);

        Intent anIntent = getIntent();
        Bundle anExtras = anIntent != null ? getIntent().getExtras() : null;
        String anErrorText = "";
        if(anExtras != null) {
            anErrorText = anExtras.getString(THE_ERROR_REPORT_ID);
        }

        myTextView = new TextView(this);
        myTextView.setMovementMethod(new ScrollingMovementMethod());
        myTextView.setText("sView Crash Report: " + anErrorText);
        setContentView(myTextView);
    }

    private TextView       myTextView;
    private ContextWrapper myContext;

}
