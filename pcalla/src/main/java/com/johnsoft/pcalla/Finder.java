package com.johnsoft.pcalla;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import android.annotation.SuppressLint;
import android.app.Activity;

public final class Finder {
    @SuppressLint("PrivateApi")
    public static void init() {
        new Thread() {
            @Override
            public void run() {
                final ArrayList<Activity> list = new ArrayList<>();
                try {
                    Thread.sleep(5000L);
                    Class<?> klass = Class.forName("android.app.ActivityThread");
                    Method currentActivityThread = klass.getDeclaredMethod("currentActivityThread");
                    currentActivityThread.setAccessible(true);
                    Object activityThread = currentActivityThread.invoke(null);
                    Field mActivities = klass.getDeclaredField("mActivities");
                    mActivities.setAccessible(true);
                    Map map = (Map) mActivities.get(activityThread);
                    Collection activityClientRecords = map.values();
                    Field activity = null;
                    for (Object activityClientRecord : activityClientRecords) {
                        if (activity == null) {
                            klass = activityClientRecord.getClass();
                            activity = klass.getDeclaredField("activity");
                            activity.setAccessible(true);
                        }
                        list.add((Activity) activity.get(activityClientRecord));
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                System.err.println("Finder.init with: " + list.toString());
            }
        }.start();
    }
}
