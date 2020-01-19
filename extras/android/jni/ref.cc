#include <jni/ref.hh>

#include <jni/env.hh>

namespace jni::ref {
	JNIEnv* jni_env::get_env() noexcept { return Env::get().handle(); }

	jmethodID get_method_id(jclass cls,
	                        const char* name,
	                        char const* prototype) noexcept {
		return jni_env::get_env()->GetMethodID(cls, name, prototype);
	}

	jmethodID get_static_method_id(jclass cls,
	                               const char* name,
	                               char const* prototype) noexcept {
		return jni_env::get_env()->GetStaticMethodID(cls, name, prototype);
	}

	jfieldID get_field_id(jclass cls,
	                      const char* name,
	                      char const* stg) noexcept {
		return jni_env::get_env()->GetFieldID(cls, name, stg);
	}

	jfieldID get_static_field_id(jclass cls,
	                             const char* name,
	                             char const* stg) noexcept {
		return jni_env::get_env()->GetStaticFieldID(cls, name, stg);
	}
}  // namespace jni::ref
