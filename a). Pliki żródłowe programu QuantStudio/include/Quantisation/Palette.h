/**********************************************************************
Class managing color palette creation by population method, median-cut,
k-means and randomPick.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once

#include <Other/WuColorQ.c>

#include "Color.hpp"

#include <Utility/UtilityMath.hpp>

#include <vector>
#include <cmath>




class Palette
{
public:
	enum InitEnum
	{
		init_DC = 0, //diagonal
		init_SD,	 //deviation

		init_WU,	 //WU
		init_MCUT,	 //Median-cut

		init_RAND,	 //uniformRandom
		init_RANDU	 //uniformRandomUnique
	};

private:
	using Uint32 = unsigned int;
	using Uint8  = unsigned char;

	//Pamiêæ na paletê barw.
	std::vector<ColorU8> colors;


	//Zwraca œredni kolor z próki pixeli.
	static Color<double> getAverageColor(const std::vector<ColorU8>& pixels)
	{
		long double r_sum = 0, g_sum = 0, b_sum = 0;

		for (auto& pixel : pixels)
		{
			r_sum += pixel.r;
			g_sum += pixel.g;
			b_sum += pixel.b;
		}

		r_sum /= (long double)pixels.size();
		g_sum /= (long double)pixels.size();
		b_sum /= (long double)pixels.size();

		return Color<double>(r_sum, g_sum, b_sum);
	}

public:

	//----------------------------------------------------------------------//
	//---------------------// OPERATORY DOSTÊPOWE //------------------------//
	//----------------------------------------------------------------------//

	operator const std::vector<ColorU8>&() const
	{
		return colors;
	}

	//Zwraca rozmiar palety:
	size_t size() const
	{
		return colors.size();
	}



	//----------------------------------------------------------------------//
	//---------// STA£E METODY INICIALIZACJI DLA METODY K-MEANS //----------//
	//----------------------------------------------------------------------//

	//Incializacja palety metod¹ przek¹tnej RGB.
	static std::vector<Color<double>> RGB_diagonal(Uint32 gray_shades)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (gray_shades < 1)
			return std::vector<Color<double>>();

		std::vector<Color<double>> palette;
		palette.resize(gray_shades);
		auto  color = palette.data();

		if (gray_shades == 1)
		{
			*color = Color<double>(128, 128, 128);
			return std::move(palette);
		}

		double luminance;

		for (Uint32 i = 0; i < gray_shades; i++)
		{
			luminance = 255.0 / (gray_shades - 1)*i;
			*(color++) = Color<double>(luminance, luminance, luminance, 255);
		}

		return std::move(palette);
	}

	//Incializacja palety metod¹ przek¹tnej bloku o rozmiarach odchyleñ standardowych w przestrzeni RGB.
	static std::vector<Color<double>> RGB_diagonal_deviation(const std::vector<ColorU8>& pixels, Uint32 palette_size)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (palette_size < 1)
			return std::vector<Color<double>>();



		//OBLICZENIE ŒREDNIEJ:
		auto mean = getAverageColor(pixels);
		if (palette_size == 1)
			return std::vector<Color<double>>(mean);
		


		//OBLICZENIE ODCHYLEÑ STANDARDOWYCH:
		long double r_sd = 0,  g_sd = 0,  b_sd = 0;
		long double r_dif = 0, g_dif = 0, b_dif = 0;

		for (auto& pixel : pixels)
		{
			r_dif = pixel.r - mean.r;
			g_dif = pixel.g - mean.r;
			b_dif = pixel.b - mean.r;

			r_sd += r_dif * r_dif;
			g_sd += g_dif * g_dif;
			b_sd += b_dif * b_dif;
		}

		r_sd /= (long double)pixels.size();
		g_sd /= (long double)pixels.size();
		b_sd /= (long double)pixels.size();

		r_sd = std::sqrt(r_sd);
		g_sd = std::sqrt(g_sd);
		b_sd = std::sqrt(b_sd);

		//OBLICZENIE KRAWÊDZI SZEŒCIANU:
		long double r_beg = 0, g_beg = 0, b_beg = 0;
		long double r_end = 0, g_end = 0, b_end = 0;
		long double r_len = 0, g_len = 0, b_len = 0;

		r_beg = mean.r - r_sd;
		r_end = mean.r + r_sd;

		g_beg = mean.g - g_sd;
		g_end = mean.g + g_sd;

		b_beg = mean.b - b_sd;
		b_end = mean.b + b_sd;

		//OGRANICZENIE KRAWÊDZ DO RGB:
		r_end = (r_end < 255.0) ? r_end : 255;
		g_end = (g_end < 255.0) ? g_end : 255;
		b_end = (b_end < 255.0) ? b_end : 255;

		r_beg = (r_beg > 0.0) ? r_beg : 0;
		g_beg = (g_beg > 0.0) ? g_beg : 0;
		b_beg = (b_beg > 0.0) ? b_beg : 0;

		//WYZNACZENIE D£UGOSCI:
		r_len = r_end - r_beg;
		g_len = g_end - g_beg;
		b_len = b_end - b_beg;



		//WYZNACZENIE PRZEKATNEJ (PALETY):
		std::vector<Color<double>> palette;
		palette.resize(palette_size);
		auto  color = palette.data();

		double percentage;

		for (Uint32 i = 0; i < palette_size; i++)
		{
			percentage = 1.0 / (palette_size - 1)*i;
			*(color++) = Color<double>(
				r_beg + r_len * percentage,
				g_beg + g_len * percentage,
				b_beg + b_len * percentage,
				255);
		}

		return std::move(palette);
	}

	template<typename T>
	static Palette vectorToPalette(const std::vector<Color<T>>& pixels)
	{
		Palette palette;
		palette.colors.resize(pixels.size());

		auto pixel = pixels.begin();

		for (auto& color : palette.colors)
			color = *pixel++;

		return std::move(palette);
	}

	//----------------------------------------------------------------------//
	//------------// GENEROWANIE PALETY W OPARCIU O PARAMETRY //------------//
	//----------------------------------------------------------------------//

	//Wybiera okreslon¹ liczbê, tak samo od siebie odleg³ych odcieni szarosci na ca³ym zakresie jasnoœci.
	//Wynikowy rozmiar palety bêdzie rowny 'gray_shades'.
    static Palette evenSpaceDivisionGray(Uint32 gray_shades)
    {
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (gray_shades < 1 || gray_shades > 256)
			return Palette();

		Palette palette;
		palette.colors.resize(gray_shades);
		auto  color = palette.colors.data();

		if (gray_shades == 1)
		{
			*color = ColorU8(128,128,128);
			return std::move(palette);
		}
		
        float luminance;

        for(Uint32 i = 0; i < gray_shades; i++)
        {
            luminance = std::round(255.f/(float)(gray_shades-1)*(float)i);
            *(color++) = ColorU8(luminance, luminance, luminance, 255);
        }

		return std::move(palette);
    }

    //Wybiera okreslon¹ liczbê, tak samo od siebie odleg³ych odcieni czerwonego, zielonego i niebieskiego na ca³ym
    //zakresie odcieni RGB. Paleta to bed¹ wszystkie mo¿liwe kombinacje tych odcieni. Wynikowy rozmiar palety bedzie
    //wiêc równy 'red_shades * green_shades * blue_shades'. W praktyce przestrzen kolorow RGB zostanie podzielona na
    //prostopadloœciany o równych rozmiarach.
	static Palette evenSpaceDivisionRGB(Uint32 red_shades, Uint32 green_shades, Uint32 blue_shades)
    {
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (
			red_shades < 1 || green_shades < 1 || blue_shades < 1 ||
			(red_shades * green_shades * blue_shades == 1) ||
			red_shades > 256 || green_shades > 256 || blue_shades > 256
		) return Palette();


		Palette palette;
		palette.colors.resize(red_shades*green_shades*blue_shades);

		auto  color = palette.colors.data();
        float r, g, b;
		
        for(Uint32 i = 0; i < red_shades; i++)
        {
            r = std::round(255.f/(float)(red_shades-1)*(float)i);

            for(Uint32 j = 0; j < green_shades; j++)
            {
                g = std::round(255.f/(float)(green_shades-1)*(float)j);

                for(Uint32 k = 0; k < blue_shades; k++)
                {
                    b = std::round(255.f/(float)(blue_shades-1)*(float)k);
                    *(color++) = ColorU8(r, g, b, 255);
                }
            }
        }

		return std::move(palette);
    }

	//Tworzy losow¹ paletê kolorów. Mo¿na okreœliæ iloœæ mo¿liwych odcieni dla ka¿dej sk³¹dowej RGB.
	static Palette random(Uint32 red_shades, Uint32 green_shades, Uint32 blue_shades)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (
			red_shades < 1 || green_shades < 1 || blue_shades < 1 ||
			(red_shades * green_shades * blue_shades == 1) ||
			red_shades > 256 || green_shades > 256 || blue_shades > 256
		) return Palette();


		Palette palette;
		palette.colors.resize(red_shades*green_shades*blue_shades);

		bool unique;
		for (auto pt = palette.colors.begin(); pt != palette.colors.end(); pt++)
		{
			do{
				unique = true;
				pt->r = std::round(std::round((float)rand() / (float)RAND_MAX*(float)(red_shades - 1))*255.f/(float)(red_shades - 1));
				pt->g = std::round(std::round((float)rand() / (float)RAND_MAX*(float)(green_shades - 1))*255.f / (float)(green_shades - 1));
				pt->b = std::round(std::round((float)rand() / (float)RAND_MAX*(float)(blue_shades - 1))*255.f / (float)(blue_shades - 1));
				pt->a = 255;

				for (auto pt2 = palette.colors.begin(); pt2 != pt; pt2++)
				{
					if (*pt2 == *pt) {
						unique = false;
						break;
					}
				}
			} while (!unique);
		}
		return std::move(palette);
	}

	//----------------------------------------------------------------------//
	//----------------------// METODY KWANTYZACJI //------------------------//
	//----------------------------------------------------------------------//

	//Generuje paletê na podstawie histogramu. Przestrzeñ RGB dzielona jest na okreœlon¹ liczbê prostopad³oœcianów
	//zale¿n¹ od wartoœci green_shades, red_shades, blue_shades. Dla ka¿dego prostopad³oœcianu sumowana jest iloœæ
	//kolorów wystêpuj¹cych w obrazie która mieœci siê w tym fragmencie przestrzeni RGB. Paleta jest tworzona ze
	//œrodkowych kolorów prostopad³oœcianów zawieraj¹cych w sobienajwiêcej kolorów.
	//K - rozmiar palety
	//Nr, Ng, Nb - mo¿liwe odcienie czerwieni, zielonego i niebieskiego
	static Palette population(const std::vector<ColorU8>& pixels, Uint32 pallete_size,
		Uint32 red_shades, Uint32 green_shades, Uint32 blue_shades)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (
			!pallete_size ||
			pixels.empty() || pixels.size() < pallete_size ||
			red_shades < 1 || green_shades < 1 || blue_shades < 1 ||
			(red_shades * green_shades * blue_shades == 1) ||
			red_shades > 256 || green_shades > 256 || blue_shades > 256
		) return Palette();


		Palette palette;
		palette.colors.resize(pallete_size);

		Uint32 r_id, g_id, b_id, id = 0;

		//Sporz¹dzenie histogramu (first: histogram, second: id).
		std::vector<std::pair<Uint32, Uint32>> histogram;
		histogram.resize(red_shades*green_shades*blue_shades);
		for (auto& pair : histogram)
			pair.second = id++;

		for (const auto& pixel : pixels)
		{
			r_id = (float)pixel.r / 256.f * (float)red_shades;
			g_id = (float)pixel.g / 256.f * (float)green_shades;
			b_id = (float)pixel.b / 256.f * (float)blue_shades;

			histogram[r_id + red_shades * g_id + red_shades * green_shades*b_id].first++;
		}

		//Posortowanie histogramu (malej¹co):
		std::sort(histogram.begin(), histogram.end(),
			[](auto &left, auto &right) { return left.first > right.first; });

		//Konwersja histogramu na paletê:
		auto  color = palette.colors.data();
		auto  hist_p = histogram.data();

		for (Uint32 i = 0; i < pallete_size; i++)
		{
			*(color++) = ColorU8(
				256.f / (float)red_shades *  (float)((*(hist_p)).second % (red_shades)) + 127.5f / (float)red_shades,
				256.f / (float)green_shades * (float)(((*(hist_p)).second % (red_shades*green_shades)) / red_shades) + 127.5f / (float)green_shades,
				256.f / (float)blue_shades *  (float)(((*(hist_p++)).second % (red_shades*green_shades*blue_shades)) / (red_shades*green_shades)) + 127.5f / (float)blue_shades,
				255
			);
		}

		return std::move(palette);
	}


private:
	template<typename T>
	static std::vector<ColorU8> medianCutRecursive(T begin, T end, Uint32 median_cuts)
	{
		// !!! NIEPOWINNO MIEC NIGDY MIEJSCA - DO DEBUGOWANIA !!! //
		if (!(end - begin)){
			std::cerr << "\n\n\n\nMEDIANCUT DEVIDE BY ZERO\n\n\n\n";
			getchar();
			return std::vector<ColorU8>();
		}


		//--------- ZAKOÑCZENIE REKURENCJI ----------//

		if (median_cuts == 0)
		{
			uint64_t r_sum = 0, g_sum = 0, b_sum = 0;

			for (auto color = begin; color != end; color++)
			{
				r_sum += color->r;
				g_sum += color->g;
				b_sum += color->b;
			}

			std::vector<ColorU8> result;
			result.reserve(1);
			result.push_back(ColorU8(
				r_sum / (end - begin),
				g_sum / (end - begin),
				b_sum / (end - begin)));

			return std::move(result);
		}

		//----------- CZÊŒC REKURENCYJNA ---------//

		ColorU8 min, max, range;

		//Ustalenie która ze sk³adowych RGB ma najwiêkszy zakres.
		max.setColor(0, 0, 0);
		min.setColor(255, 255, 255);
		range.setColor(0, 0, 0);
		for (auto color = begin; color != end; color++)
		{
			if (color->r > max.r) max.r = color->r;
			if (color->g > max.g) max.g = color->g;
			if (color->b > max.b) max.b = color->b;

			if (color->r < min.r) min.r = color->r;
			if (color->g < min.g) min.g = color->g;
			if (color->b < min.b) min.b = color->b;
		}
		range = max - min;

		//Posortowanie wed³ug sk³adowej o najwiêkszym zakresie.
		if (range.r >= range.g && range.r >= range.b)
			std::sort(begin, end, [](auto &left, auto &right) { return left.r > right.r; });

		else if (range.g >= range.r && range.g >= range.b)
			std::sort(begin, end, [](auto &left, auto &right) { return left.g > right.g; });

		else if (range.b >= range.r && range.b >= range.g)
			std::sort(begin, end, [](auto &left, auto &right) { return left.b > right.b; });

		//Kolejne podzia³y.
		auto part_a = medianCutRecursive(begin, begin + (end - begin) / 2, median_cuts - 1);
		auto part_b = medianCutRecursive(begin + (end - begin) / 2, end, median_cuts - 1);

		//Zebranie wyników.
		std::vector<ColorU8> result;
		result.reserve(part_a.size() + part_b.size());
		result.insert(result.end(), part_a.begin(), part_a.end());
		result.insert(result.end(), part_b.begin(), part_b.end());

		return std::move(result);
	}
public:
	//Generujê paletê metod¹ po³owicznego podzia³u. Barwy wszystkich pikseli s¹ sortowane
	//wzglêdem sk³adowej o najwiêkszym zakresie a nastêpnie s¹ dzielone na dwie równe po³owy.
	//W tych po³owach powtarzany jest ten proces a¿ do osi¹gniêcia ookreœlonej iloœci podzia³ow.
	//Z wszystkich pomniejszych przedzia³ów liczona jest wartoœæ œrednia koloru który nastêpnie
	//jest dodawany do palety. Rozmiar palety bêdzie wiêc równy 2^iloœæ_podzia³ów.
	//W przypadku 8 podzia³ów wynikowa paleta bêdzie siê sk³adaæ z 256 barw.
	static Palette medianCut(std::vector<ColorU8> pixels, Uint32 median_cuts)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (!median_cuts || pixels.empty() || median_cuts < 1 || pixels.size() < std::pow(2, median_cuts))
			return Palette();

		Palette palette;
		palette.colors = std::move(medianCutRecursive(pixels.begin(), pixels.end(), median_cuts));
		return std::move(palette);
	}

	//Wybiera z podanych pixeli okreœlon¹ iloœæ losowych pixeli i tworzy z nich paletê.
	static Palette uniformRandom(const std::vector<ColorU8>& pixels, Uint32 pallete_size)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (!pallete_size || pixels.empty() || pixels.size() < pallete_size)
			return Palette();

		Palette palette;
		palette.colors.resize(pallete_size);

		for (auto& color : palette.colors)
			color = pixels[std::round((float)rand() / (float)(RAND_MAX) * (float)(pixels.size() - 1))];

		return std::move(palette);
	}

	//Wybiera z podanych pixeli okreœlon¹ iloœæ losowych pixeli (bez powtrzeñ) i tworzy z nich paletê.
	static Palette uniformRandomUnique(const std::vector<ColorU8>& pixels, Uint32 pallete_size)
	{
		//SYTUACJE W KTORYCH METODA WYKONA SIE NIEPOPRAWNIE:
		if (!pallete_size || pixels.empty() || pixels.size() < pallete_size)
			return Palette();

		Palette palette;
		palette.colors.resize(pallete_size);

		std::vector<size_t> indexes(pallete_size);
		fill_with_random_indexes(indexes, pixels.size());

		auto color = palette.colors.begin();

		for (auto& index : indexes)
			*(color++) = pixels[index];

		return std::move(palette);
	}


	//Generuje paletê metod¹ segmentacji k-means.
	static Palette kMeans(const std::vector<ColorU8>& pixels, Uint32 pallete_size, Uint32 initialization_method)
	{

		if (!pallete_size || pixels.empty() || pixels.size() < pallete_size)
			return Palette();

		//------// PRZYGOTOWANIE PAMIÊCI I INICIALIZACJA //------//

		std::vector<Color<double>> clusters;

		{
			Palette clusters_uint8;

			switch (initialization_method)
			{
			//METODY BEZPOŒREDNIE:

			case init_DC:
				clusters = std::move(RGB_diagonal(pallete_size));
				break;


			case init_SD:
				clusters = std::move(RGB_diagonal_deviation(pixels, pallete_size));
				break;

			//METODY POŒREDNIE:

			case init_RAND:
				clusters_uint8 = std::move(uniformRandom(pixels, pallete_size));
				if (!clusters_uint8.size())
					return Palette(); //Nie udana inicializacja.
				break;

			case init_RANDU:
				clusters_uint8 = std::move(uniformRandomUnique(pixels, pallete_size));
				if (!clusters_uint8.size())
					return Palette(); //Nie udana inicializacja.
				break;

			case init_WU:
				clusters_uint8 = std::move(wu(pixels, pallete_size));
				if (!clusters_uint8.size())
					return Palette(); //Nie udana inicializacja.
				break;

			case init_MCUT:
				clusters_uint8 = std::move(medianCut(pixels, pallete_size));
				if (!clusters_uint8.size())
					return Palette(); //Nie udana inicializacja.
				break;

			default:
				return Palette(); //Nie udana inicializacja.
			}

			//Konwersja na double:
			if (clusters_uint8.size())
			{
				clusters.resize(pallete_size);
				auto cluster = clusters.begin();
				for (auto& color : clusters_uint8.colors)
					*(cluster++) = color;
			}
		}

		
		//------------// W£AŒCIWY ALGORYTM K-MEANS //------------//

		//Pamiêæ do przechowywnia indexu klasteru do którego aktualnie
		//przynale¿y dany pixel.
		std::vector<size_t> pixels_clusters(pixels.size());

		bool next_iteration = true;
		while(next_iteration)
		{
			//flaga okreœlaj¹ca czy wykonaæ kolejn¹ iteracjê.
			next_iteration = false;

			//1. ODNALEZIENIE NAJBLI¯SZEGO KLASTRU DLA WSZYSTKICH PIXELI:
			{
#define CLOSEST1
#ifdef CLOSEST1
				//Zmienne robocze:
				size_t closest_cluster;
				double r_dif, g_dif, b_dif;
				double old_distance;
				double new_distance;

				auto pixel_cluster = pixels_clusters.begin();

				for (auto pixel = pixels.begin(); pixel != pixels.end(); pixel++)
				{
					old_distance = std::numeric_limits<double>::infinity();

					for (auto cluster = clusters.begin(); cluster != clusters.end(); cluster++)
					{
						//Obliczenie b³êdu od koloru z palety.
						r_dif = cluster->r - pixel->r;
						g_dif = cluster->g - pixel->g;
						b_dif = cluster->b - pixel->b;

						new_distance = r_dif * r_dif + g_dif * g_dif + b_dif * b_dif;

						//Poszukiwnie najlepszego koloru.
						if (new_distance < old_distance) {
							old_distance = new_distance;
							closest_cluster = cluster - clusters.begin();
						}
					}
					*(pixel_cluster++) = closest_cluster;
				}
#endif
#ifdef CLOSEST2
#endif
			}

			//2. OBLICZENIE NOWYCH POZYCJI KLASTROW JAKO ŒREDNI¥ POZYCJI PRZYPISANYCH PIXELI:
			{
				//Miejsce na dane potrzebne do obliczeñ œrednich:
				std::vector<std::vector<double>> rgbc(4);
				for (auto& data : rgbc)
					data.resize(clusters.size());

				std::vector<double>& r_sums = rgbc[0];
				std::vector<double>& g_sums = rgbc[1];
				std::vector<double>& b_sums = rgbc[2];
				std::vector<double>& counts = rgbc[3];


				//Zliczenie przynale¿nych pixeli i sumowanie sk³aowych.
				auto pixel = pixels.begin();
				auto pixel_cluster = pixels_clusters.begin();

				for (; pixel != pixels.end();)
				{
					r_sums[*pixel_cluster] += pixel->r;
					g_sums[*pixel_cluster] += pixel->g;
					b_sums[*pixel_cluster] += pixel->b;
					counts[*pixel_cluster]++;

					pixel_cluster++;
					pixel++;
				}


				//Obliczenie œredniej - nowej pozycji klastrów.
				auto cluster = clusters.begin();
				auto r_sum = r_sums.begin();
				auto g_sum = g_sums.begin();
				auto b_sum = b_sums.begin();
				auto count = counts.begin();

				Color<double> old_cluster;

				for (size_t i = 0; i < clusters.size(); i++)
				{
					if (*count != 0)
					{
						old_cluster = *cluster;

						cluster->r = *r_sum / *count;
						cluster->g = *g_sum / *count;
						cluster->b = *b_sum / *count;

						if(Color<double>::differenceRGB_nosqrt(old_cluster, *cluster) > 3.0)
							//KOLEJNA ITERACJA: Zbyt ma³a dok³adnoœæ.
							next_iteration = true;

						cluster++;
					}
					else
						//KOLEJNA ITERACJA: Wyst¹pi³ pusty klaster.
						next_iteration = true;

					r_sum++;
					g_sum++;
					b_sum++;
					count++;
				}

				//Pozbycie siê pozosta³ej pamiêci wynikaj¹cej z pustych klastrów:
				clusters.erase(cluster, clusters.end());
			}
		}



		//-----------// KONWERSJA WYNIKOW NA UINT //------------//

		//Przygotowanie pamiêci na wyniki:
		Palette palette;
		palette.colors.resize(clusters.size());

		auto cluster = clusters.begin();

		for (auto& color : palette.colors)
			color = *(cluster++);

		return std::move(palette);
	}

	//Generuje paletê metod¹ WU:
	//https://gist.github.com/bert/1192520
	static Palette wu(const std::vector<ColorU8>& pixels, Uint32 palette_size)
	{
		if (!palette_size || pixels.empty() || pixels.size() < palette_size || palette_size > WU_MAXCOLOR)
			return Palette();

		Palette palette; 
		palette.colors.resize(palette_size);

		//Zwraca faktyczny rozmiar palety:
		palette_size = wucq::WuColorQuantizer(
			(unsigned int*)pixels.data(),
			pixels.size(), palette_size,
			(unsigned int*)palette.colors.data());

		palette.colors.resize(palette_size);

		return std::move(palette);
	}
};