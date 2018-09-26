package com.johnsoft.pcalla;

import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.view.Window;

public final class Finder {
    private static final String LOG_TAG = "Finder";

    @SuppressLint("PrivateApi")
    public static void init() {
        new Thread() {
            @SuppressWarnings("unchecked")
            @Override
            public void run() {
                try {
                    Thread.sleep(5000L);
                    String apk = "com.johnsoft.pcalldemo";
                    Activity activity = getTopActivity();
                    if (activity != null) {
                        Class klass = findClass(apk, "com.johnsoft.pcalldemo.SettingsActivity");
                        if (klass != null && klass.equals(activity.getClass())) {
                            Method setText = klass.getDeclaredMethod("setText", String.class);
                            setText.setAccessible(true);
                            setText.invoke(activity, "HHHHIIII");
                            final View view = findViewById(activity, apk, "ll");
                            if (view != null) {
                                view.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        view.setBackgroundColor(0xFF00FF0F);
                                    }
                                });
                            }
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                System.err.println("Finder.init() called");
            }
        }.start();
    }


    public static View findViewById(Activity activity, String packageName, String idName) {
        if (activity == null || idName == null) {
            return null;
        }
        int id = activity.getResources().getIdentifier(packageName + ":id/" + idName, null, null);
        Log.w(LOG_TAG, "id is === " + id);
        Window window = activity.getWindow();
        Log.w(LOG_TAG, "window is === " + window);
        View decorView = window.getDecorView();
        Log.w(LOG_TAG, "decorView is === " + decorView);
        return decorView.findViewById(id);
    }

    public static Activity getTopActivity() {
        if (!findActivityThread()) {
            return null;
        }
        try {
            Field activitiesField = activityThreadClass.getDeclaredField("mActivities");
            activitiesField.setAccessible(true);

            Map<Object, Object> activities = (Map<Object, Object>) activitiesField.get(activityThread);
            if (activities == null) {
                return null;
            }

            for (Object activityRecord : activities.values()) {
                Class activityRecordClass = activityRecord.getClass();
                Field pausedField = activityRecordClass.getDeclaredField("paused");
                pausedField.setAccessible(true);
                if (!pausedField.getBoolean(activityRecord)) {
                    Field activityField = activityRecordClass.getDeclaredField("activity");
                    activityField.setAccessible(true);
                    Activity activity = (Activity) activityField.get(activityRecord);
                    if (activity != null) {
                        Log.w(LOG_TAG, "Current top Activity is " + activity);
                    }
                    return activity;
                }
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }
        return null;
    }

    public static Class findClass(String appName, String className) throws ClassNotFoundException {
        if (saveAppClassLoader(appName)) {
            return appClassLoader.loadClass(className);
        }
        return null;
    }

    private static Class activityThreadClass;
    private static Object activityThread;
    private static ClassLoader appClassLoader;

    private static boolean findActivityThread() {
        if (activityThreadClass == null || activityThread == null) {
            synchronized (Finder.class) {
                if (activityThreadClass == null || activityThread == null) {
                    try {
                        activityThreadClass = Class.forName("android.app.ActivityThread");
                        Log.w(LOG_TAG, "Found class " + activityThreadClass);
                        activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
                        Log.w(LOG_TAG, "Found object " + activityThread);
                    } catch (Throwable e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return activityThreadClass != null && activityThread != null;
    }

    // same as Thread.currentThread().getContextClassLoader() normally
    private static boolean saveAppClassLoader(String packageName) {
        if (!findActivityThread()) {
            return false;
        }
        if (appClassLoader == null) {
            synchronized (Finder.class) {
                if (appClassLoader == null) {
                    try {
                        Field packagesField = activityThreadClass.getDeclaredField("mPackages");
                        packagesField.setAccessible(true);

                        Map<String, WeakReference<Object>> packages = (Map<String, WeakReference<Object>>) packagesField.get(activityThread);
                        if (packages == null) {
                            return false;
                        }
                        WeakReference<Object> loadedApkRef = packages.get(packageName);
                        if (loadedApkRef == null) {
                            return false;
                        }
                        Object loadedApk = loadedApkRef.get();
                        if (loadedApk == null) {
                            return false;
                        }

                        Class loadedApkClass = loadedApk.getClass();
                        Method getClassLoader = loadedApkClass.getDeclaredMethod("getClassLoader");
                        getClassLoader.setAccessible(true);
                        Object classLoader = getClassLoader.invoke(loadedApk);
                        if (classLoader != null) {
                            Log.w(LOG_TAG, packageName + ":LoadedApk getClassLoader()=" + classLoader);
                            appClassLoader = (ClassLoader) classLoader;
                        }
                    } catch (Throwable e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return appClassLoader != null;
    }
}
