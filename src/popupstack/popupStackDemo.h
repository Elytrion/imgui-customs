#pragma once
#include "PopupManager.h"
#include "demo_module.h"

class PopupStackDemo : public DemoModule
{
	PopupManager m_popupManager;
	PopupHandle pA, pB;
	PopupHandle genericPopup;
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
	m_popupManager.DrawAll();
}

inline void PopupStackDemo::DrawSelectedDemo()
{
	if (ImGui::Button("Open Demo Popup"))
	{
		genericPopup = m_popupManager.Open("Demo Popup", PopupPreset::AUTO_CENTER, [&]() {
			ImGui::Text("Hello from the demo popup!");
			if (ImGui::Button("Close"))
			{
				m_popupManager.Close(genericPopup);
				genericPopup = nullptr;
			}
			});
	}


	if (ImGui::Button("Open A"))
	{
		genericPopup = m_popupManager.Open("Popup_A", PopupPreset::AUTO_CENTER, [&]() {
			ImGui::Text("Hello from A");

			if (ImGui::Button("Close A"))
			{
				m_popupManager.Close(genericPopup);
				genericPopup = m_popupManager.Open("Popup_B", PopupPreset::AUTO_CENTER, [&]()
					{
					ImGui::Text("Hello from B");
					if (ImGui::Button("Close B"))
					{
						m_popupManager.Close(genericPopup);
						genericPopup = nullptr;
					}
				});
			}
		});
	}


	if (ImGui::Button("Run Popup Sequence"))
	{
		pA = m_popupManager.Open("Popup_A", PopupPreset::AUTO_CENTER, [&]() {
				ImGui::Text("Hello from A");
				if (ImGui::Button("Close"))
				{
					m_popupManager.Close(pA);
					pA = nullptr;
				}
			});

		m_popupManager.Close(pA);

		pB = m_popupManager.Open("Popup_B", PopupPreset::AUTO_CENTER, [&]() {
				ImGui::Text("Hello from B");
				if (ImGui::Button("Close"))
				{
					m_popupManager.Close(pB);
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


