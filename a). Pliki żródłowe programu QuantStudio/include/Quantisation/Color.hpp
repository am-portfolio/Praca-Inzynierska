/**********************************************************************
RGB color classs.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <iostream>
#include <vector>
#include <type_traits>

/*
Podstawowy szablon klasy pomocniczej okreslajacej kolor danego pixela w formacie RGBA.
Typ T okresla bazowy typ dla koloru sk³adowego.
Dla T : unsigned char Color bêdzie 4 bajtow¹ zmienn¹ a ka¿dy kolor bêdzie oœmio bitowy.
Zakres od bieli do czerni to 0-255.
*/
template<typename T>
struct Color
{
	T r, g, b, a;

	Color() = default;
	Color(T r, T g, T b, T a = 255)
		: r(r), g(g), b(b), a(a) {}

	Color(int hex)
	{
		setColor(hex);
	}

	template<typename U>
	Color(Color<U> c)
		: r(c.r), g(c.g), b(c.b), a(c.a)
	{
		if constexpr(std::is_floating_point<U>::value && std::is_integral<T>::value)
		{
			r = std::round(c.r);
			g = std::round(c.g);
			b = std::round(c.b);
		}
		else
		{
			r = c.r;
			g = c.g;
			b = c.b;
		}
	}

	void setColor(int hex)
	{
		if constexpr (std::is_same<T, unsigned char>::value)
		{
			*((int*)this) = hex;
		}
		else
		{
			a = (hex >> 24) & 0xFF;
			b = (hex >> 16) & 0xFF;
			g = (hex >> 8) & 0xFF;
			r = (hex) & 0xFF;
		}
	}
	void setColor(T r, T g, T b, T a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	void setColor(T r, T g, T b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	operator int() const
	{
		return *((int*)this);
	}



	//----------------------------------------------------------------------//
	//--------// FUNKCJE WYZNACZANIA ODLEG£OŒCI MIEDZY KOLORAMI //----------//
	//----------------------------------------------------------------------//

	//Oblicza odleg³oœæ pomiêdzy dwoma kolorami w euklidejskiej przestrzeni RGB.
	//W tej wersji [omijany jest etap pierwiastkowania.
	static int differenceRGB_nosqrt(const Color& c1, const Color& c2)
	{
		if constexpr (std::is_integral<T>::value)
		{
			const int r_dif = c1.r - c2.r;
			const int g_dif = c1.g - c2.g;
			const int b_dif = c1.b - c2.b;
			return r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;
		}
		else
		{
			const double r_dif = c1.r - c2.r;
			const double g_dif = c1.g - c2.g;
			const double b_dif = c1.b - c2.b;
			return r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;
		}
	}

	//Oblicza odleg³oœsæ pomiêdzy dwoma kolorami w euklidejskiej przestrzeni RGB.
	static double differenceRGB(const Color& c1, const Color& c2)
	{
		return std::sqrt(differenceRGB_nosqrt(c1,c2));
	}




	//----------------------------------------------------------------------//
	//-------------------------// INNE FUNKCJE //---------------------------//
	//----------------------------------------------------------------------//

	//Ucina wartoœci do zakresu 0 - 255.
	void clampValues()
	{
		r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
		g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
		b = (b < 0) ? 0 : ((b > 255) ? 255 : b);
	}

	//Przybli¿a liczby do wartoœci ca³kowitych.
	void roundValues()
	{
		if constexpr (std::is_integral<T>::value)
		{
			return;
		}
		else
		{
			r = std::round(r);
			g = std::round(g);
			b = std::round(b);
		}
	}




	//----------------------------------------------------------------------//
	//------------// PRZE£ADOWANIA OPERATOROW ARYTMETYCZNYCH //-------------//
	//----------------------------------------------------------------------//

	inline Color& operator-=(const Color& c)
	{
		r -= c.r;
		g -= c.g;
		b -= c.b;
		return *this;
	}
	inline Color& operator+=(const Color& c)
	{
		r += c.r;
		g += c.g;
		b += c.b;
		return *this;
	}
	inline Color& operator*=(const Color& c)
	{
		r *= c.r;
		g *= c.g;
		b *= c.b;
		return *this;
	}
	inline Color& operator/=(const Color& c)
	{
		r /= c.r;
		g /= c.g;
		b /= c.b;
		return *this;
	}

	inline Color& operator-(const Color& c) const
	{
		return Color(*this) -= c;
	}
	inline Color& operator+(const Color& c) const
	{
		return Color(*this) += c;
	}
	inline Color& operator*(const Color& c) const
	{
		return Color(*this) *= c;
	}
	inline Color& operator/(const Color& c) const
	{
		return Color(*this) /= c;
	}

	template<typename T> inline Color& operator-=(const T& v)
	{
		r -= v;
		g -= v;
		b -= v;
		return *this;
	}
	template<typename T> inline Color& operator+=(const T& v)
	{
		r += v;
		g += v;
		b += v;
		return *this;
	}
	template<typename T> inline Color& operator*=(const T& v)
	{
		r *= v;
		g *= v;
		b *= v;
		return *this;
	}
	template<typename T> inline Color& operator/=(const T& v)
	{
		r /= v;
		g /= v;
		b /= v;
		return *this;
	}

	template<typename T> inline Color& operator-(const T& v) const
	{
		return Color(*this) -= v;
	}
	template<typename T> inline Color& operator+(const T& v) const
	{
		return Color(*this) += v;
	}
	template<typename T> inline Color& operator*(const T& v) const
	{
		return Color(*this) *= v;
	}
	template<typename T> inline Color& operator/(const T& v) const
	{
		return Color(*this) /= v;
	}

	inline bool operator==(const Color& c) const
	{
		return r == c.r && g == c.g && b == c.b;
	}
	inline bool operator!=(const Color& c) const
	{
		return r != c.r || g != c.g || b != c.b;
	}
};

using ColorU8  = Color<unsigned char>;
using ColorU16 = Color<unsigned short>;
using ColorU32 = Color<unsigned int>;

using ColorS8 = Color<char>;
using ColorS16 = Color<short>;
using ColorS32 = Color<int>;

//PRze³adowanie strumienia
template<typename T>
std::ostream& operator<<(std::ostream& stream, const Color<T>& color) {
	if constexpr (std::is_integral<T>::value)
		stream << (long long)color.r << ", " << (long long)color.g << ", " << (long long)color.b << ", " << (long long)color.a << std::endl;
	else
		stream << color.r << ", " << color.g << ", " << color.b << ", " << color.a << std::endl;
	return stream;
}