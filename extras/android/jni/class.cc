#include <jni/class.hh>
#include <jni/env.hh>

namespace jni {
	Class Class::FindClass(char const* name) noexcept {
		return Class{Env::handle()->FindClass(name)};
	}

	Class Class::GlobalRef() noexcept {
		return Class{jclass(Env::handle()->NewGlobalRef(cls_))};
	}

	jmethodID Class::GetMethodID(const char* name,
	                             std::string const& prototype) const noexcept {
		return Env::handle()->GetMethodID(cls_, name, prototype.c_str());
	}

	jmethodID Class::GetStaticMethodID(const char* name,
	                                   std::string const& prototype) const
	    noexcept {
		return Env::handle()->GetStaticMethodID(cls_, name, prototype.c_str());
	}

	jfieldID Class::GetFieldID(const char* name,
	                           std::string const& storage) const noexcept {
		return Env::handle()->GetFieldID(cls_, name, storage.c_str());
	}

	Object Class::newByIdV(jmethodID ctor, va_list args) const {
		return Object{Env::handle()->NewObjectV(cls_, ctor, args)};
	}
};
