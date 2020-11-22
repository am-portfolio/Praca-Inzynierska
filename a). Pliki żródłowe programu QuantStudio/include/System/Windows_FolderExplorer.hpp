/**********************************************************************
Windows specific wrapped functions: file explorer.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <windows.h>
#include <ShlObj.h>
#include <iostream>
#include <filesystem>

namespace Windows
{
	//Zwraca �cie�k� do wybranego folderu. UWAGA: otwarcie explorera zmienia aktywn� lokacj� programu.
	std::filesystem::path getFolderName(HWND owner_window = NULL)
    {
		BROWSEINFO bInfo = { 0 };

        //Uchwyt do okna ktore ma zostac zablokowane i �cie�ka startowa.
		bInfo.hwndOwner = owner_window;
		bInfo.pidlRoot = NULL;

		bInfo.ulFlags = BIF_NEWDIALOGSTYLE;
		bInfo.lpfn = NULL;
		bInfo.lParam = 0;
		bInfo.iImage = -1;

		bInfo.lpszTitle = L"Wybierz folder w kt�rym zapisane zostan� wyniki...";

        //Parametry bufora na sciezke do pliku.
		const DWORD path_buffer_size = 1024*1024*4; //1mb pami�ci na �cie�k� do pliku - du�o.
		wchar_t* path_buffer = new wchar_t[path_buffer_size];

		bInfo.pszDisplayName = path_buffer;

		LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

		std::filesystem::path path;
		if (lpItem != NULL) {
			if(SHGetPathFromIDList(lpItem, path_buffer))
				path.assign(path_buffer);
		}

		delete[] path_buffer;

        return path;
    }
}
