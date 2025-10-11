#pragma once

#include "Vector.hpp"
#include <vector>

namespace Math
{

Vector3 GetMaxPoint(const Vector3& first, const Vector3& second);
Vector3 GetMinPoint(const Vector3& first, const Vector3& second);

float Factorial(float n);
float BernsteinFunction(size_t n, int i, float t);
float BernsteinDerivative(size_t n, int i, float t);
Vector3 CubicBezier(const std::vector<Vector3>& v, float t);
Vector3 CubicBezier(const std::vector<std::vector<Vector3>>& m, float u, float v);

}