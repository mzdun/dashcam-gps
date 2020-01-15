#pragma once
#include <jni.h>

#include <cstddef>  // nullptr_t
#include <jni/type_info.hh>
#include <log.hh>

namespace jni::ref {
	MAKE_LOG("ref")

	struct jni_env {
		static JNIEnv* get_env() noexcept;
	};

	namespace policy {
		template <typename JNIReference>
		struct unretained {
			static JNIReference acquire(JNIReference ref) noexcept {
				return ref;
			}
			static void release(JNIReference) noexcept {}
		};

		template <typename JNIReference>
		struct local {
			static JNIReference acquire(JNIReference ref) noexcept {
				return static_cast<JNIReference>(
				    jni_env::get_env()->NewLocalRef(static_cast<jobject>(ref)));
			}
			static void release(JNIReference ref) noexcept {
				jni_env::get_env()->DeleteLocalRef(static_cast<jobject>(ref));
			}
		};

		template <>
		struct local<jobject> {
			static jobject acquire(jobject ref) noexcept {
				return jni_env::get_env()->NewLocalRef(ref);
			}
			static void release(jobject ref) noexcept {
				jni_env::get_env()->DeleteLocalRef(ref);
			}
		};

		template <typename JNIReference>
		struct global {
			static JNIReference acquire(JNIReference ref) noexcept {
				return static_cast<JNIReference>(
				    jni_env::get_env()->NewGlobalRef(
				        static_cast<jobject>(ref)));
			}
			static void release(JNIReference ref) noexcept {
				jni_env::get_env()->DeleteGlobalRef(static_cast<jobject>(ref));
			}
		};

		template <>
		struct global<jobject> {
			static jobject acquire(jobject ref) noexcept {
				return jni_env::get_env()->NewGlobalRef(ref);
			}
			static void release(jobject ref) noexcept {
				jni_env::get_env()->DeleteGlobalRef(ref);
			}
		};

		template <typename JNIReference>
		struct weak {
			static JNIReference acquire(JNIReference ref) noexcept {
				return static_cast<JNIReference>(
				    jni_env::get_env()->NewWeakGlobalRef(
				        static_cast<jobject>(ref)));
			}
			static void release(JNIReference ref) noexcept {
				jni_env::get_env()->DeleteWeakGlobalRef(
				    static_cast<jobject>(ref));
			}
		};

		template <>
		struct weak<jobject> {
			static jobject acquire(jobject ref) noexcept {
				return jni_env::get_env()->NewWeakGlobalRef(ref);
			}
			static void release(jobject ref) noexcept {
				jni_env::get_env()->DeleteWeakGlobalRef(ref);
			}
		};
	}  // namespace policy

	enum class op {
		// the current reference will be taken and later released
		take,
		// the reference will not be used; instead new one will be acquired
		acquire
	};

#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(x) 0L
#endif

#if __has_cpp_attribute(nodiscard) >= 201902L
#define NODISCARD(REASON) [[nodiscard(REASON)]]
#else
#define NODISCARD(REASON) [[nodiscard]]
#endif

	template <typename JNIReference, typename AcquireReleasePolicy>
	class basic_reference : private AcquireReleasePolicy {
	public:
		using pointer = JNIReference;
		using policy_type = AcquireReleasePolicy;
		constexpr basic_reference() noexcept = default;
		constexpr basic_reference(std::nullptr_t) noexcept {}
		explicit basic_reference(pointer ptr, op which = op::take) noexcept {
			reset(ptr, which);
		}

		basic_reference(basic_reference const& rhs) noexcept {
			if (rhs) pointer_ = get_policy()->acquire(rhs.get());
		}

		basic_reference& operator=(basic_reference const& rhs) noexcept {
			reset(rhs.get(), op::acquire);
			return *this;
		}

		basic_reference(basic_reference&& rhs) noexcept {
			pointer_ = rhs.release();
		}

		basic_reference& operator=(basic_reference&& rhs) noexcept {
			reset(rhs.release());
			return *this;
		}

		template <typename AnotherPolicy>
		basic_reference(
		    basic_reference<pointer, AnotherPolicy> const& rhs) noexcept {
			if (rhs) {
				pointer_ = get_policy()->acquire(rhs.get());
			}
		}

		template <typename AnotherPolicy>
		basic_reference& operator=(
		    basic_reference<pointer, AnotherPolicy> const& rhs) noexcept {
			reset(rhs.get(), op::acquire);
			return *this;
		}

		template <typename AnotherPolicy>
		basic_reference(
		    basic_reference<pointer, AnotherPolicy>&& rhs) noexcept {
			if (rhs) pointer_ = get_policy()->acquire(rhs.get());
			rhs.reset();
		}

		template <typename AnotherPolicy>
		basic_reference& operator=(
		    basic_reference<pointer, AnotherPolicy>&& rhs) noexcept {
			auto new_pointer =
			    rhs ? get_policy()->acquire(rhs.get()) : pointer{};
			rhs.reset();
			reset(new_pointer);
			return *this;
		}

