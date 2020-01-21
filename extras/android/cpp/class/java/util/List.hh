#pragma once

#include <java/util/PACKAGE.hh>
#include <jni/type_base.hh>

namespace java::util {
	DEFINE_CLASS_NAME(List);

	template <typename ClassName = List_name,
	          typename Package = PACKAGE,
	          template <typename> typename Policy = jni::ref::policy::local>
	struct NonGenericList
	    : public jni::named_type_base<ClassName, Package, Policy> {
		using parent = jni::named_type_base<ClassName, Package, Policy>;
		using parent::obj;
		using parent::parent;

		jboolean add(jobject item) { return obj()[addId()](item); }

		void clear() { return obj()[clearId()](); }

		void load() {
			auto cls = jni::ref::find_class<NonGenericList>();
			addId().from(cls);
			clearId().from(cls);
		}

	private:
		static auto& addId() {
			JNI_METHOD_REF(method, "add", jboolean(jobject));
			return method;
		}
		static auto& clearId() {
			JNI_METHOD_REF(method, "clear", void());
			return method;
		}
	};

	template <typename Item, typename NonGeneric = NonGenericList<>>
	struct List : public NonGeneric {
		using parent = NonGeneric;
		using parent::obj;
		using parent::parent;

		jboolean add(Item const& item) { return parent::add(item.obj().get()); }
	};
}  // namespace java::util
