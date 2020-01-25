#include <algorithm>
#include <com/midnightbits/mgps/Library.hh>
#include <java/util/ArrayList.hh>
#include <jni/env.hh>
#include <log.hh>
#include <memory>
#include <mgps-70mai/plugin.hh>
#include <mgps/library.hh>
#include <mgps/trip.hh>

MAKE_LOG("mGPS-native")

namespace com::midnightbits::mgps {
	using namespace ::mgps;

	template <typename Item>
	struct list_add_iterator {
		java::util::List<Item>* target;
		list_add_iterator& operator++() noexcept { return *this; }
		list_add_iterator& operator++(int) noexcept { return *this; }
		list_add_iterator& operator*() noexcept { return *this; }
		list_add_iterator& operator=(Item const& item) {
			if (item.obj() != nullptr) target->add(item);
			return *this;
		}
	};

	template <typename To, typename From>
	To safe_cast(From val) {
		auto const max_val = From(std::numeric_limits<To>::max());
		if (val > max_val) val = max_val;
		return To(val);
	}

	template <typename Item>
	list_add_iterator<Item> list_adder(java::util::List<Item>& target) {
		return {&target};
	}

	struct ViewDescr {
		page kind{};
		std::vector<trip> trips{};
	};

	class Library::DataHolder {
	public:
		DataHolder();

		void before_updates();
		void load_library(fs::path const& dir);
		void after_updates();
		void export_to_java(Library& parent);

	private:
		static java::util::Date toDate(local_ms timestamp) noexcept {
			return java::util::Date::new_object(timestamp.time_since_epoch());
		}

		static Duration toDuration(ch::milliseconds ms) noexcept {
			return {ms.count()};
		}

		template <typename Item, typename Container, typename Transform>
		static java::util::List<Item> clone(Container const& items,
		                                    Transform&& pred) {
			auto result = java::util::ArrayList<Item>::new_object();
			std::transform(std::begin(items), std::end(items),
			               list_adder(result), std::forward<Transform>(pred));
			return result;
		}

		static MediaFile toFile(media_file const& media,
		                        std::vector<Clip> const& Clips) noexcept;
		static Filter toFilter(ViewDescr const& view,
		                       int id,
		                       std::vector<MediaFile> const& footage,
		                       std::vector<Page> const& Pages) noexcept;
		static Trip toTrip(mgps::trip const& trip,
		                   std::vector<MediaFile> const& footage) noexcept;
		static MediaClip toClip(video::media_clip const& clip,
		                        std::vector<MediaFile> const& footage) noexcept;
		static GpsTrace toTrace(track::trace const& trace) noexcept;
		static GpsSegment toSegment(track::polyline const& line) noexcept;
		static GpsPoint toPoint(track::gps_point const& point) noexcept;

		library lib_;
		ViewDescr views_[3]{{page::everything, {}},
		                    {page::emergency, {}},
		                    {page::parking, {}}};
	};

	Library::DataHolder::DataHolder() { lib_.make_plugin<mai::plugin>(); }

	void Library::DataHolder::before_updates() {
		for (auto& view : views_)
			view.trips.clear();
		lib_.before_update();
	}

	void Library::DataHolder::load_library(fs::path const& dir) {
		lib_.add_directory(dir);
	}

	void Library::DataHolder::after_updates() {
		lib_.after_update();
		for (auto& view : views_) {
			auto const kind = view.kind;
			auto const gap = kind == page::everything ? library::default_gap
			                                          : library::special_gap;
			view.trips = lib_.build(kind, gap);
		}
	}

	void Library::DataHolder::export_to_java(Library& parent) {
		auto Filters = parent.Filters();
		Filters.clear();

		auto Clips = Clip::values();
		auto Pages = Page::values();

		std::vector<MediaFile> footage;
		auto const& lib_footage = lib_.footage();

		footage.reserve(lib_footage.size());

		std::transform(std::begin(lib_footage), std::end(lib_footage),
		               std::back_inserter(footage),
		               [&](auto const& movie) { return toFile(movie, Clips); });

		int id = 0;
		std::transform(std::begin(views_), std::end(views_),
		               list_adder(Filters), [&](auto const& view) {
			               return toFilter(view, ++id, footage, Pages);
		               });
	}

