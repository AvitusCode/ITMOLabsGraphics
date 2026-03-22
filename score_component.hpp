#pragma once
#include "quad_component.hpp"

namespace jd
{
	class ScoreComponent final : public QuadComponent
	{
	public:
		explicit ScoreComponent(float pos);
		void setScore(int score);
		void addScore(int inc) { setScore(score_ + inc); }

		void onInit() override;
		void onUpdate(double deltaTime) override;

	private:
		void updateTransforms() override;
		void rebuildDigits();

		float pos_;
		int score_;
	};
}