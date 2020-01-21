#pragma once

#include <chrono>
#include <java/time/PACKAGE.hh>
#include <jni/type_base.hh>

namespace java::time {
	inline constexpr auto Duration_name = jni::fixed_string{"Duration"};

	struct Duration : public jni::named_type_base<Duration_name, PACKAGE> {
		using parent = jni::named_type_base<Duration_name, PACKAGE>;
		using parent::obj;
		using parent::parent;

		static Duration ofMillis(std::chrono::milliseconds ms) {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<Duration>()};
			// JNI_STATIC_METHOD_REF(method, "ofMillis", Duration(jlong));
			DEFINE_NAME(method_name__, "ofMillis");
			static jni::static_method<method_name__, Duration(jlong)> method;
			auto const jlong_ms = jlong(ms.count());
			return {cls[method](jlong_ms)};
		}
	};
}  // namespace java::time
