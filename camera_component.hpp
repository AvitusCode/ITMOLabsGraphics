#pragma once

#include "jd_types.hpp"
#include "camera_event.hpp"
#include "game_component.hpp"
#include "camera_strategy_interface.hpp"
#include "SimpleMath.h"

namespace jd
{
	struct MouseMoveEventArgs;

	class CameraComponent final : public GameComponent
	{
	public:
		CameraComponent(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);

		void onInit() override;
		void onUpdate(double deltaTime) override;
		void onResize() override;
		void onReload() override;
		void onDestroy() override;
		void Draw() override;

		void ProcessInput(const MouseMoveEventArgs& event);
		void ProcessCameraEvent(const event::CameraEvent& ev);

		DirectX::SimpleMath::Matrix GetViewMatrix() const noexcept;
		DirectX::SimpleMath::Matrix GetProjectionMatrix() const noexcept;
		DirectX::SimpleMath::Vector3 GetPosition() const noexcept;
		DirectX::SimpleMath::Quaternion GetOrientation() const noexcept;
		float GetSpeed() const noexcept;

		void SetOrientation(const DirectX::SimpleMath::Quaternion& quat);
		void SetPosition(const DirectX::SimpleMath::Vector3& position);
		void LookAt(const DirectX::SimpleMath::Vector3& target, const DirectX::SimpleMath::Vector3& up = DirectX::SimpleMath::Vector3::Up);

		void SetFOV(float fovDegrees);
		void SetAspectRatio(float aspectRatio);
		void SetNearFar(float nearPlane, float farPlane);

	private:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

	private:
		DirectX::SimpleMath::Vector3 position_;
		DirectX::SimpleMath::Quaternion orientation_;

		DirectX::SimpleMath::Matrix viewMatrix_;
		DirectX::SimpleMath::Matrix projectionMatrix_;

		float fovRadians_;
		float aspectRatio_;
		float nearPlane_;
		float farPlane_;
		float moveSpeed_;

		uptr<ICameraStrategy> cameraStrategy_;
	};
}