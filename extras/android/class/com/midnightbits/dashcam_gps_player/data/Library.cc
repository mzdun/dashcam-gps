#include <com/midnightbits/dashcam_gps_player/R.hh>
#include <com/midnightbits/dashcam_gps_player/data/Library.hh>
#include <jni/env.hh>
#include <log.hh>
#include <memory>
#include <mgps-70mai/plugin.hh>
#include <mgps/library.hh>
#include <mgps/trip.hh>

MAKE_LOG("mGPS-native")

namespace com::midnightbits::dashcam_gps_player::data {
	struct ViewInfo {
		mgps::page kind;
		int R_type::string_type::*title;
		int R_type::drawable_type::*icon;
		int const zip;
	};

	static constexpr ViewInfo view_info[] = {
	    {mgps::page::everything, &R_type::string_type::page_everything,
	     &R_type::drawable_type::ic_folder_everything, 0},
	    {mgps::page::emergency, &R_type::string_type::page_emergency,
	     &R_type::drawable_type::ic_folder_emergency, 1},
	    {mgps::page::parking, &R_type::string_type::page_parking,
	     &R_type::drawable_type::ic_folder_parking, 2}};

	struct ViewDescr {
		mgps::page kind{};
		int title{};
		int icon{};
		std::vector<mgps::trip> trips{};
	};

	struct Library::DataHolder {
		mgps::library lib_;
		ViewDescr views_[std::size(view_info)];

		DataHolder();

		void before_updates();
		void load_library(mgps::fs::path const& dir);
		void after_updates();
		void export_to_java(Library& parent);
	};

	Library::DataHolder::DataHolder() {
		lib_.make_plugin<mgps::mai::plugin>();

		R_type R;
		R.load();

		for (auto const& source : view_info) {
			auto& descr = views_[source.zip];
			descr.kind = source.kind;
			descr.title = R.string.*source.title;
			descr.icon = R.drawable.*source.icon;
		}
	}

	void Library::DataHolder::before_updates() {
		for (auto& view : views_)
			view.trips.clear();
		lib_.before_update();
	}

	void Library::DataHolder::load_library(mgps::fs::path const& dir) {
		lib_.add_directory(dir);
	}

	void Library::DataHolder::after_updates() {
		lib_.after_update();
		for (auto& view : views_) {
			auto const kind = view.kind;
			auto const gap = kind == mgps::page::everything
			                     ? mgps::library::default_gap
			                     : mgps::library::special_gap;
			view.trips = lib_.build(kind, gap);
		}
	}

	void Library::DataHolder::export_to_java(Library& parent) {
		auto Filters = parent.Filters();
		Filters.clear();
		int id = 0;

		auto env = jni::Env::get().handle();

		std::vector<MediaFile> footage{};
		{
			auto Clips = Clip::values();
			auto const& lib_footage = lib_.footage();
			footage.reserve(lib_footage.size());
			for (auto const& movie : lib_footage) {
				auto type = size_t(movie.type);
				if (type >= Clips.size()) {
					type = size_t(mgps::clip::other);
					if (type >= Clips.size()) {
						type = 0;
					}
				}
				auto const& clip = Clips[type];
				jni::ref::local<jstring> path{
				    env->NewStringUTF(movie.filename.string().c_str())};

				auto jmovie = MediaFile::new_object(path, clip);
				footage.emplace_back(jmovie);
			}
		}

		for (auto const& view : views_) {
			auto info = data::Library::Filter::new_object(
			    id, int(view.kind), view.title, view.icon,
			    int(view.trips.size()));
			Filters.add(info);
		}
	}

