#pragma once
#include <jni/field.hh>
#include <jni/method.hh>
#include <jni/ref.hh>

namespace jni::ref {
	template <typename JNIReference, typename AcquireReleasePolicy>
	class basic_binding_reference_impl
	    : public basic_reference<JNIReference, AcquireReleasePolicy> {
	public:
		using parent = basic_reference<JNIReference, AcquireReleasePolicy>;
		using pointer = typename parent::pointer;
		using policy_type = typename parent::policy_type;
		constexpr basic_binding_reference_impl() noexcept = default;
		constexpr basic_binding_reference_impl(std::nullptr_t) noexcept {}
		explicit basic_binding_reference_impl(pointer ptr,
		                                      op which = op::take) noexcept
		    : parent{ptr, which} {}

		basic_binding_reference_impl(
		    basic_binding_reference_impl const& rhs) noexcept = default;

		basic_binding_reference_impl& operator=(
		    basic_binding_reference_impl const& rhs) noexcept = default;

		basic_binding_reference_impl(
		    basic_binding_reference_impl&& rhs) noexcept = default;

		basic_binding_reference_impl& operator=(
		    basic_binding_reference_impl&& rhs) noexcept = default;

		template <typename AnotherPolicy>
		basic_binding_reference_impl(
		    basic_binding_reference_impl<pointer, AnotherPolicy> const&
		        rhs) noexcept
		    : parent{rhs} {}

		template <typename AnotherPolicy>
		basic_binding_reference_impl& operator=(
		    basic_binding_reference_impl<pointer, AnotherPolicy> const&
		        rhs) noexcept {
			parent::operator=(rhs);
			return *this;
		}

		template <typename AnotherPolicy>
		basic_binding_reference_impl(
		    basic_binding_reference_impl<pointer, AnotherPolicy>&& rhs) noexcept
		    : parent(std::move(rhs)) {}

		template <typename AnotherPolicy>
		basic_binding_reference_impl& operator=(
		    basic_binding_reference_impl<pointer, AnotherPolicy>&&
		        rhs) noexcept {
			parent::operator=(std::move(rhs));
			return *this;
		}

		template <typename AnotherPolicy>
		basic_binding_reference_impl(
		    basic_reference<pointer, AnotherPolicy> const& rhs) noexcept
		    : parent{rhs} {}

		template <typename AnotherPolicy>
		basic_binding_reference_impl(
		    basic_reference<pointer, AnotherPolicy>&& rhs) noexcept
		    : parent(std::move(rhs)) {}

		template <typename Name, typename Storage>
		typename field<Name, Storage>::template bound_field<pointer> operator[](
		    field<Name, Storage>& fld) const noexcept {
			return {this->get(), fld.from(*this)};
		}

		template <typename Name, typename Prototype>
		typename method<Name, Prototype>::template bound_call<pointer>
		operator[](method<Name, Prototype>& mth) const noexcept {
			return {this->get(), mth.from(*this)};
		}
	};

	template <typename JNIReference, typename AcquireReleasePolicy>
	class basic_binding_reference
	    : public basic_binding_reference_impl<JNIReference,
	                                          AcquireReleasePolicy> {
	public:
		using parent =
		    basic_binding_reference_impl<JNIReference, AcquireReleasePolicy>;
		using parent::parent;
	};

	template <typename AcquireReleasePolicy>
	class basic_binding_reference<jclass, AcquireReleasePolicy>
	    : public basic_binding_reference_impl<jclass, AcquireReleasePolicy> {
	public:
		using parent =
		    basic_binding_reference_impl<jclass, AcquireReleasePolicy>;
		using parent::parent;
		using parent::operator[];
		using pointer = typename parent::pointer;

		template <typename Name, typename Storage>
		typename static_field<Name, Storage>::bound_field
		operator[](static_field<Name, Storage>& fld) const noexcept {
			return {this->get(), fld.from(*this)};
		}

		template <typename Name, typename Prototype>
		typename static_method<Name, Prototype>::bound_call
		operator[](static_method<Name, Prototype>& mth) const noexcept {
			return {this->get(), mth.from(*this)};
		}

		template <typename Prototype>
		typename constructor<Prototype>::bound_call operator[](
		    constructor<Prototype>& ctor) const noexcept {
			return {this->get(), ctor.from(*this)};
		}
	};

	template <typename JNIReference = jobject>
	using binding_unretained =
	    basic_binding_reference<JNIReference, policy::unretained<JNIReference>>;

	template <typename JNIReference = jobject>
	using binding_local =
	    basic_binding_reference<JNIReference, policy::local<JNIReference>>;

	template <typename JNIReference = jobject>
	using binding_global =
	    basic_binding_reference<JNIReference, policy::global<JNIReference>>;

	template <typename JNIReference = jobject>
	using binding_weak =
	    basic_binding_reference<JNIReference, policy::weak<JNIReference>>;
}  // namespace jni::ref

namespace jni {
	template <typename JNIReference, typename AcquireReleasePolicy>
	struct type<
	    ref::basic_binding_reference<JNIReference, AcquireReleasePolicy>>
	    : type<JNIReference> {};

	template <typename JNIReference, typename AcquireReleasePolicy>
	struct conv<
	    ref::basic_binding_reference<JNIReference, AcquireReleasePolicy>> {
		using ref_type =
		    ref::basic_binding_reference<JNIReference, AcquireReleasePolicy>;
		static JNIReference unpack(ref_type const& jni_ref) {
			return jni_ref.get();
		}
		static ref_type pack(JNIReference jvm) { return ref_type{jvm}; }
	};
}  // namespace jni

namespace jni::ref {
	template <typename Policy, typename JNIReference>
	binding_local<jclass> get_class(
	    basic_reference<JNIReference, Policy> const& obj) {
		return binding_local<jclass>{
		    jni_env::get_env()->GetObjectClass(obj.get())};
	}

	binding_global<jclass> find_class(const char* name) noexcept;

	template <typename CxxClass>
	inline binding_global<jclass> find_class() {
		using PACKAGE = typename CxxClass::PACKAGE;
		return find_class(
		    (PACKAGE::package_name() + '/' + CxxClass::class_name()).c_str());
	}
}  // namespace jni::ref
