/**********************************************************************
std::ostream std::vector overloads,
std::ostream std::variant<double, std::string> overloads,
data logging in csv format.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once

#include <fstream>
#include <variant>
#include <regex>
#include <iterator>
#include <string>
#include <sstream>
#include <filesystem>


//Przydatne prze³adowanie dla filesystem:
//£¹czenie bez dodawania slasha.
std::filesystem::path operator+(const std::filesystem::path& lhs, const std::filesystem::path rhs)
{
	std::filesystem::path path = lhs;
	return std::move(path += rhs);
}


/*------------------------------------------------------------------*/
// PRZE£DADOWANIA STRUMIENIA WYJŒCIOWEGO
/*------------------------------------------------------------------*/

template<typename T, typename... Us>
std::ostream& operator<<(std::ostream& stream, const std::variant<T, Us...>& variant) {
	std::visit([&stream](const auto& data) { stream << data; }, variant);
	return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vector) {
	if(vector.empty())
		stream << "; ";
	for (auto& data : vector)
		stream << data << "; ";
	return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<std::vector<T>>& vectors) {
	for (auto& vector : vectors)
		stream << vector << std::endl;
	return stream;
}

/*------------------------------------------------------------------*/
// PRZE£ADOWANIA STRUMIENIA WEJŒCIOWEGO
/*------------------------------------------------------------------*/

//Prze³adowanie strumienia pozwalaj¹ce wczytywaæ dane do warianta.
//Je¿eli dana to liczba z jedn¹ kropk¹ to zostanie wczytane jako double, jeœli nie to string.
std::istream& operator>>(std::istream& stream, std::variant<double, std::string>& variant)
{
	//Wyra¿enie regularne sprawdzaj¹ce czy dana wartoœæ to liczba.
	const std::regex number_regex("^[0-9]*[.]{0,1}[0-9]*$");

	std::string string;
	stream >> string;

	if (!string.size())
		//Pusta komórka.
		variant = "";
	else if (std::regex_search(string, number_regex))
		//Wartoœæ zmiennoprzecinkowa.
		variant = std::stod(string);
	else
		//String.
		variant = string;

	return stream;
}

//Prze³adowanie strumienia pozwalaj¹ce wczytywaæ dane do wektora na podstawie ca³ej lini.
//Poszczególne wartoœci musz¹ byæ oddzielane , lub ;.
//Bia³e znaki na pocz¹tku i koñcu danej wartoœci lub stringa s¹ pomijane.
//Pomija wszelkie komentarze rozpoczynane od // w dowolnym miejscu.
//Ostatnia wartoœæ w wierszu nie musi byæ zakoñczona przecinkiem.
template<typename T>
std::istream& operator>>(std::istream& stream, std::vector<T>& vector)
{
	std::stringstream sstream;
	std::string line;
	std::getline(stream, line);

	//Usuniêcie komentarzy:
	const std::regex comment_regex("//.*$");
	line = std::move(std::regex_replace(line, comment_regex, ""));
	if(!line.size())
		return stream;

	//Upewnienie siê, ¿e linia jest poprawnie zakoñczona separatorem:
	//(Zapobiega pominiêciu ustatniej kolumny we wierszu)
	const std::regex last_separator_regex("[,;][\\s]*$");
	if (!std::regex_search(line, last_separator_regex))
		line.push_back(';');

	//Wyra¿enie regularne dziel¹ce kolumny w oparciu o separatory (,;).
	const std::regex cell_regex("[\\s]*([^,;]*)[\\s]*[,;]");
	auto  beg = std::sregex_iterator(line.begin(), line.end(), cell_regex);
	auto  end = std::sregex_iterator();

	vector.resize(std::distance(beg, end));
	auto data = vector.begin();

	for (; beg != end; beg++)
	{
		sstream.clear();
		sstream.str((*beg)[1].str());
		sstream >> *(data++);	

		if (sstream.fail()){
			stream.setstate(std::ios::failbit);
			return stream;
		}
	}
	return stream;
}





/*------------------------------------------------------------------*/
// W£AŒCIWA IMPLEMENTACJA
/*------------------------------------------------------------------*/

//Klasa pozwalaj¹ca zapisywaæ i odczytywaæ dane z plików csv.
//Pozwala dodawaæ pomijane komentarze oraz u³atwia zapis wektorów danych
//oraz typów variant.
class VectorLogger
{
protected:
	using Path = std::filesystem::path;

	std::fstream file;

public:
	VectorLogger() = default;
	VectorLogger(const Path& directory, bool append = false)
		: file(directory, std::fstream::out | ((append) ? std::fstream::app : 0))
	{
		file.precision(std::numeric_limits<long double>::max_digits10);
	}
	~VectorLogger() {
		file.close();
	}

	//Otwiera plik.
	void open(const Path& directory, bool append = false)
	{
		file.open(directory, std::fstream::out | ((append) ? std::fstream::app : 0));
	}

	//Zamyka plik.
	void close()
	{
		file.close();
	}

	//Dopisuje komentarz ignorowany przy wczytywaniu do otwartego pliku.
	//W przypadku sukcesu zwraca 'true'.
	bool append_comment(std::string comment, bool add_endl = true)
	{
		file << "//" << comment;
		if (add_endl) file << std::endl;
		return file.operator bool();
	}

