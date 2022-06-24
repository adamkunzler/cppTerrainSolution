#pragma once

#include <random>
#include <chrono>

class RandomGenerator
{
private:

	std::mt19937_64 rng;

	std::uniform_real_distribution<double> uniform;
	std::normal_distribution<double> normal;

	void initDistributions()
	{
		uniform = std::uniform_real_distribution<double>{ 0,1 };
		normal = std::normal_distribution<double>{ 0.5, 0.1 };
	}

public:

	/// <summary>
	/// initialize the random number generator with time-dependent seed
	/// </summary>
	RandomGenerator()
	{		
		uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };

		initDistributions();
	}

	/// <summary>
	/// initialize the reandom number generator with explicit seed
	/// </summary>
	/// <param name="seed"></param>
	RandomGenerator(unsigned long long seed)
	{
		rng.seed(seed);

		initDistributions();
	}

	~RandomGenerator() = default;

	/// <summary>
	/// uniform random number between 0.0 and 1.0
	/// </summary>
	/// <returns></returns>
	double getUniform() { return uniform(rng); }

	double getUniform(double min, double max) { return mapRange(uniform(rng), 0.0, 1.0, min, max); }

	/// <summary>
	/// normal random number between 0.0 and 1.0
	/// </summary>
	/// <returns></returns>
	double getNormal() { return normal(rng); }

	double getNormal(double min, double max) { return mapRange(getNormal(), 0.0, 1.0, min, max); }

	void resetNormal(double mean, double standardDeviation) { normal = std::normal_distribution<double>(mean, standardDeviation); }
};