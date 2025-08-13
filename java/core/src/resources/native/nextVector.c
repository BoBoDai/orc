#include <jni.h>
#include <stdlib.h>
#include "NativeTreeReader.h"

JNIEXPORT jboolean JNICALL Java_org_apache_orc_impl_NativeTreeReader_nextVector
  (JNIEnv *env, jclass cls, jobject previous, jbooleanArray isNull, jint batchSize, jobject present) {
    // 1. 检查present流是否存在
    jboolean hasPresent = (*env)->IsSameObject(env, present, NULL) == JNI_FALSE;
    jboolean hasIsNullArray = (*env)->IsSameObject(env, isNull, NULL) == JNI_FALSE;

    // 2. 获取ColumnVector字段ID
    jclass cvClass = (*env)->GetObjectClass(env, previous);
    jfieldID noNullsField = (*env)->GetFieldID(env, cvClass, "noNulls", "Z");
    jfieldID isNullField = (*env)->GetFieldID(env, cvClass, "isNull", "[Z");
    jfieldID isRepeatingField = (*env)->GetFieldID(env, cvClass, "isRepeating", "Z");

    if (!noNullsField || !isNullField || !isRepeatingField) {
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"),
                         "Failed to get ColumnVector field IDs");
        return JNI_FALSE;
    }

    // 3. 获取isNull数组指针（如果存在）
    jboolean* javaIsNull = NULL;
    if (hasIsNullArray) {
        javaIsNull = (*env)->GetBooleanArrayElements(env, isNull, NULL);
        if (!javaIsNull) return JNI_FALSE;
    }

    // 4. 获取并准备ColumnVector的isNull数组
    jbooleanArray cvIsNullArray = (*env)->GetObjectField(env, previous, isNullField);
    if (!cvIsNullArray) {
        if (hasIsNullArray) (*env)->ReleaseBooleanArrayElements(env, isNull, javaIsNull, JNI_ABORT);
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/NullPointerException"),
                         "ColumnVector.isNull is null");
    }

    jboolean* cvIsNull = (*env)->GetBooleanArrayElements(env, cvIsNullArray, NULL);
    if (!cvIsNull) {
        if (hasIsNullArray) (*env)->ReleaseBooleanArrayElements(env, isNull, javaIsNull, JNI_ABORT);
        return JNI_FALSE;
    }

    // 5. 处理null逻辑
    jboolean allNull = JNI_TRUE;
    jboolean noNulls = JNI_TRUE;
    // 获取BitFieldReader.next()方法ID（仅当需要present流时）
    jmethodID nextMethod = NULL;
    if (hasPresent) {
        jclass presentClass = (*env)->GetObjectClass(env, present);
        nextMethod = (*env)->GetMethodID(env, presentClass, "next", "()I");
        if (!nextMethod) {
            (*env)->ReleaseBooleanArrayElements(env, cvIsNullArray, cvIsNull, JNI_ABORT);
            if (hasIsNullArray) (*env)->ReleaseBooleanArrayElements(env, isNull, javaIsNull, JNI_ABORT);
                return JNI_FALSE;
            }
        }

        // 处理每个元素
        for (jint i = 0; i < batchSize; i++) {
            if (!hasIsNullArray || !javaIsNull[i]) {
                if (hasPresent && (*env)->CallIntMethod(env, present, nextMethod) != 1) {
                    noNulls = JNI_FALSE;
                    cvIsNull[i] = JNI_TRUE;
                } else {
                    cvIsNull[i] = JNI_FALSE;
                    allNull = JNI_FALSE;
                }
            } else {
                noNulls = JNI_FALSE;
                cvIsNull[i] = JNI_TRUE;
            }
        }

    // 6. 设置ColumnVector元数据
    (*env)->SetBooleanField(env, previous, noNullsField, noNulls);

    // 7. 释放资源
    (*env)->ReleaseBooleanArrayElements(env, cvIsNullArray, cvIsNull, 0);
    if (hasIsNullArray) {
        (*env)->ReleaseBooleanArrayElements(env, isNull, javaIsNull, JNI_ABORT);
    }
    return allNull;
}