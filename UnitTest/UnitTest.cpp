#include "pch.h"
#include "CppUnitTest.h"
#include "precomp.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{

#pragma warning(disable: 4505) // suppressing "unreferenced function with internal linkage has been removed"

#define TOLERANCE		0.0001f

TEST_CLASS(Reflect)
{
public:
	TEST_METHOD(ReflectHorizontalVector)
	{
		float3 i = make_float3(1.0f, -1.0f, 0.0f);
		float3 n = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(1.0f, 1.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}

	TEST_METHOD(ReflectVerticalVector)
	{
		float3 i = make_float3(-1.0f, 0.0f, 0.0f);
		float3 n = make_float3(1.0f, 0.0f, 0.0f);
		float3 expected = make_float3(1.0f, 0.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}

	TEST_METHOD(ReflectDiagonalVector)
	{
		float3 i = make_float3(1.0f, 1.0f, 0.0f);
		float3 n = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(1.0f, -1.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}

	TEST_METHOD(ReflectWithNonUnitNormal)
	{
		float3 i = make_float3(0.0f, -1.0f, 0.0f);
		float3 n = make_float3(0.0f, 2.0f, 0.0f); // Non-unit normal
		float3 expected = make_float3(0.0f, 1.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}

	TEST_METHOD(Reflect3DVector)
	{
		float3 i = make_float3(1.0f, 1.0f, 1.0f);
		float3 n = make_float3(0.0f, 0.0f, 1.0f);
		float3 expected = make_float3(1.0f, 1.0f, -1.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}

	TEST_METHOD(ReflectWithGrazeAngle)
	{
		// Vector almost parallel to the surface
		float3 i = make_float3(0.9999f, -TOLERANCE, 0.0f);
		float3 n = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(0.9999f, TOLERANCE, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
	}

	TEST_METHOD(ReflectWithArbitraryNormal)
	{
		float3 i = make_float3(1.0f, 0.0f, 0.0f);
		float3 n = normalize(make_float3(1.0f, 1.0f, 0.0f)); // 45-degree normal
		float3 expected = make_float3(0.0f, -1.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
	}

	TEST_METHOD(ReflectWithZeroIncident)
	{
		float3 i = make_float3(0.0f, 0.0f, 0.0f);
		float3 n = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(0.0f, 0.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}
};

TEST_CLASS(NormalizeFloat2)
{
public:
	TEST_METHOD(UnitVector)
	{
		float2 v = make_float2(1.0f, 0.0f);
		float2 expected = make_float2(1.0f, 0.0f);
		float2 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float2 v = make_float2(3.0f, 4.0f);
		float2 expected = make_float2(0.6f, 0.8f); // 3/5, 4/5
		float2 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
	}

	TEST_METHOD(Negative)
	{
		float2 v = make_float2(-2.0f, -2.0f);
		float magnitude = sqrtf(8.0f);
		float2 expected = make_float2(-2.0f / magnitude, -2.0f / magnitude);
		float2 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
	}

	TEST_METHOD(Length)
	{
		float2 v = make_float2(5.0f, 12.0f);
		float2 result = normalize(v);
		float length = sqrtf(result.x * result.x + result.y * result.y);

		Assert::AreEqual(1.0f, length, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float2 v = make_float2(NAN, 1.0f);
		float2 result = normalize(v);

		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
	}

	TEST_METHOD(ZeroVector)
	{
		float2 v = make_float2(0.0f, 0.0f);
		float2 result = normalize(v);

		// Expect NaN results since rsqrtf(0) is undefined
		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
	}
};

TEST_CLASS(NormalizeFloat3)
{
public:
	TEST_METHOD(UnitVector)
	{
		float3 v = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(0.0f, 1.0f, 0.0f);
		float3 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float3 v = make_float3(3.0f, 4.0f, 12.0f);
		float magnitude = sqrtf(169.0f); // sqrt(3^2 + 4^2 + 12^2)
		float3 expected = make_float3(3.0f / magnitude, 4.0f / magnitude, 12.0f / magnitude);
		float3 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
	}

	TEST_METHOD(Negative)
	{
		float3 v = make_float3(-1.0f, -1.0f, -1.0f);
		float magnitude = sqrtf(3.0f);
		float3 expected = make_float3(-1.0f / magnitude, -1.0f / magnitude, -1.0f / magnitude);
		float3 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
	}

	TEST_METHOD(Length)
	{
		float3 v = make_float3(2.0f, 3.0f, 6.0f);
		float3 result = normalize(v);
		float length = sqrtf(result.x * result.x + result.y * result.y + result.z * result.z);

		Assert::AreEqual(1.0f, length, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float3 v = make_float3(1.0f, NAN, 1.0f);
		float3 result = normalize(v);

		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
		Assert::IsTrue(isnan(result.z));
	}

	TEST_METHOD(ZeroVector)
	{
		float3 v = make_float3(0.0f, 0.0f, 0.0f);
		float3 result = normalize(v);

		// Expect NaN results since rsqrtf(0) is undefined
		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
		Assert::IsTrue(isnan(result.z));
	}
};

TEST_CLASS(NormalizeFloat4)
{
public:
	TEST_METHOD(UnitVector)
	{
		float4 v = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
		float4 expected = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
		float4 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
		Assert::AreEqual(expected.w, result.w, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float4 v = make_float4(2.0f, 3.0f, 4.0f, 5.0f);
		float magnitude = sqrtf(54.0f); // sqrt(2^2 + 3^2 + 4^2 + 5^2)
		float4 expected = make_float4(2.0f / magnitude, 3.0f / magnitude, 4.0f / magnitude, 5.0f / magnitude);
		float4 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
		Assert::AreEqual(expected.w, result.w, TOLERANCE);
	}

	TEST_METHOD(Negative)
	{
		float4 v = make_float4(-2.0f, 0.0f, -2.0f, 0.0f);
		float magnitude = sqrtf(8.0f);
		float4 expected = make_float4(-2.0f / magnitude, 0.0f, -2.0f / magnitude, 0.0f);
		float4 result = normalize(v);

		Assert::AreEqual(expected.x, result.x, TOLERANCE);
		Assert::AreEqual(expected.y, result.y, TOLERANCE);
		Assert::AreEqual(expected.z, result.z, TOLERANCE);
		Assert::AreEqual(expected.w, result.w, TOLERANCE);
	}

	TEST_METHOD(Length)
	{
		float4 v = make_float4(1.0f, 2.0f, 3.0f, 4.0f);
		float4 result = normalize(v);
		float length = sqrtf(result.x * result.x + result.y * result.y +
							 result.z * result.z + result.w * result.w);

		Assert::AreEqual(1.0f, length, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float4 v = make_float4(1.0f, 1.0f, 1.0f, NAN);
		float4 result = normalize(v);

		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
		Assert::IsTrue(isnan(result.z));
		Assert::IsTrue(isnan(result.w));
	}

	TEST_METHOD(ZeroVector)
	{
		float4 v = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 result = normalize(v);

		// Expect NaN results since rsqrtf(0) is undefined
		Assert::IsTrue(isnan(result.x));
		Assert::IsTrue(isnan(result.y));
		Assert::IsTrue(isnan(result.z));
		Assert::IsTrue(isnan(result.w));
	}
};

TEST_CLASS(LengthFloat2)
{
public:
	TEST_METHOD(ZeroVector)
	{
		float2 v = make_float2(0.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(0.0f, result, TOLERANCE);
	}

	TEST_METHOD(UnitVector)
	{
		float2 v = make_float2(1.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(1.0f, result, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float2 v = make_float2(3.0f, 4.0f);
		float expected = 5.0f; // sqrt(3^2 + 4^2)
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(LargeValues)
	{
		float2 v = make_float2(1e10f, 1e10f);
		float expected = sqrtf(2.0f) * 1e10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(SmallValues)
	{
		float2 v = make_float2(1e-10f, 1e-10f);
		float expected = sqrtf(2.0f) * 1e-10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float2 v = make_float2(1.0f, NAN);
		float result = length(v);

		Assert::IsTrue(isnan(result));
	}

	TEST_METHOD(Infinity_Input)
	{
		float2 v = make_float2(INFINITY, 1.0f);
		float result = length(v);

		Assert::IsTrue(isinf(result));
	}
};

TEST_CLASS(LengthFloat3)
{
public:
	TEST_METHOD(ZeroVector)
	{
		float3 v = make_float3(0.0f, 0.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(0.0f, result, TOLERANCE);
	}

	TEST_METHOD(UnitVector)
	{
		float3 v = make_float3(0.0f, 1.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(1.0f, result, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float3 v = make_float3(3.0f, 4.0f, 12.0f);
		float expected = sqrtf(169.0f); // sqrt(3^2 + 4^2 + 12^2)
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(LargeValues)
	{
		float3 v = make_float3(1e10f, 1e10f, 1e10f);
		float expected = sqrtf(3.0f) * 1e10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(SmallValues)
	{
		float3 v = make_float3(1e-10f, 1e-10f, 1e-10f);
		float expected = sqrtf(3.0f) * 1e-10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float3 v = make_float3(1.0f, NAN, 1.0f);
		float result = length(v);

		Assert::IsTrue(isnan(result));
	}

	TEST_METHOD(Infinity_Input)
	{
		float3 v = make_float3(INFINITY, 1.0f, 1.0f);
		float result = length(v);

		Assert::IsTrue(isinf(result));
	}
};

TEST_CLASS(LengthFloat4)
{
public:
	TEST_METHOD(ZeroVector)
	{
		float4 v = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(0.0f, result, TOLERANCE);
	}

	TEST_METHOD(UnitVector)
	{
		float4 v = make_float4(1.0f, 0.0f, 0.0f, 0.0f);
		float result = length(v);

		Assert::AreEqual(1.0f, result, TOLERANCE);
	}

	TEST_METHOD(Regular)
	{
		float4 v = make_float4(1.0f, 2.0f, 3.0f, 4.0f);
		float expected = sqrtf(30.0f); // sqrt(1^2 + 2^2 + 3^2 + 4^2)
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(LargeValues)
	{
		float4 v = make_float4(1e10f, 1e10f, 1e10f, 1e10f);
		float expected = sqrtf(4.0f) * 1e10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(SmallValues)
	{
		float4 v = make_float4(1e-10f, 1e-10f, 1e-10f, 1e-10f);
		float expected = sqrtf(4.0f) * 1e-10f;
		float result = length(v);

		Assert::AreEqual(expected, result, TOLERANCE);
	}

	TEST_METHOD(NaN_Input)
	{
		float4 v = make_float4(NAN, 1.0f, 1.0f, 1.0f);
		float result = length(v);

		Assert::IsTrue(isnan(result));
	}

	TEST_METHOD(Infinity_Input)
	{
		float4 v = make_float4(INFINITY, 1.0f, 1.0f, 1.0f);
		float result = length(v);

		Assert::IsTrue(isinf(result));
	}
};

} // namespace UnitTest
