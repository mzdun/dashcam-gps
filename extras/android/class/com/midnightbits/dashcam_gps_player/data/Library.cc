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
		for (auto const& view : views_) {
			auto info = data::Library::Filter::new_object(
			    id, int(view.kind), view.title, view.icon,
			    int(view.trips.size()));
			Filters.add(info);
		}
	}

	Library::Filter Library::Filter::new_object(int id,
	                                            int page,
	                                            int title,
	                                            int icon,
	                                            int count) {
		static jni::ref::global<jclass> cls{jni::ref::find_class<Filter>()};
		static jni::constructor<void(jint, jint, jint, jint, jint)> init{};
		return {init.bind(cls)(id, page, title, icon, count)};
	}

	java::util::List<Library::Filter> Library::Filters() const noexcept {
		JNI_FIELD_REF(var, "Filters", java::util::List<Filter>);
		return var.bind(obj()).load();
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
