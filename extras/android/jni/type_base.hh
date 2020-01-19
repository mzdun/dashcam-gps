#pragma once
#include <jni/binding_ref.hh>

namespace jni {
	struct type_base {};

	template <template <typename> typename Policy = ref::policy::local>
	struct ref_type_base : type_base {
		ref_type_base() noexcept = default;
		ref_type_base(ref_type_base const&) noexcept = default;
		ref_type_base& operator=(ref_type_base const&) noexcept = default;
		ref_type_base(ref_type_base&&) noexcept = default;
		ref_type_base& operator=(ref_type_base&&) noexcept = default;

		template <typename PolicyUsed>
		ref_type_base(ref::basic_binding_reference<jobject, PolicyUsed> const&
		                  obj) noexcept
		    : obj_{obj} {}

		template <typename PolicyUsed>
		ref_type_base(
		    ref::basic_reference<jobject, PolicyUsed> const& obj) noexcept
		    : obj_{obj} {}

		ref_type_base(jobject obj) noexcept : obj_{obj} {}
		ref::basic_binding_reference<jobject, Policy<jobject>> const& obj()
		    const noexcept {
			return obj_;
		}

	protected:
		ref::basic_binding_reference<jobject, Policy<jobject>> obj_{};
	};

	template <auto& ClassName,
	          typename Package,
	          template <typename> typename Policy = ref::policy::local>
	struct named_type_base : ref_type_base<Policy> {
		using parent_t = ref_type_base<Policy>;
		using parent_t::parent_t;
		using PACKAGE = Package;

		static constexpr auto class_name() noexcept { return ClassName; }
	};

	template <typename CxxClass>
	struct type<CxxClass,
	            std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		static constexpr auto name() noexcept {
			using PACKAGE = typename CxxClass::PACKAGE;
			constexpr auto name = 'L' + PACKAGE::package_name() + '/' +
			                      CxxClass::class_name() + ';';
			return name;
		}
	};

	template <typename CxxClass>
	struct conv<CxxClass,
	            std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		static jobject unpack(CxxClass const& wrapped) {
			return wrapped.obj().get();
		}
		static CxxClass pack(jobject jvm) { return CxxClass{jvm}; }
	};

	template <typename CxxClass>
	struct method_invocation<
	    CxxClass,
	    std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		template <typename... Args>
		static CxxClass call(jobject obj,
		                     jmethodID method,
		                     Args... args) noexcept {
			return CxxClass{ref::jni_env::get_env()->CallObjectMethod(
			    obj, method, conv<Args>::unpack(args)...)};
		}
	};

	template <typename CxxClass>
	struct static_method_invocation<
	    CxxClass,
	    std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		template <typename... Args>
		static CxxClass call(jclass cls,
		                     jmethodID method,
		                     Args... args) noexcept {
			return CxxClass{ref::jni_env::get_env()->CallStaticVoidMethod(
			    cls, method, conv<Args>::unpack(args)...)};
		}
	};

	template <typename CxxClass>
	struct field_access<
	    CxxClass,
	    std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		static CxxClass load(jobject obj, jfieldID field) noexcept {
			return CxxClass{
			    ref::jni_env::get_env()->GetObjectField(obj, field)};
		}

		static void store(jobject obj,
		                  jfieldID field,
		                  CxxClass const& value) noexcept {
			ref::jni_env::get_env()->SetObjectField(obj, field,
			                                        value.obj().get());
		}
	};

	template <typename CxxClass>
	struct static_field_access<
	    CxxClass,
	    std::enable_if_t<std::is_base_of_v<type_base, CxxClass>>> {
		static CxxClass load(jclass cls, jfieldID field) noexcept {
			return CxxClass{
			    ref::jni_env::get_env()->GetStaticObjectField(cls, field)};
		}

		static void store(jclass cls,
		                  jfieldID field,
		                  CxxClass const& value) noexcept {
			ref::jni_env::get_env()->SetStaticObjectField(cls, field,
			                                              value.obj().get());
		}
	};
}  // namespace jni
