#pragma once
#include <cstddef>

namespace jni {
	template <size_t N>
	struct fixed_string {
		char content[N] = {};

		constexpr fixed_string() noexcept = default;

		constexpr fixed_string(const char (&input)[N]) noexcept {
			for (size_t i{0}; i < N; ++i) {
				content[i] = input[i];
			}
		}
		constexpr fixed_string(const fixed_string& other) noexcept {
			for (size_t i{0}; i < N; ++i) {
				content[i] = other.content[i];
			}
		}

		constexpr size_t size() const noexcept { return N - 1; }

		constexpr const char* c_str() const noexcept { return content; }

		constexpr const char* begin() const noexcept { return content; }
		constexpr const char* end() const noexcept { return content + size(); }
		constexpr char operator[](size_t i) const noexcept {
			return content[i];
		}

		template <size_t M>
		constexpr bool is_same_as(const fixed_string<M>& rhs) const noexcept {
			if (size() != rhs.size()) return false;
			for (size_t i{0}; i != size(); ++i) {
				if (content[i] != rhs[i]) return false;
			}
			return true;
		}
	};

	template <>
	class fixed_string<0> {
		static constexpr char __empty[1] = {0};

	public:
		template <typename T>
		constexpr fixed_string(const T*) noexcept {}
		constexpr fixed_string(const fixed_string&) noexcept {}
		constexpr size_t size() const noexcept { return 0; }
		constexpr const char* c_str() const noexcept { return __empty; }
		constexpr const char* begin() const noexcept { return __empty; }
		constexpr const char* end() const noexcept { return __empty + size(); }
		constexpr char32_t operator[](size_t) const noexcept { return 0; }
	};

	template <typename CharT, size_t N>
	fixed_string(const CharT (&)[N])->fixed_string<N>;
	template <size_t N>
	fixed_string(fixed_string<N>)->fixed_string<N>;

	template <size_t L, size_t R>
	constexpr fixed_string<L + R - 1> operator+(fixed_string<L> const& lhs,
	                                            fixed_string<R> const& rhs) {
		fixed_string<L + R - 1> output{};
		for (size_t i{0}; i < L; ++i) {
			output.content[i] = lhs.content[i];
		}
		for (size_t i{0}; i < R; ++i) {
			output.content[i + L - 1] = rhs.content[i];
		}

		return output;
	}

	template <size_t L, size_t R>
	constexpr fixed_string<L + R - 1> operator+(fixed_string<L> const& lhs,
	                                            char const (&rhs)[R]) {
		fixed_string<L + R - 1> output{};
		for (size_t i{0}; i < L; ++i) {
			output.content[i] = lhs.content[i];
		}
		for (size_t i{0}; i < R; ++i) {
			output.content[i + L - 1] = rhs[i];
		}

		return output;
	}

	template <size_t L, size_t R>
	constexpr fixed_string<L + R - 1> operator+(char const (&lhs)[L],
	                                            fixed_string<R> const& rhs) {
		fixed_string<L + R - 1> output{};
		for (size_t i{0}; i < L; ++i) {
			output.content[i] = lhs[i];
		}
		for (size_t i{0}; i < R; ++i) {
			output.content[i + L - 1] = rhs.content[i];
		}

		return output;
	}

	template <size_t L>
	constexpr fixed_string<L + 1> operator+(fixed_string<L> const& lhs,
	                                        char rhs) {
		fixed_string<L + 1> output{};
		for (size_t i{0}; i < L; ++i) {
			output.content[i] = lhs.content[i];
		}
		output.content[L - 1] = rhs;
		output.content[L] = 0;

		return output;
	}

	template <size_t R>
	constexpr fixed_string<R + 1> operator+(char lhs,
	                                        fixed_string<R> const& rhs) {
		fixed_string<R + 1> output{};
		output.content[0] = lhs;
		for (size_t i{0}; i < R; ++i) {
			output.content[i + 1] = rhs.content[i];
		}
		output.content[R] = 0;

		return output;
	}
}  // namespace jni
