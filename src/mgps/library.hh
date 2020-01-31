#pragma once
#include <mgps/plugins/plugin_interface.hh>
#include <mgps/video/media_clip.hh>
#include <vector>

namespace mgps {
	enum class page : int { everything, emergency, parking };

	struct trip;
	struct logger {
		virtual ~logger();
		virtual void log(std::string const& msg) = 0;
	};

	class library {
	public:
		static constexpr ch::milliseconds default_gap = ch::minutes{10};
		static constexpr ch::milliseconds special_gap = ch::minutes{1};

		library() = default;
		void lookup_plugins(std::error_code& ec);
		void add_plugin(plugins::ptr plugin) {
			plugins_.emplace_back(std::move(plugin));
		}

		template <typename Plugin, typename... Args>
		void make_plugin(Args&&... args) {
			add_plugin(std::make_unique<Plugin>(std::forward<Args>(args)...));
		}

		void before_update();
		bool add_file(fs::path const&, logger* = nullptr);
		void add_directory(fs::path const&, logger* = nullptr);
		void after_update();
		std::vector<trip> build(page kind, ch::milliseconds max_gap) const;
		std::vector<media_file> const& footage() const noexcept {
			return footage_;
		}
		media_file const* footage(video::media_clip const& ref) const noexcept;

	private:
		void log(std::string const& msg, logger* out) {
			if (out) out->log(msg);
		}
		std::vector<plugins::ptr> plugins_{};
		std::vector<media_file> footage_{};
	};
}  // namespace mgps
