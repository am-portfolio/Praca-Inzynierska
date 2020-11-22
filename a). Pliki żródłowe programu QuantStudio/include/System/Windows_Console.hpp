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
	//Alokuje danemu w¹tkowi konsolê oraz przypisuje j¹ do domyslnego strumienia wejœciowego.
	void createConsole()
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);
	}

	//Ukrywa, a nastêpnie usuwa konsolê przypisan¹ danemu w¹tkowi.
	void deleteConsole()
	{
		ShowWindow(GetConsoleWindow(), NULL);
		FreeConsole();
	}

}