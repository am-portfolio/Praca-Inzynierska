/**********************************************************************
Windows specific wrapped functions: console.
Author: Aron Mandrella
	@2018
**********************************************************************/

#pragma once
#include <windows.h>
#include <iostream>

namespace Windows
{
	//Alokuje danemu w�tkowi konsol� oraz przypisuje j� do domyslnego strumienia wej�ciowego.
	void createConsole()
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);
	}

	//Ukrywa, a nast�pnie usuwa konsol� przypisan� danemu w�tkowi.
	void deleteConsole()
	{
		ShowWindow(GetConsoleWindow(), NULL);
		FreeConsole();
	}

}