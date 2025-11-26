#include <imgui.h>
#include <imgui_internal.h>
#include <functional>

// Flags for ImGui::Tween
enum ImGuiTweenFlags
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
	inline T Tween(const char* id, bool inside, float upDur, float downDur, const T& min, const T& max, ImGuiTweenFlags flags = ImGuiTweenFlags_StartMin,
		std::function<float(float)> easeFunc = nullptr,
		std::function<T(const T&, const T&, float)> lerpFunc = [](const T& a, const T& b, float t) { return ImLerp(a, b, t); })
	{
		ImGuiWindow* window = ImGui::GetCurrentWindowRead(); // prevents the creation of debug windows
		ImGuiStorage* storage = window->DC.StateStorage;
		ImGuiID       keyBase = window->GetID(id);
		ImGuiID       keyTime = keyBase ^ 0x1111; // stores start-time of current tween
		ImGuiID       keyState = keyBase ^ 0x3333; // stores last value of “inside”

		// snapshot keys
		ImGuiID keyInitValid = keyBase ^ 0xAAAA;
		ImGuiID keyInitIns	 = keyBase ^ 0xBBBB;
		ImGuiID keyInitT	 = keyBase ^ 0xCCCC;
		ImGuiID keyInitUp	 = keyBase ^ 0xDDDD;
		ImGuiID keyInitDown  = keyBase ^ 0xEEEE;

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
			storage->SetBool(keyInitValid, false);
		}

		float duration = inside ? upDur : downDur;
		float t = duration > 0.0f
			? ImSaturate((now - startTime) / duration)
			: 1.0f;
		float et = easeFunc ? easeFunc(t) : t;

		// capture initial parameters on first run
		if (!storage->GetBool(keyInitValid, false))
		{
			storage->SetBool(keyInitValid, true);
			storage->SetBool(keyInitIns, inside);
			storage->SetFloat(keyInitT, t);
			storage->SetFloat(keyInitUp, upDur);
			storage->SetFloat(keyInitDown, downDur);
		}

		const T& a = inside ? min : max;
		const T& b = inside ? max : min;

		return lerpFunc(a, b, et);
	}

	inline void ResetTween(const char* id)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindowRead();
		ImGuiStorage* storage = window->DC.StateStorage;
		ImGuiID keyBase = window->GetID(id);
		ImGuiID keyTime = keyBase ^ 0x1111;
		ImGuiID keyState = keyBase ^ 0x3333;

		ImGuiID keyInitValid = keyBase ^ 0xAAAA;
		ImGuiID keyInitIns = keyBase ^ 0xBBBB;
		ImGuiID keyInitT = keyBase ^ 0xCCCC;
		ImGuiID keyInitUp = keyBase ^ 0xDDDD;
		ImGuiID keyInitDown = keyBase ^ 0xEEEE;

		if (!storage->GetBool(keyInitValid, false))
			return; // nothing captured yet

		const bool  init_inside = storage->GetBool(keyInitIns, false);
		const float init_t = ImClamp(storage->GetFloat(keyInitT, 0.0f), 0.0f, 1.0f);
		const float upDur0 = ImMax(storage->GetFloat(keyInitUp, 0.0f), 0.0f);
		const float downDur0 = ImMax(storage->GetFloat(keyInitDown, 0.0f), 0.0f);
		const float dur0 = init_inside ? upDur0 : downDur0;

		// Set logical direction back
		storage->SetBool(keyState, init_inside);

		// Rebuild start time so that next tween computes t == init_t
		float now = ImGui::GetTime();
		if (dur0 > 0.0f)
			storage->SetFloat(keyTime, now - init_t * dur0);
		else
			storage->SetFloat(keyTime, now); // duration == 0 returns t = 1.0 anyway
	}
}
