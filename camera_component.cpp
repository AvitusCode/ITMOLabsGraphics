#include "camera_component.hpp"

#include "game.hpp"
#include "input_device.hpp"
#include "static_component.hpp"

#include "fps_camera.hpp"
#include "orbit_camera.hpp"

#include "logger.hpp"

#include <windows.h>

using namespace DirectX;
using namespace SimpleMath;

namespace
{
	static constexpr float MoveSpeed = 10.0f;
	static constexpr float MoveSpeedDelta = 1.0f;
	static constexpr float MouseSensitivity = 0.1f;
}

namespace jd
{
	CameraComponent::CameraComponent(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
		: fovRadians_{ fovDegrees * XM_PI / 180.0f }
		, aspectRatio_{ aspectRatio }
		, nearPlane_{ nearPlane }
		, farPlane_{ farPlane }
		, moveSpeed_{ MoveSpeed }
		, position_{ Vector3::Zero }
		, orientation_{ Quaternion::Identity }
	{
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	void CameraComponent::onInit()
	{
		position_ = Vector3(0.0f, 0.0f, -10.0f);
		cameraStrategy_ = std::make_unique<FpsCameraStrategy>();
		cameraStrategy_->Init(*this);
	}

	void CameraComponent::onUpdate(double deltaTime)
	{
		cameraStrategy_->Update(*this, deltaTime);
		UpdateViewMatrix();
		StaticComponent::updateCameraMatrices(viewMatrix_, projectionMatrix_);
	}

	void CameraComponent::onResize()
	{
		const auto& display = Game::getGame().getDisplay();
		SetAspectRatio(display.aspect);
	}

	void CameraComponent::onReload() {}
	void CameraComponent::onDestroy() {}
	void CameraComponent::Draw() {}

	Matrix CameraComponent::GetViewMatrix() const noexcept
	{
		return viewMatrix_;
	}

	Matrix CameraComponent::GetProjectionMatrix() const noexcept
	{
		return projectionMatrix_;
	}

	Vector3 CameraComponent::GetPosition() const noexcept
	{
		return position_;
	}

	Quaternion CameraComponent::GetOrientation() const noexcept
	{
		return orientation_;
	}

	float CameraComponent::GetSpeed() const noexcept
	{
		return moveSpeed_;
	}

	void CameraComponent::SetPosition(const Vector3& position)
	{
		position_ = position;
		UpdateViewMatrix();
	}

	void CameraComponent::SetOrientation(const DirectX::SimpleMath::Quaternion& quat)
	{
		orientation_ = quat;
		orientation_.Normalize();
	}

	void CameraComponent::LookAt(const Vector3& target, const Vector3& up)
	{
		viewMatrix_ = Matrix::CreateLookAt(position_, target, up);
		orientation_ = Quaternion::CreateFromRotationMatrix(viewMatrix_);
	}

	void CameraComponent::SetFOV(float fovDegrees)
	{
		fovRadians_ = fovDegrees * XM_PI / 180.0f;
		UpdateProjectionMatrix();
	}

	void CameraComponent::SetAspectRatio(float aspectRatio)
	{
		aspectRatio_ = aspectRatio;
		UpdateProjectionMatrix();
	}

	void CameraComponent::SetNearFar(float nearPlane, float farPlane)
	{
		nearPlane_ = nearPlane;
		farPlane_ = farPlane;
		UpdateProjectionMatrix();
	}

	void CameraComponent::UpdateViewMatrix()
	{
		Matrix rot = Matrix::CreateFromQuaternion(orientation_);

		Vector3 forward = Vector3::Transform(Vector3::Forward, rot);
		Vector3 up = Vector3::Transform(Vector3::Up, rot);

		viewMatrix_ = Matrix::CreateLookAt(position_, position_ + forward, up);
	}

	void CameraComponent::UpdateProjectionMatrix()
	{
		projectionMatrix_ = Matrix::CreatePerspectiveFieldOfView(
			fovRadians_,
			aspectRatio_,
			nearPlane_,
			farPlane_
		);
	}

	void CameraComponent::ProcessInput(const MouseMoveEventArgs& ev)
	{
		if (ev.wheelDelta > 0) {
			moveSpeed_ += MoveSpeedDelta;
		}
		else if (ev.wheelDelta < 0) {
			moveSpeed_ -= MoveSpeedDelta;
		}

		cameraStrategy_->ProcessInput(*this, ev);
		UpdateViewMatrix();
	}

	void CameraComponent::ProcessCameraEvent(const event::CameraEvent& ev)
	{
		using namespace event;

		switch (ev.type)
		{
		case CameraType::FPS:
			cameraStrategy_ = std::make_unique<FpsCameraStrategy>();
			break;
		case CameraType::ORBIT: {
			auto cam = std::make_unique<OrbitCameraStrategy>(ev.info);
			cameraStrategy_ = std::move(cam);
		}
			break;
		default: {
			LOG(JERROR) << "unspecified strategy!";
				cameraStrategy_.reset();
		}
			break;
		}

		if (cameraStrategy_) {
			cameraStrategy_->Init(*this);
		}
	}
}