#include <jni/method.hh>

#include <jni/env.hh>

namespace jni::access {
	JNIEnv* call_base::handle() const noexcept {
		return static_cast<JNIEnv*>(Env::handle());
	}
}
