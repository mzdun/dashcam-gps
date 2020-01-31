#include <mgps/library.hh>
#include <mgps/plugins/host/host.hh>
#include <mgps/trip.hh>

namespace mgps {
	namespace {
		class host : public plugins::host::host {
		public:
			explicit host(library& ref) : ref_{&ref} {}

			bool append(plugins::host::library_info info) override {
				plugins::ptr dyn{};
				try {
					dyn = std::make_unique<plugins::host::dynamic_plugin>(
					    std::move(info));
				} catch (std::bad_alloc&) {
					return false;
				}
				ref_->add_plugin(std::move(dyn));
				return true;
			}

		private:
			library* ref_{nullptr};
		};
	}  // namespace

	logger::~logger() {}

	void library::lookup_plugins(std::error_code& ec) {
		host{*this}.lookup_plugins(ec);
	}

	void library::before_update() { footage_.clear(); }

	bool library::add_file(fs::path const& filename, logger* out) {
		footage_.emplace_back();
		auto const filename_str = filename.string();
		for (auto& plugin : plugins_) {
			if (!plugin->probe(filename_str.c_str())) continue;
			auto& back = footage_.back();
			if (!plugin->load(filename_str.c_str(), &back)) {
				back = media_file{};
				continue;
			}
			return true;
		}
		footage_.pop_back();

		return false;
	}

	void library::add_directory(fs::path const& dirname, logger* out) {
		std::error_code ec{};

		auto items = fs::directory_iterator{dirname, ec};
		if (ec) {
			log(dirname.string() + ": " + ec.message(), out);
			return;
		}

		for (auto const& entry : items) {
			if (!entry.is_regular_file(ec) || ec) {
				if (ec) {
					log(entry.path().string() + ": " + ec.message(), out);
				}
				continue;
			}
			add_file(entry.path(), out);
		}
	}

	void library::after_update() {
		std::sort(begin(footage_), end(footage_),
		          [](auto const& lhs, auto const& rhs) {
			          return lhs.date_time < rhs.date_time;
		          });

		local_ms prev{};
		for (auto& clip : footage_) {
			if (prev > clip.date_time) clip.date_time = prev;
			prev = clip.date_time + clip.duration;
		}
	}

	namespace {
		inline bool is_compatible(clip type, page kind) {
			switch (type) {
				case clip::emergency:
					return kind != page::parking;
				case clip::parking:
					return kind != page::emergency;
				default:
					return kind == page::everything;
			}
		}

		inline void create_playlist(std::vector<size_t> const& indices,
		                            std::vector<media_file> const& footage,
		                            video::playlist& playlist) {
			playlist.media.clear();
			playlist.media.reserve(indices.size());

			ch::milliseconds playback{};

			for (auto index : indices) {
				auto& clip = footage[index];

				playlist.media.emplace_back();
				auto& ref = playlist.media.back();

				ref.offset = playback_ms{playback};
				ref.duration = clip.duration;
				ref.reference = index;

				playback += clip.duration;
			}

			playlist.duration = playback;
		}

		class edge {
		public:
			bool detached(local_ms const& current) noexcept {
				auto const prev = previous_;
				auto const first = first_;
				previous_ = current;
				first_ = false;
				return !first && (current - prev) > ch::seconds{1};
			}
			bool detached_with_first(local_ms const& current) noexcept {
				if (first_) {
					first_ = false;
					return true;
				}
				return detached(current);
			}

			bool first() const noexcept { return first_; }

		private:
			bool first_{true};
			local_ms previous_{};
		};

		inline void create_lines(video::playlist const& playlist,
		                         std::vector<media_file> const& footage,
		                         track::trace& trace) {
			{
				size_t reserve_for_lines{};
				edge line{};

				for (auto const& file : playlist.media) {
					auto const& clip = footage[file.reference];
					for (auto& point : clip.points) {
						if (line.detached_with_first(point.offset +
						                             clip.date_time))
							++reserve_for_lines;
					}
				}

				trace.lines.reserve(reserve_for_lines);
			}

			{
				size_t reserve_for_line{};
				edge line{};

				for (auto const& file : playlist.media) {
					auto const& clip = footage[file.reference];
					for (auto& point : clip.points) {
						if (line.detached(point.offset + clip.date_time)) {
							trace.lines.emplace_back();
							trace.lines.back().points.reserve(reserve_for_line);
							reserve_for_line = 0;
						}
						++reserve_for_line;
					}
				}

				if (reserve_for_line) {
					trace.lines.emplace_back();
					trace.lines.back().points.reserve(reserve_for_line);
				}
			}
			{
				size_t line_index{};
				edge line{};

				for (auto const& file : playlist.media) {
					auto const& clip = footage[file.reference];
					for (auto& point : clip.points) {
						if (line.detached(point.offset + clip.date_time))
							++line_index;

						auto copy = point;
						copy.offset += file.offset.time_since_epoch();
						auto& points = trace.lines[line_index].points;
						if (points.empty() ||
						    points.back().offset != copy.offset)
							points.push_back(copy);
						else
							points.back() = copy;
					}
				}
			}

			for (auto& line : trace.lines) {
				using namespace std::literals;
				line.offset = playback_ms{line.points.front().offset};
				line.duration =
				    line.points.empty() ? 0ms : (line.points.size() - 1) * 1s;
				for (auto& point : line.points)
					point.offset -= line.offset.time_since_epoch();
			}

			if (trace.lines.empty()) {
				trace.offset = mgps::playback_ms{};
				trace.duration = ch::milliseconds{};
			} else {
				trace.offset = trace.lines.front().offset;
				for (auto& line : trace.lines)
					line.offset -= trace.offset.time_since_epoch();
				auto& last_line = trace.lines.back();
				trace.duration =
				    last_line.offset.time_since_epoch() + last_line.duration;
			}
		}
	}  // namespace

	std::vector<trip> library::build(page kind,
	                                 ch::milliseconds max_gap) const {
		std::vector<std::vector<size_t>> view{};

		size_t index = 0;
		local_ms prev{};
		for (auto& clip : footage_) {
			struct autoinc {
				size_t* ptr;
				~autoinc() { ++*ptr; }
			} raii{&index};

			if (!is_compatible(clip.type, kind)) continue;

			if (view.empty() || ((prev + max_gap) < clip.date_time))
				view.emplace_back();

			prev = clip.date_time + clip.duration;
			view.back().emplace_back(index);
		}

		std::vector<trip> collection;

		collection.reserve(view.size());
		for (auto& fragment : view) {
			collection.emplace_back();
			auto& trip = collection.back();
			trip.owner = this;

			trip.start = [](std::vector<size_t> const& indices,
			                std::vector<media_file> const& footage) {
				if (indices.empty()) return local_ms{};
				return footage[indices.front()].date_time;
			}(fragment, footage_);

			create_playlist(fragment, footage_, trip.playlist);
			create_lines(trip.playlist, footage_, trip.trace);
		}

		return collection;
	}

	media_file const* library::footage(video::media_clip const& ref) const
	    noexcept {
		if (ref.reference >= footage_.size()) return nullptr;
		return &footage_[ref.reference];
	}

}  // namespace mgps
