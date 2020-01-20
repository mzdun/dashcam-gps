#pragma once

#include <jni/fixed_string.hh>
#include <jni/primitive.hh>

#define DEFINE_PACKAGE(NAME)                            \
	struct PACKAGE {                                    \
		static constexpr auto package_name() noexcept { \
			return jni::fixed_string{NAME};             \
		}                                               \
	}

#define DEFINE_NAME(CLS, NAME)                      \
	struct CLS {                                    \
		static constexpr auto get_name() noexcept { \
			return jni::fixed_string{NAME};         \
		}                                           \
	}

namespace jni {
	template <typename Type, typename = void>
	struct type;
	template <typename Type, typename = void>
	struct conv;
#define SIMPLE_TYPE(TYPE, JNI_TYPE, CODE)                    \
	template <>                                              \
	struct type<TYPE> {                                      \
		static constexpr auto name() noexcept {              \
			return jni::fixed_string{CODE};                  \
		}                                                    \
	};                                                       \
                                                             \
	template <>                                              \
	struct conv<TYPE> {                                      \
		static JNI_TYPE unpack(TYPE const& wrapped) {        \
			return static_cast<JNI_TYPE>(wrapped);           \
		}                                                    \
		static TYPE pack(JNI_TYPE jvm) { return TYPE{jvm}; } \
	}
#define OBJECT_TYPE(TYPE, CODE) SIMPLE_TYPE(TYPE, jobject, CODE)

#define PRIMITIVE_TYPE(JNI_TYPE, CODE)                       \
	template <>                                              \
	struct type<JNI_TYPE> {                                  \
		static constexpr auto name() noexcept {              \
			return jni::fixed_string{CODE};                  \
		}                                                    \
	};                                                       \
                                                             \
	template <>                                              \
	struct conv<JNI_TYPE> {                                  \
		static JNI_TYPE unpack(JNI_TYPE jvm) { return jvm; } \
		static JNI_TYPE pack(JNI_TYPE jvm) { return jvm; }   \
	}

#define JNI_PRIMITIVE_TYPE(JNI_TYPE, UNUSED, CODE) \
	PRIMITIVE_TYPE(JNI_TYPE, CODE);
	JNI_PRIMITIVES(JNI_PRIMITIVE_TYPE)
#undef JNI_PRIMITIVE_TYPE

	template <>
	struct type<void> {
		static constexpr auto name() noexcept { return fixed_string{"V"}; }
	};

	PRIMITIVE_TYPE(jobject, "Ljava/lang/Object;");
	PRIMITIVE_TYPE(jstring, "Ljava/lang/String;");

	template <typename Type>
	struct type<Type const> : type<Type> {};

	template <typename Type>
	struct type<Type&> : type<Type> {};

	template <typename Element>
	struct array;
	template <typename Element>
	struct type<array<Element>> {
		static constexpr auto name() noexcept {
			return "[" + type<Element>::name();
		}
	};

	template <typename Return, typename... Args>
	struct type<Return(Args...)> {
		static constexpr auto name() noexcept {
			return fixed_string{"("} + (type<Args>::name() + ... + "") + ")" +
			       type<Return>::name();
		}
	};
}  // namespace jni
