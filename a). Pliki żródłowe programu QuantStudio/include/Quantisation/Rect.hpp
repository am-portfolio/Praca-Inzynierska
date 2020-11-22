/**********************************************************************
SFML sf::Rect wrapper with type conversion.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <cassert>

/*
Podstawowy szablon klasy pomocniczej okreslajacej punkt na dwuwymiarowej p�aszczy�nie.
Wsp�rz�dne s� zapisane poprzez liczb� typu float.
Klasa posiada prze�adowania konwersji na sf::Vector2f i sf::Vector2i u�atwiaj�ce prac� z SFML.
*/

struct Rect
{
	float left, top, right, bottom;

	Rect() = default;
	template<typename T>
	Rect(T left, T top, T right, T bottom)
		: left(left), top(top), right(right), bottom(bottom) {
		assert(left < right && top < bottom);
	}

	template<typename T>
	Rect(sf::Rect<T> r)
		: left(r.left), top(r.top), right(r.left + r.width), bottom(r.top + r.height) {}
	template<typename T>
	operator sf::Rect<T>() const
	{
		return sf::Rect<T>(left, top, right - left, bottom - top);
	}

	//---- Prze�adowania operator�w arytmetycznych ----//

	inline bool operator==(const Rect& p) const
	{
		return left == p.left && top == p.top &&
			right == p.right && bottom == p.bottom;
	}
	inline bool operator!=(const Rect& p) const
	{
		return left != p.left || top != p.top ||
			right != p.right || bottom != p.bottom;
	}

	//---- Funkcje do debugowania ----//

	void _print() const
	{
		std::cout << "[" << left << ", " << top << ", " << right << ", " << bottom << "]";
	}
};