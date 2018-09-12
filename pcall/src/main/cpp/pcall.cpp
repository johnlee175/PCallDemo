#include "jvmti_helper.h"
#include "jvmti.h"
#include <inttypes.h>
#include <dlfcn.h>

namespace profiler {

    // Retrieve the app's data directory path
    static std::string GetAppDataPath() {
        Dl_info dl_info;
        dladdr((void*)Agent_OnAttach, &dl_info);
        std::string so_path(dl_info.dli_fname);
        return so_path.substr(0, so_path.find_last_of('/') + 1);
    }

    extern "C" {

    // Method signature for IterateThroughHeap extension that includes heap id.
    // Note that the signature is almost identical to IterateThroughHeap, with the
    // heap_iteration_callback in jvmtiHeapCallbacks taking a function pointer with
    // an additional int parameter.
    using IterateThroughHeapExt = jvmtiError (*)(jvmtiEnv*, jint, jclass,
                                                 const jvmtiHeapCallbacks*,
                                                 const void*);

    static IterateThroughHeapExt g_iterate_heap_ext_func = nullptr;

    // Reference to https://android.googlesource.com/platform/tools/base/+/studio-master-dev/profiler/native/perfa/perfa.cc

    void JNICALL OnClassLoad(jvmtiEnv *jvmti_env,
                             JNIEnv *jni_env,
                             jthread thread,
                             jclass klass) {
        char *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetClassSignature(klass, &sig_mutf8, nullptr));
        if (sig_mutf8 != nullptr) {
            LOGE("OnClassLoad: %s\n", sig_mutf8);
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnMethodEntry(jvmtiEnv *jvmti_env,
                               JNIEnv *jni_env,
                               jthread thread,
                               jmethodID method) {
        char *name_mutf8, *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetMethodName(method, &name_mutf8, &sig_mutf8, nullptr));
        LOGE("OnMethodEntry: %s, %s\n", name_mutf8, sig_mutf8);
        if (name_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) name_mutf8);
        }
        if (sig_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnMethodExist(jvmtiEnv *jvmti_env,
                               JNIEnv *jni_env,
                               jthread thread,
                               jmethodID method,
                               jboolean was_popped_by_exception,
                               jvalue return_value) {
        char *name_mutf8, *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetMethodName(method, &name_mutf8, &sig_mutf8, nullptr));
        LOGE("OnMethodExist: %s, %s\n", name_mutf8, sig_mutf8);
        if (name_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) name_mutf8);
        }
        if (sig_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnSingleStep(jvmtiEnv *jvmti_env,
                              JNIEnv *jni_env,
                              jthread thread,
                              jmethodID method,
                              jlocation location) {
        char *name_mutf8, *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetMethodName(method, &name_mutf8, &sig_mutf8, nullptr));
        LOGE("OnSingleStep: %s, %s %" PRId64 "\n", name_mutf8, sig_mutf8, location);
        if (name_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) name_mutf8);
        }
        if (sig_mutf8 != nullptr) {
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnVMObjectAlloc(jvmtiEnv *jvmti_env,
                                 JNIEnv *jni_env,
                                 jthread thread,
                                 jobject object,
                                 jclass object_klass,
                                 jlong size) {
        jint hash_code = 0;
        CheckJvmtiError(jvmti_env, jvmti_env->GetObjectHashCode(object, &hash_code));
        char *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetClassSignature(object_klass, &sig_mutf8, nullptr));
        if (sig_mutf8 != nullptr) {
            LOGE("OnVMObjectAlloc: %s@%" PRId32 "\n", sig_mutf8, hash_code);
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnClassPrepare(jvmtiEnv *jvmti_env,
                                JNIEnv *jni_env,
                                jthread thread,
                                jclass klass) {
        char *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetClassSignature(klass, &sig_mutf8, nullptr));
        if (sig_mutf8 != nullptr) {
            LOGE("OnClassPrepare: %s\n", sig_mutf8);
            Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
        }
    }

    void JNICALL OnClassFileLoadHook(jvmtiEnv *jvmti_env,
                                     JNIEnv *jni_env,
                                     jclass class_being_redefined,
                                     jobject loader,
                                     const char *name,
                                     jobject protection_domain,
                                     jint class_data_len,
                                     const unsigned char *class_data,
                                     jint *new_class_data_len,
                                     unsigned char **new_class_data) {
        if (name != nullptr) {
            LOGE("OnClassFileLoaded: %s\n", name);
        }
    }

    int JNICALL HeapIterationCallback(jlong class_tag,
                                      jlong size,
                                      jlong *tag_ptr,
                                      jint length,
                                      void *user_data) {
        LOGE("heapIterationCallback: %" PRId64 ", %" PRId64 "\n", class_tag, size);
        return JVMTI_VISIT_OBJECTS;
    }

    jvmtiIterationControl JNICALL HeapObjectCallback(jlong class_tag,
                                                     jlong size,
                                                     jlong *tag_ptr,
                                                     void *user_data) {
        LOGE("heapObjectCallback: %" PRId64 ", %" PRId64 "\n", class_tag, size);
        return JVMTI_ITERATION_CONTINUE;
    }

    void JNICALL StartAgentThreadFunc(jvmtiEnv* jvmti,
                                      JNIEnv* jni,
                                      void* ptr) {
        LOGE("StartAgentThreadFunc running ... ... ... ... ... ...");
    }

    JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
        jvmtiEnv *jvmti_env = CreateJvmtiEnv(vm);
        if (jvmti_env == nullptr) {
            return JNI_ERR;
        }
        SetAllCapabilities(jvmti_env);
        JNIEnv *jni_env = GetThreadLocalJNI(vm);

        jvmtiEventCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.ClassLoad = OnClassLoad;
        callbacks.MethodEntry = OnMethodEntry;
        callbacks.MethodExit = OnMethodExist;
        callbacks.SingleStep = OnSingleStep;
        callbacks.VMObjectAlloc = OnVMObjectAlloc;
        callbacks.ClassFileLoadHook = OnClassFileLoadHook; // use platform/tools/dexter
        callbacks.ClassPrepare = OnClassPrepare;
        CheckJvmtiError(jvmti_env, jvmti_env->SetEventCallbacks(&callbacks, sizeof(callbacks)));
        SetEventNotification(jvmti_env, JVMTI_ENABLE, JVMTI_EVENT_CLASS_LOAD);
        SetEventNotification(jvmti_env, JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY);
        SetEventNotification(jvmti_env, JVMTI_DISABLE, JVMTI_EVENT_METHOD_EXIT); // not need yet
        SetEventNotification(jvmti_env, JVMTI_DISABLE, JVMTI_EVENT_SINGLE_STEP); // too many
        SetEventNotification(jvmti_env, JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC);
        SetEventNotification(jvmti_env, JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK);
        SetEventNotification(jvmti_env, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE);

        // WindowManagerGlobal#getRootView(String)
        // ActivityThread#mActivities#activity
        // Application#registerActivityLifecycleCallbacks(ActivityLifecycleCallbacks)
        ScopedLocalRef<jclass> klass(jni_env, jni_env->FindClass("android/os/HandlerThread"));
        if (klass.get() == nullptr) {
            LOGE("Failed to find Thread class.");
        } else {
            jmethodID init = jni_env->GetMethodID(klass.get(), "<init>", "(Ljava/lang/String;)V");
            if (init == nullptr) {
                LOGE("Failed to find Thread.<init> method.");
            } else {
                jthread thread = jni_env->NewObject(klass.get(), init, jni_env->NewStringUTF("PCALL-Th"));
                if (thread == nullptr) {
                    LOGE("Failed to create new Thread object.");
                } else {
                    jmethodID start = jni_env->GetMethodID(klass.get(), "start", "()V");
                    if (start == nullptr) {
                        LOGE("Failed to find Thread.start method.");
                    } else {
                        jni_env->CallVoidMethod(thread, start);
                    }
                }
            }
        }

        // run agent thread
        CheckJvmtiError(jvmti_env, jvmti_env->RunAgentThread(AllocateJavaThread(jvmti_env, jni_env),
                                                             StartAgentThreadFunc,
                                                             nullptr,
                                                             JVMTI_THREAD_NORM_PRIORITY));

        // dump loaded classes
        jint class_count;
        jclass *loaded_classes;
        char *sig_mutf8;
        CheckJvmtiError(jvmti_env, jvmti_env->GetLoadedClasses(&class_count, &loaded_classes));
        for (int i = 0; i < class_count; ++i) {
            CheckJvmtiError(jvmti_env, jvmti_env->GetClassSignature(loaded_classes[i], &sig_mutf8, nullptr));
            if (sig_mutf8 != nullptr) {
                LOGE("Loaded class %s\n", sig_mutf8);
                Deallocate(jvmti_env, (unsigned char *) sig_mutf8);
            }
        }

        for (int i = 0; i < class_count; ++i) {
            jni_env->DeleteLocalRef(loaded_classes[i]);
        }
        Deallocate(jvmti_env, reinterpret_cast<unsigned char *>(loaded_classes));

        // dump all threads
        jint thread_count;
        jthread *all_threads;
        jvmtiThreadInfo thread_info;
        CheckJvmtiError(jvmti_env, jvmti_env->GetAllThreads(&thread_count, &all_threads));
        for (int i = 0; i < thread_count; ++i) {
            CheckJvmtiError(jvmti_env, jvmti_env->GetThreadInfo(all_threads[i], &thread_info));
            LOGE("Found thread %s\n", thread_info.name);
            Deallocate(jvmti_env, (unsigned char *) thread_info.name);
        }

        for (int i = 0; i < thread_count; ++i) {
            jni_env->DeleteLocalRef(all_threads[i]);
        }
        Deallocate(jvmti_env, reinterpret_cast<unsigned char *>(all_threads));

        // dump heap objects

        ScopedLocalRef<jclass> activity_lass(jni_env, jni_env->FindClass("android/app/Activity"));
        if (activity_lass.get() == nullptr) {
            LOGE("Failed to find Activity class.");
        } else {
            // JVMTI error: 98(JVMTI_ERROR_NOT_AVAILABLE)
            /* CheckJvmtiError(jvmti_env,
                            jvmti_env->IterateOverInstancesOfClass(activity_lass.get(),
                                                                   JVMTI_HEAP_OBJECT_EITHER,
                                                                   heapObjectCallback,
                                                                   nullptr)); */

            jvmtiHeapCallbacks heap_callbacks;
            memset(&heap_callbacks, 0, sizeof(heap_callbacks));
            heap_callbacks.heap_iteration_callback = HeapIterationCallback;
            // JVMTI error: 98(JVMTI_ERROR_NOT_AVAILABLE)
            /* CheckJvmtiError(jvmti_env,
                            jvmti_env->IterateThroughHeap(0, activity_lass.get(), &heap_callbacks, nullptr)); */


            // Locate heap extension functions
            jvmtiExtensionFunctionInfo* func_info;
            jint func_count = 0;
            CheckJvmtiError(jvmti_env, jvmti_env->GetExtensionFunctions(&func_count, &func_info));

            // Go through all extension functions as we need to deallocate
            for (int i = 0; i < func_count; i++) {
                if (strcmp("com.android.art.heap.iterate_through_heap_ext", func_info[i].id) == 0) {
                    g_iterate_heap_ext_func =
                            reinterpret_cast<IterateThroughHeapExt>(func_info[i].func);
                }
                Deallocate(jvmti_env, func_info[i].id);
                Deallocate(jvmti_env, func_info[i].short_description);
                for (int j = 0; j < func_info[i].param_count; j++) {
                    Deallocate(jvmti_env, func_info[i].params[j].name);
                }
                Deallocate(jvmti_env, func_info[i].params);
                Deallocate(jvmti_env, func_info[i].errors);
            }
            Deallocate(jvmti_env, func_info);
            assert(g_iterate_heap_ext_func != nullptr);
            g_iterate_heap_ext_func(jvmti_env, 0, nullptr/* activity_lass.get() */, &heap_callbacks, nullptr);
        }

        // Load in pcall.dex.jar which should be in to data/data.
        std::string agent_lib_path(GetAppDataPath());
        agent_lib_path.append("pcall.dex.jar");

        CheckJvmtiError(jvmti_env, jvmti_env->AddToBootstrapClassLoaderSearch(agent_lib_path.c_str()));
        ScopedLocalRef<jclass> finder_class(jni_env, jni_env->FindClass("com/johnsoft/pcalla/Finder"));
        if (finder_class.get() == nullptr) {
            LOGE("Failed to find Finder class.");
        } else {
            jmethodID finder_init = jni_env->GetStaticMethodID(finder_class.get(), "init", "()V");
            if (finder_init == nullptr) {
                LOGE("Failed to find Finder.init method.");
            } else {
                jni_env->CallStaticVoidMethod(finder_class.get(), finder_init);
            }
        }

        /*
         * NOTE:
         * because of `jvmti_env->AddToSystemClassLoaderSearch(agent_lib_path.c_str());` not work.
         * use the following code instread: (There is no custom class load problem in this way)
         */
        /*
        // ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
        jclass Thread_class = jni_env->FindClass("java/lang/Thread");
        jmethodID currentThread_method = jni_env->GetStaticMethodID(Thread_class, "currentThread", "()Ljava/lang/Thread;");
        jobject thread = jni_env->CallStaticObjectMethod(Thread_class, currentThread_method);
        jmethodID getContextClassLoader_method = jni_env->GetMethodID(Thread_class, "getContextClassLoader",
                                                                      "()Ljava/lang/ClassLoader;");
        jobject classLoader = jni_env->CallObjectMethod(thread, getContextClassLoader_method);

        // ((BaseDexClassLoader) classLoader).addDexPath("the-dex-path");
        jclass BaseDexClassLoader_class = jni_env->GetObjectClass(classLoader);
        jmethodID addDexPath_method = jni_env->GetMethodID(BaseDexClassLoader_class, "addDexPath", "(Ljava/lang/String;)V");
        jni_env->CallVoidMethod(classLoader, addDexPath_method, jni_env->NewStringUTF(agent_lib_path.c_str()));

        // jclass clazz = (jclass) classLoader.loadClass("com.johnsoft.pcalla.Finder");
        // Finder.init()
        jmethodID loadClass_method = jni_env->GetMethodID(BaseDexClassLoader_class, "loadClass",
                                                           "(Ljava/lang/String;)Ljava/lang/Class;");
        jclass Finder_class = (jclass) jni_env->CallObjectMethod(classLoader, loadClass_method,
                                                                  jni_env->NewStringUTF("com.johnsoft.pcalla.Finder"));
        jmethodID init_method = jni_env->GetStaticMethodID(Finder_class, "init", "()V");
        jni_env->CallStaticVoidMethod(Finder_class, init_method);
        */
        return JNI_OK;
    }

    }

}
