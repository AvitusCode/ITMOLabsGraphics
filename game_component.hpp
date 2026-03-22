#pragma once

namespace jd
{
	class GameComponent abstract
	{
	public:
		virtual ~GameComponent() = default;

		virtual void onInit() = 0;
		virtual void onUpdate(double deltaTime) = 0;
		virtual void onResize() = 0;
		virtual void onReload() = 0;
		virtual void onDestroy() = 0;
		virtual void Draw() = 0;
	};
}