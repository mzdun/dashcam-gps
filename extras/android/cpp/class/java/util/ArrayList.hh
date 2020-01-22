#pragma once

#include <java/util/List.hh>

namespace java::util {
	DEFINE_CLASS_NAME(ArrayList);

	template <typename Item>
	struct ArrayList
	    : public jni::named_type_base<ArrayList_name, PACKAGE> {

		static List<Item> new_object() {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<ArrayList>()};
			static jni::constructor<void()> init{};
			return {cls[init]()};
		}
	};
}  // namespace java::util
