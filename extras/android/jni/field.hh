#pragma once
#include <jni.h>
#include <jni/primitive.hh>
#include <jni/type_info.hh>
#include <jni/class.hh>

namespace jni::access {
	class field_base {
	protected:
		explicit field_base(jfieldID field_id) : field_id_{field_id} {}
		JNIEnv* handle() const noexcept;

		jfieldID field_id_{};
	};

	template <typename Type>
	struct field;
#define JNI_FIELD_ACCESS(TYPE, NAME, UNUSED)                             \
	template <>                                                          \
	struct field<TYPE> : field_base {                                    \
		field(jfieldID field_id) : field_base(field_id) {}               \
		TYPE load(jobject obj) noexcept {                                \
			return type<TYPE>::pack(                                     \
			    this->handle()->Get##NAME##Field(obj, this->field_id_)); \
		}                                                                \
		void store(jobject obj, TYPE const& value) noexcept {            \
			this->handle()->Set##NAME##Field(obj, this->field_id_,       \
			                                 type<TYPE>::unpack(value)); \
		}                                                                \
	};

	JNI_PRIMITIVES(JNI_FIELD_ACCESS)
#undef JNI_PRIMITIVE_TYPE

	template <typename CxxClass>
	struct object_field : field_base {
		using wrapper = typename CxxClass::wrapper;
		object_field(jfieldID field_id) : field_base(field_id) {}
		static std::string name() {
			using PACKAGE = typename CxxClass::PACKAGE;
			return "L" + PACKAGE::packageName() + "/" + CxxClass::className() +
			       ";";
		}
		wrapper load(jobject obj) noexcept {
			return type<CxxClass>::pack(
			    this->handle()->GetObjectField(obj, this->field_id_));
		}

		void store(jobject obj, CxxClass const& value) noexcept {
			this->handle()->SetObjectField(obj, this->field_id_,
			                               type<CxxClass>::unpack(value));
		}
	};
}

namespace jni::field {
	template <typename NamePolicy, typename Storage>
	struct ref {
		jfieldID from(Object const& obj) {
			if (!field_id_) from(obj.getClass());

			return field_id_;
		}

		jfieldID from(Class const& cls) {
			if (!field_id_) {
				field_id_ = cls.GetFieldID<Storage>(NamePolicy::get_name());
			}

			return field_id_;
		}

		auto load(Object const& self) {
			this->from(self);
			return self.loadById(access::field<Storage>(field_id_));
		}

		void store(Object const& self, Storage const& value) {
			this->from(self);
			return self.storeById(access::field<Storage>(field_id_), value);
		}

	private:
		jfieldID field_id_ = 0;
	};
}

#define JNI_FIELD_REF(TYPE, NAME)                                    \
	static auto& NAME() noexcept {                                   \
		struct name__ {                                              \
			static char const* get_name() noexcept { return #NAME; } \
		};                                                           \
		static jni::field::ref<name__, TYPE> ref__;                  \
		return ref__;                                                \
	}
