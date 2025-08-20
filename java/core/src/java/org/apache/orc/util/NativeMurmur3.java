package org.apache.orc.util;

public class NativeMurmur3 {

    private static final int DEFAULT_SEED = 104729;

    static {
        try {
            if (!"java".equalsIgnoreCase(System.getProperty("orc.util.murmur3"))) {
                if (!NativeResourceLoader.isLoaded("murmur3")) {
                    NativeResourceLoader.load("murmur3");
                }
                initNative();
            }
        } catch (UnsatisfiedLinkError | ExceptionInInitializerError e) {
            // ignore
        }
    }

    public static boolean isLoaded() {
        return NativeResourceLoader.isLoaded("murmur3");
    }

    public static native void initNative();

    /**
     * Murmur3 32-bit variant.
     *
     * @param data - input byte array
     * @return - hashcode
     */
    public static int hash32(byte[] data) {
        return hash32(data, data.length, DEFAULT_SEED);
    }

    /**
     * Murmur3 32-bit variant.
     *
     * @param data   - input byte array
     * @param length - length of array
     * @param seed   - seed. (default 0)
     * @return - hashcode
     */
    public static native int hash32(byte[] data, int length, int seed);

    /**
     * Murmur3 64-bit variant. This is essentially MSB 8 bytes of Murmur3 128-bit variant.
     *
     * @param data - input byte array
     * @return - hashcode
     */
    public static long hash64(byte[] data) {
        return hash64(data, 0, data.length, DEFAULT_SEED);
    }

    public static long hash64(byte[] data, int offset, int length) {
        return hash64(data, offset, length, DEFAULT_SEED);
    }

    /**
     * Murmur3 64-bit variant. This is essentially MSB 8 bytes of Murmur3 128-bit variant.
     *
     * @param data   - input byte array
     * @param length - length of array
     * @param seed   - seed. (default is 0)
     * @return - hashcode
     */
    public static native long hash64(byte[] data, int offset, int length, int seed);

    /**
     * Murmur3 128-bit variant.
     *
     * @param data - input byte array
     * @return - hashcode (2 longs)
     */
    public static long[] hash128(byte[] data) {
        return hash128(data, 0, data.length, DEFAULT_SEED);
    }

    /**
     * Murmur3 64-bit variant. This is essentially MSB 8 bytes of Murmur3 128-bit variant.
     *
     * @param data   - input byte array
     * @param length - length of array
     * @param seed   - seed. (default is 0)
     * @return - hashcode
     */
    public static native long[] hash128(byte[] data, int offset, int length, int seed);
}
