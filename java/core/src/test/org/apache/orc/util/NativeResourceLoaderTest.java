package org.apache.orc.util;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class NativeResourceLoaderTest {

    @Test
    void should_load_jni() {
        NativeResourceLoader.load("nextVector");
        assertTrue(NativeResourceLoader.isLoaded("nextVector"));
        assertFalse(NativeResourceLoader.isLoaded("fakeNextVector"));
    }
}