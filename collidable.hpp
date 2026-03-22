#pragma once
#include "SimpleMath.h"
#include <DirectXCollision.h>

namespace jd 
{
	class VelocityComponent;

	class ICollidable abstract {
	public:
		virtual ~ICollidable() = default;

		virtual DirectX::BoundingBox GetBoundingBox() const = 0;
		virtual void OnCollision(VelocityComponent& comp, const DirectX::SimpleMath::Vector2& hitPoint, const DirectX::SimpleMath::Vector2& normal) = 0;
	};
}