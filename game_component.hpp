#pragma once

namespace jd
{
	class GameComponent
	{
	public:
		virtual ~GameComponent() = default;

		virtual void onInit() = 0;
		virtual void onUpdate(double deltaTime) = 0;
		virtual void onReload() = 0;
		virtual void onDestroy() = 0;
		virtual void Draw() = 0;
	};
}