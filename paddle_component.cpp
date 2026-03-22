#include "paddle_component.hpp"
#include "game.hpp"
#include "input_device.hpp"
#include "velocity_component.hpp"

#include <array>
#include <algorithm>

namespace
{
	static constexpr float PaddleWidth = 0.05f;
	static constexpr float PaddleHeight = 0.3f;
	static constexpr float Speed = 2.0f;
}

using namespace DirectX;
using namespace SimpleMath;

namespace jd {

	PaddleComponent::PaddleComponent(Keys up, Keys down, bool isLeft, float rotSpeer)
		: QuadComponent(1)
		, up_{up}
		, down_{down}
		, isLeft_{isLeft}
		, rotationSpeed_{ rotSpeer }
	{
	}

	void PaddleComponent::onUpdate(double deltaTime) {
		const float dt = static_cast<float>(deltaTime);
		const auto& input = Game::getGame().getInputDevice();

		if (input.isKeyDown(up_)) {
			paddleY_ += Speed * dt;
		}
		if (input.isKeyDown(down_)) {
			paddleY_ -= Speed * dt;
		}

		float halfHeight = PaddleHeight * 0.5f;
		paddleY_ = std::clamp(paddleY_, bottomBound_ + halfHeight, topBound_ - halfHeight);
		currentAngle_ += rotationSpeed_ * dt;

		updateTransforms();
	}

	void PaddleComponent::updateTransforms() {
		float paddleX = isLeft_ ? leftBound_ + PaddleWidth * 0.7f + 0.05f : rightBound_ - PaddleWidth * 0.5f - 0.05f;

		Matrix scale = Matrix::CreateScale(PaddleWidth, PaddleHeight, 1.0f);
		Matrix rot = rotationSpeed_ != float{} ? Matrix::CreateRotationZ(currentAngle_) : Matrix::Identity;
		Matrix trans = Matrix::CreateTranslation(paddleX, paddleY_, 0.0f);
		instances_[0].world = scale * rot * trans;

		updateInstanceBuffer();
	}

	BoundingBox PaddleComponent::GetBoundingBox() const {

		float paddleX = isLeft_ ? leftBound_ + PaddleWidth * 0.5f + 0.05f : rightBound_ - PaddleWidth * 0.5f - 0.05f;
		Vector3 center(paddleX, paddleY_, 0.0f);
		Vector3 extents(PaddleWidth * 0.5f, PaddleHeight * 0.5f, 0.0f);

		static constexpr float angleEpsilon = 1e-6f;
		if (rotationSpeed_ != float{} || std::abs(currentAngle_) < angleEpsilon) {
			BoundingBox box;
			box.Center = center;
			box.Extents = extents;
			return box;
		}

		BoundingOrientedBox obb;
		obb.Center = center;
		obb.Extents = extents;
		obb.Orientation = Quaternion::CreateFromRotationMatrix(Matrix::CreateRotationZ(currentAngle_));

		std::array<Vector3, 8> corners;
		obb.GetCorners(corners.data());

		Vector3 minCorner = corners[0];
		Vector3 maxCorner = corners[0];
		for (int i = 1; i < corners.size(); ++i) {
			minCorner = Vector3::Min(minCorner, corners[i]);
			maxCorner = Vector3::Max(maxCorner, corners[i]);
		}

		BoundingBox aabb;
		aabb.Center = (minCorner + maxCorner) * 0.5f;
		aabb.Extents = (maxCorner - minCorner) * 0.5f;
	}

	void PaddleComponent::OnCollision(VelocityComponent& obj, const Vector2& hitPoint, const Vector2&) {

		Vector2 vel = obj.getVelocity();
		Vector2 pos = obj.getPosition();

		float halfWidth = PaddleWidth * 0.5f;
		float halfHeight = PaddleHeight * 0.5f;

		float paddleX = isLeft_ ? leftBound_ + halfWidth + 0.05f : rightBound_ - halfWidth - 0.05f;
		float paddleY = paddleY_;
		Vector2 center(paddleX, paddleY);

		Vector2 localHit = hitPoint - center;

		Matrix invRot = Matrix::CreateRotationZ(-currentAngle_);
		Vector2 localUnrotated = Vector2::Transform(localHit, invRot);

		float dx = localUnrotated.x / halfWidth;
		float dy = localUnrotated.y / halfHeight;

		static constexpr float eps = 1e-6f;
		float ax = std::abs(dx);
		float ay = std::abs(dy);

		Vector2 N_local;
		if (ax > ay + eps) {
			N_local.x = (dx > 0) ? 1.0f : -1.0f;
			N_local.y = 0.0f;
		}
		else if (ay > ax + eps) {
			N_local.x = 0.0f;
			N_local.y = (dy > 0) ? 1.0f : -1.0f;
		}
		else {
			N_local.x = (dx > 0) ? 1.0f : -1.0f;
			N_local.y = (dy > 0) ? 1.0f : -1.0f;
			N_local.Normalize();
		}


		Matrix rot = Matrix::CreateRotationZ(currentAngle_);
		Vector2 N_world = Vector2::Transform(N_local, rot);
		N_world.Normalize();

		float dot = vel.Dot(N_world);
		Vector2 newVel = vel - 2.0f * dot * N_world;
		obj.setVelocity(newVel);

		float ballRadius = obj.getRadius();
		Vector2 correction = N_world * (ballRadius + 1e-4f);
		Vector2 newPos = hitPoint + correction;
		obj.setPosition(newPos);
	}
}