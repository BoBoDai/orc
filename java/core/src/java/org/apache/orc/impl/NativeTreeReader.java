package org.apache.orc.impl;

import org.apache.orc.util.NativeResourceLoader;

public class NativeTreeReader {
    static {
        try {
            if (!"java".equalsIgnoreCase(System.getProperty("orc.nextVector.impl"))) {
                if (!isLoaded()) {
                    NativeResourceLoader.load("nextVector3");
                }
            }
        } catch (UnsatisfiedLinkError | ExceptionInInitializerError e) {
            // ignore
        }
    }

    public static boolean isLoaded() {
        return NativeResourceLoader.isLoaded("nextVector3");
    }

    public static native int nextVector(
            boolean[] isNullTarget,
            boolean[] isNullSource,
            byte[] presentBuffer,
            int batchSize,
            boolean initialHasNull,
            boolean initialAllNull
    );
}
