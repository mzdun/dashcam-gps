#pragma once

#include <chrono>
#include <cstdint>
#include <cstdlib>

namespace mgps::isom {
	enum : uint64_t {
		UNKNOWN_DURATION_32 = uint32_t(-1),
		UNKNOWN_DURATION_64 = uint64_t(-1)
	};

	struct duration_type {
		uint64_t timescale{};
		uint64_t length{UNKNOWN_DURATION_64};
		std::chrono::milliseconds to_chrono() const noexcept;
	};

	inline constexpr uint32_t mk4cc(uint32_t a,
	                                uint32_t b,
	                                uint32_t c,
	                                uint32_t d) {
		return (a << 24) | (b << 16) | (c << 8) | d;
	}

	inline constexpr uint32_t mk4cc(char const (&fcc)[5]) {
		return mk4cc(static_cast<unsigned char>(fcc[0]),
		             static_cast<unsigned char>(fcc[1]),
		             static_cast<unsigned char>(fcc[2]),
		             static_cast<unsigned char>(fcc[3]));
	}

	using four_cc = unsigned char[4];
	inline constexpr uint32_t mk4cc(four_cc& fcc) {
		return mk4cc(fcc[0], fcc[1], fcc[2], fcc[3]);
	}

	enum class box_type : std::uint32_t;

	inline constexpr box_type synth_type(char const (&fcc)[5]) {
		return static_cast<box_type>(mk4cc(fcc));
	};

	enum class box_type : std::uint32_t {
		// internal code type for unknown boxes
		UNKNOWN = mk4cc("UNKN"),

		co64 = mk4cc("co64"),
		stco = mk4cc("stco"),
		ctts = mk4cc("ctts"),
		cprt = mk4cc("cprt"),
		kind = mk4cc("kind"),
		chpl = mk4cc("chpl"),
		url = mk4cc("url "),
		urn = mk4cc("urn "),
		dinf = mk4cc("dinf"),
		dref = mk4cc("dref"),
		stdp = mk4cc("stdp"),
		edts = mk4cc("edts"),
		elst = mk4cc("elst"),
		uuid = mk4cc("uuid"),
		free = mk4cc("free"),
		hdlr = mk4cc("hdlr"),
		gmhd = mk4cc("gmhd"),
		hmhd = mk4cc("hmhd"),
		hint = mk4cc("hint"),
		mdia = mk4cc("mdia"),
		elng = mk4cc("elng"),
		mdat = mk4cc("mdat"),
		idat = mk4cc("idat"),
		mdhd = mk4cc("mdhd"),
		minf = mk4cc("minf"),
		moov = mk4cc("moov"),
		mvhd = mk4cc("mvhd"),
		stsd = mk4cc("stsd"),
		stsz = mk4cc("stsz"),
		stz2 = mk4cc("stz2"),
		stbl = mk4cc("stbl"),
		stsc = mk4cc("stsc"),
		stsh = mk4cc("stsh"),
		skip = mk4cc("skip"),
		smhd = mk4cc("smhd"),
		stss = mk4cc("stss"),
		stts = mk4cc("stts"),
		trak = mk4cc("trak"),
		tkhd = mk4cc("tkhd"),
		tref = mk4cc("tref"),
		strk = mk4cc("strk"),
		stri = mk4cc("stri"),
		strd = mk4cc("strd"),
		stsg = mk4cc("stsg"),

		udta = mk4cc("udta"),
		vmhd = mk4cc("vmhd"),
		ftyp = mk4cc("ftyp"),
		padb = mk4cc("padb"),
		pdin = mk4cc("pdin"),
		sdtp = mk4cc("sdtp"),
		cslg = mk4cc("cslg"),

		sbgp = mk4cc("sbgp"),
		sgpd = mk4cc("sgpd"),
		saiz = mk4cc("saiz"),
		saio = mk4cc("saio"),
		mfra = mk4cc("mfra"),
		mfro = mk4cc("mfro"),
		tfra = mk4cc("tfra"),

		pssh = mk4cc("pssh"),
		tenc = mk4cc("tenc"),

		// track group
		trgr = mk4cc("trgr"),
		// track group types
		trgt = mk4cc("trgt"),
		msrc = mk4cc("msrc"),
		cstg = mk4cc("cstg"),
		ster = mk4cc("ster"),

