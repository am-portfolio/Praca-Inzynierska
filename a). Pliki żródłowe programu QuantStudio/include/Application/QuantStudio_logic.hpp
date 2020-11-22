/**********************************************************************
QuantStudio - application logic.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <map>
#include <functional>
#include <filesystem>
#include <regex>

#include <Quantisation/Palette.h>
#include <Quantisation/Image.hpp>

#include <DataIO/VectorLogger.hpp>
#include <DataIO/ProgressLogger.hpp>

#include <Utility/Clock.hpp>

#include <System/Windows_Console.hpp>
#include <System/Windows_MessageBox.hpp>

#include <MatlabIO/Matlab_Tasker.hpp>

#include <Application/splashScreen.hpp>
#include <Application/waitBar.hpp>


//Gotowa klasa generuj�ca wiele wynik�w kwantyzacji dwu etapowej.
//W jaki spos�b rozbudowywa�:
//- nowe wska�niki jakosci w miejscu pod define: MIEJSCE_DEFINIOWANIA_KOLEJNYCH_WSKA�NIKOW
//- nowe metody gnerowania palety pod define: MIEJSCE_DEFINIOWANIA_KOLEJNYCH_METOD_GENEROWANIA_PALETY
//- nowe metody gnerowania pr�bki pixeli pod define: MIEJSCE_DEFINIOWANIA_KOLEJNYCH_METOD_PROBKI_PIXELI
class BatchQuantizationRunner
{
	//OBIEKT DO KOMUNIKACJI Z MATLABEM.
	MATLAB_Tasker matlab;


	//----------------------------------------------------------------------//
	//-----// PAMI�C NA DANE FUNKCJI TWORZENIA PALETY I PROBKI PIXELI //----//
	//----------------------------------------------------------------------//

	using MethodName	  = std::string;
	using PaletteFunction = std::function<Palette(const std::vector<ColorU8>&)>;
	using RescaleFunction = std::function<std::vector<ColorU8>(const ImageU8&)>;
	using RescaleAmount   = double;

	//Wektor par: metod� generowania palety ze zbindowanymi parametrami
	//wej�ciowimi + string opisuj�cy nazw� metody i u�yte parametry wej�ciowe. 
	std::vector<std::tuple<MethodName, PaletteFunction>> palette_functions;
	//Wektor par: metod� generowania pr�bki pixeli ze zbindowanymi parametrami
	//wej�ciowimi + string opisuj�cy nazw� metody i u�yte parametry wej�ciowe. 
	std::vector<std::tuple<MethodName, RescaleFunction, RescaleAmount>> rescale_functions;


	class uniqueIndex
	{
		static size_t index_count;
		size_t index;

	public:
		uniqueIndex()
		{
			index = index_count;
			index_count++;
		}
		operator const size_t() const{
			return index;
		}

		static size_t getCount()
		{
			return index_count;
		}
	};

	//ZDEFINIOWANIE NAZW WSKA�NIKOW JAKO�CI (mapa tworzona przy zapisie)
	std::map<std::string, uniqueIndex> rating_names;

	//Ilo�� pow�rze� algorytmu dla algorytm�w niedeterministycznych
	//(zapisanie zostana warto�� �rednia).
	unsigned int random_repeat_count;

	//Okre�la czy zapisywa� wszystkie graficzne wyniki w przypadku metod niedeterministycznych
	//czy tylko pierwszy (maksymalnie tyle wynik�w ile ma warto�� random_repeat_count)
	bool random_repeat_save_all_images;

	//----------------------------------------------------------------------//
	//--------// ZMIENNE DEFINIUJ�CE USTAWIENIA I INNE TYPEDEFY //----------//
	//----------------------------------------------------------------------//

	//Typ do przechowywania danych typu PSNR, TIME... dla jednej grafiki,
	//przetworzonej wszystkimi metodami tworzenia palaty i pr�bki pixeli.
	//data[palette_method][resize_method][wskaznik_jakosci]
	using ImageOutputData = std::vector<std::vector<std::vector<long double>>>;

	using Path = std::filesystem::path;


public:

	//Wczytuje dodatkowe pliki konfiguracyjne.
	void loadConfigs(const Path& config_directory)
	{
		//Funkcja zmieniaj�ca rozmiar wszystkich wektor�w sk�adowych. Pozwala to unikn��
		//nieuprawnionego dost�pu i crashu programu w sytuacji gdy w pliku by�o za ma�o danych.
		auto resize_rows = [](std::vector<std::vector<double>>& vector, size_t size)
		{
			for (auto& row : vector)
				row.resize(size);
		};

		const auto loadFile = std::bind(VectorLogger::loadFile<double>, std::placeholders::_1, true);
		const std::string extension(".csv");

		std::vector<std::vector<double>> parameters;

		//Upewnienie si�, �e folder z ustawieniami dodatkowymi istnieje.
		std::filesystem::create_directories(config_directory);




		const unsigned int random_repeat_count_default_value = 5;
		parameters = loadFile(config_directory / "random_repeat_count" + extension);
		resize_rows(parameters, 1);
		if(!parameters.size())
		{
			VectorLogger file(config_directory / "random_repeat_count" + extension);
			file.append_comment("Ilo�� powt�rze� algorytm�w niedeterministycznych,");
			file.append_comment("podczas liczenia �rednich warto�ci wska�nik�w.");
			file.append_comment("UINT[1 - ...]");
			file.append(std::vector<unsigned int>{random_repeat_count_default_value});

			random_repeat_count = random_repeat_count_default_value;
		}
		else
		{
			unsigned int value = parameters[0][0];
			random_repeat_count = (value != 0) 
				? value : 1;
		}




		const unsigned int random_repeat_save_all_images_default_value = 0;
		parameters = loadFile(config_directory / "random_repeat_save_all_images" + extension);
		resize_rows(parameters, 1);
		if (!parameters.size())
		{
			VectorLogger file(config_directory / "random_repeat_save_all_images" + extension);
			file.append_comment("Ilo�� powt�rze� algorytm�w niedeterministycznych,");
			file.append_comment("podczas liczenia �rednich warto�ci wska�nik�w.");
			file.append_comment("BOOL[0, 1]");
			file.append(std::vector<unsigned int>{random_repeat_save_all_images_default_value});

			random_repeat_save_all_images = random_repeat_save_all_images_default_value;
		}
		else
		{
			unsigned int value = parameters[0][0];
			random_repeat_save_all_images = (value == 1)
				? 1 : 0;
		}
	}

	//Wczytuje ustawienia parametr�w dla funkcji tworzenia palety i zmiany ilo�ci pixeli 
	//z odpowiednich plik�w i tworzy obiekty w kt�rych binduje te parametry z dan� funkcja.
	void loadSettings(const Path& settings_directory)
	{
		//Wczytanie dodatkowych plik�w konfiguracyjnych.
		loadConfigs(settings_directory / "config");


		//Funkcja zmieniaj�ca rozmiar wszystkich wektor�w sk�adowych. Pozwala to unikn��
		//nieuprawnionego dost�pu i crashu programu w sytuacji gdy w pliku by�o za ma�o danych.
		auto resize_rows = [](std::vector<std::vector<double>>& vector, size_t size)
		{
			for (auto& row : vector)
				row.resize(size);
		};



		const auto loadFile = std::bind(VectorLogger::loadFile<double>, std::placeholders::_1, true);
		const std::string extension(".csv");
		std::vector<std::vector<double>> parameters;
		std::stringstream sstream;

		//Upewnienie si�, �e folder z ustawieniami istnieje.
		std::filesystem::create_directories(settings_directory);


		//INFORMACJE URZYTKOWANIA W POSTACI KOMENTARZY:
		const std::vector<std::string> info = {
			" Ka�dy wiersz warto�ci odpowiada jednej kombinacji parametr�w wej�ciowych.",
			" UWAGA: Nie zmieniaj nazwy pliku, zapisz plik przed klikni�ciem �Uruchom�, ka�dy wiersz zako�cz �rednikiem.\n" };

		//-------------------------------------------------------------------//
		//------------UPEWNIENIE SI� �E WEKTORY S� OBECNIE PUSTER------------//
		//-------------------------------------------------------------------//

		palette_functions.clear();
		rescale_functions.clear();



		//-------------------------------------------------------------------//
		//--------ZA�ADOWANIE DANYCH ZWI�ZANYCH Z GENEROWANIEM PALETY--------//
		//-------------------------------------------------------------------//

//################################################################################
#define MIEJSCE_DEFINIOWANIA_KOLEJNYCH_METOD_GENEROWANIA_PALETY
//################################################################################

		if(!std::filesystem::exists(settings_directory / "p_population" + extension))
		{
			VectorLogger file(settings_directory / "p_population" + extension);
			file.append_comment(" palette_size[UINT], red_shades[UINT 1-256], green_shades[UINT 1-256], blue_shades[UINT 1-256]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'population' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append(std::vector<std::vector<int>>{
				{4,  4, 4, 4},
				{8,  4, 4, 4},
				{16, 4, 4, 4},
				{32, 8, 8, 8},
				{64, 8, 8, 8},
				{128,16,16,16},
				{256,16,16,16}
			});
		}
		parameters = loadFile(settings_directory / "p_population" + extension);
		resize_rows(parameters, 4);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "population_" << arg[0] << "_" << arg[1] << "_" << arg[2] << "_" << arg[3];
			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::population,
					std::placeholders::_1, arg[0], arg[1], arg[2], arg[3])
			);
		}



		if (!std::filesystem::exists(settings_directory / "p_medianCut" + extension))
		{
			VectorLogger file(settings_directory / "p_medianCut" + extension);
			file.append_comment(" median_cuts[UINT]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'medianCut' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(std::vector<int>{2, 3, 4, 5, 6, 7, 8});
		}
		parameters = loadFile(settings_directory / "p_medianCut" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "medianCut_" << arg[0];
			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::medianCut,
					std::placeholders::_1, arg[0])
			);
		}



		if (!std::filesystem::exists(settings_directory / "p_uniformRandom" + extension))
		{
			VectorLogger file(settings_directory / "p_uniformRandom" + extension);
			file.append_comment(" palette_size[UINT]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'uniformRandom' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(std::vector<int>{4, 8, 16, 32, 64, 128, 256});
		}
		parameters = loadFile(settings_directory / "p_uniformRandom" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "uniformRandom_" << arg[0];
			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::uniformRandom,
					std::placeholders::_1, arg[0])
			);
		}



		if (!std::filesystem::exists(settings_directory / "p_uniformRandomUnique" + extension))
		{
			VectorLogger file(settings_directory / "p_uniformRandomUnique" + extension);
			file.append_comment(" palette_size[UINT]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'uniformRandomUnique' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(std::vector<int>{4, 8, 16, 32, 64, 128, 256});
		}
		parameters = loadFile(settings_directory / "p_uniformRandomUnique" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "uniformRandomUnique_" << arg[0];
			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::uniformRandomUnique,
					std::placeholders::_1, arg[0])
			);
		}


		if (!std::filesystem::exists(settings_directory / "p_kMeans" + extension))
		{
			VectorLogger file(settings_directory / "p_kMeans" + extension);
			file.append_comment(" palette_size[UINT], initialization_method{0 - DC, 1 - SD, 2 - WU, 3 - MCUT, 4 - RAND, 5 - RANDUQ}\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'kMeans' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append(std::vector<std::vector<int>>{
				{ 4,	Palette::InitEnum::init_WU },
				{ 8,	Palette::InitEnum::init_WU },
				{ 16,	Palette::InitEnum::init_WU },
				{ 32,	Palette::InitEnum::init_WU },
				{ 64,	Palette::InitEnum::init_WU },
				{ 128,	Palette::InitEnum::init_WU },
				{ 256,	Palette::InitEnum::init_WU },

				{ 4,	Palette::InitEnum::init_SD },
				{ 8,	Palette::InitEnum::init_SD },
				{ 16,	Palette::InitEnum::init_SD },
				{ 32,	Palette::InitEnum::init_SD },
				{ 64,	Palette::InitEnum::init_SD },
				{ 128,	Palette::InitEnum::init_SD },
				{ 256,	Palette::InitEnum::init_SD },

				{ 4,	Palette::InitEnum::init_RANDU },
				{ 8,	Palette::InitEnum::init_RANDU },
				{ 16,	Palette::InitEnum::init_RANDU },
				{ 32,	Palette::InitEnum::init_RANDU },
				{ 64,	Palette::InitEnum::init_RANDU },
				{ 128,	Palette::InitEnum::init_RANDU },
				{ 256,	Palette::InitEnum::init_RANDU }
			});
		}
		parameters = loadFile(settings_directory / "p_kMeans" + extension);
		resize_rows(parameters, 2);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "kMeans_" << arg[0] << "_";

			switch((int)arg[1])
			{
			case Palette::InitEnum::init_DC:
				sstream << "DC";
				break;
			case Palette::InitEnum::init_SD:
				sstream << "SD";
				break;
			case Palette::InitEnum::init_RAND:
				sstream << "RAND";
				break;
			case Palette::InitEnum::init_RANDU:
				sstream << "RANDUQ";
				break;
			case Palette::InitEnum::init_MCUT:
				sstream << "MCUT";
				break;

			case Palette::InitEnum::init_WU:
			default:
				sstream << "WU";
				arg[1] = Palette::InitEnum::init_WU;
				break;
			}

			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::kMeans,
					std::placeholders::_1, arg[0], (int)arg[1])
			);
		}



		if (!std::filesystem::exists(settings_directory / "p_WU" + extension))
		{
			VectorLogger file(settings_directory / "p_WU" + extension);
			file.append_comment(" palette_size[UINT, 1 - 256]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia palety metod� 'WU' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(std::vector<int>{4, 8, 16, 32, 64, 128, 256});
		}
		parameters = loadFile(settings_directory / "p_WU" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			sstream.str(std::string());
			sstream << "WU_" << arg[0];
			palette_functions.emplace_back(
				sstream.str(),
				std::bind(Palette::wu,
					std::placeholders::_1, arg[0])
			);
		}



		//-------------------------------------------------------------------//
		//-----ZA�ADOWANIE DANYCH ZWI�ZANYCH Z GENEROWANIEM PROBKI PIXELI----//
		//-------------------------------------------------------------------//

		rescale_functions.emplace_back(
			std::string("none"),
			std::bind([](const ImageU8& image)->const std::vector<ColorU8>& { return image; },
				std::placeholders::_1),
			1.0f
		);


		//WEKTOR DOMY�LNEGO RESIZE:
		std::vector<double> rescale_amounts;
		for (int N = 1; N <= 8; N++) //10 stopni skalowania.
			rescale_amounts.push_back(1.0 / std::pow(2, N));

//################################################################################
#define MIEJSCE_DEFINIOWANIA_KOLEJNYCH_METOD_GENEROWANIA_PROBKI_PIXELI
//################################################################################

		if (!std::filesystem::exists(settings_directory / "r_nearestNeighbour" + extension))
		{
			VectorLogger file(settings_directory / "r_nearestNeighbour" + extension);
			file.append_comment(" percentage[DOUBLE 0-1]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia pr�bki pixeli metod� 'nearestNeighbour' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(rescale_amounts);
		}
		parameters = loadFile(settings_directory / "r_nearestNeighbour" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			rescale_functions.emplace_back(
				"nearestNeighbour",
				std::bind(ImageU8::resizeNearestNeighbour,
					std::placeholders::_1, arg[0]),
				arg[0]
			);
		}



		if (!std::filesystem::exists(settings_directory / "r_bilinear" + extension))
		{
			VectorLogger file(settings_directory / "r_bilinear" + extension);
			file.append_comment(" percentage[DOUBLE 0-1]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia pr�bki pixeli metod� 'bilinear' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(rescale_amounts);
		}
		parameters = loadFile(settings_directory / "r_bilinear" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			rescale_functions.emplace_back(
				"bilinear",
				std::bind(ImageU8::resizeBilinear,
					std::placeholders::_1, arg[0]),
				arg[0]
			);
		}



		if (!std::filesystem::exists(settings_directory / "r_random" + extension))
		{
			VectorLogger file(settings_directory / "r_random" + extension);
			file.append_comment(" percentage[DOUBLE 0-1]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia pr�bki pixeli metod� 'random' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(rescale_amounts);
		}
		parameters = loadFile(settings_directory / "r_random" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			rescale_functions.emplace_back(
				"random",
				std::bind(ImageU8::sampleRandom,
					std::placeholders::_1, arg[0]),
				arg[0]
			);
		}



		if (!std::filesystem::exists(settings_directory / "r_randomUnique" + extension))
		{
			VectorLogger file(settings_directory / "r_randomUnique" + extension);
			file.append_comment(" percentage[DOUBLE 0-1]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia pr�bki pixeli metod� 'randomUnique' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(rescale_amounts);
		}
		parameters = loadFile(settings_directory / "r_randomUnique" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			rescale_functions.emplace_back(
				"randomUnique",
				std::bind(ImageU8::sampleRandomUnique,
					std::placeholders::_1, arg[0]),
				arg[0]
			);
		}



		if (!std::filesystem::exists(settings_directory / "r_everyFew" + extension))
		{
			VectorLogger file(settings_directory / "r_everyFew" + extension);
			file.append_comment(" percentage[DOUBLE 0-1]\n");
			file.append_comment(" Ustawienia dla funkcji tworzenia pr�bki pixeli metod� 'everyFew' przy generowaniu wynik�w metod� wsadow�.");
			file.append_comment(info);
			file.append_vertical(rescale_amounts);
		}
		parameters = loadFile(settings_directory / "r_everyFew" + extension);
		resize_rows(parameters, 1);
		for (auto& arg : parameters)
		{
			rescale_functions.emplace_back(
				"everyFew",
				std::bind(ImageU8::sampleEveryFew,
					std::placeholders::_1, arg[0]),
				arg[0]
			);
		}
	}

protected:

	//Sprawdza czy w danym stringu zawarte jest has�o 'rand' pomijaj�� case.
	bool stringHasWordRandom(const std::string& string)
	{
		//REGEX WYKRYWAJ�CY S�OWO 'RANDOM' BEZ PATRZENIA NA CASE:
		const std::regex random_regex("rand",
			std::regex_constants::ECMAScript | std::regex_constants::icase);

		return std::regex_search(string, random_regex);
	}

	//FUNKCJA GENERUJ�CA POMMIARY JAKO�CI:
	std::vector<long double> makeRatings(
		const Path& image_path,
		const Path& result_path,
		const ImageU8& image,
		const std::vector<ColorXYZ>& cieLabPixels,
		const ImageU8& result,
		long double time)
	{
//################################################################################
#define MIEJSCE_DEFINIOWANIA_KOLEJNYCH_WSKA�NIKOW
//################################################################################
		auto& index = rating_names;
		if (image_path.empty())
		{
			index["COLORS"];
			index["TIME"];
			index["MAE"];
			index["RMSE"];
			index["PSNR"];
			index["DE76"];

			index["DSCSI"];
			index["HPSI"];

			return std::vector<long double>();
		}

		std::vector<long double> ratings(rating_names.size());

		//WSKA�NIKI GENEROWANE LOKALNIE:
		ratings[index["COLORS"]] = result.uniqueColors();
		ratings[index["TIME"]] = time;
		ratings[index["MAE"]] = ImageU8::MAE(image, result);
		ratings[index["RMSE"]] = ImageU8::RMSE(image, result);
		ratings[index["PSNR"]] = ImageU8::PSNR(image, result);
		ratings[index["DE76"]] = ImageU8::DELTAE_76(cieLabPixels, result);


		//WSKA�NIKI GENEROWANE W MATLABIE:
		auto data = matlab.call("HPSI", image_path, result_path);
		if (!data.size() || matlab == MATLAB_Tasker::TERMINATED)
			return std::vector<long double>();
		ratings[index["HPSI"]] = data[0][0];

		data = matlab.call("DSCSI", image_path, result_path);
		if (!data.size() || matlab == MATLAB_Tasker::TERMINATED)
			return std::vector<long double>();
		ratings[index["DSCSI"]] = data[0][0];



		return std::move(ratings);
	}

	//Wykonuje wszystkie wszystkie za�adowane funkcje dla obrazu w podanej �cie�ce,
	//i zwraca wyniki. Je�li obrazu nie uda si� wczyta� to zwr�ci pusty wektor.
	//Je�eli ustawiono zapisywanie grafik to zapisze je do: output_directory/images/image_name
	//Je�eli jakiego� wyniku nie uda si� utworzy� ze wzgl�du na np: zbyt ma�� ilo�� kolor�w
	//lub z�e parametry wej�ciowe to dany element wektora b�dzie pusty.
	//Obiekt logger jest odpowiedzialny za zapisywanie log�w do pliku i strumienia,
	//oraz za zbieranie informacji o danych zdarzeniach.
	ImageOutputData processImage(const Path& path, const Path& output_directory,
		const Path& temp_directory,
		bool save_processed_images, bool use_dither, bool relative_ratings,
		ProgressLogger& logger)
	{
		//Za�adowanie grafiki.
		ImageU8 image;
		if (!image.loadFromFile(path)){
			logger << "ERROR: failed to load " << path << "\n";
			logger.stats["error"]++;
			return ImageOutputData();
		}
		else
			logger << "FILE OPEN: opened file " << path << "\n";
		
		//Przygotowanie pami�ci na wyniki.
		ImageOutputData image_output_data;
		image_output_data.resize(palette_functions.size());
		for (auto& rescale_funs : image_output_data)
			rescale_funs.resize(rescale_functions.size());
	
		//Walidacja zmiennej 'relative_ratings':
		if (std::get<MethodName>(rescale_functions[0]) != std::string("none"))
			relative_ratings = false;

		//Przygotowanie folderu na wyniki graficzne.
		if (save_processed_images)
			std::filesystem::create_directories(
				output_directory / "images" / path.stem());

		//Przygotowanie pikseli orygina�u w wersji CIELAB,
		//w celu przy�pieszenia liczenia wska�nik�w.
		const auto cieLabPixels = std::move(image.getCieLabPixels());


		for (size_t p = 0; p < palette_functions.size(); p++) {
		for (size_t r = 0; r < rescale_functions.size(); r++)
		{
			//Informacja do konsoli:
			logger
				<< "PROCESSING:   " << path.filename().generic_string()
				<< " [" << r + 1 + rescale_functions.size() * p << "/"
				<< palette_functions.size()*rescale_functions.size() << "]"
				<< ", " << std::get<MethodName>(palette_functions[p])
				<< ", " << std::get<MethodName>(rescale_functions[r])
				<< "_" << std::get<RescaleAmount>(rescale_functions[r]) << "\n";



			//Sprawdzenie czy dana metoda bazuje na losowym algorytmie:
			//(Sprawdza czy w nazwie metody wst�puje has�o 'random' bez patrzenia na case)
			bool is_non_deterministic = false;
			if (stringHasWordRandom(std::get<MethodName>(palette_functions[p])) ||
				stringHasWordRandom(std::get<MethodName>(rescale_functions[r])))
				is_non_deterministic = true;

			//Przygotowanie pami�ci na kilka zestaw�w wska�nik�w
			//(dla algorytm�w niedeterministycznych) lub na jeden zestaw.
			//W zestawie na indexie [0] zostanie wyznaczona �rednia.
			std::vector<std::vector<long double>> ratings_try;
			if (is_non_deterministic)
				ratings_try.resize(random_repeat_count);
			else
				ratings_try.resize(1);



			bool failed_to_create_palette = false;
			//WYKONANIE ODPOWIEDNIEJ ILO�CI RAZY DANEGO ZESTAWU ALGORYTMOW:
			for (int t = 0; t < ratings_try.size(); t++)
			{
				failed_to_create_palette = false;

				//START POMIARU CZASU:
				Clock clock;

				//Wygenerowanie pr�bki pixeli oraz palety kolor�w:
				auto palette = std::get<PaletteFunction>(palette_functions[p])(
					(std::get<MethodName>(rescale_functions[r]) == std::string("none")) ?
					image : //referencja na orygina�
					std::get<RescaleFunction>(rescale_functions[r])(image));

				//STOP POMIARU CZASU:
				auto time = clock.restart();


				//Je�eli nie uda�o si� wygenerowa� palety pomijamy wyniki:
				if (!palette.size())
				{
					logger
						<< "INFO: unable to create palette - "
						<< path.filename().generic_string()
						<< ", " << std::get<MethodName>(palette_functions[p])
						<< ", " << std::get<MethodName>(rescale_functions[r])
						<< "_" << std::get<RescaleAmount>(rescale_functions[r]) << "\n";
					logger.stats["skip"]++;

					failed_to_create_palette = true;
					break;
				}
				else if (ratings_try.size() > 1)
				{
					logger
						<< "REPROCESSING: " << path.filename().generic_string()
						<< " [" << r + 1 + rescale_functions.size() * p << "/"
						<< palette_functions.size()*rescale_functions.size() << "]"
						<< ", " << std::get<MethodName>(palette_functions[p])
						<< ", " << std::get<MethodName>(rescale_functions[r])
						<< "_" << std::get<RescaleAmount>(rescale_functions[r]) << "\n";
				}

				//Zastosowanie palety:
				auto result = use_dither ?
					image.applyPalette_dither(palette) :
					image.applyPalette(palette);


				//Zapisanie przetworzonej grafiki:
				if (save_processed_images && (random_repeat_save_all_images || !t))
				{
					auto file_name = path.stem().concat(
						std::string("_") + std::get<MethodName>(palette_functions[p]) +
						std::string("_") + std::get<MethodName>(rescale_functions[r]) +
						std::string("_") + std::to_string(std::get<RescaleAmount>(rescale_functions[r])) +
						((use_dither) ? std::string("_dithering") : std::string()) +
						((is_non_deterministic) ? std::string("_VERSION") + std::to_string(t + 1) : std::string()) +
						std::string(".png"));

					if (result.saveToFile(output_directory / "images" / path.stem() / file_name)) {
						logger
							<< "NEW FILE: saved to "
							<< output_directory / "images" / path.stem() / file_name << "\n";
						logger.stats["save"]++;
					}
					else {
						logger
							<< "ERROR: failed to save "
							<< output_directory / "images" / path.stem() / file_name << "\n";
						logger.stats["error"]++;
					}
				}




				//-------------------------------------------//
				//------ ZAPISANIE GRAFIKI TYMCZASOWEJ ------//
				//-------------------------------------------//

				//Program za pierwszym razem spr�buje ponownie bez pytania.
				size_t error_counter = 0;

				//Zapisanie grafiki do folderu tymczasowego (konieczne dla wska�nik�w,
				//obliczanych przez aplikacj� MATLAB Tasker (grafiki s� przekazywane,
				//jako �ciezki do plik�w).
				bool temp_image = false;
				do {
					temp_image = result.saveToFile(temp_directory / "temp_image.bmp");

					if (temp_image == false)
					{
						logger
							<< "ERROR: faild to create temp file "
							<< temp_directory / "temp_image.bmp" << "\n";
						logger.stats["error"]++;

						if(error_counter++ != 0)
						if (!Windows::messageBoxRetryCancel(
							L"Nie uda�o si� zapisa� grafiki tymczasowej:\n" +
							temp_directory / "temp_image.bmp" +
							L"\nco uniemo�liwia wygenerowanie kolejnych wska�nik�w jako�ci. "
							"Anulowanie spowoduje przerwanie przetwarzania.\n"
							"Spr�bowa� ponownie zapisa� grafik� tymczasow�?",
							L"B��d zapisu grafiki tymczasowej.",
							GetConsoleWindow()))
						{
							//Przerwanie przetwarzania z powodu b��du zapisu.
							return ImageOutputData();
						}
					}
				}
				while (temp_image == false);



				//-------------------------------------------//
				//----- WYZNACZENIE WSKA�NIKOW JAKOSCI ------//
				//-------------------------------------------//

				//Zerowanie zmiennej.
				error_counter = 0;

				//Wygenerowanie wska�nik�w jako�ci wraz ze sprawdzeniem czy w MATLAB Tasker
				//nie wyst�pi� b��d krytyczny.
				do {
					if (matlab == MATLAB_Tasker::TERMINATED)
					{
						logger
							<< "ERROR: failed to create quality metric, MATLAB Tasker was TERMINATED." << "\n";
						logger.stats["error"]++;

						if (error_counter++ != 0)
							if (!Windows::messageBoxRetryCancel(
								L"Aplikacja MATLAB Tasker zako�czy�a prac� z powodu b��du krytycznego. "
								"Uniemo�liwia to wygenerowanie kolejnych wska�nik�w jako�ci. "
								"Anulowanie spowoduje przerwanie przetwarzania.\n"
								"Spr�bowa� ponownie nawi�za� po��czenie z programem MATLAB Tasker?",
								L"MATLAB Tasker -B��d krytyczny.",
								GetConsoleWindow()))
							{
								//Przerwanie przetwarzania z powodu b��du zapisu.
								return ImageOutputData();
							}

						logger << "RUN EXE: MATLAB Tasker..." << "\n";
						matlab.openAgain();
					}

					//Wygenerowanie i zapisanie wska�nik�w jako�ci:
					ratings_try[t] = std::move(
						makeRatings(path, temp_directory / "temp_image.bmp", 
						image, cieLabPixels,
							result, time));
				}
				while (matlab == MATLAB_Tasker::TERMINATED);
			}


			//Dla danej kombinacji metod nie uda�o si� wyznaczy� palety,
			//wyniki zostan� pomini�te.
			if (failed_to_create_palette)
				continue;

			//WYZNACZENIE �REDNICH WSKA�NIKOW:
			if(ratings_try.size() > 1)
			for (int rt = 0; rt < rating_names.size(); rt++)
			{
				//Obliczenie sumy pr�b dla ka�dego wska�nika:
				for (int t = 1; t < ratings_try.size(); t++)
					ratings_try[0][rt] += ratings_try[t][rt];

				ratings_try[0][rt] /= ratings_try.size();
			}

			/**********************************************/

			//FINALIZACJA PRZETWARZANIA.
			logger.stats["processed"]++;

			//PRZEKOPIOWANIE FINALNYCH WYNIK�W:
			auto& ratings = image_output_data[p][r];
			ratings = std::move(ratings_try.front());

			//OBLICZENIE WSKA�NIKOW PROCENTOWYCH:
			if (relative_ratings && r != 0)
			{
				auto& index = rating_names;
				auto& none_ratings = image_output_data[p][0];

				for (auto id : index)
					ratings[id.second] = 
						ratings[id.second] / none_ratings[id.second];
			}

			//-------------------------------------------//
			//----- ZAPISYWANIE V1 - JEDEN DU�Y PLIK ----//
			//-------------------------------------------//
			{
				VectorLogger file(output_directory / "combined_data.csv", true);

				//Stworzenie nazw kolumn przy pierwszym otwarciu.
				if (logger.stats["processed"] == 1)
				{
					file.append(
						std::string("%%Dithering=") +
						((use_dither) ? std::string("True") : std::string("False"))
					);
					file.append(
						std::string("%%Data")
					);

					std::vector<std::variant<std::string>> column_names =
					{ "IMAGE", "PALETTE_METHOD", "RESCALE_METHOD", "RESCALE_AMOUNT" };
					for (auto& name : rating_names)
						column_names.emplace_back(name.first);
					file.append(column_names);
				}

				//Wypisanie nowych wynik�w do pliku:
				std::vector<std::variant<long double, std::string>> row =
				{
					path.stem().generic_string(),
					std::get<MethodName>(palette_functions[p]),
					std::get<MethodName>(rescale_functions[r]),
					std::get<RescaleAmount>(rescale_functions[r])
				};
				for (auto& rating : rating_names)
					row.emplace_back(ratings[rating.second]);

				file.append(row);
			}

		}}
		return std::move(image_output_data);
	}

