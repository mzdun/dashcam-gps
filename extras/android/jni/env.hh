//
// Created by Marcin on 13.01.2020.
//

#pragma once
#include <jni.h>
#include <jni/type_info.hh>

namespace jni {
	class Object;
	class Class;
	struct basic_ref;
	namespace access {
		class field_base;
		class call_base;
	}

	class Env {
	public:
		class Handle {
			JNIEnv* operator->() const { return get()->real_; }
			explicit operator JNIEnv*() const { return get()->real_; }

			friend class Object;
			friend class Class;
			friend struct basic_ref;
			friend class access::field_base;
			friend class access::call_base;
		};
		static Handle handle() { return {}; }

	protected:
		explicit Env(JNIEnv* real) : real_{real}, prev_{stack_} {
			stack_ = this;
		}
		~Env() { stack_ = prev_; }

		static Env* get() noexcept { return stack_; }

	private:
		JNIEnv* real_{nullptr};
		Env* prev_;
		static Env* stack_;
	};

	class EnvCall : private Env {
	public:
		explicit EnvCall(JNIEnv* env) : Env{env} {}
	};
}