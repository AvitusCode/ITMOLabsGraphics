#include "line_component.hpp"
#include "game.hpp"
#include "exception.hpp"

#include "SimpleMath.h"

using namespace DirectX;
using namespace SimpleMath;

namespace jd
{
	LineComponent::LineComponent(size_t segmentCount)
		: QuadComponent(segmentCount)
		, segmentCount_(segmentCount)
	{
	}

	void LineComponent::onInit()
	{
		QuadComponent::onInit();
		updateTransforms();
	}

	void LineComponent::onUpdate(double)
	{
		// pass
	}

	void LineComponent::rebuildSegments()
	{
		const float step = (topBound_ - bottomBound_) / static_cast<float>(segmentCount_ + 1);
		for (size_t i = 0; i < segmentCount_; ++i)
		{
			float y = bottomBound_ + static_cast<float>(i + 1) * step;
			Matrix scale = Matrix::CreateScale(LINE_SIZE, LINE_SIZE, 1.0f);
			Matrix trans = Matrix::CreateTranslation(0.0f, y, 0.0f);
			instances_[i].world = scale * trans;
		}
	}

	void LineComponent::updateTransforms()
	{
		rebuildSegments();
		updateInstanceBuffer();
	}
}