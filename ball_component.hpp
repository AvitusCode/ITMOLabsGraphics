#pragma once
#include "jd_types.hpp"
#include "quad_component.hpp"
#include "velocity_component.hpp"
#include "collidable.hpp"

#include <vector>
#include <random>


namespace jd 
{
	class ScoreComponent;

	class BallComponent final : public QuadComponent, public VelocityComponent {
	public:
		BallComponent(wptr<ScoreComponent> first, wptr<ScoreComponent> second);

		void onUpdate(double deltaTime) override;

		void addCollider(ICollidable* collider);
		void reset();

	private:
		void updateTransforms() override;
		std::mt19937 rng_{ std::random_device{}() };

		wptr<ScoreComponent> first_;
		wptr<ScoreComponent> second_;

		std::vector<ICollidable*> colliders_;
	};
}