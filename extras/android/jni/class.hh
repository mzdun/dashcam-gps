//
// Created by Marcin on 13.01.2020.
//

#pragma once
#include <jni.h>
#include <jni/type_info.hh>

namespace jni {
	class Class {
	public:
		explicit Class(jclass cls = nullptr) : cls_{cls} {}
		explicit operator bool() const noexcept { return cls_ != nullptr; }
		jclass handle() const noexcept { return cls_; }

		static Class FindClass(char const* name) noexcept;

		Class GlobalRef() noexcept;

		template <typename CxxClass>
		static Class FindClass() noexcept {
			using PACKAGE = typename CxxClass::PACKAGE;
			return Class::FindClass(
			    (PACKAGE::packageName() + "/" + CxxClass::className()).c_str());
		}

		template <typename... Args>
		Object newById(jmethodID ctor, Args... args) const {
			return newByIdInternal(ctor, type<Args>::unpack(args)...);
		}

		template <typename Prototype>
		jmethodID GetMethodID(const char* name) const noexcept {
			return GetMethodID(name, jni::type<Prototype>::name());
		}

		template <typename Prototype>
		jmethodID GetStaticMethodID(const char* name) const noexcept {
			return GetStaticMethodID(name, jni::type<Prototype>::name());
		}

		template <typename Storage>
		jfieldID GetFieldID(const char* name) const noexcept {
			return GetFieldID(name, jni::type<Storage>::name());
		}

	private:
		jmethodID GetMethodID(const char* name,
		                      std::string const& prototype) const noexcept;
		jmethodID GetStaticMethodID(const char* name,
		                            std::string const& prototype) const
		    noexcept;
		jfieldID GetFieldID(const char* name, std::string const& storage) const
		    noexcept;

		Object newByIdInternal(jmethodID ctor, ...) const {
			va_list args;
			va_start(args, ctor);
			auto result = newByIdV(ctor, args);
			va_end(args);
			return result;
		}
		Object newByIdV(jmethodID ctor, va_list args) const;
		jclass cls_;
	};
}