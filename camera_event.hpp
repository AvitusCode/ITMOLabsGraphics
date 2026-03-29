#pragma once
#include "jd_types.hpp"
#include "SimpleMath.h"
#include "static_component.hpp"

namespace jd::event
{
	enum class CameraType : uint32_t
	{
		NO_TYPE = 0u,
		FPS,
		ORBIT
	};

	struct CameraEvent
	{
		float fov;
		StaticInfo info;
		CameraType type;
	};
}