#pragma once
#include <jni.h>

namespace jni {
	class Class;
	class Object {
	public:
		explicit Object(jobject obj = nullptr) : obj_{obj} {}
		Class getClass() const noexcept;
		explicit operator bool() const noexcept { return obj_ != nullptr; }
		explicit operator jobject() const noexcept { return obj_; }
		jobject handle() const noexcept { return obj_; }

		template <typename Caller, typename... Args>
		auto callById(Caller caller, Args... args) const {
			return caller.call(obj_, args...);
		}
		template <typename Field>
		auto loadById(Field fld) const {
			return fld.load(obj_);
		}
		template <typename Field, typename Arg>
		void storeById(Field fld, Arg const& arg) const {
			return fld.store(obj_, arg);
		}

	private:
		jobject obj_;
	};
}