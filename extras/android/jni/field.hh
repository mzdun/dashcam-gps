#pragma once
#include <jni.h>

#include <jni/ref.hh>
#include <jni/type_info.hh>

namespace jni {
	template <typename Type, typename = void>
	struct field_access;
	template <typename Type, typename = void>
	struct static_field_access;
#define JNI_FIELD_ACCESS(TYPE, NAME, UNUSED)                                  \
	template <>                                                               \
	struct field_access<TYPE> {                                               \
		static TYPE load(jobject obj, jfieldID field) noexcept {              \
			return conv<TYPE>::pack(                                          \
			    ref::jni_env::get_env()->Get##NAME##Field(obj, field));       \
		}                                                                     \
		static void store(jobject obj,                                        \
		                  jfieldID field,                                     \
		                  TYPE const& value) noexcept {                       \
			ref::jni_env::get_env()->Set##NAME##Field(                        \
			    obj, field, conv<TYPE>::unpack(value));                       \
		}                                                                     \
	};                                                                        \
	template <>                                                               \
	struct static_field_access<TYPE> {                                        \
		static TYPE load(jclass cls, jfieldID field) noexcept {               \
			return conv<TYPE>::pack(                                          \
			    ref::jni_env::get_env()->GetStatic##NAME##Field(cls, field)); \
		}                                                                     \
		static void store(jclass cls,                                         \
		                  jfieldID field,                                     \
		                  TYPE const& value) noexcept {                       \
			ref::jni_env::get_env()->SetStatic##NAME##Field(                  \
			    cls, field, conv<TYPE>::unpack(value));                       \
		}                                                                     \
	};

	JNI_PRIMITIVES(JNI_FIELD_ACCESS)
#undef JNI_PRIMITIVE_TYPE

	template <typename JNIReference, typename AcquireReleasePolicy>
	struct field_access<
	    ref::basic_reference<JNIReference, AcquireReleasePolicy>> {
		using ref_type =
		    ref::basic_reference<JNIReference, AcquireReleasePolicy>;

		static ref_type load(jobject obj, jfieldID field) noexcept {
			return ref_type{
			    ref::jni_env::get_env()->GetObjectField(obj, field)};
		}

		static void store(jobject obj,
		                  jfieldID field,
		                  ref_type const& value) noexcept {
			ref::jni_env::get_env()->SetObjectField(obj, field, value.get());
		}
	};

	template <typename JNIReference, typename AcquireReleasePolicy>
	struct static_field_access<
	    ref::basic_reference<JNIReference, AcquireReleasePolicy>> {
		using ref_type =
		    ref::basic_reference<JNIReference, AcquireReleasePolicy>;

		static ref_type load(jclass cls, jfieldID field) noexcept {
			return ref_type{
			    ref::jni_env::get_env()->GetStaticObjectField(cls, field)};
		}

		static void store(jclass cls,
		                  jfieldID field,
		                  ref_type const& value) noexcept {
			ref::jni_env::get_env()->SetStaticObjectField(cls, field,
			                                              value.get());
		}
	};

	template <typename JNIReference,
	          template <typename...>
	          typename Access,
	          typename Storage>
	struct bound_field {
		using field_access = Access<Storage>;
		JNIReference ref;
		jfieldID field;

		bound_field() = delete;
		bound_field(JNIReference ref, jfieldID field) noexcept
		    : ref{ref}, field{field} {};

		auto load() noexcept { return field_access::load(ref, field); }
		void store(Storage const& value) noexcept {
			return field_access::store(ref, field, value);
		}
	};

	template <typename Name, typename Storage>
	struct field {
		jfieldID from(ref::local<jobject> const& obj) {
			if (!field_id_) {
				return from(ref::get_class(obj));
			}

			return field_id_;
		}

		jfieldID from(ref::local<jclass> const& cls) {
			if (!field_id_) {
				field_id_ = ref::get_field_id<Name, Storage>(cls.get());
			}

			return field_id_;
		}

		template <typename JNIReference>
		using bound_field =
		    jni::bound_field<JNIReference, field_access, Storage>;

		template <typename JNIReference, typename Policy>
		bound_field<JNIReference> bind(
		    ref::basic_reference<JNIReference, Policy> const& obj) noexcept {
			return {obj.get(), from(obj)};
		};

	protected:
		jfieldID field_id_{nullptr};
	};

	template <typename Name, typename Storage>
	struct static_field {
		jfieldID from(ref::local<jclass> const& cls) {
			if (!field_id_) {
				field_id_ = ref::get_static_field_id<Name, Storage>(cls.get());
			}

			return field_id_;
		}

		using bound_field =
		    jni::bound_field<jclass, static_field_access, Storage>;

		template <typename Policy>
		bound_field bind(
		    ref::basic_reference<jclass, Policy> const& cls) noexcept {
			return {cls.get(), this->from(cls)};
		};

	protected:
		jfieldID field_id_{nullptr};
	};
}  // namespace jni

#define JNI_FIELD_REF(VAR, NAME, TYPE) \
	DEFINE_NAME(VAR##_name__, NAME);   \
	static jni::field<VAR##_name__, TYPE> VAR;
