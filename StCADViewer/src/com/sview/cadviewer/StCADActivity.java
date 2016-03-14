/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2016
 */
package com.sview.cadviewer;

import android.os.Bundle;

/**
 * Customize StActivity
 */
public class StCADActivity extends com.sview.StActivity {

    /**
     * Virtual method defining StApplication class.
     */
    @Override
    public String getStAppClass() {
        return "cad";
    }

    /**
     * Create activity.
     */
    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        super.onCreate(theSavedInstanceState);
    }

}
