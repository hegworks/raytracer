#include "pch.h"
#include "CppUnitTest.h"
#include "precomp.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{

#pragma warning(disable: 4505) // suppressing "unreferenced function with internal linkage has been removed"

TEST_CLASS(UnitTest)
{
public:
	TEST_METHOD(Reflect)
	{
		float3 i = make_float3(1.0f, -1.0f, 0.0f);
		float3 n = make_float3(0.0f, 1.0f, 0.0f);
		float3 expected = make_float3(1.0f, 1.0f, 0.0f);
		float3 result = reflect(i, n);
		Assert::AreEqual(expected.x, result.x);
		Assert::AreEqual(expected.y, result.y);
		Assert::AreEqual(expected.z, result.z);
	}
};

}
