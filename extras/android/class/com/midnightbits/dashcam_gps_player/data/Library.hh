#pragma once

#include <com/midnightbits/dashcam_gps_player/data/PACKAGE.hh>
#include <java/util/List.hh>
#include <vector>

namespace com::midnightbits::dashcam_gps_player::data {
	static constexpr auto Library_name = jni::fixed_string{"Library"};
	static constexpr auto Library_Filter_name = Library_name + "$Filter";
	static constexpr auto Library_Clip_name = Library_name + "$Clip";
	static constexpr auto Library_MediaFile_name = Library_name + "$MediaFile";
	static constexpr auto Library_MediaClip_name = Library_name + "$MediaClip";
	static constexpr auto Library_Trip_name = Library_name + "$Trip";

	struct Library : jni::named_type_base<Library_name, data::PACKAGE> {
	private:
		struct DataHolder;

	public:
		using parent = jni::named_type_base<Library_name, PACKAGE>;
		using parent::parent;

		struct Clip : jni::named_type_base<Library_Clip_name,
		                                   PACKAGE,
		                                   jni::ref::policy::global> {
			using parent = jni::named_type_base<Library_Clip_name,
			                                    PACKAGE,
			                                    jni::ref::policy::global>;
			using parent::parent;

			static std::vector<Clip> values();
		};

		struct MediaFile
		    : jni::named_type_base<Library_MediaFile_name, PACKAGE> {
			using parent =
			    jni::named_type_base<Library_MediaFile_name, PACKAGE>;
			using parent::parent;

			static MediaFile new_object(jni::ref::local<jstring> path,
			                            Clip const& clip);
		};

		struct MediaClip
		    : jni::named_type_base<Library_MediaClip_name, PACKAGE> {
			using parent =
			    jni::named_type_base<Library_MediaClip_name, PACKAGE>;
			using parent::parent;

			static MediaClip new_object(jlong offset,
			                            jlong durationMS,
			                            MediaFile const& file);
		};

		struct Trip : jni::named_type_base<Library_Trip_name, PACKAGE> {
			using parent = jni::named_type_base<Library_Trip_name, PACKAGE>;
			using parent::parent;

			static Trip new_object(java::util::List<MediaClip> const& playlist);
		};

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
