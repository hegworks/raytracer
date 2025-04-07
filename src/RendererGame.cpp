#include "precomp.h"

#include <common.h>

#ifdef _GAME
void Renderer::Tick(const float deltaTime)
{
	const Timer t;
	const float scale = 1.0f / static_cast<float>(acmCounter++);

#pragma omp parallel for schedule(dynamic)
	for(int y = 0; y < SCRHEIGHT; y++)
	{
		const int yTimesSCRWDTH = y * SCRWIDTH;
		for(int x = 0; x < SCRWIDTH; x++)
		{
			const int pixelIndex = x + yTimesSCRWDTH;
			//TODO remove useAA
			const float xOffset = useAA ? threadRng.RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			const float yOffset = useAA ? threadRng.RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			float3 stochasticDOFTraced(0);
			for(int s = 0; s < spp; ++s)
			{
				const float2 defocusRand = threadRng.RandomPointOnCircle(pixelSeeds[pixelIndex]);
				Ray r = camera.GetPrimaryRay(static_cast<float>(x) + xOffset, static_cast<float>(y) + yOffset, defocusRand);
				stochasticDOFTraced += Trace(r, pixelIndex, 0, false, false);
			}
			float3 traced = stochasticDOFTraced / static_cast<float>(spp);
			if(dot(traced, traced) > dbgFF * dbgFF) traced = dbgFF * normalize(traced); // firefly suppressor
			accumulator[pixelIndex] += float4(traced, 0);
			const float4 avg = accumulator[pixelIndex] * scale;
			float4 gammaCorrected = float4(sqrtf(avg.x), sqrtf(avg.y), sqrtf(avg.z), 1);
			screen->pixels[pixelIndex] = RGBF32_to_RGB8(&gammaCorrected);
		}
	}

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if(alpha > 0.05f) alpha *= 0.5f;
	const float fps = 1000.0f / avg, rps = (SCRSIZE) / (avg * 1000);
	dfps = fps, drps = rps, davg = avg;
	//printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps);

	//TODO disable camera movement and keyboard object rotation in _GAME
	const bool cameraChanged = camera.HandleInput(deltaTime);
	const bool objectRotationChanged = HandleKeyboardRotations(deltaTime);
	if(cameraChanged || objectRotationChanged || resetAccumulator || !useACM)
	{
		memset(accumulator, 0, SCRSIZE * 16);
		acmCounter = 1;
		resetAccumulator = false;
	}

	m_gameManager.Tick(deltaTime);
}
#endif