		/*Adobe's protection boxes*/
		adkm = mk4cc("adkm"),
		ahdr = mk4cc("ahdr"),
		adaf = mk4cc("adaf"),
		aprm = mk4cc("aprm"),
		aeib = mk4cc("aeib"),
		akey = mk4cc("akey"),
		flxs = mk4cc("flxs"),

#ifndef GPAC_DISABLE_ISOM_FRAGMENTS
		/*Movie Fragments*/
		mvex = mk4cc("mvex"),
		mehd = mk4cc("mehd"),
		trex = mk4cc("trex"),
		trep = mk4cc("trep"),
		moof = mk4cc("moof"),
		mfhd = mk4cc("mfhd"),
		traf = mk4cc("traf"),
		tfhd = mk4cc("tfhd"),
		trun = mk4cc("trun"),
#endif

		/*MP4 extensions*/
		dpnd = mk4cc("dpnd"),
		iods = mk4cc("iods"),
		esds = mk4cc("esds"),
		mpod = mk4cc("mpod"),
		sync = mk4cc("sync"),
		ipir = mk4cc("ipir"),

		nmhd = mk4cc("nmhd"),
		sthd = mk4cc("sthd"),
		/*reseved
		sdhd	= mk4cc("sdhd"),
		odhd	= mk4cc("odhd"),
		crhd	= mk4cc("crhd"),
		*/
		mp4s = mk4cc("mp4s"),
		mp4a = mk4cc("mp4a"),
		mp4v = mk4cc("mp4v"),

		/*AVC / H264 extension*/
		avcC = mk4cc("avcC"),
		btrt = mk4cc("btrt"),
		m4ds = mk4cc("m4ds"),
		pasp = mk4cc("pasp"),
		clap = mk4cc("clap"),
		avc1 = mk4cc("avc1"),
		avc2 = mk4cc("avc2"),
		avc3 = mk4cc("avc3"),
		avc4 = mk4cc("avc4"),
		svcC = mk4cc("svcC"),
		svc1 = mk4cc("svc1"),
		svc2 = mk4cc("svc2"),
		mvcC = mk4cc("mvcC"),
		mvc1 = mk4cc("mvc1"),
		mvc2 = mk4cc("mvc2"),
		mhc1 = mk4cc("mhc1"),
		mhv1 = mk4cc("mhv1"),

		hvcC = mk4cc("hvcC"),
		hvc1 = mk4cc("hvc1"),
		hev1 = mk4cc("hev1"),
		hvt1 = mk4cc("hvt1"),

		hvc2 = mk4cc("hvc2"),
		hev2 = mk4cc("hev2"),
		lhv1 = mk4cc("lhv1"),
		lhe1 = mk4cc("lhe1"),
		lht1 = mk4cc("lht1"),

		lhvC = mk4cc("lhvC"),

		av1C = mk4cc("av1C"),
		av01 = mk4cc("av01"),

		/*WebM*/
		vpcC = mk4cc("vpcC"),
		vp08 = mk4cc("vp08"),
		vp09 = mk4cc("vp09"),
		SmDm = mk4cc("SmDm"),
		CoLL = mk4cc("CoLL"),

		/*Opus*/
		Opus = mk4cc("Opus"),
		dOps = mk4cc("dOps"),

		/*LASeR extension*/
		lsrC = mk4cc("lsrC"),
		lsr1 = mk4cc("lsr1"),

		/*3GPP extensions*/
		damr = mk4cc("damr"),
		d263 = mk4cc("d263"),
		devc = mk4cc("devc"),
		dqcp = mk4cc("dqcp"),
		dsmv = mk4cc("dsmv"),
		tsel = mk4cc("tsel"),

		/* 3GPP Adaptive Streaming extensions */
		styp = mk4cc("styp"),
		tfdt = mk4cc("tfdt"),
		sidx = mk4cc("sidx"),
		ssix = mk4cc("ssix"),
		leva = mk4cc("leva"),
		pcrb = mk4cc("pcrb"),

