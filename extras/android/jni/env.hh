#pragma once
#include <jni.h>

namespace jni {
	class Env {
	public:
		~Env() noexcept;
		static Env& get() noexcept;
		JNIEnv* handle() noexcept {
			if (!actual_) update_handle();
			return actual_;
		}
		void handle(JNIEnv*) noexcept;

		static void on_load(JavaVM*) noexcept;

	private:
		explicit Env() noexcept = default;
		void update_handle() noexcept;

		JNIEnv* actual_{nullptr};
		bool owned_env_{false};
		static JavaVM* vm_;
	};

	class EnvCall {
	public:
		explicit EnvCall(JNIEnv* env) { Env::get().handle(env); }
	};
}  // namespace jni