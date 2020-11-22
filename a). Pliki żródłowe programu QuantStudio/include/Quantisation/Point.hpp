/**********************************************************************
SFML sf::Vector2 wrapper with type conversion.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

/*
Podstawowy szablon klasy pomocniczej okreslajacej punkt na dwuwymiarowej p³aszczyŸnie.
Wspó³rzêdne s¹ zapisane poprzez liczbê typu float.
Klasa posiada prze³adowania konwersji na sf::Vector2f i sf::Vector2i u³atwiaj¹ce pracê z SFML.
*/

struct Point
{
	float x, y;


	Point() = default;
	template<typename T>
	Point(T x, T y) : x(x), y(y) {}


	template<typename T>
	Point(sf::Vector2<T> v) : x(v.x), y(v.y) {}
	template<typename T>
	operator sf::Vector2<T>() const
	{
		return sf::Vector2<T>((T)x, (T)y);
	}

	//---- Prze³adowania operatorów arytmetycznych ----//

	inline Point& operator-=(const Point& p)
	{
		x -= p.x;
		y -= p.y;
		return *this;
	}
	inline Point& operator+=(const Point& p)
	{
		x += p.x;
		y += p.y;
		return *this;
	}
	inline Point& operator*=(const Point& p)
	{
		x *= p.x;
		y *= p.y;
		return *this;
	}
	inline Point& operator/=(const Point& p)
	{
		x /= p.x;
		y /= p.y;
		return *this;
	}

	inline Point& operator-(const Point& p) const
	{
		return Point(*this) -= p;
	}
	inline Point& operator+(const Point& p) const
	{
		return Point(*this) += p;
	}
	inline Point& operator*(const Point& p) const
	{
		return Point(*this) *= p;
	}
	inline Point& operator/(const Point& p) const
	{
		return Point(*this) /= p;
	}

	template<typename T> inline Point& operator-=(const T& v)
	{
		x -= (float)v;
		y -= (float)v;
		return *this;
	}
	template<typename T> inline Point& operator+=(const T& v)
	{
		x += (float)v;
		y += (float)v;
		return *this;
	}
	template<typename T> inline Point& operator*=(const T& v)
	{
		x *= (float)v;
		y *= (float)v;
		return *this;
	}
	template<typename T> inline Point& operator/=(const T& v)
	{
		x /= (float)v;
		y /= (float)v;
		return *this;
	}

	template<typename T> inline Point& operator-(const T& v)
	{
		return Point(*this) -= v;
	}
	template<typename T> inline Point& operator+(const T& v)
	{
		return Point(*this) += v;
	}
	template<typename T> inline Point& operator*(const T& v)
	{
		return Point(*this) *= v;
	}
	template<typename T> inline Point& operator/(const T& v)
	{
		return Point(*this) /= v;
	}


	inline bool operator==(const Point& p) const
	{
		return x == p.x && y == p.y;
	}
	inline bool operator!=(const Point& p) const
	{
		return x != p.x || y != p.y;
	}

	//---- Funkcje do debugowania ----//

	void _print()
	{
		std::cout << "[" << x << ", " << y << "]";
	}
};