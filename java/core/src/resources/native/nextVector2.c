#include <jni.h>
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

    jboolean* target = (*env)->GetBooleanArrayElements(env, isNullTarget, NULL);
    jboolean* source = isNullSource ? (*env)->GetBooleanArrayElements(env, isNullSource, NULL) : NULL;
    jbyte* present = presentBuffer ? (*env)->GetByteArrayElements(env, presentBuffer, NULL) : NULL;

    jboolean hasNull = initialHasNull;
    jboolean allNull = initialAllNull;

    for (int i = 0; i < batchSize; i++) {
        if (source && source[i]) {
            target[i] = JNI_TRUE;
            hasNull = JNI_TRUE;
            continue;
        }

        jboolean isPresentNull = present && present[i] != 0;
        if (isPresentNull) {
            target[i] = JNI_TRUE;
            hasNull = JNI_TRUE;
        } else {
            target[i] = JNI_FALSE;
            allNull = JNI_FALSE;
        }
    }

    (*env)->ReleaseBooleanArrayElements(env, isNullTarget, target, 0);
    if (source) {
        (*env)->ReleaseBooleanArrayElements(env, isNullSource, source, JNI_ABORT);
    }
    if (present) {
        (*env)->ReleaseByteArrayElements(env, presentBuffer, present, JNI_ABORT);
    }

    return (hasNull ? 1 : 0) | (allNull ? 2 : 0);
}