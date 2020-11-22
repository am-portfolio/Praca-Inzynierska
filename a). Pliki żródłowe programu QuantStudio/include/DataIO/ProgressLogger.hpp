#pragma once

#include "VectorLogger.hpp"
#include <map>


//Klasa s�u��ca do zapisywania log�w.
//Ma prze�adowany operator << kt�ry zapisuje dane
//jednocze�nie do pliku i standardowego strumienia.
//Dodatkowo obiekt posiada publiczn� map� typu
//std::map<std::string, size_t> w kt�rej mo�na zapisywa�
//r�ne wartosci inkrementowalne.
class ProgressLogger
{
	using Path = std::filesystem::path;

	std::fstream file;

public:
	std::map<std::string, size_t> stats;

	ProgressLogger() = default;
	ProgressLogger(const Path& directory)
		: file(directory, std::fstream::out)
	{
		file.precision(std::numeric_limits<long double>::max_digits10);
	}
	~ProgressLogger() {
		file.close();
	}

	//Otwiera plik.
	void open(const Path& directory)
	{
		file.open(directory, std::fstream::out);
	}

	//Zamyka plik.
	void close()
	{
		file.close();
	}

	//Wypisuje dane jednocze�nie do pliku i standardowego wyj�cia.
	template<typename T>
	ProgressLogger& operator<<(const T& to_log)
	{
		file << to_log;
		std::cout << to_log;
		return *this;
	}
};