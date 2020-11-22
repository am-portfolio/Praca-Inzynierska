/**********************************************************************
Windows specific wrapped functions: messege box.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <windows.h>
#include <iostream>
#include <filesystem>

namespace Windows
{
	bool messageBoxYesNo(const std::wstring& text, const std::wstring& caption, HWND owner_window = NULL)
	{	//NO - 7, YES - 6
		return !(MessageBox(owner_window, text.c_str(), caption.c_str(), MB_YESNO | MB_ICONQUESTION) - 6);
	}

	bool messageBoxRetryCancel(const std::wstring& text, const std::wstring& caption, HWND owner_window = NULL)
	{	//CANCEL - 2, RETRY - 4
		return MessageBox(owner_window, text.c_str(), caption.c_str(), MB_RETRYCANCEL | MB_ICONWARNING) == IDRETRY;
	}

	void messageBoxInfo(const std::wstring& text, const std::wstring& caption, HWND owner_window = NULL)
	{
		MessageBox(owner_window, text.c_str(), caption.c_str(), MB_ICONINFORMATION);
	}

	void messageBoxError(const std::wstring& text, const std::wstring& caption, HWND owner_window = NULL)
	{
		MessageBox(owner_window, text.c_str(), caption.c_str(), MB_ICONERROR);
	}
}