		/*3GPP text / MPEG-4 StreamingText*/
		ftab = mk4cc("ftab"),
		tx3g = mk4cc("tx3g"),
		styl = mk4cc("styl"),
		hlit = mk4cc("hlit"),
		hclr = mk4cc("hclr"),
		krok = mk4cc("krok"),
		dlay = mk4cc("dlay"),
		href = mk4cc("href"),
		tbox = mk4cc("tbox"),
		blnk = mk4cc("blnk"),
		twrp = mk4cc("twrp"),

		/* ISO Base Media File Format Extensions for MPEG-21 */
		meta = mk4cc("meta"),
		xml = mk4cc("xml "),
		bxml = mk4cc("bxml"),
		iloc = mk4cc("iloc"),
		pitm = mk4cc("pitm"),
		ipro = mk4cc("ipro"),
		infe = mk4cc("infe"),
		iinf = mk4cc("iinf"),
		iref = mk4cc("iref"),
		enca = mk4cc("enca"),
		encv = mk4cc("encv"),
		resv = mk4cc("resv"),
		enct = mk4cc("enct"),
		encs = mk4cc("encs"),
		encf = mk4cc("encf"),
		encm = mk4cc("encm"),
		sinf = mk4cc("sinf"),
		rinf = mk4cc("rinf"),
		frma = mk4cc("frma"),
		schm = mk4cc("schm"),
		schi = mk4cc("schi"),

		stvi = mk4cc("stvi"),

		metx = mk4cc("metx"),
		mett = mk4cc("mett"),

		/* ISMA 1.0 Encryption and Authentication V 1.0 */
		iKMS = mk4cc("iKMS"),
		iSFM = mk4cc("iSFM"),
		iSLT = mk4cc("iSLT"),

		/* Hinting boxes */
		rtp = mk4cc("rtp "),
		srtp = mk4cc("srtp"),
		fdp = mk4cc("fdp "),
		rrtp = mk4cc("rrtp"),
		rtcp = mk4cc("rtcp"),
		hnti = mk4cc("hnti"),
		sdp = mk4cc("sdp "),
		hinf = mk4cc("hinf"),
		name = mk4cc("name"),
		trpy = mk4cc("trpy"),
		nump = mk4cc("nump"),
		totl = mk4cc("totl"),
		npck = mk4cc("npck"),
		tpyl = mk4cc("tpyl"),
		tpay = mk4cc("tpay"),
		maxr = mk4cc("maxr"),
		dmed = mk4cc("dmed"),
		dimm = mk4cc("dimm"),
		drep = mk4cc("drep"),
		tmin = mk4cc("tmin"),
		tmax = mk4cc("tmax"),
		pmax = mk4cc("pmax"),
		dmax = mk4cc("dmax"),
		payt = mk4cc("payt"),
		rely = mk4cc("rely"),
		tims = mk4cc("tims"),
		tsro = mk4cc("tsro"),
		snro = mk4cc("snro"),
		rtpo = mk4cc("rtpo"),
		tssy = mk4cc("tssy"),
		rssr = mk4cc("rssr"),
		srpp = mk4cc("srpp"),

		// FEC boxes
		fiin = mk4cc("fiin"),
		paen = mk4cc("paen"),
		fpar = mk4cc("fpar"),
		fecr = mk4cc("fecr"),
		segr = mk4cc("segr"),
		gitn = mk4cc("gitn"),
		fire = mk4cc("fire"),
		fdsa = mk4cc("fdsa"),
		fdpa = mk4cc("fdpa"),
		extr = mk4cc("extr"),

		/*internal type for track and item references*/
		REFT = mk4cc("REFT"),
		REFI = mk4cc("REFI"),
		GRPT = mk4cc("GRPT"),

#ifndef GPAC_DISABLE_ISOM_ADOBE
		/* Adobe extensions */
		abst = mk4cc("abst"),
		afra = mk4cc("afra"),
		asrt = mk4cc("asrt"),
		afrt = mk4cc("afrt"),
#endif

		/* Apple extensions */

