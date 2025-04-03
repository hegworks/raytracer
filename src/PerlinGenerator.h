#pragma once

class PerlinGenerator
{
public:
	// Perlin noise implementation - https://stackoverflow.com/questions/29711668/perlin-noise-generation
	inline static int m_numOctaves = 7, primeIndex = 0;
	inline static float m_persistence = 0.8f;
	inline static float m_amplitude = 1.0f;
	inline static int primes[10][3] =
	{
		{ 995615039, 600173719, 701464987 },
		{ 831731269, 162318869, 136250887 },
		{ 174329291, 946737083, 245679977 },
		{ 362489573, 795918041, 350777237 },
		{ 457025711, 880830799, 909678923 },
		{ 787070341, 177340217, 593320781 },
		{ 405493717, 291031019, 391950901 },
		{ 458904767, 676625681, 424452397 },
		{ 531736441, 939683957, 810651871 },
		{ 997169939, 842027887, 423882827 }
	};

	static float Noise(const int i, const int x, const int y)
	{
		int n = x + y * 57;
		n = (n << 13) ^ n;
		const int a = primes[i][0], b = primes[i][1], c = primes[i][2];
		const int t = (n * (n * n * a + b) + c) & 0x7fffffff;
		return 1.0f - static_cast<float>(t) / 1073741824.0f;
	}

	static float SmoothedNoise(const int i, const int x, const int y)
	{
		const float corners = (Noise(i, x - 1, y - 1) + Noise(i, x + 1, y - 1) + Noise(i, x - 1, y + 1) + Noise(i, x + 1, y + 1)) / 16;
		const float sides = (Noise(i, x - 1, y) + Noise(i, x + 1, y) + Noise(i, x, y - 1) + Noise(i, x, y + 1)) / 8;
		const float center = Noise(i, x, y) / 4;
		return corners + sides + center;
	}

	static float Interpolate(const float a, const float b, const float x)
	{
		const float ft = x * 3.1415927f, f = (1 - cosf(ft)) * 0.5f;
		return a * (1 - f) + b * f;
	}

	static float InterpolatedNoise(const int i, const float x, const float y)
	{
		const int integer_X = static_cast<int>(x), integer_Y = static_cast<int>(y);
		const float fractional_X = x - integer_X, fractional_Y = y - integer_Y;
		const float v1 = SmoothedNoise(i, integer_X, integer_Y);
		const float v2 = SmoothedNoise(i, integer_X + 1, integer_Y);
		const float v3 = SmoothedNoise(i, integer_X, integer_Y + 1);
		const float v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1);
		const float i1 = Interpolate(v1, v2, fractional_X);
		const float i2 = Interpolate(v3, v4, fractional_X);
		return Interpolate(i1, i2, fractional_Y);
	}

	static float noise2D(const float x, const float y)
	{
		float total = 0, frequency = static_cast<float>(2 << m_numOctaves), amplitude = m_amplitude;
		for(int i = 0; i < m_numOctaves; ++i)
		{
			frequency /= 2, amplitude *= m_persistence;
			total += InterpolatedNoise((primeIndex + i) % 10, x / frequency, y / frequency) * amplitude;
		}
		return total / frequency;
	}

	static float noise3D(const float x, const float y, const float z)
	{
		float noise = 0.0f;
		float frequency = 5; // (float)(2 << m_numOctaves);
		float amplitude = m_amplitude;
		for(int i = 0; i < m_numOctaves; ++i)
		{
			// get all permutations of noise for each individual axis
			const float noiseXY = InterpolatedNoise(i, x * frequency, y * frequency);
			const float noiseXZ = InterpolatedNoise(i, x * frequency, z * frequency);
			const float noiseYZ = InterpolatedNoise(i, y * frequency, z * frequency);
			// reverse of the permutations of noise for each individual axis
			const float noiseYX = InterpolatedNoise(i, y * frequency, x * frequency);
			const float noiseZX = InterpolatedNoise(i, z * frequency, x * frequency);
			const float noiseZY = InterpolatedNoise(i, z * frequency, y * frequency);
			// use the average of the noise functions
			noise += (noiseXY + noiseXZ + noiseYZ + noiseYX + noiseZX + noiseZY) * amplitude;
			amplitude *= m_persistence;
			frequency *= 2.0f;
		}
		// use the average of all octaves
		return noise;
	}
};
