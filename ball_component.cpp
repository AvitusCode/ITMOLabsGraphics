#include "ball_component.hpp"
#include "score_component.hpp"
#include "game.hpp"
#include "logger.hpp"

#include <mmsystem.h>
#include <DirectXMath.h>

#pragma comment(lib, "winmm.lib")

using namespace DirectX;
using namespace SimpleMath;

namespace
{
	static constexpr float SIZE_FACTOR = 0.03f;
	static constexpr float SPEED_MUL = 1.1f;

	void playSound()
	{
		BOOL success = PlaySound(L"./resources/sounds/pong_hit.wav", nullptr, SND_FILENAME | SND_ASYNC);
		if (!success) [[unlikely]] {
			DLOG(JERROR) << "Problems with sound!";
		}
	}
}

namespace jd {
	BallComponent::BallComponent(wptr<ScoreComponent> first, wptr<ScoreComponent> second)
		: QuadComponent(1)
		, first_{ std::move(first) }
		, second_{ std::move(second) }
	{
		setRadius(SIZE_FACTOR * 0.5f);
		reset();
	}

	void BallComponent::addCollider(ICollidable* collider) {
		if (collider) {
			colliders_.push_back(collider);
		}
		else {
			LOG(WARNING) << "Collidable object with nullptr";
		}
	}

	void BallComponent::reset() {
		std::uniform_int_distribution way(-2, 1);
		position_ = Vector2::Zero;
		velocity_ = Vector2(way(rng_) < 0 ? -1.0f : 1.0f, way(rng_) < 0 ? -0.8f : 0.8f) * 0.5f;
	}

	void BallComponent::onUpdate(double deltaTime) {
		const auto dt = static_cast<float>(deltaTime);

		Vector2 newPos = position_ + velocity_ * dt;

		const float halfSize = SIZE_FACTOR * 0.5f;
		// TODO: modernize collision
		if (newPos.y + halfSize > topBound_) {
			newPos.y = topBound_ - halfSize;
			velocity_.y = -velocity_.y;
		}
		else if (newPos.y - halfSize < bottomBound_) {
			newPos.y = bottomBound_ + halfSize;
			velocity_.y = -velocity_.y;
		}

		BoundingBox ballBox;
		ballBox.Center = Vector3(newPos.x, newPos.y, 0.0f);
		ballBox.Extents = Vector3(halfSize, halfSize, 0.0f);

		for (ICollidable* collider : colliders_) {
			auto colliderBox = collider->GetBoundingBox();
			if (ballBox.Intersects(colliderBox)) {
				playSound();

				SimpleMath::Vector2 hitPoint = newPos;
				SimpleMath::Vector2 normal = SimpleMath::Vector2(1.0f, 0.0f);

				collider->OnCollision(*this, hitPoint, normal);
				velocity_ *= SPEED_MUL;
				break; 
			}
		}

		position_ = newPos;

		if (position_.x - halfSize > rightBound_) {
			if (auto first = first_.lock()) {
				first->addScore(1);
				reset();
			}
		}
		else if (position_.x + halfSize < leftBound_) {
			if (auto second = second_.lock()) {
				second->addScore(1);
				reset();
			}
		}

		updateTransforms();
	}

	void BallComponent::updateTransforms() {
		Matrix scale = Matrix::CreateScale(SIZE_FACTOR);
		Matrix trans = Matrix::CreateTranslation(position_.x, position_.y, 0.0f);
		instances_[0].world = scale * trans;
		updateInstanceBuffer();
	}
}