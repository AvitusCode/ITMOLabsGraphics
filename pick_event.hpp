#pragma once
#include "jd_types.hpp"
#include "SimpleMath.h"

namespace jd::event
{
	struct SimplePickEvent
	{
		DirectX::SimpleMath::Vector4 color;
		uint32_t indx;
	};
}