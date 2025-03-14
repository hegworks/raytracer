#pragma once

// random number generator - Marsaglia's xor32
// This is a high-quality RNG that uses a single 32-bit seed. More info:
// https://www.researchgate.net/publication/5142825_Xorshift_RNGs

// RNG seed. NOTE: in a multithreaded application, don't use a single seed!
class RNG
{
public:
// WangHash: calculates a high-quality seed based on an arbitrary non-zero
// integer. Use this to create your own seed based on e.g. thread index.
	uint WangHash(uint s)
	{
		s = (s ^ 61) ^ (s >> 16);
		s *= 9, s = s ^ (s >> 4);
		s *= 0x27d4eb2d;
		s = s ^ (s >> 15);
		return s;
	}
	uint InitSeed(const uint seedBase)
	{
		return WangHash((seedBase + 1) * 17);
	}

	// Calculate a random number based on a specific seed
	uint RandomUInt(uint& customSeed)
	{
		customSeed ^= customSeed << 13;
		customSeed ^= customSeed >> 17;
		customSeed ^= customSeed << 5;
		return customSeed;
	}

	float RandomFloat(uint& customSeed) { return RandomUInt(customSeed) * 2.3283064365387e-10f; }

	float RandomFloat(uint& customSeed, const float min, const float max)
	{
		return min + (RandomFloat(customSeed) * (max - min));
	}

	/// Inclusive min Exclusive max
	uint RandomUInt(uint& customSeed, const int min, const int max)
	{
		return min + (RandomUInt(customSeed) % (max - min));
	}

	// from Sebastian Lague https://youtu.be/Qz0KTGYJtUk?si=YTTg56J1Yi21FZYJ&t=1979
	float2 RandomPointOnCircle(uint& customSeed)
	{
		float angle = RandomFloat(customSeed) * 2.0f * PI;
		float2 pointOnCircle = float2(cos(angle), sin(angle));
		return pointOnCircle * sqrt(RandomFloat(customSeed));
	}

	float3 RandomPointOnSphere(uint& customSeed)
	{
		float3 randPoint(0);
		while(true)
		{
			randPoint =
			{
				RandomFloat(customSeed,-1.0f,1.0f),
				RandomFloat(customSeed,-1.0f,1.0f),
				RandomFloat(customSeed,-1.0f,1.0f),
			};
			float len = length(randPoint);
			if(len > 1e-160 && len <= 1.0f)
			{
				return normalize(randPoint);
			}
		}
	}
};
