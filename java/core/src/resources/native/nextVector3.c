#include <jni.h>
#include <string.h>
#include "nextVector.h"

JNIEXPORT jint JNICALL Java_org_apache_orc_impl_NativeTreeReader_nextVector(
    JNIEnv* env,
    jclass cls,
    jbooleanArray isNullTarget,
    jbooleanArray isNullSource,
    jbyteArray presentBuffer,
    jint batchSize,
    jboolean initialHasNull,
    jboolean initialAllNull) {

    jbyte* target = (*env)->GetByteArrayElements(env, (jbyteArray)isNullTarget, NULL);
    jbyte* source = isNullSource ? (*env)->GetByteArrayElements(env,  (jbyteArray)isNullSource, NULL) : NULL;
    jbyte* present = presentBuffer ? (*env)->GetByteArrayElements(env, presentBuffer, NULL) : NULL;

    jboolean hasNull = initialHasNull;
    jboolean allNull = initialAllNull;

    if (!source && !present) {
        memset(target, 0, batchSize);
    }
    else if (!present) {
        memcpy(target, source, batchSize);
        if (!hasNull) {
            for (int i = 0; i < batchSize; i++) {
                if (target[i]) {
                    hasNull = JNI_TRUE;
                    break;
                }
            }
        }
        allNull = JNI_FALSE;
    }
    else if (!source) {
        for (int i = 0; i < batchSize; i++) {
            jbyte val = present[i] ? 1 : 0;
            target[i] = val;
            if (val) hasNull = JNI_TRUE;
            else allNull = JNI_FALSE;
        }
    }
    else {
        for (int i = 0; i < batchSize; i++) {
            jbyte val = source[i] || present[i];
            target[i] = val;
            if (val) {
                hasNull = JNI_TRUE;
            } else {
                allNull = JNI_FALSE;
            }
        }
    }

    (*env)->ReleaseByteArrayElements(env, (jbyteArray)isNullTarget, target, 0);
    if (source) {
        (*env)->ReleaseByteArrayElements(env, (jbyteArray)isNullSource, source, JNI_ABORT);
    }
    if (present) {
        (*env)->ReleaseByteArrayElements(env, presentBuffer, present, JNI_ABORT);
    }

    return (hasNull ? 1 : 0) | (allNull ? 2 : 0);
}