	//Dopisuje komentarze ignorowane przy wczytywaniu do otwartego pliku.
	//W przypadku sukcesu zwraca 'true'.
	bool append_comment(std::vector<std::string> comments)
	{
		for (auto comment : comments)
			append_comment(comment);
		return file.operator bool();
	}

	//Dopisuje dan¹ w formacie CSV do otwartego pliku.
	//W przypadku sukcesu zwraca 'true'.
	template<typename T>
	bool append(const T& data, bool add_endl = true)
	{
		file << data << ";";
		if (add_endl) file << std::endl;
		return file.operator bool();
	}

	//Dopisuje wektor danych w formacie CSV do otwartego pliku.
	//W przypadku sukcesu zwraca 'true'.
	template<typename T>
	bool append(const std::vector<T>& vector, bool add_endl = true)
	{
		file << vector;
		if (add_endl) file << std::endl;
		return file.operator bool();
	}

	//Dopisuje wektor danych traktuj¹c go jako pojedyñcz¹ kolumnê w formacie CSV.
	//W przypadku sukcesu zwraca 'true'.
	template<typename T>
	bool append_vertical(const std::vector<T>& vector)
	{
		for (auto& data : vector)
			file << data << ";" << std::endl;
		return file.operator bool();
	}

	//Dopisuje wektory wektorów danych w formacie CSV do otwartego pliku.
	//W przypadku sukcesu zwraca 'true'.
	template<typename T>
	bool append(const std::vector<std::vector<T>>& vectors)
	{
		bool result = true;
		for (auto& vector : vectors)
			result = (result) ? append(vector) : false;
		return result;
	}



	//---- STATYCZNA METODA WCZYTUJ¥CA DANE ----//

	//Wczytuje plik w formacie CSV. Dane zostan¹ zapisane w wektorze wektorów.
	//Wektory zewnêtrze odpowiadaj¹ danej lini, a wewnêtrzne danej wartoœci.
	//Typ zwracanego wektora to std::vector<std::vector<T>>.
	//- Wartoœci mog¹ byæ oddzielone , lub ;.
	//- Dopuszczalne s¹ komentarze zaczynane poprzez '\\'.
	template<typename T>
	static auto loadFile(const Path& directory, bool remove_empty_rows = true)
	{
		std::fstream file(directory, std::fstream::in);
		if (!file)
		{
			std::cerr << "Unable to open file error.\nFile: \""
				<< directory << "\"." << std::endl;
			return std::vector<std::vector<T>>();
		}

		std::vector<std::vector<T>> vectors;
		size_t current_line = 1;
		
		file.seekg(std::fstream::beg);
		while (!file.eof())
		{
			//Pobranie ewentualnych bia³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();

			//Wczytanie kolejnej lini z pliku.
			vectors.emplace_back();
			file >> vectors.back();
			if (file.fail())
			{
				vectors.pop_back();
				std::cerr << "Data error.\nFile: \""
					<< directory << "\".\nLine: "
					<< current_line << "." << std::endl;
				file.clear();
			}
			//Usuniêcie pustych wierszy:
			else if (!vectors.back().size() && remove_empty_rows)
				vectors.pop_back();

			current_line++;

			//Pobranie ewentualnych bi¹³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();
		}

		file.close();

		return vectors;
	}

	template<typename T>
	static auto loadLine(const Path& directory, size_t line_number)
	{
		std::fstream file(directory, std::fstream::in);
		if (!file)
		{
			std::cerr << "Unable to open file error.\nFile: \""
				<< directory << "\"." << std::endl;
			return std::vector<T>();
		}

		std::string unused_line;
		std::vector<T> vector;

		size_t current_line = 0;

		file.seekg(std::fstream::beg);
		while (!file.eof())
		{
			//Pobranie ewentualnych bia³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();

			if(current_line != line_number)
				std::getline(file, unused_line);
			else
			{
				file >> vector;
				if (file.fail())
				{
					std::cerr << "Data error.\nFile: \""
						<< directory << "\".\nLine: "
						<< current_line << "." << std::endl;
				}
				return vector;
			}

			current_line++;

			//Pobranie ewentualnych bi¹³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();
		}
		return vector;
	}

	template<typename T>
	static auto loadFromLine(const Path& directory, size_t line_number, bool remove_empty_rows = true)
	{
		std::fstream file(directory, std::fstream::in);
		if (!file)
		{
			std::cerr << "Unable to open file error.\nFile: \""
				<< directory << "\"." << std::endl;
			return std::vector<std::vector<T>>();
		}

		std::string unused_line;
		std::vector<std::vector<T>> vectors;
		size_t current_line = 0;

		file.seekg(std::fstream::beg);
		while (!file.eof())
		{
			//Pobranie ewentualnych bia³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();

			if (current_line < line_number)
				std::getline(file, unused_line);
			else
			{
				//Wczytanie kolejnej lini z pliku.
				vectors.emplace_back();
				file >> vectors.back();
				if (file.fail())
				{
					vectors.back().resize(0);
					file.clear();
				}
				//Usuniêcie pustych wierszy:
				else if (!vectors.back().size() && remove_empty_rows)
					vectors.pop_back();
			}


			current_line++;

			//Pobranie ewentualnych bi¹³ych znaków.
			while (file.peek() == '\n' || file.peek() == '\t' || file.peek() == ' ')
				file.get();
		}

		file.close();

		return vectors;
	}

};