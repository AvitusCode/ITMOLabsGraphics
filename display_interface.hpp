#pragma once

namespace jd
{
	class IDisplay
	{
	public:
		virtual ~IDisplay() = default;
		virtual void Display() = 0;
	};
}