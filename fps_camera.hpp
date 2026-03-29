#pragma once
#include "camera_strategy_interface.hpp"

namespace jd
{
	class CameraComponent;

	class FpsCameraStrategy final : public ICameraStrategy
	{
	public:
		void Init(CameraComponent& camera) override;
		void Update(CameraComponent& camera, double deltaTime) override;
		void ProcessInput(CameraComponent& camera, const MouseMoveEventArgs& event) override;

	private:
		float yaw_{ 0.0f };
		float pitch_{ 0.0f };
	};
}