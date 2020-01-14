#include <jni/refs.hh>
#include <jni/env.hh>

namespace jni {
	JNIEnv* basic_ref::handle() noexcept {
		return static_cast<JNIEnv*>(Env::handle());
	}
}