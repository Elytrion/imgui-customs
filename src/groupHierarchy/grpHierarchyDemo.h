#pragma once
#include "imguiGrpHierarchy.h"
#include "demo_module.h"

class GrpHierarchyDemo : public DemoModule
{
public:
	GrpHierarchyDemo() : DemoModule("Group Hierarchy", "Group Hierarchy Demo Panel") {}
protected:
	void DrawSelectedDemo()
	{
		DrawPlaceholderText();
	}
	void OnPrePanel() override
	{

	}
	void DrawDemoPanel() override
	{
		DrawPlaceholderText();
	}
};
