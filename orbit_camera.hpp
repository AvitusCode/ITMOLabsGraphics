#pragma once

#include "camera_strategy_interface.hpp"
#include "static_component.hpp"
#include "SimpleMath.h"

namespace jd
{
	class CameraComponent;

	class OrbitCameraStrategy final : public ICameraStrategy
	{
	public:
		explicit OrbitCameraStrategy(StaticInfo info);
		void Init(CameraComponent& camera) override;
		void Update(CameraComponent& camera, double deltaTime) override;
		void ProcessInput(CameraComponent& camera, const MouseMoveEventArgs& ev) override;

	private:
		const StaticInfo info_;
		float radius_;
		DirectX::SimpleMath::Quaternion rotation_{ DirectX::SimpleMath::Quaternion::Identity };
	};
}