#pragma once
#include "quad_component.hpp"
#include "collidable.hpp"
#include "keys.hpp"

namespace jd 
{
	class PaddleComponent final : public QuadComponent, public ICollidable {
	public:
		PaddleComponent(Keys up, Keys down, bool isLeft, float rotSpeer = float{});

		void onUpdate(double deltaTime) override;

		DirectX::BoundingBox GetBoundingBox() const override;
		void OnCollision(VelocityComponent& ballVel, const DirectX::SimpleMath::Vector2& hitPoint, const DirectX::SimpleMath::Vector2&) override;

	private:
		void updateTransforms() override;

		Keys up_;
		Keys down_;

		bool isLeft_;
		float paddleY_{ 0.0f };

		float currentAngle_{ 0.0f }; // radians
		float rotationSpeed_{};
	};
}