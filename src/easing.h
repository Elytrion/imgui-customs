#pragma once
#include <corecrt_math.h>
#include <corecrt_math_defines.h>

namespace Easing
{
	inline float easeInSine(float t) {
		return 1.0f - cosf((t * M_PI) / 2.0f);
	}

	inline float easeOutSine(float t) {
		return sinf((t * M_PI) / 2.0f);
	}

	inline float easeInOutSine(float t) {
		return -(cosf(M_PI * t) - 1.0f) / 2.0f;
	}

	inline float easeInQuad(float t) {
		return t * t;
	}

	inline float easeOutQuad(float t) {
		return 1.0f - (1.0f - t) * (1.0f - t);
	}

	inline float easeInOutQuad(float t) {
		return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2) / 2.0f;
	}

	inline float easeInCubic(float t) {
		return t * t * t;
	}

	inline float easeOutCubic(float t) {
		return 1.0f - powf(1.0f - t, 3);
	}

	inline float easeInOutCubic(float t) {
		return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3) / 2.0f;
	}

	inline float easeInQuart(float t) {
		return t * t * t * t;
	}

	inline float easeOutQuart(float t) {
		return 1.0f - powf(1.0f - t, 4);
	}

	inline float easeInOutQuart(float t) {
		return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 4) / 2.0f;
	}

	inline float easeInQuint(float t) {
		return t * t * t * t * t;
	}

	inline float easeOutQuint(float t) {
		return 1.0f - powf(1.0f - t, 5);
	}

	inline float easeInOutQuint(float t) {
		return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 5) / 2.0f;
	}

	inline float easeInExpo(float t) {
		return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f);
	}

	inline float easeOutExpo(float t) {
		return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
	}

	inline float easeInOutExpo(float t) {
		if (t == 0.0f || t == 1.0f) return t;
		return t < 0.5f ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
	}

	inline float easeInCirc(float t) {
		return 1.0f - sqrtf(1.0f - t * t);
	}

	inline float easeOutCirc(float t) {
		return sqrtf(1.0f - powf(t - 1.0f, 2));
	}

	inline float easeInOutCirc(float t) {
		if (t < 0.5f) {
			return (1.0f - sqrtf(1.0f - 4.0f * t * t)) / 2.0f;
		}
		else {
			return (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2)) + 1.0f) / 2.0f;
		}
	}

	inline float easeInBack(float t) {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.f;

		return c3 * t * t * t - c1 * t * t;
	}

	inline float easeOutBack(float t) {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.f;
		return 1.0f + c3 * powf(t - 1.0f, 3) + c1 * powf(t - 1.0f, 2);
	}

	inline float easeInOutBack(float t) {
		const float c1 = 1.70158f;
		const float c2 = c1 * 1.525f;
		return (t < 0.5f) ?
			(powf(2.0f * t, 2) * ((c2 + 1.f) * 2.0f * t - c2)) / 2.0f :
			(powf(2.0f * t - 2.0f, 2) * ((c2 + 1.f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
	}

	inline float easeInElastic(float t) {
		const float c4 = (2.0f * M_PI) / 3.0f;
		return t == 0.0f ? 0.0f : (t == 1.0f ? 1.0f : -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4));
	}

	inline float easeOutElastic(float t) {
		const float c4 = (2.0f * M_PI) / 3.0f;
		return t == 0.0f ? 0.0f : (t == 1.0f ? 1.0f : powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f);
	}

	inline float easeInOutElastic(float t) {
		const float c5 = (2.0f * M_PI) / 4.5f;
		if (t == 0.0f || t == 1.0f) return t;
		if (t < 0.5f) {
			return -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f;
		}
		else {
			return (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
		}
	}

	inline float easeOutBounce(float t) {
		const float n1 = 7.5625f;
		const float d1 = 2.75f;
		if (t < 1.f / d1) {
			return n1 * t * t;
		}
		else if (t < 2.f / d1) {
			return n1 * (t -= 1.5f / d1) * t + 0.75f;
		}
		else if (t < 2.5f / d1) {
			return n1 * (t -= 2.25f / d1) * t + 0.9375f;
		}
		else {
			return n1 * (t -= 2.625f / d1) * t + 0.984375f;
		}
	}

	inline float easeInBounce(float t) {
		return 1.0f - easeOutBounce(1.0f - t);
	}

	inline float easeInOutBounce(float t) {
		if (t < 0.5f) {
			return (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f;
		}
		else {
			return (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f;
		}
	}

}