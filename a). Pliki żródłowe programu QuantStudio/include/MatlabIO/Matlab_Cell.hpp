#pragma once

#include <DataIO/VectorLogger.hpp>
#include <type_traits>

class MATLAB_Cell
{
	using Path = std::filesystem::path;
	VectorLogger file;

	/*FORMAT NAG£OWKA KA¯DEGO ELEMENTU CELLI (WSZYSTKO W JEDNYM WIERSZU):
		%%nazwa,
		czy_ma_nazwy_wierszy, czy_ma_nazwy_kolumn,
		ile_wierszy_opuœciæ, ile_kolumn_opuœciæ,
		wymuœ_tablicê_2D, funkcje wstêpnego przetwarzania(wiele)... , < - przecinek na koñcu.
	*/

public:
	MATLAB_Cell() = default;
	MATLAB_Cell(const Path& directory)
		: file(directory, false)
	{}

	//Otwiera plik.
	void open(const Path& directory)
	{
		file.open(directory);
	}

	//Zamyka plik.
	void close()
	{
		file.close();
	}


	//--- DODAWANIE PUSTEGO OBIEKTU ---//

	//Strutura pustej celi.
	struct Empty{};

	//Dodaje pusty argument do celli.
	bool add()
	{
		return file.append(std::string("%%"));
	}

	//Dodaje pusty argument do celli.
	bool add(Empty)
	{
		return file.append(std::string("%%"));
	}

	//--- DODAWANIE POJEDYÑCZYCH DANYCH ---//

	template<typename T>
	bool add(const T& value)
	{
		using Type  = typename std::remove_cv<std::remove_reference<T>::type>::type;
		using DType = typename std::decay<Type>::type;

		const std::string name = "";

		//NAG£OWEK:
		if constexpr (std::is_integral<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double", "round"
			});
		}
		else if constexpr (std::is_floating_point<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double"
			});
		}
		else if constexpr (std::is_same<Type, Path>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"char", "@(x) x(2:end - 1)"
			});
		}
		else if constexpr (std::is_same<Type, std::string>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name,
					0, 0, 0, 0, 1,
					"char"
			});
		}
		else if constexpr (std::is_array<Type>::value && std::is_same<DType, char*>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name,
					0, 0, 0, 0, 1,
					"char"
			});
		}
		else
			static_assert(false, "Type T isn't a defined type.");

		//DANE:
		return file.append(value);
	}


	//--- DODAWANIE WEKTOROW DANYCH ---//

	template<typename T>
	bool add(const std::vector<T>& vector)
	{
		//Dla pojedyñczych / zerowych wektorów.
		if (vector.size() == 0)
			return add();
		else if (vector.size() == 1)
			return add(vector.front());
		

		using Type = typename std::remove_cv<std::remove_reference<T>::type>::type;
		const std::string name = "";

		//NAG£OWEK:
		if constexpr (std::is_integral<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double", "round"
			});
		}
		else if constexpr (std::is_floating_point<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double"
			});
		}
		else if constexpr (std::is_same<Type, Path>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"@(x) cellfun(@(y) y(2:end - 1), x, 'UniformOutput',false)"
			});

		}
		else if constexpr (std::is_same<Type, std::string>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name,
					0, 0, 0, 0, 1,
					""
			});
		}
		else
			static_assert(false, "Type T isn't a defined type.");

		//DANE:
		return file.append(vector);
	}


	//--- DODAWANIE MACIERZY DANYCH ---//

	template<typename T>
	bool add(const std::vector<std::vector<T>>& matrix)
	{
		//Dla pojedyñczych / zerowych macierzy.
		if (matrix.size() == 0)
			return add();
		else if (vector.size() == 1)
			return add(matrix.front());


		using Type = typename std::remove_cv<std::remove_reference<T>::type>::type;
		const std::string name = "";

		//NAG£OWEK:
		if constexpr (std::is_integral<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double", "round"
			});
		}
		else if constexpr (std::is_floating_point<T>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"str2double"
			});
		}
		else if constexpr (std::is_same<Type, Path>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name, 0, 0, 0, 0, 1,
					"@(x) cellfun(@(y) y(2:end - 1), x, 'UniformOutput',false)"
			});
		}
		else if constexpr (std::is_same<Type, std::string>::value) {
			file.append(std::vector<std::variant<double, std::string>>{
				std::string("%%") + name,
					0, 0, 0, 0, 1,
					""
			});
		}
		else
			static_assert(false, "Type T isn't a defined type.");

		//DANE:
		return file.append(matrix);
	}

	//--- PRZE£ADOWANIE REKURENCYJNE ---//

	template<typename Arg1, typename Arg2, typename... Args>
	bool add(const Arg1& arg1, const Arg2& arg2, const Args&... args)
	{
		add(arg1);
		return add(arg2, args...);
	}
};