		ilst = mk4cc("ilst"),
		A9nam = mk4cc(0xA9, 'n', 'a', 'm'),
		A9cmt = mk4cc(0xA9, 'c', 'm', 't'),
		A9day = mk4cc(0xA9, 'd', 'a', 'y'),
		A9art = mk4cc(0xA9, 'A', 'R', 'T'),
		A9trk = mk4cc(0xA9, 't', 'r', 'k'),
		A9alb = mk4cc(0xA9, 'a', 'l', 'b'),
		A9com = mk4cc(0xA9, 'c', 'o', 'm'),
		A9wrt = mk4cc(0xA9, 'w', 'r', 't'),
		A9too = mk4cc(0xA9, 't', 'o', 'o'),
		A9cpy = mk4cc(0xA9, 'c', 'p', 'y'),
		A9des = mk4cc(0xA9, 'd', 'e', 's'),
		A9gen = mk4cc(0xA9, 'g', 'e', 'n'),
		A9grp = mk4cc(0xA9, 'g', 'r', 'p'),
		A9enc = mk4cc(0xA9, 'e', 'n', 'c'),
		aART = mk4cc("aART"),
		pgap = mk4cc("pgap"),
		gnre = mk4cc("gnre"),
		disk = mk4cc("disk"),
		trkn = mk4cc("trkn"),
		tmpo = mk4cc("tmpo"),
		cpil = mk4cc("cpil"),
		covr = mk4cc("covr"),
		data = mk4cc("data"),

		mdir = mk4cc("mdir"),
		chap = mk4cc("chap"),
		text = mk4cc("text"),

		/*OMA (P)DCF boxes*/
		ohdr = mk4cc("ohdr"),
		grpi = mk4cc("grpi"),
		mdri = mk4cc("mdri"),
		odtt = mk4cc("odtt"),
		odrb = mk4cc("odrb"),
		odkm = mk4cc("odkm"),
		odaf = mk4cc("odaf"),

		/*3GPP DIMS */
		dims = mk4cc("dims"),
		dimc = mk4cc("dimC"),
		disT = mk4cc("diST"),

		ac3 = mk4cc("ac-3"),
		dac3 = mk4cc("dac3"),
		ec3 = mk4cc("ec-3"),
		dec3 = mk4cc("dec3"),
		dvcc = mk4cc("dvcC"),
		dvhe = mk4cc("dvhe"),

		subs = mk4cc("subs"),

		rvcc = mk4cc("rvcc"),

		vttC = mk4cc("vttC"),
		vttc = mk4cc("vttc"),
		vtte = mk4cc("vtte"),
		vtta = mk4cc("vtta"),
		ctim = mk4cc("ctim"),
		iden = mk4cc("iden"),
		sttg = mk4cc("sttg"),
		payl = mk4cc("payl"),
		wvtt = mk4cc("wvtt"),

		stpp = mk4cc("stpp"),
		sbtt = mk4cc("sbtt"),

		stxt = mk4cc("stxt"),
		txtc = mk4cc("txtC"),

		prft = mk4cc("prft"),

		/* Image File Format Boxes */
		ispe = mk4cc("ispe"),
		colr = mk4cc("colr"),
		pixi = mk4cc("pixi"),
		rloc = mk4cc("rloc"),
		irot = mk4cc("irot"),
		ipco = mk4cc("ipco"),
		iprp = mk4cc("iprp"),
		ipma = mk4cc("ipma"),
		grpl = mk4cc("grpl"),
		ccst = mk4cc("ccst"),
		auxc = mk4cc("auxC"),
		auxi = mk4cc("auxi"),
		oinf = mk4cc("oinf"),
		tols = mk4cc("tols"),

		/* MIAF Boxes */
		clli = mk4cc("clli"),
		mdcv = mk4cc("mdcv"),

		altr = mk4cc("altr"),

		/*ALL INTERNAL BOXES - NEVER WRITTEN TO FILE!!*/

		/*generic handlers*/
		GNRM = mk4cc("GNRM"),
		GNRV = mk4cc("GNRV"),
		GNRA = mk4cc("GNRA"),
		/*base constructor of all hint formats (currently only RTP uses it)*/
		ghnt = mk4cc("ghnt"),
		/*for compatibility with old files hinted for DSS - needs special
		   parsing*/
		void_ = mk4cc("VOID"),

		/*MS Smooth - these are actually UUID boxes*/
		UUID_PSSH = mk4cc("PSSH"),
		UUID_MSSM = mk4cc("MSSM"), /*Stream Manifest box*/
		UUID_TENC = mk4cc("TENC"),
		UUID_TFRF = mk4cc("TFRF"),
		UUID_TFXD = mk4cc("TFXD"),

