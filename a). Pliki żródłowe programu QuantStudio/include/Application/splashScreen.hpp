
#pragma once

#pragma comment (lib, "dwmapi.lib")
#include <Dwmapi.h>

#include <SFML/Graphics.hpp>
#include <future>
#include <Windows.h>
#include <filesystem>

//Tworzy splash screen (grafikê w przeŸroczystym okienku) blokuj¹c¹ w¹tek na okreœlony czas.
bool createSplashScreen(const std::filesystem::path& image, unsigned int splash_time)
{
	//ZA£ADOWANIE GRAFIKI:
	sf::Texture splash_texture;
	if (!splash_texture.loadFromFile(image.generic_string()))
		return false;

	splash_texture.setSmooth(true);

	//STWORZENIE OKNA:
	unsigned int window_width = splash_texture.getSize().x;
	unsigned int window_height = splash_texture.getSize().y;

	sf::RenderWindow window(
		sf::VideoMode(window_width, window_height),
		"", sf::Style::None);
	window.setFramerateLimit(60);

	//STWORZENIE OBIEKTU SFML DO WYŒWIETLENIA:
	sf::RectangleShape splash;
	splash.setSize(sf::Vector2f(window_width, window_height));
	splash.setTexture(&splash_texture);


	//KONIECZNE OPERACJE W WINAPI:
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(window.getSystemHandle(), GWL_EXSTYLE, WS_POPUP | WS_VISIBLE | WS_EX_TOOLWINDOW);
	DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);


	//Narysowanie grafiki.
	window.clear(sf::Color::Transparent);
	window.draw(splash);
	window.display();


	//Odczekanie okreœlonego czasu i zamkniêcie okienka.
	Sleep(splash_time);
	window.close();

	return true;
}
//Tworzy splash screen (grafikê w przeŸroczystym okienku) blokuj¹c¹ w¹tek na okreœlony czas.
auto createSplashScreenPromise(const std::filesystem::path& image)
{
	auto impl = [](auto image, auto future_obj) {
		//ZA£ADOWANIE GRAFIKI:
		sf::Texture splash_texture;
		if (!splash_texture.loadFromFile(image.generic_string()))
			return false;

		splash_texture.setSmooth(true);

		//STWORZENIE OKNA:
		unsigned int window_width = splash_texture.getSize().x * 0.8;
		unsigned int window_height = splash_texture.getSize().y * 0.8;

		sf::RenderWindow window(
			sf::VideoMode(window_width, window_height),
			"", sf::Style::None);
		window.setFramerateLimit(60);

		//STWORZENIE OBIEKTU SFML DO WYŒWIETLENIA:
		sf::RectangleShape splash;
		splash.setSize(sf::Vector2f(window_width, window_height));
		splash.setTexture(&splash_texture);


		//KONIECZNE OPERACJE W WINAPI:
		MARGINS margins;
		margins.cxLeftWidth = -1;
		SetWindowLong(window.getSystemHandle(), GWL_EXSTYLE, WS_POPUP | WS_VISIBLE | WS_EX_TOOLWINDOW);
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);


		//Narysowanie grafiki.
		window.clear(sf::Color::Transparent);
		window.draw(splash);
		window.display();


		//Odczekanie okreœlonego czasu i zamkniêcie okienka.
		SetWindowPos(window.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		future_obj.wait();
		window.close();
	};

	std::promise<bool> p;

	auto future_obj = p.get_future();
	std::thread splash_screen_thread(
		impl, image, std::move(future_obj));
	splash_screen_thread.detach();

	return std::move(p);
}
