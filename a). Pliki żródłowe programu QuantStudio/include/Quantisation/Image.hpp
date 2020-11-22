/**********************************************************************
Image managing class. Resizing, cropping, comparing, quantization,
loading, saving.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include "Color.hpp"
#include "ColorXYZ.hpp"
#include "Rect.hpp"
#include "Palette.h"

#include <Utility/AsyncTasker.hpp>

#include <cassert>
#include <SFML/Graphics.hpp>
#include <numeric>
#include <vector>
#include <filesystem>


class ImageU8
{
	using Path   = std::filesystem::path;
	using String = sf::String;

	//Rozmiary grafiki i pamiêæ na pixele.
	size_t width, height;
	std::vector<ColorU8> pixels;

	//Nazwa danego obrazka.
	String imagename;

	//Przygotowuje pamieæ incicializowan¹ zerami na nowe dane.
	ImageU8(size_t width, size_t height, String image_name)
	{
		assert(width*height && !imagename.isEmpty() && !image_name.isEmpty());

		this->width = width;
		this->height = height;
		pixels.resize(width*height);
		imagename = image_name;
	}

public:
	ImageU8() = default;
	ImageU8(const Path& path) {
		loadFromFile(path);
	}

	ImageU8(const ImageU8&) = default;
	ImageU8& operator=(const ImageU8&) = default;

	ImageU8(ImageU8&& img)
	{
		width = img.width;
		height = img.height;
		imagename = std::move(img.imagename);
		pixels = std::move(img.pixels);

		img.width = 0;
		img.height = 0;
	}
	ImageU8& operator=(ImageU8&& img)
	{
		width = img.width;
		height = img.height;
		imagename = std::move(img.imagename);
		pixels = std::move(img.pixels);

		img.width = 0;
		img.height = 0;

		return *this;
	}

	//Konwersja na wektor pixeli.
	operator const std::vector<ColorU8>&() const
	{
		return pixels;
	}




	//----------------------------------------------------------------------//
	//--------------------// ZAPIS I ODCZYT Z PLIKU //----------------------//
	//----------------------------------------------------------------------//

	//Wczytuje obraz we wskazanej œcie¿ce.
	//Wspierane formaty to: bmp, tga, png, jpg, gif, psd, hdr, pic.
	bool loadFromFile(const Path& path)
	{
		if (!std::filesystem::exists(path))
			return false;

		sf::Image sf_image;
		if (!sf_image.loadFromFile(path.generic_string()))
			return false;

		width = sf_image.getSize().x;
		height = sf_image.getSize().y;
		pixels.resize(width*height);
		std::memcpy(pixels.data(), sf_image.getPixelsPtr(), width*height * 4);

		this->imagename = path.stem().generic_string();
		return true;
	}

	//Zapisuje obraz, format jest wnioskowany z roszerzenia.
	//Wspierane formaty: bmp, tga, jpg, png.
	bool saveToFile(const Path& path)
	{
		assert(!isEmpty());
		if (isEmpty())
			return false;

		sf::Image sf_image;
		sf_image.create(width, height, getPixelsPtr());
		return sf_image.saveToFile(path.generic_string());
	}




	//----------------------------------------------------------------------//
	//--------------------// FUNKCJE ZMIANY ROZMIARU //---------------------//
	//----------------------------------------------------------------------//

	//Skaluje grafikê metod¹ najbli¿szego s¹siada.
	//Parametry okreœlaj¹ wynikowy rozmiar.
	static ImageU8 resizeNearestNeighbourXY(const ImageU8& image, int new_width, int new_height)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || new_width <= 0 || new_height <= 0)
			return ImageU8();


		ImageU8 new_image(new_width, new_height, image.imagename);
		std::vector<ColorU8>& new_pixels = new_image.pixels;

		const size_t width = image.width, height = image.height;
		const std::vector<ColorU8>& pixels = image.pixels;


		//+1 to remedy an early rounding problem
		int x_ratio = (int)((width << 16) / new_width) + 1;
		int y_ratio = (int)((height << 16) / new_height) + 1;

		int x2, y2;
		
		for (int i = 0; i < new_height; i++) {
			for (int j = 0; j < new_width; j++) {
				x2 = ((j*x_ratio) >> 16);
				y2 = ((i*y_ratio) >> 16);
				new_pixels[(i*new_width) + j] = pixels[(y2*width) + x2];
			}
		}
		return std::move(new_image);
	}
	//Skaluje grafikê metod¹ najbli¿szego s¹siada.
	//Parametry okreœlaj¹ stosunek wynikowego rozmiaru do oryginalnego.
	static ImageU8 resizeNearestNeighbourXY(const ImageU8& image, double scale_x, double scale_y)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || scale_x <= 0 || scale_y <= 0)
			return ImageU8();

		return std::move(resizeNearestNeighbourXY(
			image,
			(int)std::round(scale_x * (double)image.width),
			(int)std::round(scale_y * (double)image.height)));
	}

	//Skaluje grafikê metod¹ najbli¿szego s¹siada.
	//Parametr okreœlaja stosunek wynikowej iloœci pixeli do oryginalnej.
	//Innywmi s³owy: wynikowe osie bêd¹ mieæ rozmiary x * sqrt(size).
	static ImageU8 resizeNearestNeighbour(const ImageU8& image, double size)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || size <= 0)
			return ImageU8();

		return std::move(resizeNearestNeighbourXY(
			image,
			(int)std::round(std::sqrt(size) * (double)image.width),
			(int)std::round(std::sqrt(size) * (double)image.height)));
	}



	//Skaluje grafikê metod¹ interpolacji liniowej (FF dla kana³u alfa).
	//Parametry okreœlaj¹ wynikowy rozmiar.
	static ImageU8 resizeBilinearXY(const ImageU8& image, int new_width, int new_height)
	{
		//Sytuacja b³êdna:
		if (image.width*image.height == 0 || new_width <= 0 || new_height <= 0)
			return image;


		ImageU8 new_image(new_width, new_height, image.imagename);
		std::vector<ColorU8>& new_pixels = new_image.pixels;

		const size_t width = image.width, height = image.height;
		const std::vector<ColorU8>& pixels = image.pixels;


		int a, b, c, d, x, y, index;
		float x_ratio = ((float)(width - 1)) / new_width;
		float y_ratio = ((float)(height - 1)) / new_height;
		float x_diff, y_diff, blue, red, green;
		int offset = 0;
		for (int i = 0; i<new_height; i++) {
			for (int j = 0; j<new_width; j++) {
				x = (int)(x_ratio * j);
				y = (int)(y_ratio * i);
				x_diff = (x_ratio * j) - x;
				y_diff = (y_ratio * i) - y;
				index = (y*width + x);
				a = pixels[index];
				b = pixels[index + 1];
				c = pixels[index + width];
				d = pixels[index + width + 1];

				// Yb = Ab(1-w1)(1-h1) + Bb(w1)(1-h1) + Cb(h1)(1-w1) + Db(wh)
				blue = (a & 0xff)*(1 - x_diff)*(1 - y_diff) + (b & 0xff)*(x_diff)*(1 - y_diff) +
					(c & 0xff)*(y_diff)*(1 - x_diff) + (d & 0xff)*(x_diff*y_diff);

				// Yg = Ag(1-w1)(1-h1) + Bg(w1)(1-h1) + Cg(h1)(1-w1) + Dg(wh)
				green = ((a >> 8) & 0xff)*(1 - x_diff)*(1 - y_diff) + ((b >> 8) & 0xff)*(x_diff)*(1 - y_diff) +
					((c >> 8) & 0xff)*(y_diff)*(1 - x_diff) + ((d >> 8) & 0xff)*(x_diff*y_diff);

				// Yr = Ar(1-w1)(1-h1) + Br(w1)(1-h1) + Cr(h1)(1-w1) + Dr(wh)
				red = ((a >> 16) & 0xff)*(1 - x_diff)*(1 - y_diff) + ((b >> 16) & 0xff)*(x_diff)*(1 - y_diff) +
					((c >> 16) & 0xff)*(y_diff)*(1 - x_diff) + ((d >> 16) & 0xff)*(x_diff*y_diff);

				new_pixels[offset++] =
					0xff000000 | // hardcoded alpha
					((((int)red) << 16) & 0xff0000) |
					((((int)green) << 8) & 0xff00) |
					((int)blue);
			}
		}
		return std::move(new_image);
	}

	//Skaluje grafikê metod¹ interpolacji liniowej (FF dla kana³u alfa).
	//Parametry okreœlaj¹ stosunek wynikowego rozmiaru do oryginalnego.
	static ImageU8 resizeBilinearXY(const ImageU8& image, double scale_x, double scale_y)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || scale_x <= 0 || scale_y <= 0)
			return ImageU8();

		return std::move(resizeBilinearXY(
			image,
			(int)std::round(scale_x * (double)image.width),
			(int)std::round(scale_y * (double)image.height)));
	}

	//Skaluje grafikê metod¹ interpolacji liniowej (FF dla kana³u alfa).
	//Parametr okreœlaja stosunek wynikowej iloœci pixeli do oryginalnej.
	//Innywmi s³owy: wynikowe osie bêd¹ mieæ rozmiary x * sqrt(size).
	static ImageU8 resizeBilinear(const ImageU8& image, double size)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || size <= 0)
			return ImageU8();

		return std::move(resizeBilinearXY(
			image,
			(int)std::round(std::sqrt(size) * (double)image.width),
			(int)std::round(std::sqrt(size) * (double)image.height)));
	}




	//----------------------------------------------------------------------//
	//---------------// FUNKCJE ZWRACAJ¥CE PROBKE PIXELI //-----------------//
	//----------------------------------------------------------------------//

	//Tworzy próbkê pixeli poprzez losowanie. W trakcie losowania nie jest sprawdzane,
	//czy dany pixel nie zosta³ ju¿ wylosowany wczeœniej. Wartoœæ 'percentage' okreœla ilosæ
	//pixeli jaka zostanie wylosowana w stosunku do oryginalnej iloœci pixeli.
	static std::vector<ColorU8> sampleRandom(const ImageU8& image, double percentage)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || percentage <= 0 || percentage >= 1)
			return ImageU8();


		const size_t width = image.width, height = image.height;
		const std::vector<ColorU8>& pixels = image.pixels;
		const size_t count = std::round((double)(width*height)*percentage);

		std::vector<ColorU8> result;
		result.resize(count);

		for (auto& color : result)
			color = pixels[std::round((float)rand() / (float)(RAND_MAX) * (float)(pixels.size() - 1))];

		return std::move(result);
	}

	//Tworzy próbkê pixeli poprzez losowanie. By unikn¹æ powtórzeñ Najlpierw losowane s¹
	//unikatowe indexy pixeli a nastêpnie pobierane s¹ pixele. Wartoœæ 'percentage' okreœla ilosæ
	//pixeli jaka zostanie wylosowana w stosunku do oryginalnej iloœci pixeli.
	static std::vector<ColorU8> sampleRandomUnique(const ImageU8& image, double percentage)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || percentage <= 0 || percentage >= 1)
			return ImageU8();


		const size_t width = image.width, height = image.height;
		const std::vector<ColorU8>& pixels = image.pixels;
		const size_t count = std::round((double)(width*height)*percentage);

		std::vector<size_t> indexes;
		indexes.resize(count);
		fill_with_random_indexes(indexes, image.pixels.size());

		std::vector<ColorU8> result;
		result.resize(count);
		auto color = result.begin();

		for (auto& index : indexes)
			*(color++) = pixels[index];

		return std::move(result);
	}

	//Tworzy próbkê pixeli poprzez wybranie pewnego procenta pixeli z oryginalnego obrazu.
	//W przeciwieñstwie do metody resizeNearestNeighbour obraz nie s¹ traktowany jako tablica
	//dwuwymiarowa tylko jednowymiatowa tablica pixeli.
	static std::vector<ColorU8> sampleEveryFew(const ImageU8& image, double percentage)
	{
		//Sytuacja b³êdna:
		if ((image.width*image.height == 0) || percentage <= 0 || percentage >= 1)
			return ImageU8();

		const size_t size = image.height * image.width;
		const size_t size_new = percentage * size;
		const size_t step_size = ((double)1.f) / percentage;

		std::vector<ColorU8> result(size_new);

		auto pixel = image.pixels.data();
		auto beg = result.data();
		auto end = beg + result.size();
		
		for (;beg != end;)
		{
			*beg = *pixel;
			++beg;
			pixel += step_size;
		}		
		
		return std::move(result);
	}




	//----------------------------------------------------------------------//
	//----------------// FUNKCJE OCENY JAKOŒCI ALGORYTMU //-----------------//
	//----------------------------------------------------------------------//

	//Oblicza b³¹d wartoœci bezwzglêdnych MAE obrazu drugiego wzglêdem pierwszego.
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca nieskoñczonoœæ.
	//Obliczneia s¹ wykonywane w wielu w¹tkach.
	static long double MAE(const ImageU8& orginal, const ImageU8& processed)
	{
		//Sprawdzenie poprawnoœci danych wejœciowych:
		assert(!orginal.isEmpty() && !processed.isEmpty());
		assert(orginal.width == processed.width);
		assert(orginal.height == processed.height);
		assert(orginal.pixels.size() == processed.pixels.size());
		if (
			orginal.isEmpty() || processed.isEmpty() ||
			orginal.width != processed.width ||
			orginal.height != processed.height ||
			orginal.pixels.size() != processed.pixels.size()
			) return std::numeric_limits<long double>::infinity();


		//Zmienna sumuj¹ca wyniki z wielu w¹tków:
		AsyncVariable<double> sum = 0;

		//Obliczenie sumy dla danego zakresu:
		auto rmse_range = [&sum](auto pre, auto pre_end, auto post, auto post_end)
		{
			double partial_sum = 0;
			int r_dif, b_dif, g_dif;

			for (; pre != pre_end; pre++, post++)
			{
				r_dif = pre->r - post->r;
				g_dif = pre->g - post->g;
				b_dif = pre->b - post->b;

				partial_sum += std::abs(r_dif) + std::abs(g_dif) + std::abs(b_dif);
			}

			sum += partial_sum;
		};

		//Podzielenie pracy na w¹tki:
		ASYNC.addRangeTask(
			rmse_range,
			orginal.pixels.cbegin(), orginal.pixels.cend(),
			processed.pixels.cbegin(), processed.pixels.cend()
		);
		ASYNC.notifyAndWait();

		//Wylicznie œredniej i pierwiastka:
		sum /= (long double)(3 * orginal.pixels.size());

		return sum;
	}

	//Oblicza b³¹d œredniokwadratowy RMSE obrazu drugiego wzglêdem pierwszego.
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca nieskoñczonoœæ.
	//Obliczneia s¹ wykonywane w wielu w¹tkach.
	static long double RMSE(const ImageU8& orginal, const ImageU8& processed)
	{
		//Sprawdzenie poprawnoœci danych wejœciowych:
		assert(!orginal.isEmpty() && !processed.isEmpty());
		assert(orginal.width == processed.width);
		assert(orginal.height == processed.height);
		assert(orginal.pixels.size() == processed.pixels.size());
		if (
			orginal.isEmpty() || processed.isEmpty() ||
			orginal.width != processed.width ||
			orginal.height != processed.height ||
			orginal.pixels.size() != processed.pixels.size()
			) return std::numeric_limits<long double>::infinity();


		//Zmienna sumuj¹ca wyniki z wielu w¹tków:
		AsyncVariable<double> sum = 0;

		//Obliczenie sumy dla danego zakresu:
		auto rmse_range = [&sum](auto pre, auto pre_end, auto post, auto post_end)
		{
			double partial_sum = 0;
			int r_dif, b_dif, g_dif;
			
			for (; pre != pre_end; pre++, post++)
			{
				r_dif = pre->r - post->r;
				g_dif = pre->g - post->g;
				b_dif = pre->b - post->b;

				partial_sum += r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;
			}
			sum += partial_sum;
		};

		//Podzielenie pracy na w¹tki:
		ASYNC.addRangeTask(
			rmse_range,
			orginal.pixels.cbegin(), orginal.pixels.cend(),
			processed.pixels.cbegin(), processed.pixels.cend()
		);
		ASYNC.notifyAndWait();

		//Wylicznie œredniej i pierwiastka:
		sum /= (long double)(3 * orginal.pixels.size());
		sum = sqrt(sum);

		return sum;
	}

	//Oblicza stosunek szczytowego sygna³u do szumu PSNR obrazu drugiego wzglêdem pierwszego.
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca zero.
	static long double PSNR(long double rmse)
	{
		if (rmse == std::numeric_limits<long double>::infinity())
			return 0;
		else if (rmse == 0)
			return std::numeric_limits<long double>::infinity();
		else
			return 20.L * std::log10(255.L / rmse);
	}

	//Oblicza stosunek szczytowego sygna³u do szumu PSNR obrazu drugiego wzglêdem pierwszego.
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca zero.
	//Obliczneia s¹ wykonywane w wielu w¹tkach.
	static long double PSNR(const ImageU8& orginal, const ImageU8& processed)
	{
		return PSNR(RMSE(orginal, processed));
	}

	//Oblicza delta e czyli odleg³oœæ w przestrzeni CIE76 (CIELAB)
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca nieskoñczonoœæ.
	//Obliczneia s¹ wykonywane w wielu w¹tkach.
	static long double DELTAE_76(const ImageU8& orginal, const ImageU8& processed)
	{
		//Sprawdzenie poprawnoœci danych wejœciowych:
		assert(!orginal.isEmpty() && !processed.isEmpty());
		assert(orginal.width == processed.width);
		assert(orginal.height == processed.height);
		assert(orginal.pixels.size() == processed.pixels.size());
		if (
			orginal.isEmpty() || processed.isEmpty() ||
			orginal.width != processed.width ||
			orginal.height != processed.height ||
			orginal.pixels.size() != processed.pixels.size()
		) return std::numeric_limits<long double>::infinity();


		//Zmienna sumuj¹ca wyniki z wielu w¹tków:
		AsyncVariable<long double> sum = 0;

		//Obliczenie sumy dla danego zakresu:
		auto cielab_range = [&sum](auto pre, auto pre_end, auto post, auto post_end)
		{
			long double partial_sum = 0;
			double x_dif, y_dif, z_dif;
			ColorXYZ c1, c2;

			for (; pre != pre_end; pre++, post++)
			{
				c1.rgbToCieLab(*pre);
				c2.rgbToCieLab(*post);

				x_dif = c1.x - c2.x;
				y_dif = c1.y - c2.y;
				z_dif = c1.z - c2.z;

				partial_sum += std::sqrt(x_dif * x_dif + y_dif * y_dif + z_dif * z_dif);
			}

			sum += partial_sum;
		};

		//Podzielenie pracy na w¹tki:
		ASYNC.addRangeTask(
			cielab_range,
			orginal.pixels.cbegin(), orginal.pixels.cend(),
			processed.pixels.cbegin(), processed.pixels.cend()
		);
		ASYNC.notifyAndWait();

		//Policzenie œredniej:
		sum /= (long double)orginal.pixels.size();

		return sum;
	}

	//Oblicza delta e czyli odleg³oœæ w przestrzeni CIE76 (CIELAB)
	//Gdy dane wejsciowe nie s¹ poprawne (np grafiki maj¹ ró¿ne rozmiary), zwraca nieskoñczonoœæ.
	//Obliczneia s¹ wykonywane w wielu w¹tkach.
	//To prze³adowanie przyjmuje wektor pikseli orygina³u w postaci CIELAB.
	static long double DELTAE_76(const std::vector<ColorXYZ>& orginal, const ImageU8& processed)
	{
		//Sprawdzenie poprawnoœci danych wejœciowych:
		assert(!orginal.empty() && !processed.isEmpty());
		assert(orginal.size() == processed.pixels.size());
		if (
			orginal.empty() || processed.isEmpty() ||
			orginal.size() != processed.pixels.size()
			) return std::numeric_limits<long double>::infinity();


		//Zmienna sumuj¹ca wyniki z wielu w¹tków:
		AsyncVariable<long double> sum = 0;

		//Obliczenie sumy dla danego zakresu:
		auto cielab_range = [&sum](auto pre, auto pre_end, auto post, auto post_end)
		{
			long double partial_sum = 0;
			double x_dif, y_dif, z_dif;
			ColorXYZ c2;

			for (; pre != pre_end; pre++, post++)
			{
				c2.rgbToCieLab(*post);

				x_dif = pre->x - c2.x;
				y_dif = pre->y - c2.y;
				z_dif = pre->z - c2.z;

				partial_sum += std::sqrt(x_dif * x_dif + y_dif * y_dif + z_dif * z_dif);
			}

			sum += partial_sum;
		};

		//Podzielenie pracy na w¹tki:
		ASYNC.addRangeTask(
			cielab_range,
			orginal.cbegin(), orginal.cend(),
			processed.pixels.cbegin(), processed.pixels.cend()
		);
		ASYNC.notifyAndWait();

		//Policzenie œredniej:
		sum /= (long double)orginal.size();

		return sum;
	}

	//----------------------------------------------------------------------//
	//---------------------// RO¯NE FUNKCJE UTILITY //----------------------//
	//----------------------------------------------------------------------//

	//Konwertuje piksele do postaci CIELAB.
	std::vector<ColorXYZ> getCieLabPixels() const
	{
		std::vector<ColorXYZ> cie_pixels(pixels.size());
		auto begin = cie_pixels.begin();

		for (auto& pixel : pixels)
			(begin++)->rgbToCieLab(pixel);

		return std::move(cie_pixels);
	}


	//Sprawdza czy grafika nie jest pusta.
	bool isEmpty() const
	{
		return !(width*height);
	}

	//Zwraca aktualn¹ szerokoœæ grafiki.
	size_t getWidth() const
	{
		return width;
	}
	//Zwraca aktualn¹ wysokoœæ grafiki.
	size_t getHeight() const
	{
		return height;
	}

	//Zwraca wskaŸnik na pixele RGBA 8bit na kolor.
	uint8_t* getPixelsPtr()
	{
		assert(!isEmpty());
		return (uint8_t*)pixels.data();
	}
	//Zwraca sta³y wskaŸnik na pixele RGBA 8bit na kolor.
	const uint8_t* getPixelsPtr() const
	{
		assert(!isEmpty());
		return (const uint8_t*)pixels.data();
	}


	//Zwraca nadan¹ nazwê danej grafiki (domyœlnie taka sama jak
	//wczytanego pliku bez rozszerzenia).
	String getImagename() const
	{
		assert(!isEmpty());
		return imagename;
	}
	//Pozwala okreœliæ now¹ nadan¹ nazwê obrazka (bez rozszerzenia).
	void setImagename(const String& image_name)
	{
		assert(!isEmpty());
		imagename = image_name;
	}




	//----------------------------------------------------------------------//
	//----------------// RO¯NE FUNKCJE MODYFIKUJ¥CE OBRAZ //----------------//
	//----------------------------------------------------------------------//

	//Przycina grafikê tak by zosta³ tylko wskazany prostok¹t.
	auto cropImage(const Rect& rect) const
	{
		assert(!isEmpty());
		assert(rect.right < width && rect.bottom < height);

		ImageU8 new_image(
			rect.right - rect.left,
			rect.bottom - rect.top, imagename);

		auto new_pixel = new_image.pixels.data();
		auto old_pixel = pixels.data();

		for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width;  x++)
		{
			if (x >= rect.left && x < rect.right &&
				y >= rect.top&&y < rect.bottom)
				*(new_pixel++) = (*old_pixel);
			old_pixel++;
		}
		
		return std::move(new_image);
	}





	//----------------------------------------------------------------------//
	//---------------// FUNKCJE ZWIAZANE Z KWANTYZACJ¥ //-------------------//
	//----------------------------------------------------------------------//

	//Zwraca iloœæ unikalnych kolorow w obrazie.
	unsigned int uniqueColors() const
	{
		if (!pixels.size())
			return 0;

		//Ustawienie jedynek w wyzerowanej tablicy wszystkich kolorow.
		std::vector<int> colors(256 * 256 * 256, 0);
		for (const auto& pixel : pixels)
			colors[pixel.r + pixel.g * 256 + pixel.b * 256 * 256] = 1;
		//note. inaczej mo¿na by pixel.g << 8

		//Sumowanie wszystkich jedynek w tablicy.
		return std::accumulate(colors.cbegin(), colors.cend(), 0);
	}

	//Funkcja kwantyzujaca barwy do zadanej palety kolorów. Skantyzowany
	//obraz jest zwracany jako wynik funkcji.
	//***Dzia³a wielow¹tkowo.
	ImageU8 applyPalette(const std::vector<ColorU8>& palette) const
	{	
		assert(!isEmpty());
		assert(palette.size());

		//Pamiêæ na grafikê wynikow¹.
		ImageU8 new_image = *this;


		//Funkcja kwantyzuj¹ca po ka¿dym pixelu:
		auto quantizetizeRange = [&palette](auto begin, auto end)
		{
			//Zmienne robocze:
			ColorU8 closest_color;
			int r_dif, g_dif, b_dif;
			int old_distance;
			int new_distance;

			for (auto pixel = begin; pixel != end; pixel++)
			{
				old_distance = 255 * 255 * 3; //Maxymalna wartoœæ dla RGB_U8.

				for (const auto& pallete_color : palette)
				{
					//Obliczenie b³êdu od koloru z palety.
					r_dif = pallete_color.r - pixel->r;
					g_dif = pallete_color.g - pixel->g;
					b_dif = pallete_color.b - pixel->b;

					new_distance = r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;

					//Poszukiwnie najlepszego koloru.
					if (new_distance < old_distance) {
						old_distance = new_distance;
						closest_color = pallete_color;
					}
				}
				*pixel = closest_color;
			}
		};


		//Podzielenie pracy na dostêpne w¹tki.
		auto begin = new_image.pixels.begin();
		auto end = new_image.pixels.end();

		size_t n_thread = ASYNC.hardwareConcurrency();
		size_t range = std::round((float)new_image.pixels.size()/(float)n_thread);

		for (unsigned int i = 0; i < n_thread; i++)
		{
			ASYNC.addTask(quantizetizeRange,
				begin + i * range, (i < n_thread - 1) ? begin + (i + 1)*range : end);
		}
		ASYNC.notifyAll();
		ASYNC.waitForThreads();


		return std::move(new_image);
	}

	//Funkcja kwantyzujaca barwy do zadanej palety kolorów. Skantyzowany
	//obraz jest zwracany jako wynik funkcji. Podczas kwantyzacji jest dodatkwo
	//stosowana propagaja b³êdu.
	ImageU8 applyPalette_dither(const std::vector<ColorU8>& palette) const
	{
		assert(!isEmpty());
		assert(palette.size());

		//Pamiêæ na grafikê wynikow¹.
		ImageU8 new_image = *this;

		//Przekonwertowanie danych na float.
		std::vector<Color<float>> float_pixels;
		float_pixels.reserve(new_image.pixels.size());
		for (auto& p : new_image.pixels)
			float_pixels.emplace_back(p);

		//Zmienne robocze:
		ColorU8 closest_color;
		Color<float> error;
		float r_dif, g_dif, b_dif;
		float old_distance;
		float new_distance;

		size_t x = 0, y = 0;

		//Kwantyzacja po ka¿dym pixelu:
		for (auto pixel = float_pixels.begin(); pixel != float_pixels.end(); pixel++)
		{
			old_distance = 256 * 256 * 256; 

			for (const auto& pallete_color : palette)
			{
				//Obliczenie b³êdu od koloru z palety.
				r_dif = (float)pallete_color.r - pixel->r;
				g_dif = (float)pallete_color.g - pixel->g;
				b_dif = (float)pallete_color.b - pixel->b;

				new_distance = r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;

				//Poszukiwnie najlepszego koloru.
				if (new_distance < old_distance) {
					old_distance = new_distance;
					closest_color = pallete_color;
				}
			}

			//PROPAGACJA B£ÊDU:

			error = *pixel - Color<float>(closest_color);

			if (x < new_image.width - 1)
			*(pixel + 1) += error * (7.f / 16.f);

			if (y < height < -1){
				if (x < new_image.width - 1)
					*(pixel - 1 + new_image.width) += error * (3.f / 16.f);

				*(pixel + new_image.width) += error * (5.f / 16.f);

				if (x > 0)
					*(pixel + 1 + new_image.width) += error * (1.f / 16.f);
			}

			//Liczniki pozycji.
			x++;
			if (x >= new_image.width){
				x = 0; y++;
			}

			//Zapisanie koloru.
			*pixel = Color<float>(closest_color);
		}

		//Przepisanie danych z float na Uint8
		auto result = float_pixels.data();
		for (auto& p : new_image.pixels)
			p = *(result++);

		return std::move(new_image);
	}
};