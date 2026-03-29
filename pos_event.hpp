#pragma once
#include "jd_types.hpp"
#include "SimpleMath.h"

namespace jd::event
{
	struct PosEvent
	{
		DirectX::SimpleMath::Matrix world;
		uint32_t indx;
		float time;
		float radius;
	};
}