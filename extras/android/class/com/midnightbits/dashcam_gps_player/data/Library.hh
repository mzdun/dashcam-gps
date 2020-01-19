#pragma once

#include <com/midnightbits/dashcam_gps_player/data/PACKAGE.hh>
#include <java/util/List.hh>

namespace com::midnightbits::dashcam_gps_player::data {
	static constexpr auto Library_name = jni::fixed_string{"Library"};
	static constexpr auto Library_Filter_name = Library_name + "$Filter";

	struct Library : jni::named_type_base<Library_name, data::PACKAGE> {
	private:
		struct DataHolder;

	public:
		using parent = jni::named_type_base<Library_name, PACKAGE>;
		using parent::parent;

		struct Filter : jni::named_type_base<Library_Filter_name, PACKAGE> {
			using parent = jni::named_type_base<Library_Filter_name, PACKAGE>;
			using parent::parent;

			static Filter new_object(int id,
			                         int page,
			                         int title,
			                         int icon,
			                         int count);
		};

		java::util::List<Filter> Filters() const noexcept;

		// JNI API
		void loadDirectories(jobjectArray dirs);
	};
}  // namespace com::midnightbits::dashcam_gps_player::data