public:
	BatchQuantizationRunner()
	{
		srand(time(NULL));
		makeRatings({}, {}, {}, {}, {}, 0);
	}

	//Przetwarza wszystkie obrazy znajduj�ce si� na podanych �cie�kach. Pierwszy element wektora
	//�cie�ek to musi by� �cie�ka do folderu, a kolejne to nazwy plik�w z rozszerzeniami.
	//Output directory to �cie�ka w kt�rej zostan� zapisane wyniki oraz przetworzone obrazy.
	//Je�eli jakiego� wyniku nie uda si� utworzy� ze wzgl�du na np: zbyt ma�� ilo�� kolor�w
	//lub z�e parametry wej�ciowe to w danej kolumnie zostan� zapisane puste stringi.
	//Dodatkowe parametry pozwalaj� okre�li� czy zapisywa� przetworzone grafiki oraz czy u�ywa� ditheringu.
	std::wstring processImages(std::vector<Path> paths, const Path& output_directory,
		const Path& matlab_tasker_directory, //<-- �cierzka do aplikacji po�rednicz�cej.
		const Path& temp_directory, //<-- �ciezka na folder plik�w tymczasowych
		bool save_processed_images, bool use_dither, bool relative_ratings,
		bool create_diagrams)
	{
		if (paths.size() < 2)
			return std::wstring();

		//-------------------------------------------//
		//----------- STWORZENIE KONSOLI ------------//
		//-------------------------------------------//

		//Alokacja konsoli i standardowego wyj�cia:
		Windows::createConsole();


		//-------------------------------------------//
		//--------- PRZYGOTOWANIE FOLDEROW ----------//
		//-------------------------------------------//

		//Przygotowanie folderu na wyniki:
		std::filesystem::create_directories(output_directory);

		//Przygotowanie obiektu odpowiedzialnego za tworzenie log�w.
		ProgressLogger logger(output_directory / "logs.txt");
		logger << "NEW FILE: saved to " << output_directory / "logs.txt" << "\n";
		logger.stats["save"] = 1;
		logger.stats["error"] = 0;
		logger.stats["skip"] = 0;
		logger.stats["processed"] = 0;

		{
			//Przygotowanie pliku do zapisywania surowych wynik�w 'na bierz�co'.
			VectorLogger file(output_directory / "combined_data.csv");
			logger << "NEW FILE: saved to " << output_directory / "combined_data.csv" << "\n";
			logger.stats["save"]++;
			file.close();
		}



		//-------------------------------------------//
		//---- NAWI�ZANIE KOMUNIKACJI Z MATLABEM ----//
		//-------------------------------------------//

		//Program za pierwszym razem spr�buje ponownie bez pytania.
		size_t error_counter = 0;

		logger << "RUN EXE: MATLAB Tasker " << matlab_tasker_directory << "\n";
		while (!matlab.open(matlab_tasker_directory))
		{
			logger << "ERROR: failed to start MATLAB Tasker" << "\n";
			logger.stats["error"]++;

			if(error_counter++ != 0)
			if (!Windows::messageBoxRetryCancel(
				L"Nie uda�o si� uruchomi� aplikacji "
				"MATLAB Tasker koniecznej do wykonania cz�ci oblicze�.\nSpr�bowa� ponownie?",
				L"MATLAB Tasker - B��d uruchomienia.",
				GetConsoleWindow()))
			{
				logger << "---ABORT---" << "\n";

				Windows::messageBoxError(
					L"Przetwarzanie zosta�o anulowane gdy� nie uda�o nawi�za� si� komunikacji z aplikacj� MATLAB Tasker. "
					"Upewnij si�, i� posiadasz poprawnie zainstalowany pakiet 'MATLAB Runtime v.901':\n"
					"https://www.mathworks.com/products/compiler/matlab-runtime.html.\nUpewnij si� r�wnie�, �e aplikacja MATLAB Tasker znajduje si� "
					"pod podan� �cie�k�:\n" + matlab_tasker_directory.wstring() + " .",
					L"MATLAB Tasker - B��d uruchomienia.",
					GetConsoleWindow());

				return std::wstring();
			}
		}

		//-------------------------------------------//
		//------- POBRANIE USTAWIE� WYKRESOW --------//
		//-------------------------------------------//

		if (create_diagrams)
		do {
			if (matlab == MATLAB_Tasker::TERMINATED)
			{
				logger
					<< "ERROR: failed to create diagrams_settings." << "\n";
				logger.stats["error"]++;

				if (!Windows::messageBoxRetryCancel(
					L"Aplikacja MATLAB Tasker zako�czy�a prac� z powodu b��du krytycznego. "
					"Uniemo�liwia to pobranie informacji potrzebnych do wygenerowania wykres�w. "
					"Anulowanie spowoduje przerwanie przetwarzania.\n"
					"Spr�bowa� ponownie nawi�za� po��czenie z programem MATLAB Tasker?",
					L"MATLAB Tasker - B��d krytyczny.",
					GetConsoleWindow()))
				{
					return std::wstring();
				}

				logger << "RUN EXE: MATLAB Tasker..." << "\n";
				matlab.openAgain();
			}

			matlab.call("createDiagramsSettingsFile",
				temp_directory / "diagrams_settings.mat"
			);
		} while (matlab == MATLAB_Tasker::TERMINATED);
		


		//-------------------------------------------//
		//------------ OBS�UGA BACKUPOW -------------//
		//-------------------------------------------//

		//Przygotowanie folderu na buckupy (na wypadek przerwania przetwa�ania):
		//Backupy s� zapisywane po przetworzeniu ca�o�ciowo ka�dej grafiki.

		std::filesystem::path backups_path = output_directory / "_backups";
		std::filesystem::create_directories(backups_path);

		//Przygotowanie informacji kompatybilno�ciowych backup�w:
		std::vector<std::string> p_methods, r_methods, m_methods;
		for (auto p_fun : palette_functions)
			p_methods.emplace_back(std::get<MethodName>(p_fun));
		for (auto r_fun : rescale_functions)
			r_methods.emplace_back(std::get<MethodName>(r_fun) + "_" + std::to_string(std::get<RescaleAmount>(r_fun)));
		for (auto name : rating_names)
			m_methods.emplace_back(name.first);

		//PAMI�C NA WYNIKI:
		std::vector<ImageOutputData> data;

		//WCZYTANIE BACKUPOW:
		std::vector<Path> backup_files;
		std::vector<ImageOutputData> backup_data;
		for (std::filesystem::path path : std::filesystem::directory_iterator(backups_path))
		{
			logger << "FILE OPEN: checking backup compatibility " << path << "\n";

			auto file_path		  = VectorLogger::loadLine<std::filesystem::path>(path, 0);
			auto backup_config	  = VectorLogger::loadLine<size_t>(path, 1);
			auto backup_p_methods = VectorLogger::loadLine<std::string>(path, 2);
			auto backup_r_methods = VectorLogger::loadLine<std::string>(path, 3);
			auto backup_m_methods = VectorLogger::loadLine<std::string>(path, 4);

			if (backup_config.size() != 3 || file_path.size() != 1)
				continue;

			if (
				backup_p_methods == p_methods &&
				backup_r_methods == r_methods &&
				backup_m_methods == m_methods &&
				backup_config[0] == use_dither &&
				backup_config[1] == random_repeat_count &&
				backup_config[2] == relative_ratings)
			{
				auto raw_data = VectorLogger::loadFromLine<long double>(path, 5, false);

				if (raw_data.size() == palette_functions.size() * rescale_functions.size())
				{
					logger << "BACKUP FOUND: " << path << "\n";

					ImageOutputData d;
					d.resize(palette_functions.size());
					for (auto& p_data : d)
						p_data.resize(rescale_functions.size());

					auto metrics = raw_data.begin();
					for (auto& p_data : d)
					for (auto& r_data : p_data)
					{
						r_data = std::move(*metrics++);
					}
							
					backup_data.emplace_back(std::move(d));
					backup_files.push_back(file_path.front());
				}				
			}
		}
		if (backup_files.size())
		{
			if (Windows::messageBoxYesNo(
				L"Znaleziono " + std::to_wstring(backup_files.size()) + L" kompatybilnych backup�w.\n Za�adowa�?"
				, L"Znaleziono backupy.", GetConsoleWindow()))
			{
				logger << "LOADING BACKUPS..." << "\n";

				//Za�adowanie:
				data = std::move(backup_data);


				//Wykrycie tych smaych plik�w:
				std::vector<Path> same_files;

				for (auto backup_path : backup_files)
				{
					if (backup_path.parent_path() / "random_string" == paths[0] / "random_string")
					{
						if (std::find(paths.begin() + 1, paths.end(), backup_path.filename()) != paths.end())
							same_files.emplace_back(backup_path.filename());
					}
				}
				if (same_files.size())
				{
					std::wstring common_files;
					for (auto& same_file : same_files)
						common_files += same_file.filename().wstring() + L" , ";
					common_files.resize(common_files.size() - 3);

					logger << "COMMON FILES FOUND: " << same_files.size() << "\n";

					if (Windows::messageBoxYesNo(
						L"Dla nast�puj�cych plik�w:\n" +
						common_files +
						L"\nznaleziono wygenerowane dane w plikach backupu.\n"
						L"Usun�� te pliki z listy plik�w do przetworzenia?"
						, L"Pliki zosta�y ju� przetworzone.", GetConsoleWindow()))
					{
						logger << "ERASING COMMON FILES: " << same_files.size() << "\n";

						for (auto same_file : same_files)
							paths.erase(std::remove(paths.begin(), paths.end(), same_file), paths.end());
					}
				}

				//Usuni�cie �cie�ki:
				for (auto& backup_path : backup_files)
					backup_path = backup_path.filename(); //Nazwy backup�w (powinny by� nazwami obrazu).

				//Dodanie nazw za�adowanych plik�w do �cierzek:
				paths.insert(paths.begin() + 1, backup_files.begin(), backup_files.end());

				logger << "BACKUPS LOADED." << "\n";
			}
			else
			{
				logger << "DISCARDING BACKUPS..." << "\n";

				backup_files.clear();
				backup_data.clear();
			}
		}

		//-------------------------------------------//
		//------------- PRZETWARZANIE ---------------//
		//-------------------------------------------//

		//Przetworzenie wszystkich obraz�w i zebranie wynik�w.
		data.resize(paths.size() - 1);
		for (size_t i = backup_files.size(); i < data.size(); i++)
		{
			//Petwarzanie:
			auto image_data = std::move(processImage(
				paths[0] / paths[i + 1], output_directory,
				temp_directory,
				save_processed_images, use_dither, relative_ratings,
				logger));


			//Je�eli po��czenie z matlabem zosta�o zamkni�te
			//to znak, �e wys�pi� b��d.
			//Konieczne przerwanie przetwarzania.
			if (matlab == MATLAB_Tasker::TERMINATED)
			{
				logger << "---ABORT---" << "\n";
				return std::wstring();
			}


			//Zapisanie backupu:
			{
				VectorLogger backup_file(output_directory / "_backups" / "backup_" + paths[i + 1].stem() + ".csv");
				backup_file.append(paths[0] / paths[i + 1]);
				backup_file.append(std::vector<size_t>{use_dither, random_repeat_count, relative_ratings}); //Inne ustawienia.
				backup_file.append(p_methods);
				backup_file.append(r_methods);
				backup_file.append(m_methods);

				for (auto& p : image_data)
					for (auto& r : p)
						backup_file.append(r);

				logger << "NEW FILE: saved to " << output_directory / "_backups" / "backup_" + paths[i + 1].stem() + ".csv" << "\n";
				logger.stats["save"]++;
			}


			//Dopisanie danych do listy wynik�w:
			data[i] = std::move(image_data);
		}


		//-------------------------------------------//
		//--- ZAPISYWANIE V2 - PODZIELONE NA OBRAZ --//
		//-------------------------------------------//
		{
			//Stworzenie opisu kolumn (potem KOLEJNE NAZWY METOD).
			std::vector<std::string> column_names;
			column_names.emplace_back("RESCALE_METHOD");
			column_names.emplace_back("RESCALE_AMOUNT");
			for (auto& palette_fuction : palette_functions)
				column_names.emplace_back(
					std::get<MethodName>(palette_fuction));


			for (size_t i = 0; i < data.size(); i++)
			{
				// !!! GRAIKI KTORE NIE UDALO SIE WCZYTAC !!! //
				if (data[i].empty()) continue;

				//Przygotowanie pliku na wyniki:
				std::filesystem::create_directories(output_directory / "per_image_data");
				VectorLogger file(output_directory / "per_image_data" / paths[i + 1].stem() + ".csv");
				logger << "NEW FILE: saved to " << output_directory / "per_image_data" / paths[i + 1].stem() + ".csv" << "\n";
				logger.stats["save"]++;

				file.append(
					std::string("%%Dithering=") +
					((use_dither) ? std::string("True") : std::string("False"))
				);

				for (auto& rating : rating_names)
				{
					//Przygotowanie kolejnej partii wynik�w:
					file.append(std::string("%%") + rating.first); //NAZWA WSKA�NIKA
					file.append(column_names);

					//Wypisanie wszystkich wierszy dla danego wska�nika.
					for (size_t r = 0; r < rescale_functions.size(); r++)
					{
						std::vector<std::variant<long double, std::string>> row = {
							std::get<MethodName>(rescale_functions[r]),
							std::get<RescaleAmount>(rescale_functions[r])
						};

						for (size_t p = 0; p < palette_functions.size(); p++)
						{
							// !!! PALETY KTORYCH NIE UDALO SIE STWORZYC !!! //
							if (data[i][p][r].empty()) {
								row.emplace_back(""); // Pusta kolumna
								continue;
							}

							row.emplace_back(data[i][p][r][rating.second]); //Dany wska�nik.
						}
						file.append(row);
					}
				}
			}
		}



		//-------------------------------------------//
		//-- ZAPISYWANIE V3 - PODZIELONE NA METOD� --//
		//-------------------------------------------//
		std::vector<Path> per_method_paths;
		{
			//Stworzenie opisu kolumn (potem KOLEJNE NAZWY OBRAZOW).
			std::vector<std::string> column_names;
			column_names.emplace_back("RESCALE_METHOD");
			column_names.emplace_back("RESCALE_AMOUNT");
			for (size_t i = 0; i < data.size(); i++)
			{
				// !!! GRAIKI KTORE NIE UDALO SIE WCZYTAC !!! //
				if (data[i].empty()) continue;

				column_names.emplace_back(
					paths[i + 1].stem().generic_string());
			}

			//Przygotowanie folderu na wyniki:
			std::filesystem::create_directories(output_directory / "per_method_data");

			for (size_t p = 0; p < palette_functions.size(); p++)
			{
				//Przygotowanie pliku na wyniki:
				Path per_method_path = output_directory / "per_method_data" / std::get<MethodName>(palette_functions[p]) + ".csv";
				VectorLogger file(per_method_path);
				per_method_paths.push_back(per_method_path);


				logger << "NEW FILE: saved to " << output_directory / "per_method_data" /
					std::get<MethodName>(palette_functions[p]) + ".csv" << "\n";
				logger.stats["save"]++;

				file.append(
					std::string("%%Dithering=") +
					((use_dither) ? std::string("True") : std::string("False"))
				);

				for (auto& rating : rating_names)
				{
					//Przygotowanie kolejnej partii wynik�w:
					file.append(std::string("%%") + rating.first); //NAZWA WSKA�NIKA
					file.append(column_names);


					//Wypisanie wszystkich wierszy dla danego wska�nika.
					for (size_t r = 0; r < rescale_functions.size(); r++)
					{
						std::vector<std::variant<long double, std::string>> row = {
							std::get<MethodName>(rescale_functions[r]),
							std::get<RescaleAmount>(rescale_functions[r])
						};

						for (size_t i = 0; i < data.size(); i++)
						{
							// !!! GRAIKI KTORE NIE UDALO SIE WCZYTAC !!! //
							if (data[i].empty()) continue;

							// !!! PALETY KTORYCH NIE UDALO SIE STWORZYC !!! //
							if (data[i][p][r].empty()) {
								row.emplace_back(""); // Pusta kolumna
								continue;
							}

							row.emplace_back(data[i][p][r][rating.second]); //Dany wska�nik.
						}
						file.append(row);
					}
				}
			}
		}



		//-------------------------------------------//
		//----------- GENEROWANIE WYKRESOW ----------//
		//-------------------------------------------//

		if (create_diagrams)
		do {
			if (matlab == MATLAB_Tasker::TERMINATED)
			{
				logger
					<< "ERROR: failed to create diagrams." << "\n";
				logger.stats["error"]++;

				if (!Windows::messageBoxRetryCancel(
					L"Aplikacja MATLAB Tasker zako�czy�a prac� z powodu b��du krytycznego. "
					"Uniemo�liwia to przeprowadzenie ko�cowej fazy: generowania wykres�w.\n"
					"Spr�bowa� ponownie nawi�za� po��czenie z programem MATLAB Tasker?",
					L"MATLAB Tasker - B��d krytyczny.",
					GetConsoleWindow()))
				{
					return std::wstring();
				}

				logger << "RUN EXE: MATLAB Tasker..." << "\n";
				matlab.openAgain();
			}

			matlab.call("createDiagrams",
				temp_directory / "diagrams_settings.mat",
				per_method_paths,
				output_directory / "diagrams",
				paths[0]
				);
		} while (matlab == MATLAB_Tasker::TERMINATED);



		//-------------------------------------------//
		//---- ZAKO�CZENIE KOMUNIKACJI Z MATLABEM ---//
		//-------------------------------------------//

		matlab.close();


		//-------------------------------------------//
		//------------- PODSUMOWANIE ----------------//
		//-------------------------------------------//

		logger.close();

		std::wstring summary;
		summary += L"Przetworzono: " + std::to_wstring(logger.stats["processed"]) + L"\n";
		summary += L"Pomini�to: " + std::to_wstring(logger.stats["skip"]) + L"\n";
		summary += L"Nowe pliki: " + std::to_wstring(logger.stats["save"]) + L"\n";
		summary += L"B��dy: " + std::to_wstring(logger.stats["error"]);

		//-------------------------------------------//
		//-- ZAMKNI�CIE KONSOLI I ZWROCENIE WYNIKOW -//
		//-------------------------------------------//

		//Zwolnienie konsoli.
		Windows::deleteConsole();


		return std::move(summary);
	}



	
	std::vector<std::string> getPaletteFuntions() const
	{
		//Funkcja znajduj�ca pierwsze wyst�pienie danego znaku.
		auto find_char_position = [](const std::string& s, char c)->size_t{
			size_t i = 0;
			for (auto& ch : s){
				if (ch == c) return i;
				else i++;}
			return i;
		};

		std::vector<std::string> groups;
		std::string method_name;

		//Wybranie unikalnych nazw grup metod generowania pelety:
		for (auto& palette_function : palette_functions)
		{
			method_name = std::get<MethodName>(palette_function);
			method_name = std::string(method_name.begin(),
				method_name.begin() + find_char_position(method_name, '_'));

			if (std::find(groups.begin(), groups.end(),
				method_name) == groups.end())
				groups.emplace_back(method_name);
		}

		return std::move(groups);
	}

	std::vector<std::string> getRescaleFuntions() const
	{
		std::vector<std::string> groups;
		std::string method_name;

		//Wybranie unikalnych nazw grup metod generowania pr�bki pixeli:
		for (auto& rescale_function : rescale_functions)
		{
			method_name = std::get<MethodName>(rescale_function);

			if (std::find(groups.begin(), groups.end(),
				method_name) == groups.end())
				groups.emplace_back(method_name);
		}

		return std::move(groups);
	}

	auto removePaletteFuntions(const std::vector<std::string>& groups)
	{
		//Funkcja znajduj�ca pierwsze wyst�pienie danego znaku.
		auto find_char_position = [](const std::string& s, char c)->size_t{
			size_t i = 0;
			for (auto& ch : s){
				if (ch == c) return i;
				else i++;}
			return i;
		};

		std::string method_name;

		for (auto it = palette_functions.begin(); it != palette_functions.end();)
		{
			method_name = std::get<MethodName>(*it);
			method_name = std::string(method_name.begin(),
				method_name.begin() + find_char_position(method_name, '_'));

			if (std::find(groups.begin(), groups.end(),
				method_name) != groups.end())
				it = palette_functions.erase(it);
			else
				++it;
		}
	}

	auto removeRescaleFuntions(const std::vector<std::string>& groups)
	{
		std::string method_name;

		for (auto it = rescale_functions.begin(); it != rescale_functions.end();)
		{
			method_name = std::get<MethodName>(*it);

			if (std::find(groups.begin(), groups.end(),
				method_name) != groups.end())
				it = rescale_functions.erase(it);
			else
				++it;
		}
	}

	//Zwraca ilo�� wynik�w jaka zostanie stworzona z jednej grafiki:
	//metody_kwantyzacji*wersje_parametr�w*metody_tworzenia_pr�ki*wersje_parametr�w
	size_t resultsPerImage() const
	{
		return palette_functions.size() * rescale_functions.size();
	}
};

//Iterator do klasy unikatowych index�w:
size_t BatchQuantizationRunner::uniqueIndex::index_count = 0;