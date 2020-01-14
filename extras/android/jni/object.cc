#include <jni/object.hh>
#include <jni/class.hh>
#include <jni/env.hh>

namespace jni {
	Class Object::getClass() const noexcept {
		return Class{Env::handle()->GetObjectClass(obj_)};
	}
};
