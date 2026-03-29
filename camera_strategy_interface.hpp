#pragma once
#include "jd_types.hpp"
#include "SimpleMath.h"

namespace jd
{
	struct MouseMoveEventArgs;
	class CameraComponent;

	class ICameraStrategy abstract
	{
	public:
		virtual ~ICameraStrategy() = default;

		virtual void Init(CameraComponent& camera) = 0;
		virtual void Update(CameraComponent& camera, double deltaTime) = 0;
		virtual void ProcessInput(CameraComponent& camera, const MouseMoveEventArgs& event) = 0;
	};
}