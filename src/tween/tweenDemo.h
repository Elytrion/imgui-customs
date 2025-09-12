#pragma once
#include "imguiTween.h"
#include "demo_module.h"

class TweenDemo : public DemoModule
{
public:
	TweenDemo() : DemoModule("Imgui Tween", "Tween Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

void TweenDemo::DrawSelectedDemo()
{

}

void TweenDemo::OnPrePanel()
{

}

void TweenDemo::DrawDemoPanel()
{

}


