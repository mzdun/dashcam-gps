#pragma once
#include <mgps/loader/plugin_interface.hh>
#include <mgps/video/media_clip.hh>
#include <vector>

namespace mgps {
	enum class page : int { everything, emergency, parking };

	struct trip;
	struct MGPS_EXPORT logger {
		virtual ~logger();
		virtual void log(std::string const& msg) = 0;
	};

	class library {
	public:
		static constexpr ch::milliseconds default_gap = ch::minutes{10};
		static constexpr ch::milliseconds special_gap = ch::minutes{1};

		library() = default;
		void MGPS_EXPORT lookup_plugins(std::error_code& ec);
		void add_plugin(loader::ptr plugin) {
			plugins_.emplace_back(std::move(plugin));
		}

		template <typename Plugin, typename... Args>
		void make_plugin(Args&&... args) {
			add_plugin(std::make_unique<Plugin>(std::forward<Args>(args)...));
		}

		void MGPS_EXPORT before_update();
		bool MGPS_EXPORT add_file(fs::path const&, logger* = nullptr);
		void MGPS_EXPORT add_directory(fs::path const&, logger* = nullptr);
		void MGPS_EXPORT after_update();
		std::vector<trip> MGPS_EXPORT build(page kind,
		                                    ch::milliseconds max_gap) const;
		std::vector<media_file> const& footage() const noexcept {
			return footage_;
		}
		MGPS_EXPORT media_file const* footage(
		    video::media_clip const& ref) const noexcept;

		std::vector<loader::ptr> const& plugins() const noexcept {
			return plugins_;
		}

	private:
		void log(std::string const& msg, logger* out) {
			if (out) out->log(msg);
		}
		std::vector<loader::ptr> plugins_{};
		std::vector<media_file> footage_{};
	};
}  // namespace mgps
