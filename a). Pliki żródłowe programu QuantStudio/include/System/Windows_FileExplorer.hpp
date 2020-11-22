/**********************************************************************
Windows specific wrapped functions: file explorer.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <windows.h>
#include <iostream>
#include <filesystem>

namespace Windows
{
	//Zwraca wektor ze œcie¿kami do wybranych plików. Jeœli wektor jest pusty to znaczy ¿e nic nie wybrano, a je¿eli
	//coœ wybrano to pierwszy element wektora to bêdzie œcie¿k¹ do folderu, a ka¿dy kolejny element bêdzie nazw¹ pliku
	//wraz z rozszerzeniem. Mo¿na u¿ywaæ w wielu w¹tkach na raz. UWAGA: otwarcie explorera zmienia aktywn¹ lokacjê programu.
	std::vector<std::filesystem::path> getOpenFileName(HWND owner_window = NULL, bool allow_many = true, const std::wstring& root_dir = L"")
    {
        OPENFILENAMEW ofn = {0};
        ofn.lStructSize = sizeof(ofn);

        //Uchwyt do okna ktore ma zostac zablokowane i œcie¿ka startowa.
        ofn.hwndOwner = owner_window;
        ofn.lpstrInitialDir = root_dir.c_str();

		//Filtry ukrywania plików w explorerze i index domyœlnego filtru.
		ofn.lpstrFilter = L"Graphics\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tga;*.hdr;*.pic;*.psd";// "\0All Files\0 * \0\0";
        ofn.nFilterIndex = 1;

        //Parametry bufora na sciezke do pliku.
		const DWORD path_buffer_size = 1024*1024*4; //4mb pamiêci na œcie¿ki do plików - du¿o.
		wchar_t* path_buffer = new wchar_t[path_buffer_size];
        ofn.lpstrFile = path_buffer;
        ofn.nMaxFile = path_buffer_size;

        ofn.Flags = (allow_many ? OFN_ALLOWMULTISELECT : NULL) | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if (!GetOpenFileNameW(&ofn))
		{
			delete[] path_buffer;
			return std::vector<std::filesystem::path>();
		}

		//-----// WYDOBYCIE NAZW PLIKOW I ŒCIERZKI //-----//

		std::vector<std::filesystem::path> result;

		//Przypadek gdy wybrano wiele plików:
		if (*(path_buffer + ofn.nFileOffset - 1) == L'\0')
		{
			wchar_t* p = path_buffer;
			std::wstring string = p;
			p += (string.length() + 1);
			result.push_back(string);

			while (*p) {
				string = p;
				p += (string.length() + 1);
				result.push_back(string);
			}
		}
		//Przypadek gdy wybrano tylko jeden plik.
		else
		{
			result.emplace_back(path_buffer, path_buffer + ofn.nFileOffset);
			result.emplace_back(path_buffer + ofn.nFileOffset);
		}

		delete[] path_buffer;
        return result;
    }


	//W przypadku niepowodzenia lub anulowania funkcja zwraca pusty wektor. Gdy lokalizacja zostanie poprawnie wybrana
	//dunkcja zwróci trzy elementowy wektor. Na pierwszej pozycji œcierzkê do folderu, na drugiej nazwa pliku bez rozszerzenia
	//i na trzeciej rozszerzenie zaczynaj¹ce siê od kropki. UWAGA: otwarcie explorera zmienia aktywn¹ lokacjê programu.
	std::vector<std::filesystem::path> getSaveFileName(const std::wstring& default_ext = L"", HWND owner_window = NULL, const std::wstring& root_dir = L"")
    {
		const DWORD path_buffer_size = 1024*1024*1; //1mb pamiêci na œcie¿kê do pliku - du¿o.
		wchar_t* path_buffer = new wchar_t[path_buffer_size];

        OPENFILENAMEW sfn = {0};
        sfn.lStructSize = sizeof(sfn);

        //Uchwyt do okna ktore ma zostac zablokowane.
        sfn.hwndOwner = owner_window;

        //Ustawienia sciezki startowej, filtrow, domyslnego filtru i rozszerzenia.
        sfn.lpstrInitialDir = root_dir.c_str();
        sfn.lpstrDefExt = default_ext.c_str();

		//Filtry ukrywania plików w explorerze i index domyœlnego filtru.
		sfn.lpstrFilter = L"Graphics\0*.png;*.jpg;*.bmp;*.tga"; //"\0All Files\0 * \0\0";
        sfn.nFilterIndex = 1;

        //Parametry bufora na sciezke do pliku.
        sfn.lpstrFile = path_buffer;
        sfn.nMaxFile = path_buffer_size;

        sfn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;


		if (!GetSaveFileNameW(&sfn))
		{
			return std::vector<std::filesystem::path>();
			delete[] path_buffer;
		}  

		//-----// WYDOBYCIE ŒCIERZKI, NAZWY I ROZSZERZENIA //-----//

		std::vector<std::filesystem::path> result;
		result.emplace_back(path_buffer, path_buffer + sfn.nFileOffset - 1);
		result.emplace_back(path_buffer + sfn.nFileOffset + 1, path_buffer + sfn.nFileExtension - 1);
		result.emplace_back(path_buffer + sfn.nFileExtension - 1);

		delete[] path_buffer;
        return result;
    }
}
