#include "score_component.hpp"
#include "game.hpp"
#include "exception.hpp"

#include <algorithm>
#include <xutility>

using namespace DirectX;
using namespace SimpleMath;

namespace
{
	static constexpr float DIGIT_WIDTH = 0.12f;
	static constexpr float DIGIT_HEIGHT = 0.18f;
	static constexpr float SEGMENT_THICKNESS = 0.02f;
	static constexpr float DIGIT_SPACING = DIGIT_WIDTH * 1.2f;
	static constexpr size_t DIGITS = 14ull; // two digits * 7 segments
	static constexpr int MAX_NUMBER = 99;

	struct SegmentGeo {
		float offsetX;
		float offsetY;
	    float scaleX; 
		float scaleY;
	};

	static constexpr SegmentGeo segGeom[7] = {
		{ 0.0f,  (DIGIT_HEIGHT - SEGMENT_THICKNESS) * 0.5f, DIGIT_WIDTH - 2.0f * SEGMENT_THICKNESS, SEGMENT_THICKNESS },
		{ (DIGIT_WIDTH - SEGMENT_THICKNESS) * 0.5f,  DIGIT_HEIGHT * 0.25f, SEGMENT_THICKNESS, DIGIT_HEIGHT * 0.5f - SEGMENT_THICKNESS },
		{ (DIGIT_WIDTH - SEGMENT_THICKNESS) * 0.5f, -DIGIT_HEIGHT * 0.25f, SEGMENT_THICKNESS, DIGIT_HEIGHT * 0.5f - SEGMENT_THICKNESS },
		{ 0.0f, -(DIGIT_HEIGHT - SEGMENT_THICKNESS) * 0.5f, DIGIT_WIDTH - 2.0f * SEGMENT_THICKNESS, SEGMENT_THICKNESS },
		{ -(DIGIT_WIDTH - SEGMENT_THICKNESS) * 0.5f, -DIGIT_HEIGHT * 0.25f, SEGMENT_THICKNESS, DIGIT_HEIGHT * 0.5f - SEGMENT_THICKNESS },
		{ -(DIGIT_WIDTH - SEGMENT_THICKNESS) * 0.5f,  DIGIT_HEIGHT * 0.25f, SEGMENT_THICKNESS, DIGIT_HEIGHT * 0.5f - SEGMENT_THICKNESS },
		{ 0.0f, 0.0f, DIGIT_WIDTH - 2.0f * SEGMENT_THICKNESS, SEGMENT_THICKNESS }
	};

	static constexpr bool digitSegments[10][7] = {
		{1,1,1,1,1,1,0}, // 0
		{0,1,1,0,0,0,0}, // 1
		{1,1,0,1,1,0,1}, // 2
		{1,1,1,1,0,0,1}, // 3
		{0,1,1,0,0,1,1}, // 4
		{1,0,1,1,0,1,1}, // 5
		{1,0,1,1,1,1,1}, // 6
		{1,1,1,0,0,0,0}, // 7
		{1,1,1,1,1,1,1}, // 8
		{1,1,1,1,0,1,1}  // 9
	};
}

namespace jd
{
	ScoreComponent::ScoreComponent(float pos)
		: QuadComponent(DIGITS) 
		, pos_{ pos }
		, score_{ 0 }
	{
	}

	void ScoreComponent::setScore(int score)
	{
		score_ = std::clamp(score, 0, MAX_NUMBER);
		updateTransforms();
	}

	void ScoreComponent::onInit()
	{
		QuadComponent::onInit();
		updateTransforms();
	}

	void ScoreComponent::onUpdate(double)
	{
	    // pass
	}

	void ScoreComponent::rebuildDigits()
	{
		static constexpr size_t SIZE_SEG = std::size(segGeom);
		const float digitY = topBound_ - DIGIT_HEIGHT * 1.2f;
		const int tens = score_ / 10;
		const int units = score_ % 10;
		const float tensX = pos_ - DIGIT_SPACING * 0.5f;
		const float unitsX = pos_ + DIGIT_SPACING * 0.5f;

		auto setDigit = [&](int digit, float centerX, size_t baseIdx) {
			for (int seg = 0; seg < SIZE_SEG; ++seg) {
				size_t idx = baseIdx + seg;
				if (digitSegments[digit][seg]) {
					Matrix world =
						Matrix::CreateScale(segGeom[seg].scaleX, segGeom[seg].scaleY, 1.0f) *
						Matrix::CreateTranslation(segGeom[seg].offsetX, segGeom[seg].offsetY, 0.0f) *
						Matrix::CreateTranslation(centerX, digitY, 0.0f);
					instances_[idx].world = world;
				}
				else {
					instances_[idx].world = Matrix::CreateScale(0.0f);
				}
			}
			};

		setDigit(tens, tensX, 0);
		setDigit(units, unitsX, SIZE_SEG);
	}

	void ScoreComponent::updateTransforms()
	{
		rebuildDigits();
		updateInstanceBuffer();
	}
}