		~basic_reference() { reset(); }

		void reset(pointer ptr = pointer{}, op which = op::take) noexcept {
			auto old_pointer = pointer_;

			if (which == op::take) {
				pointer_ = ptr;
			} else if (ptr) {
				pointer_ = get_policy()->acquire(ptr);
			}

			if (old_pointer) {
				get_policy()->release(old_pointer);
			}
		}

		void reset(std::nullptr_t) noexcept { reset(); }

		NODISCARD(
		    "Not using result of release() may lead to leaks. "
		    "You may want reset() instead.")
		pointer release() noexcept {
			auto old_pointer = pointer_;
			pointer_ = pointer{};
			return old_pointer;
		}

		[[nodiscard]] pointer get() const noexcept { return pointer_; }

		[[nodiscard]] policy_type* get_policy() noexcept { return this; }

		[[nodiscard]] policy_type const* get_policy() const noexcept {
			return this;
		}

		explicit operator bool() const noexcept { return !!get(); }

		bool is_null() const noexcept {
			if (!*this) return false;
			return jni_env::get_env()->IsSameObject(get(), nullptr) == JNI_TRUE;
		}

		bool operator==(basic_reference const& rhs) const noexcept {
			return (get() == rhs.get()) || (jni_env::get_env()->IsSameObject(
			                                    get(), rhs.get()) == JNI_TRUE);
		}

		bool operator!=(basic_reference const& rhs) const noexcept {
			return !(*this == rhs);
		}

	protected:
		JNIReference pointer_{};
	};

	template <typename JNIReference = jobject>
	using unretained =
	    basic_reference<JNIReference, policy::unretained<JNIReference>>;

	template <typename JNIReference = jobject>
	using local = basic_reference<JNIReference, policy::local<JNIReference>>;

	template <typename JNIReference = jobject>
	using global = basic_reference<JNIReference, policy::global<JNIReference>>;

	template <typename JNIReference = jobject>
	using weak = basic_reference<JNIReference, policy::weak<JNIReference>>;
}  // namespace jni::ref

namespace jni {
	template <typename JNIReference, typename AcquireReleasePolicy>
	struct type<ref::basic_reference<JNIReference, AcquireReleasePolicy>>
	    : type<JNIReference> {};

	template <typename JNIReference, typename AcquireReleasePolicy>
	struct conv<ref::basic_reference<JNIReference, AcquireReleasePolicy>> {
		using ref_type =
		    ref::basic_reference<JNIReference, AcquireReleasePolicy>;
		static JNIReference unpack(ref_type const& jni_ref) {
			return jni_ref.get();
		}
		static ref_type pack(JNIReference jvm) { return ref_type{jvm}; }
	};
}  // namespace jni

namespace jni::ref {
	template <typename Policy, typename JNIReference>
	local<jclass> get_class(basic_reference<JNIReference, Policy> const& obj) {
		return local<jclass>{jni_env::get_env()->GetObjectClass(obj.get())};
	}

	jmethodID get_method_id(jclass cls,
	                        const char* name,
	                        char const* prototype) noexcept;

	template <typename Name, typename Prototype>
	inline jmethodID get_method_id(jclass cls) noexcept {
		return get_method_id(cls, Name::get_name().c_str(),
		                     jni::type<Prototype>::name().c_str());
	}

	template <typename... Args>
	inline jmethodID get_constructor_id(jclass cls) noexcept {
		return get_method_id(cls, "<init>",
		                     jni::type<void(Args...)>::name().c_str());
	}

	jmethodID get_static_method_id(jclass cls,
	                               const char* name,
	                               char const* prototype) noexcept;

	template <typename Name, typename Prototype>
	inline jmethodID get_static_method_id(jclass cls) noexcept {
		return get_static_method_id(cls, Name::get_name().c_str(),
		                            jni::type<Prototype>::name().c_str());
	}

	jfieldID get_field_id(jclass cls,
	                      const char* name,
	                      char const* stg) noexcept;

	template <typename Name, typename Storage>
	inline jfieldID get_field_id(jclass cls) noexcept {
		return get_field_id(cls, Name::get_name().c_str(),
		                    jni::type<Storage>::name().c_str());
	}

	jfieldID get_static_field_id(jclass cls,
	                             const char* name,
	                             char const* stg) noexcept;

	template <typename Name, typename Storage>
	inline jfieldID get_static_field_id(jclass cls) noexcept {
		return get_static_field_id(cls, Name::get_name().c_str(),
		                           jni::type<Storage>::name().c_str());
	}

	global<jclass> find_class(const char* name) noexcept;

	template <typename CxxClass>
	inline global<jclass> find_class() {
		using PACKAGE = typename CxxClass::PACKAGE;
		return find_class(
		    (PACKAGE::package_name() + '/' + CxxClass::class_name()).c_str());
	}
}  // namespace jni::ref
