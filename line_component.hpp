#pragma once
#include "quad_component.hpp"

namespace jd
{
	class LineComponent final : public QuadComponent
	{
	public:
		inline static constexpr size_t DEFAULT_LINE_COUNT = 30;
		inline static float LINE_SIZE = 0.02f;

		LineComponent(size_t segmentCount = DEFAULT_LINE_COUNT);
		void onInit() override;
		void onUpdate(double deltaTime) override;

	private:
		void updateTransforms() override;
		void rebuildSegments();

		size_t segmentCount_;
	};
}