/**********************************************************************
XYZ color class and color space conversions.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <iostream>
#include <vector>
#include <array>
#include "Color.hpp"


//----------------------------------------------------------------------//
//-----------// PREDEFINIOWANE MACIERZE KONWERSJI RGB->XYZ //-----------//
//----------------------------------------------------------------------//
constexpr std::array<double, 9> rgb_to_xyz_matrix_CIERGB_E2 =
{
	0.49000,  0.31000,  0.20000,
	0.17697,  0.81240,  0.01063,
	0.00000,  0.01000,  0.99000
};
constexpr std::array<double, 9> rgb_to_xyz_matrix_CIERGB_E =
{
	0.4887180,  0.3106803,  0.2006017,
	0.1762044,  0.8129847,  0.0108109,
	0.0000000,  0.0102048,  0.9897952
};
constexpr std::array<double, 9> rgb_to_xyz_matrix_CIERGB_D50 =
{
	0.4868870,  0.3062984,  0.1710347,
	0.1746583,  0.8247541,  0.0005877,
   -0.0012563,  0.0169832,  0.8094831
};
constexpr std::array<double, 9> rgb_to_xyz_matrix_sRGB_D65 =
{
	0.4124564,  0.3575761,  0.1804375,
	0.2126729,  0.7151522,  0.0721750,
	0.0193339,  0.1191920,  0.9503041
};
constexpr std::array<double, 9> rgb_to_xyz_matrix_sRGB_D50 =
{
	0.4360747,  0.3850649,  0.1430804,
	0.2225045,  0.7168786,  0.0606169,
	0.0139322,  0.0971045,  0.7141733
};




//----------------------------------------------------------------------//
//----------------------// IMPLEMENTACJA KLASY //-----------------------//
//----------------------------------------------------------------------//

//Klasa kolorów w przestrzeni RGB. U¿ywana do liczenia wskaŸników jakosci.
struct ColorXYZ
{
	using Color = ColorXYZ;

	double x, y, z;

	ColorXYZ() = default;
	ColorXYZ(double x, double y, double z)
		: x(x), y(y), z(z) {}

	void setColor(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}


	//Konwertuje kolor rgb o wartoœciach z zakresu [0-255] na kolor w przestrzeni xyz.
	//By dokonaæ konwersji nalerzy podaæ dodatkowo macierz konwersji:
	// http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
	//Indexy w macierzy s¹ traktowane jak kolejne elementy macierzy 3x3 id¹c od lewego górnego rogu.
	void rgbToXyz_old(const ColorU8& _rgb, const std::array<double, 9>& matrix)
	{
		ColorXYZ rgb(_rgb.r, _rgb.g, _rgb.b);
		
		rgb.x /= 255.0;
		rgb.y /= 255.0;
		rgb.z /= 255.0;

		x = matrix[0] * rgb.x + matrix[1] * rgb.y + matrix[2] * rgb.z;
		y = matrix[3] * rgb.x + matrix[4] * rgb.y + matrix[5] * rgb.z;
		z = matrix[6] * rgb.x + matrix[7] * rgb.y + matrix[8] * rgb.z;
	}

	void rgbToXyz(const ColorU8& _rgb, const std::array<double, 9>& matrix)
	{
		ColorXYZ rgb(_rgb.r, _rgb.g, _rgb.b);

		rgb.x /= 255.0;
		rgb.y /= 255.0;
		rgb.z /= 255.0;

		//Dostosowanie krzywej gamma.
		rgb.x = ((rgb.x > 0.04045) ? pow((rgb.x + 0.055) / 1.055, 2.4) : (rgb.x / 12.92)) * 100.0;
		rgb.y = ((rgb.y > 0.04045) ? pow((rgb.y + 0.055) / 1.055, 2.4) : (rgb.y / 12.92)) * 100.0;
		rgb.z = ((rgb.z > 0.04045) ? pow((rgb.z + 0.055) / 1.055, 2.4) : (rgb.z / 12.92)) * 100.0;

		x = matrix[0] * rgb.x + matrix[1] * rgb.y + matrix[2] * rgb.z;
		y = matrix[3] * rgb.x + matrix[4] * rgb.y + matrix[5] * rgb.z;
		z = matrix[6] * rgb.x + matrix[7] * rgb.y + matrix[8] * rgb.z;
	}

	void rgbToCieLab_old(const ColorU8& rgb)
	{
		ColorXYZ CieXyz;
		CieXyz.rgbToXyz_old(rgb, rgb_to_xyz_matrix_CIERGB_E);

		CieXyz.x = (CieXyz.x > 0.00885645167) ? std::cbrt(CieXyz.x) : 7.78703703704 * CieXyz.x + 0.13793103448;
		CieXyz.y = (CieXyz.y > 0.00885645167) ? std::cbrt(CieXyz.y) : 7.78703703704 * CieXyz.y + 0.13793103448;
		CieXyz.z = (CieXyz.z > 0.00885645167) ? std::cbrt(CieXyz.z) : 7.78703703704 * CieXyz.z + 0.13793103448;

		x = 116.0 * CieXyz.y - 16.0; //L*
		y = 500.0 * (CieXyz.x - CieXyz.y); //a*
		z = 200.0 * (CieXyz.y - CieXyz.z); //b*
	}

	void rgbToCieLab(const ColorU8& rgb)
	{
		ColorXYZ CieXyz;
		CieXyz.rgbToXyz(rgb, rgb_to_xyz_matrix_sRGB_D65);

		CieXyz.x /= 95.047;
		CieXyz.y /= 100.00; //D65 * 100
		CieXyz.z /= 108.883;

		CieXyz.x = (CieXyz.x > 0.008856) ? std::cbrt(CieXyz.x) : 7.787 * CieXyz.x + 16.0 / 116.0;
		CieXyz.y = (CieXyz.y > 0.008856) ? std::cbrt(CieXyz.y) : 7.787 * CieXyz.y + 16.0 / 116.0;
		CieXyz.z = (CieXyz.z > 0.008856) ? std::cbrt(CieXyz.z) : 7.787 * CieXyz.z + 16.0 / 116.0;

		x = 116.0 * CieXyz.y - 16.0; //L*
		y = 500.0 * (CieXyz.x - CieXyz.y); //a*
		z = 200.0 * (CieXyz.y - CieXyz.z); //b*
	}

	//Oblicza odleg³oœæ pomiêdzy dwoma kolorami w euklidejskiej przestrzeni RGB.
	//W tej wersji [omijany jest etap pierwiastkowania.
	static int difference_nosqrt(const Color& c1, const Color& c2)
	{
		const double x_dif = c2.x - c1.x;
		const double y_dif = c2.y - c1.y;
		const double z_dif = c2.z - c1.z;
		return x_dif * x_dif + y_dif * y_dif + z_dif * z_dif;
	}

	//Oblicza odleg³oœsæ pomiêdzy dwoma kolorami w euklidejskiej przestrzeni RGB.
	static double difference(const Color& c1, const Color& c2)
	{
		const double x_dif = c2.x - c1.x;
		const double y_dif = c2.y - c1.y;
		const double z_dif = c2.z - c1.z;
		return std::sqrt(x_dif * x_dif + y_dif * y_dif + z_dif * z_dif);
	}


	//---- Prze³adowania operatorów arytmetycznych ----//

	inline Color& operator-=(const Color& c)
	{
		x -= c.x;
		y -= c.y;
		z -= c.z;
		return *this;
	}
	inline Color& operator+=(const Color& c)
	{
		x += c.x;
		y += c.y;
		z += c.z;
		return *this;
	}
	inline Color& operator*=(const Color& c)
	{
		x *= c.x;
		y *= c.y;
		z *= c.z;
		return *this;
	}
	inline Color& operator/=(const Color& c)
	{
		x /= c.x;
		y /= c.y;
		z /= c.z;
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
		x -= v;
		y -= v;
		z -= v;
		return *this;
	}
	template<typename T> inline Color& operator+=(const T& v)
	{
		x += v;
		y += v;
		z += v;
		return *this;
	}
	template<typename T> inline Color& operator*=(const T& v)
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}
	template<typename T> inline Color& operator/=(const T& v)
	{
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	template<typename T> inline Color& operator-(const T& v)
	{
		return Color(*this) -= v;
	}
	template<typename T> inline Color& operator+(const T& v)
	{
		return Color(*this) += v;
	}
	template<typename T> inline Color& operator*(const T& v)
	{
		return Color(*this) *= v;
	}
	template<typename T> inline Color& operator/(const T& v)
	{
		return Color(*this) /= v;
	}

	inline bool operator==(const Color& c) const
	{
		return x == c.x && y == c.y && z == c.z;
	}
	inline bool operator!=(const Color& c) const
	{
		return x != c.x || y != c.y || z != c.z;
	}
};

//PRze³adowanie strumienia
std::ostream& operator<<(std::ostream& stream, const ColorXYZ& color) {
		stream << color.x << ", " << color.y << ", " << color.z << std::endl;
	return stream;
}