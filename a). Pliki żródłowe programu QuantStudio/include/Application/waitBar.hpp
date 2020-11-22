
#pragma once

#pragma comment (lib, "dwmapi.lib")
#include <Dwmapi.h>

#include <SFML/Graphics.hpp>
#include <future>
#include <Windows.h>
#include <filesystem>

#include <Application/GUI/Button.hpp>


class WaitBar
{
	sf::RenderWindow	window;
	sf::Texture			waitbar_texture;
	sf::RectangleShape	waitbar_bg;
	sf::RectangleShape	waitbar_bar;

	sf::Font font_title;
	sf::Font font_text;

	bool is_open;

	double percentage = 1;

	std::wstring title = L"Proszê czekaæ...";

public:
	WaitBar(
		const std::filesystem::path& texture,
		const std::filesystem::path& title_font,
		const std::filesystem::path& text_font
		)
		: is_open(false)
	{
		//HARCODED WARTOŒCI:

		sf::Rect<double> bar_rect;
		bar_rect.left = 22;
		bar_rect.top = 96;
		bar_rect.width = 779 - bar_rect.left;
		bar_rect.height = 131 - bar_rect.top;

		sf::Color bar_color(70, 150, 255, 170);

		sf::Rect<double> taskbar_rect;
		taskbar_rect.left = 22;
		taskbar_rect.top = 27;
		taskbar_rect.width = 779 - taskbar_rect.left;
		taskbar_rect.height = 91 - taskbar_rect.top;

		//----------------------------------//

		font_title.loadFromFile(title_font.generic_string());
		font_text.loadFromFile(text_font.generic_string());



		is_open = false;

		//ZA£ADOWANIE GRAFIKI:
		if (!waitbar_texture.loadFromFile(texture.generic_string()))
			return;

		waitbar_texture.setSmooth(true);


		//OKREŒLENIE ROZMIAROW:
		const unsigned int window_width  = waitbar_texture.getSize().x;
		const unsigned int window_height = waitbar_texture.getSize().y;


		//STWORZENIE OBIEKTU SFML DO WYŒWIETLENIA:
		waitbar_bg.setSize(sf::Vector2f(window_width, window_height));
		waitbar_bg.setTexture(&waitbar_texture);


		//STWORZENIE OKNA:
		sf::RenderWindow window(
			sf::VideoMode(window_width, window_height),
			"", sf::Style::None);
		window.setFramerateLimit(60);


		//KONIECZNE OPERACJE W WINAPI:
		MARGINS margins;
		margins.cxLeftWidth = -1;
		SetWindowLong(window.getSystemHandle(), GWL_EXSTYLE, WS_VISIBLE);
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);

		SetWindowPos(window.getSystemHandle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


		//-------------------------------------------------//
		//-----------------AKTUALIZACJA--------------------//
		//-------------------------------------------------//

		//UPDATE STANU PASKA:
		if (percentage > 1)
			percentage = 1;
		else if (percentage < 0)
			percentage = 0;

		waitbar_bar.setPosition(bar_rect.left, bar_rect.top);
		waitbar_bar.setSize(sf::Vector2f(bar_rect.width*percentage, bar_rect.height));
		waitbar_bar.setFillColor(bar_color);

		//UPDATE NAPISU U GORY:
		sf::Text title_obj;
		title_obj.setFont(font_title);
		title_obj.setCharacterSize(27);
		title_obj.setColor(sf::Color(204,210,255));


		title_obj.setString(title);
		title_obj.setPosition(35, 40);
		
		//auto bounds = title_obj.getLocalBounds();
		//tit

		title_obj.setString(title + L"czekaj\nczekaj");


		//-------------------------------------------------//
		//-------------------RYSOWANIE---------------------//
		//-------------------------------------------------//

		//Narysowanie grafiki.
		window.clear(sf::Color::Transparent);
		window.draw(waitbar_bg);
		window.draw(waitbar_bar);
		window.draw(title_obj);
		window.display();



		sf::Event event;
		bool grabbedWindow = false;
		sf::Vector2i grabbedOffset = { 0, 0 };
		while (window.isOpen())
		{
			window.waitEvent(event);
			{
				switch (event.type)
				{
				case sf::Event::MouseMoved:
					//for (auto& object : mousable_objects)
					//	object->processMoveEvent(event.mouseMove.x, event.mouseMove.y);

					if (grabbedWindow)
						window.setPosition(sf::Mouse::getPosition() + grabbedOffset);
					break;


				case sf::Event::MouseButtonReleased:
					if (event.mouseButton.button == sf::Mouse::Left)
						grabbedWindow = false;
					break;


				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Left)
					{
						//for (auto& object : mousable_objects)
						//	object->processClickEvent(event.mouseButton.x, event.mouseButton.y);

						grabbedOffset = window.getPosition() - sf::Mouse::getPosition();
						if (taskbar_rect.contains(event.mouseButton.x, event.mouseButton.y))
							grabbedWindow = true;
					}
					break;


				case sf::Event::LostFocus:
					grabbedWindow = false;
				case sf::Event::MouseLeft:
					//for (auto& object : mousable_objects)
					//	object->processMoveEvent(-1, -1);
					break;


				case sf::Event::Closed:
					window.close();
					break;
				}
			}
		}
		system("pause");

		//Odczekanie okreœlonego czasu i zamkniêcie okienka.
		window.close();
	}


};
