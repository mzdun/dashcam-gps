#pragma once

#include <java/util/List.hh>

namespace java::util {
	DEFINE_CLASS_NAME(ArrayList);

	template <typename ClassName = ArrayList_name,
	          typename Package = PACKAGE,
	          template <typename> typename Policy = jni::ref::policy::local>
	struct NonGenericArrayList
	    : public jni::named_type_base<ClassName, Package, Policy> {
		using parent = jni::named_type_base<ClassName, Package, Policy>;
		using parent::obj;
		using parent::parent;
	};

	template <typename Item, typename NonGeneric = NonGenericArrayList<>>
	struct ArrayList : public List<Item, NonGeneric> {
		using parent = List<Item, NonGeneric>;
		using parent::obj;
		using parent::parent;

		static List<Item> new_object() {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<ArrayList>()};
			static jni::constructor<void()> init{};
			return {cls[init]()};
		}
	};
}  // namespace java::util
