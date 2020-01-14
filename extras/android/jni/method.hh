#pragma once
#include <jni.h>
#include <jni/primitive.hh>
#include <jni/type_info.hh>
#include <jni/class.hh>

namespace jni::access {
	class call_base {
	protected:
		explicit call_base(jmethodID method_id) : method_id_{method_id} {}
		JNIEnv* handle() const noexcept;

		jmethodID method_id_{};
	};

	template <typename Type>
	struct method;
#define JNI_METHOD_ACCESS(TYPE, NAME, UNUSED)                           \
	template <>                                                         \
	struct method<TYPE> : call_base {                                   \
		method(jmethodID method_id) : call_base(method_id) {}           \
		template <typename... Args>                                     \
		TYPE call(jobject obj, Args... args) noexcept {                 \
			return type<TYPE>::pack(this->handle()->Call##NAME##Method( \
			    obj, this->method_id_, type<Args>::unpack(args)...));   \
		}                                                               \
	};

	JNI_PRIMITIVES(JNI_METHOD_ACCESS)
#undef JNI_METHOD_ACCESS

	template <>
	struct method<void> : call_base {
		method(jmethodID method_id) : call_base(method_id) {}
		template <typename... Args>
		void call(jobject obj, Args... args) noexcept {
			this->handle()->CallVoidMethod(obj, this->method_id_,
			                               type<Args>::unpack(args)...);
		}
	};

	template <typename Type>
	struct static_method;
#define JNI_STATIC_METHOD_ACCESS(TYPE, NAME, UNUSED)                          \
	template <>                                                               \
	struct static_method<TYPE> : call_base {                                  \
		static_method(jmethodID method_id) : call_base(method_id) {}          \
		template <typename... Args>                                           \
		TYPE call(jclass cls, Args... args) noexcept {                        \
			return type<TYPE>::pack(this->handle()->CallStatic##NAME##Method( \
			    cls, this->method_id_, type<Args>::unpack(args)...));         \
		}                                                                     \
	};

	JNI_PRIMITIVES(JNI_STATIC_METHOD_ACCESS)
#undef JNI_STATIC_METHOD_ACCESS

	template <>
	struct static_method<void> : call_base {
		static_method(jmethodID method_id) : call_base(method_id) {}
		template <typename... Args>
		void call(jclass cls, Args... args) noexcept {
			this->handle()->CallStaticVoidMethod(cls, this->method_id_,
			                                     type<Args>::unpack(args)...);
		}
	};
}

namespace jni::method::holder {
	template <typename Final>
	struct base {
		jmethodID from(Object const& obj) {
			if (!method_id_) {
				static_cast<Final*>(this)->from(obj.getClass());
			}

			return method_id_;
		}

	protected:
		jmethodID method_id_{nullptr};
	};

	template <typename NamePolicy, typename Prototype>
	struct virt : base<virt<NamePolicy, Prototype>> {
		using base<virt<NamePolicy, Prototype>>::from;
		jmethodID from(Class const& cls) {
			if (!this->method_id_) {
				this->method_id_ =
				    cls.GetMethodID<Prototype>(NamePolicy::get_name());
			}

			return this->method_id_;
		}
	};

	template <typename NamePolicy, typename Prototype>
	struct stat : base<stat<NamePolicy, Prototype>> {
		using base<stat<NamePolicy, Prototype>>::from;
		jmethodID from(Class const& cls) {
			if (!this->method_id_) {
				this->method_id_ =
				    cls.GetStaticMethodID<Prototype>(NamePolicy::get_name());
			}

			return this->method_id_;
		}
	};
}
namespace jni::method {
	template <typename NamePolicy, typename Func>
	struct ref_base;

	template <typename NamePolicy, typename Ret, typename... Args>
	struct ref_base<NamePolicy, Ret(Args...)>
	    : holder::virt<NamePolicy, Ret(Args...)> {
		Ret call(Object const& self, Args... args) {
			this->from(self);
			return self.callById(access::method<Ret>(this->method_id_),
			                     args...);
		}
	};

	template <typename NamePolicy, typename... Args>
	struct ref_base<NamePolicy, void(Args...)>
	    : holder::virt<NamePolicy, void(Args...)> {
		void call(Object const& self, Args... args) {
			this->from(self);
			self.callById(access::method<void>(this->method_id_), args...);
		}

		Object new_object(Class const& cls, Args... args) {
			this->from(cls);
			return cls.newById(this->method_id_, args...);
		}
	};

	template <typename NamePolicy, typename Func>
	struct ref : ref_base<NamePolicy, Func> {};

	struct init_name {
		static const char* get_name() noexcept { return "<init>"; }
	};
}

namespace jni::static_method {
	template <typename NamePolicy, typename Func>
	struct ref_base;

	template <typename NamePolicy, typename Ret, typename... Args>
	struct ref_base<NamePolicy, Ret(Args...)>
	    : method::holder::stat<NamePolicy, Ret(Args...)> {
		Ret call(Object const& self, Args... args) {
			this->from(self);
			return self.callById(access::method<Ret>(this->method_id_),
			                     args...);
		}
	};

	template <typename NamePolicy, typename... Args>
	struct ref_base<NamePolicy, void(Args...)>
	    : method::holder::stat<NamePolicy, void(Args...)> {
		void call(Object const& self, Args... args) {
			this->from(self);
			self.callById(access::method<void>(this->method_id_), args...);
		}
	};

	template <typename NamePolicy, typename Func>
	struct ref : ref_base<NamePolicy, Func> {};
}

#define JNI_METHOD_REF(NAME, PROTOTYPE)                              \
	static auto& NAME() noexcept {                                   \
		struct name__ {                                              \
			static char const* get_name() noexcept { return #NAME; } \
		};                                                           \
		static jni::method::ref<name__, PROTOTYPE> ref__;            \
		return ref__;                                                \
	}

#define JNI_CONSTRUCTOR_REF(PROTOTYPE)                                    \
	static auto& constructor() noexcept {                                 \
		static jni::method::ref<jni::method::init_name, PROTOTYPE> ref__; \
		return ref__;                                                     \
	}

#define JNI_STATIC_METHOD_REF(NAME, PROTOTYPE)                       \
	static auto& NAME() noexcept {                                   \
		struct name__ {                                              \
			static char const* get_name() noexcept { return #NAME; } \
		};                                                           \
		static jni::static_method::ref<name__, PROTOTYPE> ref__;     \
		return ref__;                                                \
	}
