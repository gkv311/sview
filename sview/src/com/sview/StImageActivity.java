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
public class StImageActivity extends StActivity {

    /**
     * Virtual method defining StApplication class.
     */
    @Override
    public String getStAppClass() {
        return "image";
    }

    /**
     * Create activity.
     */
    @Override
    public void onCreate(Bundle theSavedInstanceState) {
        super.onCreate(theSavedInstanceState);
    }

}
