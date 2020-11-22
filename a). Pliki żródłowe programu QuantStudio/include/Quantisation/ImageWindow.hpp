#pragma once
#include "Image.hpp"
#include "Point.hpp"

#include <System/Windows_FileExplorer.hpp>
#include <Utility/UtilityMath.hpp>


class ImageWindow
{
	//Pamie� na dane grafik.
	std::vector<ImageU8>	 images;
	std::vector<sf::Texture> textures;
	std::vector<sf::Sprite>  sprites;

public:

	ImageWindow(const std::vector<ImageU8>& images)
	{
		loadTextures(images);
		updateLoop();
	}

private:

	//Za�adowanie danych do obiekt�w sf::texture (VRAM),
	//i przygotowanie isterfejsu wy�wietlania sf::sprite.
	//Oraz zapisanie kopi grafik w celu ewentualnego zapisu
	//ich wybrango fragmentu.
	void loadTextures(const std::vector<ImageU8>& source_images)
	{
		assert(!source_images.empty());

#ifdef DEBUG
		bool same_size = true;
		for (auto& source_image : source_images)
			if (source_image.getHeight() != source_images[0].getHeight() ||
				source_image.getWidth() != source_images[0].getWidth())
				same_size = false;
		assert(same_size);
#endif

		images = source_images;
		textures.resize(images.size());
		sprites.resize(images.size());

		auto texture = textures.begin();
		auto sprite = sprites.begin();

		for (const auto& image : images)
		{
			texture->create(image.getWidth(), image.getHeight());
			texture->update(image.getPixelsPtr());
			texture->setSmooth(false);
			sprite->setTexture(*texture);
			texture++;
			sprite++;
		}
	}


	//Zwraca rozmiar grafiki (zak�ada �e wszystkie obrazki s� tego
	//samego rozmiaru).
	inline Point getImageSize()
	{
		return textures[0].getSize();
	}


	//Okre�la domy�lny rozmiar kwadratowego okienka na podstawie
	//rozdzielczo�ci u�ywanego ekranu.
	inline Point getDefaultWindowSize()
	{
		auto desktop_mode = sf::VideoMode::getDesktopMode();
		float default_size = ((desktop_mode.height > desktop_mode.width)
			? desktop_mode.width : desktop_mode.height) * 0.7f;
		return Point(default_size, default_size);
	}

	//Okre�la domy�ln� warto�� zoom (tak� by orazek zajmowa� jak najwi�cej miejsca)
	//w oparciu o rozmiar okienka i obrazka.
	inline float getDefaultZoom(const Point& window_size)
	{
		Point image_size = getImageSize();

		//Obliczenie wsp�lczynnika przez jaki trzeba przemnozyc prostok�t wewnetrzny
		//by miesci� sie w zewnetrznym i zajmowal jak najwiecej miejsca.
		float scale_factor = (window_size.x / image_size.x > window_size.y / image_size.y) ?
			image_size.y / window_size.y :
			image_size.x / window_size.x;

		return std::log(1.f / scale_factor) / std::log(1.1f);
	}

	//Zwraca domy�ln� pozycj� �wiata (tak� by obrazek by� wy�rodkowany).
	inline Point getDefaultViewCenter()
	{
		return getImageSize() / 2.0f;
	}


