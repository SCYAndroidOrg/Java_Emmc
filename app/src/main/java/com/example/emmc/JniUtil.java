package com.example.emmc;

public class JniUtil {
    public static native String stringFromJNI();
    static {//加载so
        System.loadLibrary("JniHelper");
    }
}
