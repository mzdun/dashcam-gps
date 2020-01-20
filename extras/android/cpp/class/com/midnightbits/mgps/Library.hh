#pragma once

#include <com/midnightbits/mgps/PACKAGE.hh>
#include <java/util/Date.hh>
#include <java/util/List.hh>
#include <vector>

namespace com::midnightbits::mgps {
	static constexpr auto Library_name = jni::fixed_string{"Library"};
	static constexpr auto Clip_name = jni::fixed_string{"Clip"};
	static constexpr auto Page_name = jni::fixed_string{"Page"};

	struct Clip
	    : jni::named_type_base<Clip_name, PACKAGE, jni::ref::policy::global> {
		using parent =
		    jni::named_type_base<Clip_name, PACKAGE, jni::ref::policy::global>;
		using parent::parent;

		static std::vector<Clip> values();
	};

	struct Page
	    : jni::named_type_base<Page_name, PACKAGE, jni::ref::policy::global> {
		using parent =
		    jni::named_type_base<Page_name, PACKAGE, jni::ref::policy::global>;
		using parent::parent;

		static std::vector<Page> values();
	};

	template <typename Final, auto& Name, typename... Vals>
	struct data_class : jni::named_type_base<Name, PACKAGE> {
		using parent = jni::named_type_base<Name, PACKAGE>;
		using parent::parent;
		data_class(Vals... vals) : parent{create_(vals...)} {}

		static jni::ref::local<jobject> create_(Vals... args) {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<Final>()};

			static jni::constructor<void(Vals...)> init{};
			return {cls[init](args...)};
		}

		static Final new_object(Vals... args) {
			static jni::ref::binding_global<jclass> cls{
			    jni::ref::find_class<Final>()};

			static jni::constructor<void(Vals...)> init{};
			return {cls[init](args...)};
		}
	};

#define DATA_CLASS(NAME, ...)                                      \
	static constexpr auto NAME##_name = jni::fixed_string{#NAME};  \
	struct NAME : data_class<NAME, NAME##_name, __VA_ARGS__> {     \
		using parent = data_class<NAME, NAME##_name, __VA_ARGS__>; \
		using parent::parent;                                      \
	};

	DATA_CLASS(Duration, jlong);
	DATA_CLASS(MediaFile,
	           jstring,
	           java::util::Date const&,
	           Duration const&,
	           Clip const&);
	DATA_CLASS(MediaClip, Duration const&, Duration const&, MediaFile const&);
	DATA_CLASS(GpsPoint, jdouble, jdouble, jlong, Duration const&);
	DATA_CLASS(GpsSegment,
	           Duration const&,
	           Duration const&,
	           jlong,
	           java::util::List<GpsPoint> const&);

	DATA_CLASS(GpsTrace, Duration const&, java::util::List<GpsSegment> const&);

	DATA_CLASS(Trip,
	           java::util::Date const&,
	           Duration const&,
	           java::util::List<MediaClip> const&,
	           GpsTrace const&);

	DATA_CLASS(Filter, int, Page const&, java::util::List<Trip> const&);

	struct Library : jni::named_type_base<Library_name, PACKAGE> {
	private:
		class DataHolder;

	public:
		using parent = jni::named_type_base<Library_name, PACKAGE>;
		using parent::parent;

		java::util::List<Filter> Filters() const noexcept;

		// JNI API
		void loadDirectories(jobjectArray dirs);
	};
}  // namespace com::midnightbits::mgps
