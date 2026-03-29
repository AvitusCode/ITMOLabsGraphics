#include "orbit_camera.hpp"
#include "game.hpp"
#include "input_device.hpp"
#include "SimpleMath.h"
#include "camera_component.hpp"

#include <algorithm>

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	namespace
	{
		static constexpr float MouseSensitivity = 0.01f;
		static constexpr float RotationSpeed = 1.5f;
		static constexpr float ZoomSpeed = 0.01f;
		static constexpr float MinRadius = 1.0f;
		static constexpr float MaxRadius = 100.0f;
	}

	OrbitCameraStrategy::OrbitCameraStrategy(StaticInfo info)
		: info_{info}
		, radius_{ info_.comp_ptr->getRadius(info_.indx) }
	{ }

	void OrbitCameraStrategy::Init(CameraComponent&)
	{
		rotation_ = Quaternion::Identity;
	}

	void OrbitCameraStrategy::Update(CameraComponent& camera, double deltaTime)
	{
		const auto& input = Game::getGame().getInputDevice();
		const float dt = static_cast<float>(deltaTime);

		Vector3 worldUp = Vector3::Up;

		if (input.isKeyDown(Keys::Q))
		{
			Quaternion q = Quaternion::CreateFromAxisAngle(worldUp, -RotationSpeed * dt);
			rotation_ = q * rotation_;
		}

		if (input.isKeyDown(Keys::E))
		{
			Quaternion q = Quaternion::CreateFromAxisAngle(worldUp, RotationSpeed * dt);
			rotation_ = q * rotation_;
		}

		rotation_.Normalize();

		Vector3 forward = Vector3::Transform(Vector3::Forward, rotation_);
		Vector3 position = info_.comp_ptr->getPos(info_.indx) - forward * radius_;

		camera.SetPosition(position);
		camera.SetOrientation(rotation_);
	}

	void OrbitCameraStrategy::ProcessInput(CameraComponent&, const MouseMoveEventArgs& ev)
	{
		const float dx = static_cast<float>(ev.offset.x) * MouseSensitivity;
		const float dy = static_cast<float>(ev.offset.y) * MouseSensitivity;

		Vector3 right = Vector3::Transform(Vector3::Right, rotation_);
		Vector3 up = Vector3::Transform(Vector3::Up, rotation_);

		Quaternion yaw = Quaternion::CreateFromAxisAngle(up, dx);
		Quaternion pitch = Quaternion::CreateFromAxisAngle(right, dy);

		rotation_ = yaw * rotation_ * pitch;
		rotation_.Normalize();

		if (ev.wheelDelta != 0)
		{
			radius_ -= static_cast<float>(ev.wheelDelta) * ZoomSpeed;
			radius_ = std::clamp(radius_, MinRadius, MaxRadius);
		}
	}
}