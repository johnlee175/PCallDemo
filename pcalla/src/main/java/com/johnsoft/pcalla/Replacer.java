package com.johnsoft.pcalla;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import com.johnsoft.pcalldemo.SettingsActivity;

public class Replacer {
    public static String wrapGetString(String text) {
        System.err.println("Replacer found " + text);
        return "INI -- do it do it you know";
    }

    public static void wrapDoSomething(Object settingsActivity) {
        System.err.println("Replacer wrapDoSomething called ========= ======= " + settingsActivity);
    }

    public static void replaceColorIt(SettingsActivity settingsActivity) {
        try {
            Field field = settingsActivity.getClass().getDeclaredField("v2");
            field.setAccessible(true);
            Object textView = field.get(settingsActivity);
            Method method = textView.getClass().getMethod("setBackgroundColor", int.class);
            method.invoke(textView, 0xFFFF5091);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
