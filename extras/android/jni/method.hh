#pragma once
#include <jni.h>

#include <algorithm>
#include <jni/ref.hh>
#include <jni/type_info.hh>

namespace jni {
	template <typename Type>
	struct method_invocation;
#define JNI_METHOD_INVOCATION(TYPE, NAME, UNUSED)               \
	template <>                                                 \
	struct method_invocation<TYPE> {                            \
		template <typename... Args>                             \
		static TYPE call(jobject obj,                           \
		                 jmethodID method,                      \
		                 Args... args) noexcept {               \
			return conv<TYPE>::pack(                            \
			    ref::jni_env::get_env()->Call##NAME##Method(    \
			        obj, method, conv<Args>::unpack(args)...)); \
		}                                                       \
	};

	JNI_PRIMITIVES(JNI_METHOD_INVOCATION)
#undef JNI_METHOD_INVOCATION

	template <>
	struct method_invocation<void> {
		template <typename... Args>
		static void call(jobject obj, jmethodID method, Args... args) noexcept {
			ref::jni_env::get_env()->CallVoidMethod(
			    obj, method, conv<Args>::unpack(args)...);
		}
	};

	template <typename Type>
	struct static_method_invocation;
#define JNI_STATIC_METHOD_INVOCATION(TYPE, NAME, UNUSED)           \
	template <>                                                    \
	struct static_method_invocation<TYPE> {                        \
		template <typename... Args>                                \
		static TYPE call(jclass cls,                               \
		                 jmethodID method,                         \
		                 Args... args) noexcept {                  \
			return conv<TYPE>::pack(                               \
			    ref::jni_env::get_env()->CallStatic##NAME##Method( \
			        cls, method, conv<Args>::unpack(args)...));    \
		}                                                          \
	};

	JNI_PRIMITIVES(JNI_STATIC_METHOD_INVOCATION)
