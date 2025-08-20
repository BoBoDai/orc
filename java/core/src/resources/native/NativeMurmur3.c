#include <jni.h>
#include <stdint.h>
#include "NativeMurmur3.h"

// 全局缓存
static jclass NativeMurmur3_class = NULL;

// 初始化方法：预注册方法 ID 和类引用
JNIEXPORT void JNICALL Java_org_apache_orc_util_NativeMurmur3_initNative
  (JNIEnv *env, jclass clazz) {

    NativeMurmur3_class = (*env)->NewGlobalRef(env, clazz);
}

#define C1_32 0xcc9e2d51
#define C2_32 0x1b873593
#define M_32  5
#define N_32  0xe6546b64
#define R1_32 15
#define R2_32 13

#define C1 0x87c37b91114253d5ULL
#define C2 0x4cf5ad432745937fULL
#define R1 31
#define R2 27
#define R3 33
#define M 5
#define N1 0x52dce729ULL
#define N2 0x38495ab5ULL
#define DEFAULT_SEED 104729
#define ROTL32(x, r) ((x << r) | (x >> (32 - r)))

// 64位循环左移
static inline uint64_t rotl64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

// fmix64: 最终混合函数
static inline uint64_t fmix64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

JNIEXPORT jint JNICALL Java_org_apache_orc_util_NativeMurmur3_hash32 (JNIEnv *env, jclass cls, jbyteArray data, jint length, jint seed) {
    if (data == NULL || length < 0) {
        return 0;
    }

    jbyte *bytes = (*env)->GetByteArrayElements(env, data, NULL);
    if (bytes == NULL) {
        return 0; // 内存分配失败
    }

    uint32_t hash = (uint32_t)seed;
    const int nblocks = length >> 2;

    for (int i = 0; i < nblocks; i++) {
        uint32_t i_4 = i << 2;
        uint32_t k = (uint32_t)(bytes[i*4 + 0] & 0xFF)
                | ((uint32_t)(bytes[i*4 + 1] & 0xFF) << 8)
                | ((uint32_t)(bytes[i*4 + 2] & 0xFF) << 16)
                | ((uint32_t)(bytes[i*4 + 3] & 0xFF) << 24);
        k *= C1_32;
        k = ROTL32(k, R1_32);
        k *= C2_32;
        hash ^= k;
        hash = ROTL32(hash, R2_32) * M_32 + N_32;
    }

    const int tail_start = nblocks * 4;
    uint32_t k1 = 0;
    switch (length - tail_start) {
        case 3:
            k1 ^= (uint32_t)(bytes[tail_start + 2]) << 16;
        case 2:
            k1 ^= (uint32_t)(bytes[tail_start + 1]) << 8;
        case 1:
            k1 ^= (uint32_t)(bytes[tail_start]);
            k1 *= C1_32;
            k1 = ROTL32(k1, R1_32);
            k1 *= C2_32;
            hash ^= k1;
            break;
        case 0:
            break;
    }

    hash ^= (uint32_t)length;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6bU;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35U;
    hash ^= (hash >> 16);

    (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);

    return (jint)hash;
}

JNIEXPORT jlong JNICALL Java_org_apache_orc_util_NativeMurmur3_hash64 (JNIEnv *env, jclass cls, jbyteArray data, jint offset, jint length, jint seed) {
        if (data == NULL || length < 0) {
            return 0;
        }
        jbyte *bytes = (*env)->GetByteArrayElements(env, data, NULL);
        if (bytes == NULL) {
            return 0L;
        }

        uint64_t hash = (uint64_t)seed;
        const int nblocks = length >> 3;

        // 处理完整的 8 字节块
        for (int i = 0; i < nblocks; i++) {
            const int i8 = i << 3;
            const int pos = offset + i8;

            uint64_t k = (uint64_t)(bytes[pos + 0] & 0xFF)
                       | ((uint64_t)(bytes[pos + 1] & 0xFF) << 8)
                       | ((uint64_t)(bytes[pos + 2] & 0xFF) << 16)
                       | ((uint64_t)(bytes[pos + 3] & 0xFF) << 24)
                       | ((uint64_t)(bytes[pos + 4] & 0xFF) << 32)
                       | ((uint64_t)(bytes[pos + 5] & 0xFF) << 40)
                       | ((uint64_t)(bytes[pos + 6] & 0xFF) << 48)
                       | ((uint64_t)(bytes[pos + 7] & 0xFF) << 56);

            k *= C1;
            k = rotl64(k, R1);
            k *= C2;
            hash ^= k;
            hash = rotl64(hash, R2) * M + N1;
        }

        uint64_t k1 = 0;
        const int tailStart = nblocks << 3;
        const int tailPos = offset + tailStart;
        const int tailLen = length - tailStart;

        switch (tailLen) {
            case 7:
                k1 ^= (uint64_t)(bytes[tailPos + 6] & 0xFF) << 48;
                // fall through
            case 6:
                k1 ^= (uint64_t)(bytes[tailPos + 5] & 0xFF) << 40;
                // fall through
            case 5:
                k1 ^= (uint64_t)(bytes[tailPos + 4] & 0xFF) << 32;
                // fall through
            case 4:
                k1 ^= (uint64_t)(bytes[tailPos + 3] & 0xFF) << 24;
                // fall through
            case 3:
                k1 ^= (uint64_t)(bytes[tailPos + 2] & 0xFF) << 16;
                // fall through
            case 2:
                k1 ^= (uint64_t)(bytes[tailPos + 1] & 0xFF) << 8;
                // fall through
            case 1:
                k1 ^= (uint64_t)(bytes[tailPos + 0] & 0xFF);
                k1 *= C1;
                k1 = rotl64(k1, R1);
                k1 *= C2;
                hash ^= k1;
                break;
            case 0:
                break;
        }

        hash ^= (uint64_t)length;
        hash = fmix64(hash);

        (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);

        return (jlong)hash;
}

