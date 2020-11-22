#pragma once
#include <MatlabIO/Matlab_Cell.hpp>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

class MATLAB_Tasker
{
	enum
	{
		MATLAB_IS_FREE = 0,
		MATLAB_IS_ACTIVE = 1,
		SAFETY_CHECK_INTERVAL = 100
	};


	//Zmienna statyczna indeksuj�ca taskery.
	static size_t tasker_count;
	size_t tasker_id;

	using Path = std::filesystem::path;

	//Uchwyty systemowe.
	HANDLE hThread;
	HANDLE hProcess;

	//Miejsca stworzonych plik�w.
	Path program_path, file_flag, file_data;

	//..._wait_sec - interwa� czasowy co jaki sprawdzany bd plik z flag� stanu,
	//odpowiednio po stronie matlaba i stronie aplikacji.
	unsigned int app_wait_sec, matlab_wait_msc;



	//Usuwa pliki tymczasowe i zamyka po��czenia.
	void removeTempFilesAndCloseHandles()
	{
		//Zamykanie uchwyt�w:
		CloseHandle(hProcess);
		CloseHandle(hThread);
		hThread = hProcess = NULL;
		
		//Usuwanie plik�w:
		std::filesystem::remove(file_flag);
		std::filesystem::remove(file_data);
	}

	//Zwraca pierwszy bajt z pliku z flag�.
	bool getState()
	{
		std::fstream file(file_flag, std::fstream::in | std::fstream::binary);
		return file.get();
	}
	//Ustawia pierwszy bajt w pliku z flag�.
	void setState(bool value)
	{
		std::ofstream file(file_flag, std::fstream::out | std::fstream::binary | std::fstream::trunc);
		file.put((char)value);
	}

	//Funkcja wstrzymuje dzia�anie w�tku do momentu zako�czenia pracy,
	//przez matlaba (wykonania funkcji / uruchomienia). Je�eli wyst�pi�
	//b��d i MATLAB zosta� zamkni�ty to posprz�ta zmienne i zwr�ci false.
	bool waitForMATLAB()
	{
		//Po��czenie nie jest aktywne.
		if (!isOpen())
			return false;
		
		unsigned int loop_iteration = 0;

		while (getState() != MATLAB_IS_FREE)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(app_wait_sec));

			loop_iteration++;
			//Sprawdzenie czy nie utracono po��czenia.
			if (loop_iteration > SAFETY_CHECK_INTERVAL)
			{
				loop_iteration = 0;

				if (!isOpen())
					return false;
			}
		}

		return true;
	}


	//Funkcja wywo�ywana przez konstruktory
	void InitializeObiject()
	{
		tasker_count++;
		tasker_id = tasker_count;

		hThread = NULL;
		hProcess = NULL;

		app_wait_sec = 10;
		matlab_wait_msc = 25;
	}

