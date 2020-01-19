#include <jni/env.hh>
#include <log.hh>

namespace jni {
	MAKE_LOG("Env")

	JavaVM* Env::vm_{nullptr};

	Env::~Env() noexcept {
		if (owned_env_) {
			vm_->DetachCurrentThread();
		}
	}

	Env& Env::get() noexcept {
		thread_local Env current{};
		return current;
	}

	void Env::handle(JNIEnv* ptr) noexcept {
		if (!actual_) {
			actual_ = ptr;
			owned_env_ = false;
		}
	}

	void Env::on_load(JavaVM* vm) noexcept { vm_ = vm; }

	void Env::update_handle() noexcept {
		if (!actual_) {
			if (!vm_) {
				LOG(FATAL) << "Env::get().handle() called from deatched "
				              "thread, but the JavaVM is missing... ";
			}

			JNIEnv* env = nullptr;
			auto owned_env = false;
			auto const result =
			    vm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

			if (result == JNI_EDETACHED) {
				int attach = vm_->AttachCurrentThread(
#if defined(ANDROID) || defined(__ANDROID__)
				    &env,
#else
				    reinterpret_cast<void**>(&env),
#endif
				    nullptr);
				if (attach != JNI_OK) {
					LOG(FATAL)
					    << "Failed to attach JNI to current thread; error: "
					    << attach;
				}

				owned_env = true;
			} else if (result != JNI_OK) {
				LOG(FATAL) << "Failed to get JNIEnv for current thread; error:"
				           << result;
			}
            actual_ = env;
			owned_env_ = owned_env;
		}
	}
};  // namespace jni
