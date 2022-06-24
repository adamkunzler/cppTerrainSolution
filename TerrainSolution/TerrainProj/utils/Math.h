#pragma once

/// <summary>
/// Map a value in source range it's equivalent value in target range
/// </summary>
/// <param name="value">The value to map</param>
/// <param name="minSource">min value of source range</param>
/// <param name="maxSource">max value of source range</param>
/// <param name="minTarget">min value of target range</param>
/// <param name="maxTarget">max value of target range</param>
/// <returns></returns>
double mapRange(double value, double minSource, double maxSource, double minTarget, double maxTarget)
{
	//https://math.stackexchange.com/a/377174
	double val = ((value - minSource) * ((maxTarget - minTarget) / (maxSource - minSource))) + minTarget;
	return std::clamp(val, minTarget, maxTarget);
}