#undef JNI_STATIC_METHOD_INVOCATION

	template <>
	struct static_method_invocation<void> {
		template <typename... Args>
		static void call(jclass cls, jmethodID method, Args... args) noexcept {
			ref::jni_env::get_env()->CallStaticVoidMethod(
			    cls, method, conv<Args>::unpack(args)...);
		}
	};

	template <typename Type>
	struct prefered_return {
		using type = Type;
	};

	template <typename Type>
	using prefered_return_type = typename prefered_return<Type>::type;

	template <>
	struct prefered_return<jobject> {
		using type = ref::local<jobject>;
	};
	template <>
	struct prefered_return<jclass> {
		using type = ref::local<jclass>;
	};
	template <>
	struct prefered_return<jthrowable> {
		using type = ref::local<jthrowable>;
	};
	template <>
	struct prefered_return<jstring> {
		using type = ref::local<jstring>;
	};
	template <>
	struct prefered_return<jarray> {
		using type = ref::local<jarray>;
	};
	template <>
	struct prefered_return<jbooleanArray> {
		using type = ref::local<jbooleanArray>;
	};
	template <>
	struct prefered_return<jbyteArray> {
		using type = ref::local<jbyteArray>;
	};
	template <>
	struct prefered_return<jcharArray> {
		using type = ref::local<jcharArray>;
	};
	template <>
	struct prefered_return<jshortArray> {
		using type = ref::local<jshortArray>;
	};
	template <>
	struct prefered_return<jintArray> {
		using type = ref::local<jintArray>;
	};
	template <>
	struct prefered_return<jlongArray> {
		using type = ref::local<jlongArray>;
	};
	template <>
	struct prefered_return<jfloatArray> {
		using type = ref::local<jfloatArray>;
	};
	template <>
	struct prefered_return<jdoubleArray> {
		using type = ref::local<jdoubleArray>;
	};
	template <>
	struct prefered_return<jobjectArray> {
		using type = ref::local<jobjectArray>;
	};

	template <typename ReturnType>
	struct constructor_invocation {
		using preferred = prefered_return_type<ReturnType>;
		template <typename... Args>
		static preferred call(jclass obj,
		                      jmethodID method,
		                      Args... args) noexcept {
			return conv<preferred>::pack(ref::jni_env::get_env()->NewObject(
			    obj, method, conv<Args>::unpack(args)...));
		}
	};

	template <typename JNIReference,
	          template <typename>
	          typename Invocation,
	          typename Prototype>
	struct bound_call;

	template <typename JNIReference,
	          template <typename>
	          typename Invocation,
	          typename Ret,
	          typename... Args>
	struct bound_call<JNIReference, Invocation, Ret(Args...)> {
		using invocation = Invocation<Ret>;
		JNIReference jni_ref;
		jmethodID method;

		bound_call() = delete;
		bound_call(JNIReference jni_ref, jmethodID method) noexcept
		    : jni_ref{jni_ref}, method{method} {};

		Ret operator()(Args... args) && {
			return invocation::call(jni_ref, method, args...);
		}
	};

	template <typename Name, typename Prototype>
	struct method {
		jmethodID from(ref::local<jobject> const& obj) {
			if (!method_id_) {
				return from(ref::get_class(obj));
			}

			return method_id_;
		}

		jmethodID from(ref::local<jclass> const& cls) {
			if (!method_id_) {
				method_id_ = ref::get_method_id<Name, Prototype>(cls.get());
			}

			return method_id_;
		}

		template <typename JNIReference>
		using bound_call =
		    jni::bound_call<JNIReference, method_invocation, Prototype>;

		template <typename JNIReference, typename Policy>
		bound_call<JNIReference> bind(
		    ref::basic_reference<JNIReference, Policy> const& obj) noexcept {
			return {obj.get(), from(obj)};
		};

	protected:
		jmethodID method_id_{nullptr};
	};

	template <typename Name, typename Prototype>
	struct static_method {
		jmethodID from(ref::local<jclass> const& cls) {
			if (!method_id_) {
				method_id_ =
				    ref::get_static_method_id<Name, Prototype>(cls.get());
			}

			return method_id_;
		}

		using bound_call =
		    jni::bound_call<jclass, static_method_invocation, Prototype>;

		template <typename Policy>
		bound_call bind(
		    ref::basic_reference<jclass, Policy> const& cls) noexcept {
			return {cls.get(), from(cls)};
		};

	protected:
		jmethodID method_id_{nullptr};
	};

	template <typename Prototype>
	struct constructor;

	template <typename Type>
	struct void2object {
		using type = Type;
	};

	template <>
	struct void2object<void> {
		using type = ref::local<jobject>;
	};

	template <typename Ret, typename... Args>
	struct constructor<Ret(Args...)> {
		using real_ret = typename void2object<Ret>::type;
		jmethodID from(ref::local<jclass> const& cls) {
			if (!method_id_) {
				method_id_ = ref::get_constructor_id<Args...>(cls.get());
			}

			return method_id_;
		}

		using bound_call =
		    jni::bound_call<jclass, constructor_invocation, real_ret(Args...)>;

		template <typename Policy>
		bound_call bind(
		    ref::basic_reference<jclass, Policy> const& cls) noexcept {
			return {cls.get(), from(cls)};
		};

	protected:
		jmethodID method_id_{nullptr};
	};
}  // namespace jni

#define JNI_METHOD_REF(VAR, NAME, PROTOTYPE) \
	DEFINE_NAME(VAR##_name__, NAME);         \
	static jni::method<VAR##_name__, PROTOTYPE> VAR;

#define JNI_CONSTRUCTOR_REF(VAR, PROTOTYPE) \
	static jni::constructor<PROTOTYPE> VAR;

#define JNI_STATIC_METHOD_REF(VAR, NAME, PROTOTYPE) \
	DEFINE_NAME(VAR##_name__, NAME);                \
	static jni::static_method<VAR##_name__, PROTOTYPE> VAR;
