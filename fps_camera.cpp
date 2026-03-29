#include "fps_camera.hpp"
#include "game.hpp"
#include "SimpleMath.h"
#include "input_device.hpp"
#include "camera_component.hpp"

using namespace DirectX::SimpleMath;

namespace jd
{
	void FpsCameraStrategy::Init(CameraComponent& camera)
	{
		Quaternion q = camera.GetOrientation();

		Vector3 forward = Vector3::Transform(Vector3::Forward, q);
		forward.Normalize();

		pitch_ = DirectX::XMConvertToDegrees(std::asin(forward.y));
		yaw_ = DirectX::XMConvertToDegrees(std::atan2(forward.x, forward.z));
	}

	void FpsCameraStrategy::Update(CameraComponent& camera, double deltaTime)
	{
		auto& input = Game::getGame().getInputDevice();
		float speed = camera.GetSpeed() * static_cast<float>(deltaTime);

		Matrix rot = Matrix::CreateFromQuaternion(camera.GetOrientation());
		Vector3 forward = Vector3::Transform(Vector3::Forward, rot);
		Vector3 right = Vector3::Transform(Vector3::Right, rot);
		Vector3 up = Vector3::Up;

		forward.Normalize();
		right.Normalize();

		auto pos = camera.GetPosition();
		if (input.isKeyDown(Keys::W)) pos = pos + forward * speed;
		if (input.isKeyDown(Keys::S)) pos = pos - forward * speed;
		if (input.isKeyDown(Keys::A)) pos = pos - right * speed;
		if (input.isKeyDown(Keys::D)) pos = pos + right * speed;
		if (input.isKeyDown(Keys::Space)) pos = pos + up * speed;
		if (input.isKeyDown(Keys::Q)) pos = pos - up * speed;
		camera.SetPosition(pos);
	}

	void FpsCameraStrategy::ProcessInput(CameraComponent& camera, const MouseMoveEventArgs& event)
	{
		float dx = static_cast<float>(event.offset.x) * 0.1f;
		float dy = static_cast<float>(event.offset.y) * 0.1f;

		yaw_ -= dx;
		pitch_ -= dy;

		if (pitch_ > 89.0f) pitch_ = 89.0f;
		if (pitch_ < -89.0f) pitch_ = -89.0f;

		Quaternion qYaw = Quaternion::CreateFromAxisAngle(Vector3::Up, DirectX::XMConvertToRadians(yaw_));
		Quaternion qPitch = Quaternion::CreateFromAxisAngle(Vector3::Right, DirectX::XMConvertToRadians(pitch_));

		camera.SetOrientation(qPitch * qYaw);
	}
}