		mp3 = mk4cc(".mp3"),

		trik = mk4cc("trik"),
		bloc = mk4cc("bloc"),
		ainf = mk4cc("ainf"),

		ihdr = mk4cc("ihdr"),
		jP = mk4cc("jP  "),
		jp2h = mk4cc("jp2h"),
		jp2k = mk4cc("jp2k"),
		jpeg = mk4cc("jpeg"),
		png = mk4cc("png "),

		/* apple QT box */
		alis = mk4cc("alis"),
		wide = mk4cc("wide"),
		gmin = mk4cc("gmin"),
		tapt = mk4cc("tapt"),
		clef = mk4cc("clef"),
		prof = mk4cc("prof"),
		enof = mk4cc("enof"),
		wave = mk4cc("wave"),
		chan = mk4cc("chan"),
		enda = mk4cc("enda"),
		tmcd = mk4cc("tmcd"),
		tcmi = mk4cc("tcmi"),
		fiel = mk4cc("fiel"),
		gama = mk4cc("gama"),
		chrm = mk4cc("chrm"),

		c608 = mk4cc("c608"),
		apch = mk4cc("apch"),
		apco = mk4cc("apco"),
		apcn = mk4cc("apcn"),
		apcs = mk4cc("apcs"),
		apcf = mk4cc("apcf"),
		ap4x = mk4cc("ap4x"),
		ap4h = mk4cc("ap4h"),

		/* from drm_sample.c */
		h264b = mk4cc("264b"),
		h265b = mk4cc("265b"),
		av1b = mk4cc("av1b"),
		vp9b = mk4cc("vp9b"),

		/*MPEG-H 3D audio*/
		mha1 = mk4cc("mha1"),
		mha2 = mk4cc("mha2"),
		mhm1 = mk4cc("mhm1"),
		mhm2 = mk4cc("mhm2"),
		mhaC = mk4cc("mhaC"),

		AUXV = mk4cc("AUXV"),

		/*QTFF audio codes*/
		audio_raw = mk4cc("raw "),
		audio_twos = mk4cc("twos"),
		audio_sowt = mk4cc("sowt"),
		audio_fl32 = mk4cc("fl32"),
		audio_fl64 = mk4cc("fl64"),
		audio_in24 = mk4cc("in24"),
		audio_in32 = mk4cc("in32"),
		audio_ulaw = mk4cc("ulaw"),
		audio_alaw = mk4cc("alaw"),
		audio_ADPCM = mk4cc(0x6D, 0x73, 0x00, 0x02),
		audio_IMA_ADPCM = mk4cc(0x6D, 0x73, 0x00, 0x11),
		audio_dvca = mk4cc("dvca"),
		audio_QDMC = mk4cc("QDMC"),
		audio_QDMC2 = mk4cc("QDM2"),
		audio_Qcelp = mk4cc("Qclp"),
		audio_kMP3 = mk4cc(0x6D, 0x73, 0x00, 0x55),
	};

	struct box_info {
		uint64_t offset;
		uint64_t size;
		box_type type;
	};

	inline std::uint16_t reverse(std::uint16_t in) {
		return uint16_t(((in & 0xFF) << 8) | ((in >> 8) & 0xFF));
	}

	inline std::uint32_t reverse(std::uint32_t in) {
		return ((in & 0xFF) << 24) | (((in >> 8) & 0xFF) << 16) |
		       (((in >> 16) & 0xFF) << 8) | (((in >> 24) & 0xFF));
	}