JNIEXPORT jlongArray JNICALL Java_org_apache_orc_util_NativeMurmur3_hash128(JNIEnv *env, jclass cls, jbyteArray data, jint offset, jint length, jint seed) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, data, NULL);
    if (bytes == NULL) {
        return NULL; // 内存错误
    }

    uint64_t h1 = (uint64_t)seed;
    uint64_t h2 = (uint64_t)seed;

    const int nblocks = length >> 4;

    // 处理 16 字节块
    for (int i = 0; i < nblocks; i++) {
        const int i16 = i << 4;
        const int pos = offset + i16;

        uint64_t k1 = (uint64_t)(bytes[pos + 0] & 0xFF)
                    | ((uint64_t)(bytes[pos + 1] & 0xFF) << 8)
                    | ((uint64_t)(bytes[pos + 2] & 0xFF) << 16)
                    | ((uint64_t)(bytes[pos + 3] & 0xFF) << 24)
                    | ((uint64_t)(bytes[pos + 4] & 0xFF) << 32)
                    | ((uint64_t)(bytes[pos + 5] & 0xFF) << 40)
                    | ((uint64_t)(bytes[pos + 6] & 0xFF) << 48)
                    | ((uint64_t)(bytes[pos + 7] & 0xFF) << 56);

        uint64_t k2 = (uint64_t)(bytes[pos + 8] & 0xFF)
                    | ((uint64_t)(bytes[pos + 9] & 0xFF) << 8)
                    | ((uint64_t)(bytes[pos + 10] & 0xFF) << 16)
                    | ((uint64_t)(bytes[pos + 11] & 0xFF) << 24)
                    | ((uint64_t)(bytes[pos + 12] & 0xFF) << 32)
                    | ((uint64_t)(bytes[pos + 13] & 0xFF) << 40)
                    | ((uint64_t)(bytes[pos + 14] & 0xFF) << 48)
                    | ((uint64_t)(bytes[pos + 15] & 0xFF) << 56);

        k1 *= C1;
        k1 = rotl64(k1, R1);
        k1 *= C2;
        h1 ^= k1;
        h1 = rotl64(h1, R2);
        h1 += h2;
        h1 = h1 * M + N1;

        // Mix k2
        k2 *= C2;
        k2 = rotl64(k2, R3);
        k2 *= C1;
        h2 ^= k2;
        h2 = rotl64(h2, R1);
        h2 += h1;
        h2 = h2 * M + N2;
    }

    const int tailStart = nblocks << 4;
    const int tailPos = offset + tailStart;
    const int tailLen = length - tailStart;

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch (tailLen) {
        case 15:
            k2 ^= (uint64_t)(bytes[tailPos + 14] & 0xFF) << 48;
        case 14:
            k2 ^= (uint64_t)(bytes[tailPos + 13] & 0xFF) << 40;
        case 13:
            k2 ^= (uint64_t)(bytes[tailPos + 12] & 0xFF) << 32;
        case 12:
            k2 ^= (uint64_t)(bytes[tailPos + 11] & 0xFF) << 24;
        case 11:
            k2 ^= (uint64_t)(bytes[tailPos + 10] & 0xFF) << 16;
        case 10:
            k2 ^= (uint64_t)(bytes[tailPos + 9] & 0xFF) << 8;
        case 9:
            k2 ^= (uint64_t)(bytes[tailPos + 8] & 0xFF);
            k2 *= C2;
            k2 = rotl64(k2, R3);
            k2 *= C1;
            h2 ^= k2;

        case 8:
            k1 ^= (uint64_t)(bytes[tailPos + 7] & 0xFF) << 56;
        case 7:
            k1 ^= (uint64_t)(bytes[tailPos + 6] & 0xFF) << 48;
        case 6:
            k1 ^= (uint64_t)(bytes[tailPos + 5] & 0xFF) << 40;
        case 5:
            k1 ^= (uint64_t)(bytes[tailPos + 4] & 0xFF) << 32;
        case 4:
            k1 ^= (uint64_t)(bytes[tailPos + 3] & 0xFF) << 24;
        case 3:
            k1 ^= (uint64_t)(bytes[tailPos + 2] & 0xFF) << 16;
        case 2:
            k1 ^= (uint64_t)(bytes[tailPos + 1] & 0xFF) << 8;
        case 1:
            k1 ^= (uint64_t)(bytes[tailPos + 0] & 0xFF);
            k1 *= C1;
            k1 = rotl64(k1, R1);
            k1 *= C2;
            h1 ^= k1;
            break;
        case 0:
            break;
    }

    // Finalization
    h1 ^= (uint64_t)length;
    h2 ^= (uint64_t)length;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);

    jlongArray result = (*env)->NewLongArray(env, 2);
    if (result == NULL) {
        return NULL;
    }

    jlong hash[2] = { (jlong)h1, (jlong)h2 };
    (*env)->SetLongArrayRegion(env, result, 0, 2, hash);

    return result;
}

// 卸载时释放全局引用
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) return;

    if (NativeMurmur3_class) {
        (*env)->DeleteGlobalRef(env, NativeMurmur3_class);
        NativeMurmur3_class = NULL;
    }
}