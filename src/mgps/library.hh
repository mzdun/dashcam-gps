#pragma once
#include <mgps/plugins/plugin_interface.hh>
#include <mgps/video/media_clip.hh>
#include <vector>

namespace mgps {
	enum class page : int { everything, emergency, parking };

	struct trip;
	class library {
	public:
		static constexpr ch::milliseconds default_gap = ch::minutes{10};
		static constexpr ch::milliseconds special_gap = ch::minutes{1};

		library() = default;
		void add_plugin(plugins::ptr plugin) {
			plugins_.emplace_back(std::move(plugin));
		}

		template <typename Plugin, typename... Args>
		void make_plugin(Args&&... args) {
			add_plugin(std::make_unique<Plugin>(std::forward<Args>(args)...));
		}

		void before_update();
		bool add_file(fs::path const&);
		void add_directory(fs::path const&);
		void after_update();
		std::vector<trip> build(page kind, ch::milliseconds max_gap) const;
		media_file const* footage(video::media_clip const& ref) const noexcept;

	private:
		std::vector<plugins::ptr> plugins_{};
		std::vector<media_file> footage_{};
	};
}  // namespace mgps