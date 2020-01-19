#include <com/midnightbits/dashcam_gps_player/R.hh>
#include <jni/field.hh>
#include <jni/ref.hh>

namespace com::midnightbits::dashcam_gps_player {
	using namespace jni;

	template <auto& name>
	static auto find_class() {
		return ref::local<jclass>{
		    ref::jni_env::get_env()->FindClass(name.c_str())};
	}

#define LOAD_VALUE(NAME)                       \
	NAME = [](ref::local<jclass> const& cls) { \
		DEFINE_NAME(name, #NAME);              \
		using fld = static_field<name, jint>;  \
		return fld{}.bind(cls).load();         \
	}(cls);

	void R_type::string_type::load() {
		static constexpr auto R_string_name =
		    PACKAGE::package_name() + "/R$string";

		auto cls = find_class<R_string_name>();

		LOAD_VALUE(page_everything);
		LOAD_VALUE(page_emergency);
		LOAD_VALUE(page_parking);
	}

	void R_type::drawable_type::load() {
		static constexpr auto R_drawable_name =
		    PACKAGE::package_name() + "/R$drawable";

		auto cls = find_class<R_drawable_name>();

		LOAD_VALUE(ic_folder_everything);
		LOAD_VALUE(ic_folder_emergency);
		LOAD_VALUE(ic_folder_parking);
	}

};  // namespace com::midnightbits::dashcam_gps_player