	std::vector<Library::Clip> Library::Clip::values() {
		constexpr auto prototype = "()[" + jni::type<Clip>::name();
		auto env = jni::Env::get().handle();
		auto cls = jni::ref::find_class<Clip>();
		static auto valuesId =
		    env->GetStaticMethodID(cls.get(), "values", prototype.c_str());

		auto values = jni::ref::local<jobjectArray>{
		    jobjectArray(env->CallStaticObjectMethod(cls.get(), valuesId))};

		std::vector<Library::Clip> clips;
		auto const count = env->GetArrayLength(values.get());
		clips.reserve(size_t(count));
		for (int index = 0; index < count; ++index) {
			auto value = jni::ref::local<>{
			    env->GetObjectArrayElement(values.get(), index)};
			clips.emplace_back(std::move(value));
		}
		return clips;
	}

	Library::MediaFile Library::MediaFile::new_object(
	    jni::ref::local<jstring> path,
	    Clip const& clip) {
		static jni::ref::binding_global<jclass> cls{
		    jni::ref::find_class<MediaFile>()};
		static jni::constructor<void(jstring, Clip)> init{};
		return {cls[init](path.get(), clip)};
	}

	Library::MediaClip Library::MediaClip::new_object(jlong offset,
	                                                  jlong durationMS,
	                                                  MediaFile const& file) {
		static jni::ref::binding_global<jclass> cls{
		    jni::ref::find_class<MediaClip>()};
		static jni::constructor<void(jlong, jlong, MediaFile const&)> init{};
		return {cls[init](offset, durationMS, file)};
	}

	Library::Trip Library::Trip::new_object(
	    java::util::List<MediaClip> const& playlist) {
		static jni::ref::binding_global<jclass> cls{
		    jni::ref::find_class<Trip>()};
		static jni::constructor<void(java::util::List<MediaClip> const&)> init{};
		return {cls[init](playlist)};
	}

	Library::Filter Library::Filter::new_object(int id,
	                                            int page,
	                                            int title,
	                                            int icon,
	                                            int count) {
		static jni::ref::binding_global<jclass> cls{
		    jni::ref::find_class<Filter>()};
		static jni::constructor<void(jint, jint, jint, jint, jint)> init{};
		return {cls[init](id, page, title, icon, count)};
	}

	java::util::List<Library::Filter> Library::Filters() const noexcept {
		JNI_FIELD_REF(var, "Filters", java::util::List<Filter>);
		return obj()[var].load();
	}

	// JNI API
	void Library::loadDirectories(jobjectArray dirs) {
		using namespace std::chrono;
		using clock = steady_clock;
		using duration = clock::duration;

		DataHolder data{};

		duration load_library{};
		auto start = clock::now();
		data.before_updates();
		auto env = jni::Env::get().handle();
		auto const dirCount = env->GetArrayLength(dirs);
		for (auto index = decltype(dirCount){}; index < dirCount; ++index) {
			auto str = static_cast<jstring>(
			    env->GetObjectArrayElement(dirs, index));  // NOLINT

			auto const length = env->GetStringLength(str);
			auto const chars = env->GetStringChars(str, nullptr);

			static_assert(sizeof(jchar) == sizeof(char16_t),
			              "This code assumes jchar is 16 bit long");
			auto const utf16 = reinterpret_cast<char16_t const*>(chars);

			auto const load_start = clock::now();
			data.load_library(
			    std::u16string_view{utf16, static_cast<size_t>(length)});
			auto const load_stop = clock::now();
			load_library += load_stop - load_start;

			env->ReleaseStringChars(str, chars);
		}
		auto const stop_loop = clock::now();
		data.after_updates();

		auto const updates = clock::now();
		data.export_to_java(*this);
		auto const stop = clock::now();

		auto const loop_dur = floor<milliseconds>(stop_loop - start).count();
		auto const build_dur = floor<milliseconds>(updates - stop_loop).count();
		auto const upload_dur = floor<milliseconds>(stop - stop_loop).count();
		auto const dur = floor<milliseconds>(stop - start).count();
		LOG(DEBUG) << "dur: " << dur << "ms, for: " << loop_dur
		           << "ms, build: " << build_dur << "ms, upload: " << upload_dur
		           << "ms";
	}
}  // namespace com::midnightbits::dashcam_gps_player::data
