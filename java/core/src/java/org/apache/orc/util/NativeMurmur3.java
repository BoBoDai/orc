/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.orc.util;

/**
 * Murmur3 is successor to Murmur2 fast non-crytographic hash algorithms.
 * <p>
 * Murmur3 32 and 128 bit variants.
 * 32-bit Java port of https://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp#94
 * 128-bit Java port of https://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp#255
 * <p>
 * This is a public domain code with no copyrights.
 * From homepage of MurmurHash (https://code.google.com/p/smhasher/),
 * "All MurmurHash versions are public domain software, and the author disclaims all copyright
 * to their code."
 */
public class NativeMurmur3 {

    private static final int DEFAULT_SEED = 104729;

    static {
        try {
            if (!"java".equalsIgnoreCase(System.getProperty("orc.util.murmur3"))) {
                if (!NativeResourceLoader.isLoaded("murmur3")) {
                    NativeResourceLoader.load("murmur3");
                }
            }
        } catch (UnsatisfiedLinkError | ExceptionInInitializerError e) {
            // ignore
        }
    }

    public static boolean isLoaded() {
        return NativeResourceLoader.isLoaded("murmur3");
    }

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
