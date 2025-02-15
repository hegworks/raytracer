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
	uint InitSeed(uint seedBase)
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
};
