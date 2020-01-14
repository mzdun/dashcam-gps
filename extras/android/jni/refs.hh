#pragma once
#include <jni.h>
#include <utility>

namespace jni {
	struct basic_ref {
	protected:
		static JNIEnv* handle() noexcept;
	};

	template <typename Storage>
	struct local_ref : basic_ref {
		template <typename... Args>
		local_ref(Args&&... args) : stg_{std::forward<Args>(args)...} {}
		~local_ref() { basic_ref::handle()->DeleteLocalRef(stg_.handle()); }

		Storage& ref() noexcept { return stg_; }
		Storage const& ref() const noexcept { return stg_; }

	private:
		Storage stg_;
	};

	template <typename Storage>
	struct global_ref : basic_ref {
		template <typename... Args>
		global_ref(Args&&... args) : stg_{std::forward<Args>(args)...} {}
		~global_ref() { basic_ref::handle()->DeleteGlobalRef(stg_.handle()); }

		Storage& ref() noexcept { return stg_; }
		Storage const& ref() const noexcept { return stg_; }

	private:
		Storage stg_;
	};
}
