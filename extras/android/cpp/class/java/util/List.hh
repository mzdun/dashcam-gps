#pragma once

#include <java/util/PACKAGE.hh>
#include <jni/type_base.hh>

namespace java::util {
	DEFINE_CLASS_NAME(List);

	template <typename Item>
	struct List
	    : public jni::named_type_base<List_name, PACKAGE> {
		using parent = jni::named_type_base<List_name, PACKAGE>;
		using parent::obj;
		using parent::parent;

		jboolean add(Item const& item) { return add(item.obj().get()); }
		jboolean add(jobject item) { return obj()[addId()](item); }

		void clear() { return obj()[clearId()](); }

		void load() {
			auto cls = jni::ref::find_class<List>();
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
}  // namespace java::util