	MediaFile Library::DataHolder::toFile(
	    media_file const& movie,
	    std::vector<Clip> const& Clips) noexcept {
		auto type = size_t(movie.type);
		if (type >= Clips.size()) {
			type = size_t(clip::other);
			if (type >= Clips.size()) {
				type = 0;
			}
		}
		auto const& clip = Clips[type];
		jni::ref::local<jstring> path{jni::Env::get().handle()->NewStringUTF(
		    movie.filename.string().c_str())};

		return {path.get(), toDate(movie.date_time), toDuration(movie.duration),
		        clip};
	}

	Filter Library::DataHolder::toFilter(
	    ViewDescr const& view,
	    int id,
	    std::vector<MediaFile> const& footage,
	    std::vector<Page> const& Pages) noexcept {
		auto const index = size_t(view.kind);
		return {id, Pages[index],
		        clone<Trip>(view.trips, [&](auto const& trip) {
			        return toTrip(trip, footage);
		        })};
	}

	Trip Library::DataHolder::toTrip(
	    mgps::trip const& trip,
	    std::vector<MediaFile> const& footage) noexcept {
		auto start = toDate(trip.start);
		auto duration = toDuration(trip.playlist.duration);

		auto playlist = clone<MediaClip>(
		    trip.playlist.media,
		    [&](auto const& clip) { return toClip(clip, footage); });

		return Trip::new_object(start, duration, playlist, toTrace(trip.trace));
	}

	MediaClip Library::DataHolder::toClip(
	    video::media_clip const& clip,
	    std::vector<MediaFile> const& footage) noexcept {
		if (clip.reference >= footage.size()) return nullptr;
		auto const& file_ref = footage[clip.reference];
		return {toDuration(clip.offset.time_since_epoch()),
		        toDuration(clip.duration), file_ref};
	}

	GpsTrace Library::DataHolder::toTrace(track::trace const& trace) noexcept {
		return {toDuration(trace.offset.time_since_epoch()),
		        clone<GpsSegment>(trace.lines, toSegment)};
	}

	GpsSegment Library::DataHolder::toSegment(
	    track::polyline const& line) noexcept {
		auto const distanceInMetres = safe_cast<jlong>(line.distance());
		return {toDuration(line.offset.time_since_epoch()),
		        toDuration(line.duration), distanceInMetres,
		        clone<GpsPoint>(line.points, toPoint)};
	}

	GpsPoint Library::DataHolder::toPoint(
	    track::gps_point const& point) noexcept {
		using namespace std::literals;

		auto const lat = point.lat.as_float();
		auto const lon = point.lon.as_float();
		auto const kmph = safe_cast<jint>(point.kmph.km);

		return {lat, lon, kmph, toDuration(1s)};
	}

	template <typename Enum>
	std::vector<Enum> enum_values() {
		constexpr auto prototype = "()[" + jni::type<Enum>::name();
		auto env = jni::Env::get().handle();
		auto cls = jni::ref::find_class<Enum>();
		static auto valuesId =
		    env->GetStaticMethodID(cls.get(), "values", prototype.c_str());

		auto values = jni::ref::local<jobjectArray>{
		    jobjectArray(env->CallStaticObjectMethod(cls.get(), valuesId))};

		std::vector<Enum> cxx_values;
		auto const count = env->GetArrayLength(values.get());
		cxx_values.reserve(size_t(count));
		for (int index = 0; index < count; ++index) {
			auto value = jni::ref::local<>{
			    env->GetObjectArrayElement(values.get(), index)};
			cxx_values.emplace_back(std::move(value));
		}
		return cxx_values;
	}

	std::vector<Clip> Clip::values() { return enum_values<Clip>(); }

	std::vector<Page> Page::values() { return enum_values<Page>(); }

	java::util::List<Filter> Library::Filters() const noexcept {
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
}  // namespace com::midnightbits::mgps
