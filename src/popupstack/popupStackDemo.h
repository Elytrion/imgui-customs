#pragma once
#include "PopupStack.h"
#include "demo_module.h"

class PopupStackDemo : public DemoModule
{
	PopupStack m_popupStack;
	PopupHandle pA, pB;
public:
	PopupStackDemo() : DemoModule("Popup Stack", "Popup Stack Demo Panel") {}
	void BackgroundUpdate() override;
protected:
	void DrawSelectedDemo() override;
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void PopupStackDemo::BackgroundUpdate()
{
	m_popupStack.DrawAll();
}

inline void PopupStackDemo::DrawSelectedDemo()
{
	if (ImGui::Button("Open A"))
	{
		pA = m_popupStack.Open("Popup_A", PopupPreset::AUTO_CENTER, [&]() {
			ImGui::Text("Hello from A");

			if (ImGui::Button("Open B"))
			{
				pB = m_popupStack.Open("Popup_B", PopupPreset::AUTO_CENTER, [&]() {
					ImGui::Text("Hello from B");
					if (ImGui::Button("Close A"))
					{
						m_popupStack.Close(pA);
						pB = nullptr;
					}
					ImGui::SameLine();
					if (ImGui::Button("Close B"))
					{
						m_popupStack.Close(pB);
						pB = nullptr;
					}
				});
			}
			if (ImGui::Button("Close"))
			{
				m_popupStack.Close(pA);
				pA = nullptr;
			}
		});
	}


	if (ImGui::Button("Run Popup Sequence"))
	{
		pA = m_popupStack.Open("Popup_A", PopupPreset::AUTO_CENTER, [&]() {
				ImGui::Text("Hello from A");
				if (ImGui::Button("Close"))
				{
					m_popupStack.Close(pA);
					pA = nullptr;
				}
			});

		m_popupStack.Close(pA);

		pB = m_popupStack.Open("Popup_B", PopupPreset::AUTO_CENTER, [&]() {
				ImGui::Text("Hello from B");
				if (ImGui::Button("Close"))
				{
					m_popupStack.Close(pB);
					pB = nullptr;
				}
			});
	}
}

inline void PopupStackDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 600.0f, 550.0f }, ImGuiCond_Appearing);
}

inline void PopupStackDemo::DrawDemoPanel()
{
   
}


