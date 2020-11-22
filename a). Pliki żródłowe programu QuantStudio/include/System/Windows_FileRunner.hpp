/**********************************************************************
Windows specific wrapped functions: file running.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <filesystem>

#include "Windows_MessageBox.hpp"

namespace Windows
{
	//Uruchamia inny plik/program na dysku w trybie przegl¹dania.
	void openExternalFile(const std::filesystem::path& file, HWND owner_window = NULL)
	{
		if (!std::filesystem::exists(file))
		{
			messageBoxError(
				L"Podana œcie¿ka nie istnieje:\n" + file.wstring(),
				L"Nie znaleziono pliku.", owner_window);
			return;
		}

		ShellExecute(owner_window, L"open", file.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

	//Uruchamia inny plik/program na dysku w trybie edycji.
	//Wstrzymuje wywo³uj¹cy w¹tek do momentu zakoñczenia edycji.
	void editExternalFile(const std::filesystem::path& file, HWND owner_window = NULL)
	{
		if (!std::filesystem::exists(file))
		{
			messageBoxError(
				L"Podana œcie¿ka nie istnieje:\n" + file.wstring(),
				L"Nie znaleziono pliku.", owner_window);
			return;
		}

		static int count = 0;
		if(!count)
		messageBoxInfo(
			L"Aplikacja zostanie wstrzymana do momentu zamkniêcia edytora.",
			L"Edycja danych.", owner_window);
		count++;

		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = owner_window;
		ShExecInfo.lpVerb = L"edit";
		ShExecInfo.lpFile = file.c_str();
		ShExecInfo.lpParameters = L"";
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;

		ShowWindow(owner_window, SW_MINIMIZE);
		ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);
		ShowWindow(owner_window, SW_RESTORE);

		//Wersja bez wstrzymywania w¹tku:
		////ShellExecute(owner_window, L"edit", file.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

	//Otwiera folder pod podan¹ œcierzk¹.
	void exploreExternalFolder(const std::filesystem::path& file, HWND owner_window = NULL)
	{
		if (!std::filesystem::exists(file))
		{
			messageBoxError(
				L"Podana œcie¿ka nie istnieje:\n" + file.wstring(),
				L"Nie znaleziono pliku.", owner_window);
			return;
		}

		ShellExecute(owner_window, L"explore", file.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}
}