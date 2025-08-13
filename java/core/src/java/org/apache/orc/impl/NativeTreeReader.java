package org.apache.orc.impl;

import org.apache.hadoop.hive.ql.exec.vector.ColumnVector;
import org.apache.orc.util.NativeResourceLoader;

public class NativeTreeReader {
    static {
        try {
            if (!"java".equalsIgnoreCase(System.getProperty("orc.nextVector.impl"))) {
                NativeResourceLoader.load("nextVector");
            }
        } catch (UnsatisfiedLinkError | ExceptionInInitializerError e) {
            // ignore
        }
    }

    public static boolean isLoaded() {
        return NativeResourceLoader.isLoaded("nextVector");
    }

    public static native boolean nextVector(ColumnVector previous, boolean[] isNull, int batchSize, BitFieldReader present);
}
