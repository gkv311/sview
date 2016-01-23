/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2016
 */
package com.sview;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.view.SurfaceHolder;

/**
 * Wrapping class.
 */
public class StS3dvSurface {

    public int S3DV_MODE_MONO             = 0;
    public int S3DV_MODE_PARALLEL_PAIR    = 1;
    public int S3DV_MODE_COLUMN_INTERLACE = 3;

    public boolean isValid() { return mySurf != null; }

    /**
     * Main constructor.
     */
    public StS3dvSurface(SurfaceHolder theHolder) {
        ClassLoader aClassLoader = StS3dvSurface.class.getClassLoader();
        try {
            Class<?> anS3dvClass = aClassLoader.loadClass("com.s3dv.s3dvsurface.S3DVSurface");
            Constructor<?> anS3dvConstr = anS3dvClass.getConstructor(SurfaceHolder.class);
            mySetV3DType = anS3dvClass.getMethod("SetV3DType", int.class, int.class, int.class);
            mySurf       = anS3dvConstr.newInstance(theHolder);
        } catch(ClassNotFoundException theEx) {
            theEx.printStackTrace();
        } catch(NoSuchMethodException theEx) {
            theEx.printStackTrace();
        } catch(IllegalArgumentException theEx) {
            theEx.printStackTrace();
        } catch(InstantiationException theEx) {
            theEx.printStackTrace();
        } catch(IllegalAccessException theEx) {
            theEx.printStackTrace();
        } catch(InvocationTargetException theEx) {
            theEx.printStackTrace();
        }
    }

    /**
     * Setup stereo input.
     */
    void setStereo(boolean theIsOn) {
        if(mySurf == null) {
            return;
        }

        try {
            if(theIsOn) {
                mySetV3DType.invoke(mySurf, S3DV_MODE_PARALLEL_PAIR, S3DV_MODE_COLUMN_INTERLACE, 0);
                //mySetV3DType.invoke(mySurf, S3DV_MODE_COLUMN_INTERLACE, S3DV_MODE_COLUMN_INTERLACE, 0);
            } else {
                mySetV3DType.invoke(mySurf, S3DV_MODE_MONO, S3DV_MODE_MONO, 0);
            }
        } catch(IllegalArgumentException theEx) {
            theEx.printStackTrace();
        } catch(IllegalAccessException theEx) {
            theEx.printStackTrace();
        } catch(InvocationTargetException theEx) {
            theEx.printStackTrace();
        }
    }

    Object mySurf       = null;
    Method mySetV3DType = null;

}
