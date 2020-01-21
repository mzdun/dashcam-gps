#include <jni/binding_ref.hh>

namespace jni::ref {
	binding_global<jclass> find_class(const char* name) noexcept {
		local<jclass> result{jni_env::get_env()->FindClass(name)};
		return result;
	}
}  // namespace jni::ref
