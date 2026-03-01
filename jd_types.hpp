#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>

namespace jd
{
	template<typename T, typename D = std::default_delete<T>>
	using uptr = std::unique_ptr<T, D>;

	template<typename T>
	using sptr = std::shared_ptr<T>;

	// before c++23
	template<typename Enum>
	constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
		return std::underlying_type_t<Enum>(e);
	}
}