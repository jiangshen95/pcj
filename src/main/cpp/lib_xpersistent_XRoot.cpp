/* Copyright (C) 2017  Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 only, as published by the Free Software Foundation.
 * This file has been designated as subject to the "Classpath"
 * exception as provided in the LICENSE file that accompanied
 * this code.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details (a copy
 * is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "lib_xpersistent_XRoot.h"
#include "persistent_heap.h"
#include "util.h"

#define HEADER_REFCOUNT_OFFSET 12

static JNIEnv *env_global = NULL;
static jclass class_global = NULL;
static jmethodID mid = 0;

JNIEXPORT jlong JNICALL Java_lib_xpersistent_XRoot_nativeGetRootOffset
  (JNIEnv *env, jclass klass)
{
    return D_RO(get_root())->root_memory_region.oid.off;
}

JNIEXPORT jboolean JNICALL Java_lib_xpersistent_XRoot_nativeRootExists
  (JNIEnv *env, jclass klass)
{
    return (check_root_exists() ? JNI_TRUE : JNI_FALSE);
}

JNIEXPORT jlong JNICALL Java_lib_xpersistent_XRoot_nativeCreateRoot
  (JNIEnv *env, jclass klass, jlong root_size)
{
    create_root(root_size);
    return D_RO(get_root())->root_memory_region.oid.off;
}

JNIEXPORT void JNICALL Java_lib_xpersistent_XRoot_nativeRetrieveAddrs
  (JNIEnv *env, jobject root)
{
    jclass cls = env->GetObjectClass(root);
    jmethodID mid = env->GetMethodID(cls, "addToAddrs", "(J)V");
    if (mid == 0) {
        printf("Failed to get the addToAddrs method on XRoot!\n");
        fflush(stdout);
        exit(-1);
    }

    TOID(object) mr;
    POBJ_FOREACH_TYPE(pool, mr) {
        uint64_t classInfoAddr = *((uint64_t*)((uint64_t)pmemobj_direct(mr.oid)));
        if (classInfoAddr == 0U) {
            // printf("Ignoring null classinfo addr\n");
            // fflush(stdout);
            POBJ_FREE(&mr);
        } else {
            int ref_count = *((int*)((uint64_t)pmemobj_direct(mr.oid) + 8U));
            // printf("Addr %lu has refcount %d\n", mr.oid.off, ref_count);
            // fflush(stdout);
            if (ref_count == 0) {
                env->CallLongMethod(root, mid, mr.oid.off);
            }
        }
    }
}