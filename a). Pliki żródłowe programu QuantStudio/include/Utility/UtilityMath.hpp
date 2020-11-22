/**********************************************************************
Utility math functions.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <cassert>
#include <vector>
#include <random>
#include <algorithm>

#include <cmath>
#include <ctime>

//Przeprowadza hard clipping wartoœci d.
template<typename T>
inline double clamp(T d, T min, T max) {
	assert(min <= max);
	const T t = d < min ? min : d;
	return t > max ? max : t;
}

//Wartoœæ d po przekoczeniu zakresu z prawej zacznie
//wracaæ w lewo, a po przkeorczeniu zakresu z prawej
//zacznie wracaæ w lew¹ stronê.
//UWAGA: dzia³a tylko w przypadku jednego odbicia.
template<typename T>
inline double mirror(T d, T min, T max) {
	assert(min <= max);
	const T t = d < min ? 2*min - d : d;
	return t > max ? 2*max - t : t;
}

//Wype³nia podany wektor losowymi indexami z zakresu 0-size-1.
void fill_with_random_indexes_old(std::vector<size_t>& vector, size_t size)
{
	bool unique;
	for (auto pt = vector.begin() ; pt != vector.end(); pt++)
	{
		do
		{
			unique = true;
			*pt = std::round((float)rand() / (float)RAND_MAX*(float)(size - 1));

			for (auto pt2 = vector.begin(); pt2 != pt; pt2++)
			{
				if (*pt2 == *pt) {
					unique = false;
					break;
				}
			}
		} while (!unique);
	}
}

//Wype³nia podany wektor losowymi indexami z zakresu 0-size-1.
//Nowa wersja: generuje tablicê indexów, tasujê j¹, a nastêpnie kasuje czêœæ wyników.
void fill_with_random_indexes(std::vector<size_t>& vector, size_t size)
{
	std::vector<size_t> indexes;
	indexes.resize(size);
	size_t i = 0;
	for (auto& index : indexes)
		index = i++;

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(indexes.begin(), indexes.end(), g);

	indexes.resize(vector.size());
	vector = std::move(indexes);
}