	inline uint64_t reverse(uint64_t in) {
		return (uint64_t(reverse(uint32_t(in & 0xFFFF'FFFF))) << 32) |
		       reverse(uint32_t((in >> 32) & 0xFFFF'FFFF));
	}

	struct storage {
		virtual ~storage() = default;
		virtual uint64_t offset() const noexcept = 0;
		virtual bool eof() const noexcept = 0;
		virtual uint64_t load(void*, uint64_t) = 0;
		virtual uint64_t tell() = 0;
		virtual uint64_t seek(uint64_t) = 0;
		virtual uint64_t seek_end() = 0;

		bool is_isom();

		template <typename T>
		T get() {
			T out{};
			if (load(&out, sizeof(T)) == sizeof(T)) return out;
			return {};
		}

		uint32_t get_u32() {
			four_cc code;
			static_assert(sizeof(code) == 4, "");
			if (load(code, sizeof(code))) return mk4cc(code);
			return 0xFFFFFFFF;
		}

		uint64_t get_u64() {
			auto const upper = uint64_t(get_u32()) << 32;
			return upper | get_u32();
		}

		box_type get_type() {
			four_cc code;
			static_assert(sizeof(code) == 4, "");
			if (load(code, sizeof(code)))
				return static_cast<box_type>(mk4cc(code));
			return box_type::UNKNOWN;
		}
	};

	class range_storage : public storage {
		storage* stg_;
		uint64_t lower_;
		uint64_t position_;
		uint64_t upper_;

	public:
		range_storage(storage* stg, uint64_t offset, uint64_t size);
		range_storage(storage* stg, box_info const& box)
		    : range_storage{stg, box.offset, box.size} {}
		uint64_t offset() const noexcept override;
		bool eof() const noexcept override;
		uint64_t load(void* buffer, uint64_t length) override;
		uint64_t tell() override;
		uint64_t seek(uint64_t offset) override;
		uint64_t seek_end() override;
	};

	struct boxes {
		bool has_box(storage&);
		box_info const& box() const noexcept { return box_; }

	private:
		box_info box_{};
		uint64_t next_box_at_{0};
	};

	template <typename Final, typename Destination>
	class box_reader {
	public:
		using destination_type = Destination;

		box_reader(Destination& destination) noexcept
		    : destination_{&destination} {}

	protected:
		Destination& data() noexcept { return *destination_; }
		Final& self() noexcept { return *static_cast<Final*>(this); }

	private:
		Destination* destination_;
	};

	template <typename BoxReader>
	inline bool read_box(storage& full,
	                     box_info const& box,
	                     typename BoxReader::destination_type& output) {
		return BoxReader{output}.read(full, box);
	}

	template <typename Final, typename Destination>
	class tree_box_reader : public box_reader<Final, Destination> {
	public:
		using parent_type = box_reader<Final, Destination>;
		using parent_type::parent_type;

		bool read(storage& full, box_info const& box) {
			range_storage bits{&full, box};

			boxes reader{};
			while (reader.has_box(bits)) {
				if (!this->self().on_child(full, reader.box())) return false;
			}
			return true;
		}

		bool on_child(storage& /*full*/, box_info const& /*box*/) {
			return false;
		}
	};

	template <typename Final, typename Destination>
	class leaf_box_reader : public box_reader<Final, Destination> {
	public:
		using parent_type = box_reader<Final, Destination>;
		using parent_type::parent_type;

		bool read(storage& full, box_info const& box) {
			return this->self().on_box(full, box);
		}

		bool on_box(storage& /*full*/, box_info const& /*box*/) {
			return false;
		}
	};
}  // namespace mgps::isom

#define TREE_BOX_READER(NAME, STORAGE_TYPE)                                    \
	struct NAME : ::mgps::isom::tree_box_reader<NAME, STORAGE_TYPE> {          \
		using parent_type = ::mgps::isom::tree_box_reader<NAME, STORAGE_TYPE>; \
		using parent_type::parent_type;                                        \
		inline bool on_child(::mgps::isom::storage& full,                      \
		                     ::mgps::isom::box_info const& box);               \
	};                                                                         \
	bool NAME ::on_child([[maybe_unused]] ::mgps::isom::storage& full,         \
	                     [[maybe_unused]] ::mgps::isom::box_info const& box)

#define LEAF_BOX_READER(NAME, STORAGE_TYPE)                                    \
	struct NAME : ::mgps::isom::leaf_box_reader<NAME, STORAGE_TYPE> {          \
		using parent_type = ::mgps::isom::leaf_box_reader<NAME, STORAGE_TYPE>; \
		using parent_type::parent_type;                                        \
		inline bool on_box(::mgps::isom::storage& full,                        \
		                   ::mgps::isom::box_info const& box);                 \
	};                                                                         \
	bool NAME ::on_box([[maybe_unused]] ::mgps::isom::storage& full,           \
	                   [[maybe_unused]] ::mgps::isom::box_info const& box)

namespace mgps {
	using isom::box_type;
}
