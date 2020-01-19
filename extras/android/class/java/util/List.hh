#pragma once

#include <java/util/PACKAGE.hh>
#include <jni/type_base.hh>

namespace java::util {
	inline constexpr auto List_name = jni::fixed_string{"List"};

	template <auto& ClassName = List_name,
	          typename Package = PACKAGE,
	          template <typename> typename Policy = jni::ref::policy::local>
	struct NonGenericList
	    : public jni::named_type_base<ClassName, Package, Policy> {
		using parent = jni::named_type_base<ClassName, Package, Policy>;
		using parent::obj;
		using parent::parent;

		jboolean add(jobject item) { return addId().bind(obj())(item); }

		void clear() { return clearId().bind(obj())(); }

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

	template <typename Item>
	struct List : public NonGenericList<List_name, PACKAGE> {
		using parent = NonGenericList<List_name, PACKAGE>;
		using parent::obj;
		using parent::parent;

		jboolean add(Item const& item) { return parent::add(item.obj().get()); }
	};
}  // namespace java::util
