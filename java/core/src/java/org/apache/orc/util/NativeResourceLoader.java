package org.apache.orc.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class NativeResourceLoader {

    private static final Map<String, Boolean> LOADED_MAPS = new ConcurrentHashMap<>();

    private static String getOS() {
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("linux")) return "linux";
        if (os.contains("win")) return "win";
        if (os.contains("mac") || os.contains("darwin")) return "darwin";
        throw new UnsupportedOperationException("Unsupported OS: " + os);
    }

    private static String getArch() {
        String arch = System.getProperty("os.arch").toLowerCase();
        return switch (arch) {
            case "amd64", "x86_64" -> "x86_64";
            case "aarch64", "arm64" -> "aarch64";
            case "x86", "i386", "i686" -> "x86";
            default -> arch;
        };
    }

    private static String getExtension() {
        return switch (getOS()) {
            case "win" -> ".dll";
            case "darwin" -> ".dylib";
            default -> ".so";
        };
    }

    private static String getResourcePath(String libName) {
        String os = getOS();
        String arch = getArch();
        String fullName = (os.equals("win") ? libName : "lib" + libName) + getExtension();
        return "/native/" + os + "/" + arch + "/" + fullName;
    }

    public static synchronized boolean isLoaded(String libName) {
        Boolean isLoaded = LOADED_MAPS.get(libName);
        if (isLoaded == null) {
            return false;
        }
        return isLoaded;
    }

    private static void loadLibraryFile(final String libFileName) {
        System.load(libFileName);
    }

    public static synchronized void load(String libName) {
        if (LOADED_MAPS.getOrDefault(libName, false)) {
            return;
        }

        if (libName == null || libName.trim().isEmpty()) {
            throw new IllegalArgumentException("Native library name cannot be null or empty");
        }
        String resourcePath = getResourcePath(libName);

        InputStream in = NativeResourceLoader.class.getResourceAsStream(resourcePath);
        if (in == null) {
            throw new RuntimeException("Native library not found in classpath: " + resourcePath + "\n" +
                    "Please make sure the file exists in: src/main/resources" + resourcePath);
        }

        File tempLib = null;
        FileOutputStream out = null;
        try {
            tempLib = File.createTempFile("lib" + libName + getExtension(), null);
            tempLib.deleteOnExit();
            out = new FileOutputStream(tempLib);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
            try {
                out.flush();
                out.close();
                out = null;
            } catch (IOException e) {
                // ignore
            }
            try {
                loadLibraryFile(tempLib.getAbsolutePath());
            } catch (UnsatisfiedLinkError e) {
                // ignore
            }
            LOADED_MAPS.put(libName, true);
        } catch (IOException e) {
            // ignore
        } finally {
            try {
                in.close();
                if (out != null) {
                    out.close();
                }
                if (tempLib != null && tempLib.exists()) {
                    tempLib.delete();
                }
            } catch (IOException e) {
                // ignore
            }
        }
    }

}
