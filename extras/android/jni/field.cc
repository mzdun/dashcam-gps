#include <jni/field.hh>

#include <jni/env.hh>

namespace jni::access {
	JNIEnv* field_base::handle() const noexcept {
		return static_cast<JNIEnv*>(Env::handle());
	}
}
