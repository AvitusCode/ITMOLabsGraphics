#pragma once

#include <windows.h>
#include <memory>
#include <type_traits>

namespace jd
{
	struct MenuDeleter
	{
		void operator()(HMENU hMenu) const
		{
			if (hMenu) {
				DestroyMenu(hMenu);
			}
		}
	};

	using menu_ptr = std::unique_ptr<std::remove_pointer_t<HMENU>, MenuDeleter>;
}