public:
	enum
	{
		TERMINATED = 0,
		RUNNING = 1,
	};


	MATLAB_Tasker()
	{
		InitializeObiject();
	}
	MATLAB_Tasker(const Path& program_path,
		unsigned int app_wait_sec = 10, unsigned int matlab_wait_msc = 25)
	{
		InitializeObiject();
		open(program_path,
			app_wait_sec, matlab_wait_msc);
	}

	//Uruchamia aplikacj� zarz�dzaj�c� funkcjami w MATLAB'ie i nawi�zuje po��czenie.
	bool open(const Path& program,
		unsigned int app_wait_sec = 10, unsigned int matlab_wait_msc = 25)
	{
		if (isOpen())
			close();

		//---- STWORZENIE PLIKOW KOMUNIKACYJNYCH -----//
		program_path = std::filesystem::absolute(program);

		Path temp_directory = std::filesystem::absolute(program_path).parent_path();

		file_data = temp_directory / program_path.stem()
			+ "_" + std::to_string(tasker_id) + "_data.csv";
		file_flag = temp_directory / program_path.stem()
			+ "_" + std::to_string(tasker_id) + "_flag.bin";

		this->matlab_wait_msc = matlab_wait_msc;
		this->app_wait_sec = app_wait_sec;



		//Przygotowanie plik�w.
		std::fstream file;
		file.open(file_data, std::fstream::out | std::fstream::trunc);
		file.close();

		//Inicializacja flagi:
		setState(true); //UWAGA: na pocz�tku wystawia jedynk�, matlab da 0 gdy ju� wystartuje.




		//------ URUCHOMIENIE NOWEGO PROCESU --------//

		//Struktury WinApi
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);

		//String uruchomieniowy aplikacji.
		std::wstring command_line =
			L"\"" + program_path.wstring() + L"\" " +
			L"\"" + file_data.wstring() + L"\" " +
			L"\"" + file_flag.wstring() + L"\" " +
			std::to_wstring(matlab_wait_msc / 1000.0);

		// Start the child process. 
		if (!CreateProcess(NULL,	// No module name (use command line)
			command_line.data(),	// Command line max 32768 chars
			NULL,					// Process handle not inheritable
			NULL,					// Thread handle not inheritable
			FALSE,					// Set handle inheritance to FALSE
			0,						// No creation flags
			NULL,					// Use parent's environment block
			NULL,					// Use parent's starting directory 
			&si,					// Pointer to STARTUPINFO structure
			&pi)					// Pointer to PROCESS_INFORMATION structure
			)
		{
			std::cout << "MATLAB Tasker: [INIT ERROR] FAILED TO OPEN\n"
				<< program_path << std::endl;
			removeTempFilesAndCloseHandles();
			return false;
		}

		std::cout << "MATLAB Tasker: RUNNING..." << std::endl;

		//Zapianie danych:
		hThread = pi.hThread;
		hProcess = pi.hProcess;

		//Odczekanie a� MATLAB si� rozgrzeje.
		return waitForMATLAB();;
	}

	//Uruchamia aplikacj� zarz�dzaj�c� funkcjami w MATLAB'ie i nawi�zuje po��czenie,
	//u�ywaj�c takich samych parametr�w jak poprzednim razem.
	bool openAgain()
	{
		if (!isOpen())
			return open(program_path, app_wait_sec, matlab_wait_msc);
		else
			return true;
	}

	//Zamyka po��czenie z matlabem i aplikacj�.
	void close()
	{
		if (!isOpen())
			return;

		call("close all"); //Zamkni�cie wykres�w.
		call("exit");	   //Wy��czenie programu.

		// Wait until child process exits.
		WaitForSingleObject(hProcess, INFINITE);

		std::cout << "MATLAB Tasker: CORRECTLY CLOSED" << std::endl;

		//Sprz�tanie:
		removeTempFilesAndCloseHandles();
	}

	//Sprawdza czy po��czenie jest aktywne.
	bool isOpen()
	{
		if (hProcess == NULL)
			return false;

		DWORD lpExitCode;
		BOOL  res;
		//Sprawdzenie czy program nie zosta� aby zamkni�ty z powodu b��du.
		res = GetExitCodeProcess(hProcess, &lpExitCode);
		if (lpExitCode == STILL_ACTIVE)
			return true;
		else {
			std::cout << "MATLAB Tasker: [FATAL ERROR] CONNECTION LOST" << std::endl;
			removeTempFilesAndCloseHandles();
			return false;
		}
	}

	//Sprawdza czy po��czenie jest aktywne.
	operator bool()
	{
		return isOpen();
	}

	//Wywo�uje funkcj� o podanej nazwie z podanymi argumentami.
	template<typename... Args>
	auto call(const std::string& function_name, const Args&... args)
	{
		//Nie otwarto po��czenia.
		if (!isOpen()) {
			std::cout << "MATLAB Tasker: [CALL ERROR] NO ACTIVE CONNECTION" << std::endl;
			return std::vector<std::vector<double>>();
		}

		//Po uruchomieniu MATLABA na w wyj�ciu pliku flagi powinno by� 0.
		//tz mo�na modyfikowa� dane - przygotowanie pliku.
		{
			MATLAB_Cell cell(file_data);
			cell.add(function_name, args...);
		}

		//Wystawienie informacji o nowym poleceniu.
		setState(true);

		//Czekanie na wyniki:
		if(waitForMATLAB())
			//Pobranie i zwr�cenie wyniku:
			return std::move(VectorLogger::loadFile<double>(file_data));
		else
			//Wyst�pi� b��d i zamkni�to MATLABa:
			return std::vector<std::vector<double>>();	
	}



	//Destruktor usuwa pliki tymczasowe.
	~MATLAB_Tasker()
	{
		close();
	}
};
size_t MATLAB_Tasker::tasker_count = 0;