#include <imgui.h>
#include <imgui_internal.h>

// Flags for ImGui::Tween
enum ImGuiTweenFlags_
{
	ImGuiTweenFlags_None = 0,
	ImGuiTweenFlags_StartMin = 0x1, // tween starts defaulted at min value
	ImGuiTweenFlags_StartMax = 0x2, // tween starts defaulted at max value
};

namespace ImGui
{
	// Tween a value between min and max based on the inside state, with customizable durations and easing functions.
	// Each tween is independent and can be controlled by the user, and hence must have a unique ID provided by the user.
	template<typename T>
	T Tween(const char* id, bool inside, float upDur, float downDur, const T& min, const T& max, ImGuiTweenFlags_ flags = ImGuiTweenFlags_StartMin,
		std::function<float(float)> easeFunc = nullptr,
		std::function<T(const T&, const T&, float)> lerpFunc = [](const T& a, const T& b, float t) { return ImLerp(a, b, t); })
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiStorage* storage = window->DC.StateStorage;
		ImGuiID       keyBase = window->GetID(id);
		ImGuiID       keyTime = keyBase ^ 0x1111; // stores start-time of current tween
		ImGuiID       keyState = keyBase ^ 0x3333; // stores last value of “inside”

		// we store the last state and time inside state storage to ensure all tweens are independent
		// without forcing the user to store their own states
		bool defaultInside = (flags & ImGuiTweenFlags_StartMax);
		float defaultStartTime = ImGui::GetTime() - (defaultInside ? upDur : downDur);

		bool   prevInside = storage->GetBool(keyState, defaultInside);
		float  startTime = storage->GetFloat(keyTime, defaultStartTime);
		float  now = ImGui::GetTime();

		if (inside != prevInside)
		{
			startTime = now;
			storage->SetFloat(keyTime, now);
			storage->SetBool(keyState, inside);
		}

		float duration = inside ? upDur : downDur;
		float t = duration > 0.0f
			? ImSaturate((now - startTime) / duration)
			: 1.0f;
		float et = easeFunc ? easeFunc(t) : t;

		const T& a = inside ? min : max;
		const T& b = inside ? max : min;

		return lerpFunc(a, b, et);
	}


}
