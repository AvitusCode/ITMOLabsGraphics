#pragma once
#include "SimpleMath.h"

namespace jd 
{
	class VelocityComponent {
	public:
		[[nodiscard]] DirectX::SimpleMath::Vector2 getPosition() const noexcept { return position_; }
		[[nodiscard]] DirectX::SimpleMath::Vector2 getVelocity() const noexcept { return velocity_; }
		[[nodiscard]] float getRadius() const noexcept { return radius_; }
		void setPosition(const DirectX::SimpleMath::Vector2& pos) noexcept { position_ = pos; }
		void setVelocity(const DirectX::SimpleMath::Vector2& vel) noexcept { velocity_ = vel; }
		void setRadius(float r) noexcept { radius_ = r; }
		
	protected:
		DirectX::SimpleMath::Vector2 position_;
		DirectX::SimpleMath::Vector2 velocity_;
		float radius_;
	};
}