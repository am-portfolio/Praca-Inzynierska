/**********************************************************************
Time measurements utility class.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <chrono>

class Clock
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start;

public:
	//Restartuje zegar.
	Clock()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	//Zwraca czas w sekundach od ostatniego wywo³ania metody 'restart'.
	auto getTime()
	{
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<long double>(now - start).count();
	}

	//Zeruje zegar i zwraca czas w sekundach od poprzedniego restartu.
	auto restart()
	{
		auto time = getTime();
		start = std::chrono::high_resolution_clock::now();
		return time;
	}
};