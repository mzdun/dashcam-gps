#pragma once

#include <java/util/PACKAGE.hh>
#include <jni/type_base.hh>
#include <chrono>

namespace java::util {
	DEFINE_CLASS_NAME(Date);

	struct Date : public jni::named_type_base<Date_name, PACKAGE> {
		using parent = jni::named_type_base<Date_name, PACKAGE>;
		using parent::obj;
		using parent::parent;

		static Date new_object(std::chrono::milliseconds ms) {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<Date>()};
			static jni::constructor<void(jlong)> init{};
			auto const jlong_ms = ms.count();
			return {cls[init](jlong_ms)};
		}
	};
}  // namespace java::util
