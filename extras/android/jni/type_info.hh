//
// Created by Marcin on 13.01.2020.
//

#pragma once

#include <string>
#include <jni/primitive.hh>
#include <jni/object.hh>

#define DEFINE_PACKAGE(NAME)                              \
	struct PACKAGE {                                      \
		static std::string packageName() { return NAME; } \
	}

#define DEFINE_NAME(CLS, NAME)                                   \
	struct CLS {                                                 \
		static char const* get_name() noexcept { return #NAME; } \
	}

namespace jni {
	template <typename Type>
	struct type;
#define SIMPLE_TYPE(TYPE, JNI_TYPE, CODE)                    \
	template <>                                              \
	struct type<TYPE> {                                      \
		static std::string name() { return CODE; }           \
		static JNI_TYPE unpack(TYPE const& wrapped) {        \
			return static_cast<JNI_TYPE>(wrapped);           \
		}                                                    \
		static TYPE pack(JNI_TYPE jvm) { return TYPE{jvm}; } \
	}
#define OBJECT_TYPE(TYPE, CODE) SIMPLE_TYPE(TYPE, jobject, CODE)

#define PRIMITIVE_TYPE(JNI_TYPE, CODE)                       \
	template <>                                              \
	struct type<JNI_TYPE> {                                  \
		static std::string name() { return CODE; }           \
		static JNI_TYPE unpack(JNI_TYPE jvm) { return jvm; } \
		static JNI_TYPE pack(JNI_TYPE jvm) { return jvm; }   \
	}

	template <typename CxxClass>
	struct object_type {
		using wrapper = typename CxxClass::wrapper;
		static std::string name() {
			using PACKAGE = typename CxxClass::PACKAGE;
			return "L" + PACKAGE::packageName() + "/" + CxxClass::className() +
			       ";";
		}
		static jobject unpack(Object const& wrapped) {
			return static_cast<jobject>(wrapped);
		}
		static wrapper pack(jobject jvm) { return {Object{jvm}}; }
	};

#define JNI_PRIMITIVE_TYPE(JNI_TYPE, UNUSED, CODE) \
	PRIMITIVE_TYPE(JNI_TYPE, CODE);
	JNI_PRIMITIVES(JNI_PRIMITIVE_TYPE)
#undef JNI_PRIMITIVE_TYPE
	OBJECT_TYPE(Object, "Ljava/lang/Object;");

	template <>
	struct type<void> {
		static std::string name() { return "V"; }
	};

	PRIMITIVE_TYPE(jstring, "Ljava/lang/String;");

	template <typename Type>
	struct type<Type const> : type<Type> {};

	template <typename Type>
	struct type<Type&> : type<Type> {};

	template <typename Element>
	struct array;
	template <typename Element>
	struct type<array<Element>> {
		static std::string name() { return "[" + type<Element>::name(); }
	};

	template <typename Return, typename... Args>
	struct type<Return(Args...)> {
		static std::string name() {
			return "(" + (type<Args>::name() + ... + std::string{}) + ")" +
			       type<Return>::name();
		}
	};
}
