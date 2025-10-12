#include "VectorUtils.hpp"

#include <cmath>
#include <algorithm>

namespace engine::math
{
Vector3 GetMaxPoint(const Vector3& first, const Vector3& second)
{
	return Vector3(
		std::max(static_cast<float>(first.GetX()), static_cast<float>(second.GetX())),
		std::max(static_cast<float>(first.GetY()), static_cast<float>(second.GetY())),
		std::max(static_cast<float>(first.GetZ()), static_cast<float>(second.GetZ())));
}

Vector3 GetMinPoint(const Vector3& first, const Vector3& second)
{
	return Vector3(
		std::min(static_cast<float>(first.GetX()), static_cast<float>(second.GetX())),
		std::min(static_cast<float>(first.GetY()), static_cast<float>(second.GetY())),
		std::min(static_cast<float>(first.GetZ()), static_cast<float>(second.GetZ())));
}
float Factorial(float n)
{
	if (IsNear(n, 1) || IsNearZero(n))
		return 1;

	return n * Factorial(n - 1);
}
#ifdef USE_BEZIER
float BernsteinFunction(size_t n, int i, float t)
{
	return Factorial(n) / (Factorial(i) * Factorial(n - i)) * std::powf(t, i) * std::powf(1 - t, n - i);
}
float BernsteinDerivative(size_t n, int i, float t)
{
	return Factorial(n) / (Factorial(i) * Factorial(n - i)) * std::pow(1 - t, n - i) * std::pow(t, i - 1) * (n * t - i)
		/ (t - 1);
}
Vector3 CubicBezier(const std::vector<Vector3>& v, float t)
{
	Vector3 finalPt = Vector3(EZeroTag::kOrigin);

	for (int i = 0; i < v.size(); i++)
	{
		finalPt += (BernsteinFunction(v.size() - 1, i, t) * v[i]);
	}

	return finalPt;
}
Vector3 CubicBezier(const std::vector<std::vector<Vector3>>& m, float u, float v)
{
	Vector3 finalPt = Vector3(EZeroTag::kOrigin);

	for (int i = 0; i < m.size(); i++)
	{
		finalPt += BernsteinFunction(m.size() - 1, i, v) * CubicBezier(m[i], u);
	}

	return finalPt;
}
#endif

}  // namespace engine::math