	//G��wna p�tla okienka z grafikami.
	void updateLoop()
	{
		//----// INICIALIZACJA //----//
		sf::RenderWindow window;
		sf::View view;
		Point window_size = getDefaultWindowSize();

		float zoom = getDefaultZoom(window_size);
		view.setCenter(getDefaultViewCenter());
		view.setSize(window_size * std::pow(1.1f, -zoom));

		size_t current_image = 0;

		window.create(sf::VideoMode(window_size.x, window_size.y), "", sf::Style::Default);
		window.setTitle(images[current_image].getImagename());

		//----// PIERWSZA KLATKA //----//

		window.setView(view);
		window.clear(sf::Color(240, 240, 240));
		window.draw(sprites[current_image]);
		window.display();

		//----// P�TLA ZDARZE� //----//

		sf::Event event;

		Point mouse_pos = sf::Mouse::getPosition();
		Point mouse_pos_prev = sf::Mouse::getPosition();
		Point event_pixel;
		Point coord_pre;
		Point coord_post;

		while (window.isOpen())
		{
			window.waitEvent(event);

			switch (event.type)
			{
				//Zamkniecie okna.
			case sf::Event::Closed:
				window.close();
				break;


				//Przybli�anie i oddalanie.
			case sf::Event::MouseWheelScrolled:
				if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
				{
					//Wsp�rz�dna scrollu wzgl�dem okienka.
					event_pixel = { event.mouseWheelScroll.x, event.mouseWheelScroll.y };
					//Wsp�rzedna scrollu wzgl�dem '�wiata' (uwzgl�dnia aktualne przekszta�cenie view).
					coord_pre = window.mapPixelToCoords(event_pixel);

					//Dodanie scrollu do aktualnego stanu zoom.
					zoom += (float)event.mouseWheelScroll.delta;

					//Ustawienia �rodka �wiada w miejsce przyblizenia i powi�kszenie wzgl�dem �rodka.
					view.setCenter(coord_pre);
					view.setSize(window_size * std::pow(1.1f, -zoom));

					//Obliczenie przesuni�cia kompensuj�cego (tak by wskazany pixel pozosta� w starej pozycji
					//po powi�kszeniu obrazu.
					coord_post = window.mapPixelToCoords(event_pixel, view);
					view.move(coord_pre - coord_post);
				}
				break;


				//Przes�wanie grafiki.
			case sf::Event::MouseMoved:
				mouse_pos_prev = mouse_pos;
				mouse_pos = { event.mouseMove.x, event.mouseMove.y };
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					coord_pre = window.mapPixelToCoords(mouse_pos_prev);
					coord_post = window.mapPixelToCoords(mouse_pos);
					view.move((coord_pre - coord_post));
				}
				break;


				//Zmiana rozmiaru okinka.
			case sf::Event::Resized:
				window_size = { event.size.width, event.size.height };
				view.setSize(window_size * std::pow(1.1f, -zoom));
				break;


				//Wcisniecie klawisza
			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
					zoom = 0;
					view.setCenter(getDefaultViewCenter());
					view.setSize(window_size * std::pow(1.1f, -zoom));
					break;
				case sf::Keyboard::Enter:
					zoom = getDefaultZoom(window_size);
					view.setCenter(getDefaultViewCenter());
					view.setSize(window_size * std::pow(1.1f, -zoom));
					break;
				case sf::Keyboard::Left:
					if (current_image > 0) {
						current_image--;
						window.setTitle(images[current_image].getImagename());
					}
					break;
				case sf::Keyboard::Right:
					if (current_image + 1 < images.size()) {
						current_image++;
						window.setTitle(images[current_image].getImagename());
					}
					break;

					//Zapis grafik:
				case sf::Keyboard::S:
					if (event.key.control)
					{
						Point top_left = view.getCenter() - view.getSize() / 2.f;
						Rect crop_rect = { top_left.x, top_left.y, top_left.x + view.getSize().x, top_left.y + view.getSize().y };

						crop_rect.left = std::round(clamp(crop_rect.left, 0.f, getImageSize().x));
						crop_rect.right = std::round(clamp(crop_rect.right, 0.f, getImageSize().x));
						crop_rect.top = std::round(clamp(crop_rect.top, 0.f, getImageSize().y));
						crop_rect.bottom = std::round(clamp(crop_rect.bottom, 0.f, getImageSize().y));

						saveImages(crop_rect);
					}
					break;
				}
				break;
			}

			//----// AKTUALIZACJA WY�WIETLANEGO OBRAZU //----//

			window.setView(view);
			window.clear(sf::Color(240, 240, 240));
			window.draw(sprites[current_image]);
			window.display();
		}
	}

	void saveImages(const Rect& crop_rect)
	{
		auto save_path = Windows::getSaveFileName();
		if (save_path.empty())
			return;

		std::cout << "SAVING CROPPED IMAGES\nRECTANGLE: ";
		crop_rect._print();
		std::cout << "\n";

		std::cout << "--- CROPPING:\n";
		std::vector<ImageU8> cropped_images;
		for (auto& image : images)
		{
			std::cout << "\t" << image.getImagename().toAnsiString() << "\n";
			cropped_images.emplace_back(std::move(image.cropImage(crop_rect)));
		}

		std::cout << "--- SAVING:\n";
		for (auto& cropped_image : cropped_images)
		{
			auto path = save_path[0] / save_path[1].concat("_").concat(cropped_image.getImagename().toAnsiString()).concat(save_path[2].c_str());
			std::cout << "\t" << path << "\n";
			cropped_image.saveToFile(path);
		}

		std::cout << "DONE---";
	}
};