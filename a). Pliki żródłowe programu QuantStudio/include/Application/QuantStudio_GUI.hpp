/**********************************************************************
QuantStudio - application GUI.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <map>
#include <thread>

//Zarzπdzanie kwantyzacjπ wielu plikÛw:
#include "QuantStudio_logic.hpp"

//GUI:
#include "GUI/Checkbox.hpp"
#include "GUI/Button.hpp"

//Operacje systemowe:
#include <System/Windows_FileExplorer.hpp>
#include <System/Windows_FileRunner.hpp>
#include <System/Windows_MessageBox.hpp>
#include <System/Windows_FolderExplorer.hpp>



class BatchApplication
{
	enum
	{
		bg_left = 0,
		bg_right,
		bg_right_middle,
		text_bright,
		text_bright_hover,
		text_bright_click,
		text_dark,
		text_dark_hover,
		text_dark_click
	};

	std::vector<sf::Color> color =
	{
		sf::Color(0x0d0c15FF), //bg_left
		sf::Color(0x292f5899), //bg_right
		sf::Color(0x454a78FF), //bg_right_middle
		sf::Color(0xccd2ffFF), //text_bright
		sf::Color(0xdce0ffFF), //text_bright_hover
		sf::Color(0xdce0ffFF), //text_bright_click
		sf::Color(0x8f98e2FF), //text_dark
		sf::Color(0x9ca6eeFF), //text_dark_hover
		sf::Color(0x9ca6ffFF)  //text_dark_click
	};

	/*
	std::vector<sf::Color> color =
	{
	sf::Color(0x0d0c15FF), //bg_left
	sf::Color(0x292f5888), //bg_right
	sf::Color(0x353a68FF), //bg_right_middle
	sf::Color(0xbcc2f0FF), //text_bright
	sf::Color(0xccd0f0FF), //text_bright_hover
	sf::Color(0xccd0f0FF), //text_bright_click
	sf::Color(0x7f88d2FF), //text_dark
	sf::Color(0x9ca6eeFF), //text_dark_hover
	sf::Color(0x9ca6ffFF)  //text_dark_click
	};
	*/


	//åCIEØKA DO FOLDERU Z PLIKIEM EXE:
	const std::filesystem::path working_directory;
	const std::filesystem::path temp_directory;

	//OBIEKT ZARZ•DZAJACY KWANTYZACJ•:
	BatchQuantizationRunner quantization_runner;

	//SFML ASSETS:
	sf::Texture create_diagrams_texture;
	sf::Texture check_texture;
	sf::Texture wrench_texture;
	sf::Texture help_texture;
	sf::Texture run_texture;
	sf::Texture load_texture;
	sf::Texture logo_texture;
	sf::Texture bg_stripes_texture;
	sf::Texture bg_polygon_texture;
	sf::Texture close_texture;
	sf::Texture minimalize_texture;
	sf::Font font_regular;
	sf::Font font_bold;
	sf::Font font_medium;
	
	//INNE OBIEKTY SFML:
	sf::RenderWindow window;
	sf::Event event;

	//FLAGA DO SYGNALIZOWANIA AKCJI NA GUZIKU URUCHOM
	bool ready_to_run = false;
	//FLAGA DO SYGNALIZOWANIA AKCJI NA GUZIKU STW”RZ WYKRESY
	bool create_diagrams_script = false;

	//WSKAèNIKI NA OBIEKTY AKTYWNE I PASYWNE:
	std::vector<sf::Drawable*> drawable_objects;
	std::vector<Mouseable*>    mousable_objects;

	//PAMI C NA OBIEKTY INTERFACEU:
	std::vector<Button> buttons;
	std::map<std::string, Checkbox> checkboxes;
	std::vector<sf::Text> texts;
	std::vector<sf::RectangleShape> graphics;
	std::vector<sf::RectangleShape> solids;

	//NAPIS POWIAZANY Z ILOSCIA WCZYTANYCH GRAFIK:
	sf::Text selected_files_text;

	//OBIEKT TAKSK BARU:
	sf::RectangleShape taskbar;

	//åCIERZKI DO OBRAZOW DO PRZETWORZENIA:
	std::vector<std::filesystem::path> selected_files;

	//Funkcja ustawiajπca dla danego przycisku edycji kolory z ustalonej palety.
	auto setButtonColors(Button& button, bool use_bright = false)
	{
		if (!use_bright)
		{
			button.setOverlayColors(
				color[text_dark], color[text_dark_hover],
				color[text_dark_click]);
			button.setButtonOutlineColors(
				sf::Color::Transparent, sf::Color::Transparent,
				sf::Color::Transparent);
			button.setButtonFillColors(
				sf::Color::Transparent, sf::Color::Transparent,
				sf::Color::Transparent);
		}
		else
		{
			button.setOverlayColors(
				color[text_bright], color[text_bright_hover],
				color[text_bright_click]);
			button.setButtonOutlineColors(
				sf::Color::Transparent, sf::Color::Transparent,
				sf::Color::Transparent);
			button.setButtonFillColors(
				sf::Color::Transparent, sf::Color::Transparent,
				sf::Color::Transparent);
		}
	};

	//Waliduje wszystkie ustawienia w interfejsie: ilosÊ wczytanych grafik, poprawnoúÊ
	//checkboxÛw. Jeøeli ustawiono wszystko co potrzeba by rozpoczπÊ przetwarzanie to
	//zwrÛÊi prawdÍ, w przeciwnym wypadku fa≥sz.
	bool validateInputs()
	{
		//NIE WYBRANO GRAFIKI:
		if (selected_files.size() < 2){
			std::wstring text =
				L"Przed uruchomieniem musisz wybraÊ co najmniej jednπ grafikÍ.";
			Windows::messageBoxInfo(text, L"WybÛr grafik - b≥πd", window.getSystemHandle());
			return false;
		}

		//POBRANIE NAZW NIE-ZAZNACZONYCH CHECKBOXOW:
		std::vector<std::string> method_names;
		for (auto& checkbox : checkboxes)
			if (!std::get<Checkbox>(checkbox))
				method_names.push_back(checkbox.first);

		//PONOWNE WCZYTANIE DANYCH BY ZAKTUALIZOWAC NADPISANE PLIKI:
		quantization_runner.loadSettings(working_directory / "settings");

		//USUNIECIE DANYCH KTORE NIE BY£Y ZAZNACZONE:
		quantization_runner.removePaletteFuntions(method_names);
		quantization_runner.removeRescaleFuntions(method_names);

		//SPRAWDZENIE CZY WYBRANO PRZYNAJMNIEJ JEDNA METODE DANEGO TYPU:
		auto p_rest = quantization_runner.getPaletteFuntions();
		auto r_rest = quantization_runner.getRescaleFuntions();

		if (!p_rest.size() || !r_rest.size())
		{
			std::wstring text = 
				L"Przed uruchomieniem, musisz wybraÊ co najmniej jednπ "
				"metodÍ tworzenia palety kolorÛwm oraz co najmniej jednπ metodÍ "
				"tworzenia prÛbki pixeli.";

			Windows::messageBoxInfo(text, L"WybÛr opcji - b≥πd", window.getSystemHandle());

			//PONOWNE ZA£ADOWANIE W CELU WYZEROWANIA:
			quantization_runner.loadSettings(working_directory / "settings");
			return false;
		}

		//WALIDACJA WYNIKOW PROCENTOWYCH:
		if (checkboxes["Wyniki procentowe"])
		{
			bool removed_none = false;
			for (auto name : method_names)
				if (name == std::string("none"))
					removed_none = true;

			if (removed_none == true || r_rest.size() <= 1)
			{
				std::wstring text =
					L"Wybrano opcjÍ 'wyniki procentowe'. "
					"By poprawnie wyznaczyÊ wskaüniki wzglÍdne konieczne "
					"jest wybranie co najmniej dwÛch metod skalowania w tym metody 'none'.";

				Windows::messageBoxInfo(text, L"Wyniki procentowe - b≥πd", window.getSystemHandle());
				return false;
			}
		}

		//WALIDACJA TWORZENIA WYKRES”W:
		if (checkboxes["Generuj wykresy"])
		{
			if (!checkboxes["Wyniki procentowe"])
			{
				std::wstring text =
					L"Wybrano opcjÍ 'generuj wykresy'. "
					"By poprawnie stworzyÊ wykresy konieczne jest rÛwnieø "
					"wybranie w ustawieniach opcji 'wyniki procentowe'.";

				Windows::messageBoxInfo(text, L"Generuj wykresy - b≥πd", window.getSystemHandle());
				return false;
			}
		}

		//INFORMACJA O DITHERINGU:
		if (checkboxes["Zastosuj dithering"])
		{
			std::wstring text =
				L"Wybrano opcjÍ 'zastosuj dithering'. "
				"Kwantyzacja z ditheringiem, zaimplementowana w tym programie, "
				"w przeciwieÒstwie do implementacji zwyk≥ej kwantyzacji, "
				"nie dzia≥a wielowπtkowo. Przetwarzanie grafik, "
				"bÍdzie trwaÊ d≥uøej. KontynuowaÊ?";

			if (!Windows::messageBoxYesNo(text, L"Dithering - ostrzeøenie", window.getSystemHandle()))
				return false;
		}

		//INFORMACJA O ZAPISYWANIU GRAFIK:
		if (checkboxes["Zapisuj grafiki"])
		{
			//Zapisanych zostanie ponad 100 grafik wynikowych.
			if ((selected_files.size() - 1) * quantization_runner.resultsPerImage() > 100)
			{
				std::wstring text =
					L"Wybrano opcjÍ 'zapisuj grafiki'. "
					"Przy obecnej konfiguracji na dysku zostanie zapisana "
					"nastÍpujπca iloúÊ nowych obrazÛw w formacie PNG: ";
				text += std::to_wstring(
					(selected_files.size() - 1) * quantization_runner.resultsPerImage());
				text += L". KontynuowaÊ?";

				if (!Windows::messageBoxYesNo(text, L"Zapis grafik - ostrzeøenie", window.getSystemHandle()))
					return false;
			}
		}

		return true;
	}

	//Dodaje na koniec wektora obiektÛw typu text nowy text wg podanych ustawieÒ.
	void createText(const sf::String& string, const sf::Font& font, unsigned int size,
		const sf::Color& color, unsigned int x, unsigned int y
		)
	{
		texts.emplace_back(
			string, font,
			size);
		texts.back().setPosition(sf::Vector2f(x, y));
		texts.back().setFillColor(color);
		texts.back().setLetterSpacing(0.8);
		
	}

	//Tworzy checkboxy oraz guziki edycji plikÛw ze szczegÛ≥ami dzia≥ania danej metody.
	//Size zmienia skaluje wszystkie elementy w relacji do siebie. X - Y pozycja top-left.
	//UWAGA1: Checkboxy i nazwy przy nich sπ tworzone w oparciu o infrmacje jakie zwraca
	//		  wywo≥anie getPaletteFuntions i getPaletteFuntions obiektu quantization_runner.
	//UWAGA2: Nazwy plikÛw do edycji sπ generowane poprzez dodanie do tego co zwrÛci≥y te funkcje
	//		  prefiksu 'p_' lub 'r_' oraz rozszerzenia '.csv'.
	//UWAGA3: Funkcja zak≥ada øe te pliki znajdujπ siÍ w folderze working_directory / "settings"
	//WYWO£AC PO STWORZENIU OKIENKA!
	void createCheckboxes(unsigned int x, unsigned int y, float scale)
	{
		//Funkcja ustawiajπca dla danego checkboxa kolory z ustalonej palety.
		auto setCheckboxColors = [this](Checkbox& checkbox)
		{
			checkbox.setCheckOverlayColors(
				sf::Color::Transparent, sf::Color::Transparent,
				color[text_bright], color[text_bright]);
			checkbox.setTextColors(
				color[text_dark], color[text_dark_hover],
				color[text_dark], color[text_dark_hover]);
			checkbox.setBoxOutlineColors(
				color[text_dark], color[text_dark_hover],
				color[text_dark], color[text_dark_hover]);
			checkbox.setBoxFillColors(
				sf::Color::Transparent, sf::Color(255, 255, 255, 10),
				sf::Color::Transparent, sf::Color(255, 255, 255, 10));
		};

		//ZA£ADOWANIE DANYCH Z FOLDERU working_directory / "settings":
		quantization_runner.loadSettings(working_directory / "settings");

		//Pobranie stringÛw z nazwami metod kreowania palet i prÛbek pixeli:
		auto p_funs = quantization_runner.getPaletteFuntions();
		auto r_funs = quantization_runner.getRescaleFuntions();

		unsigned int size = 23 * scale;
		unsigned int checkbox_margin = size * 0.2;
		unsigned int checkbox_size = size;
		unsigned int x_offset = checkbox_size * 1.5 + x;
		unsigned int y_offset = checkbox_size * 1.3 + y;

		//CHECKBOXY DLA METOD GENEROWANIA PALETY:

		createText(
			"Tworzenie prÛbki pixeli:", font_bold,
			checkbox_size * 1.1,
			color[text_bright],
			x_offset - checkbox_size * 0.1, y_offset);
		y_offset += checkbox_size * 1.3 + checkbox_margin * 2;

		for (auto& r_fun : r_funs)
		{
			if (r_fun != std::string("none"))
			{
				Button button(
					checkbox_size, checkbox_size,
					x_offset, y_offset,
					&wrench_texture,
					std::bind(
						Windows::editExternalFile,
						working_directory / "settings" / "r_" + r_fun + ".csv",
						window.getSystemHandle()));
				setButtonColors(button);
				buttons.emplace_back(std::move(button));
			}

			Checkbox checkbox(
				checkbox_size,
				x_offset + checkbox_size + checkbox_margin * 1.5, y_offset,
				r_fun, font_medium,
				true, &check_texture);
			setCheckboxColors(checkbox);
			checkboxes.emplace(r_fun, std::move(checkbox));

			y_offset += checkbox_size + checkbox_margin;
		}

		//CHECKBOXY DLA METOD GENEROWANIA PROBKI PIXELI:

		y_offset += checkbox_size * 1;

		createText(
			"Tworzenie palety:", font_bold,
			checkbox_size * 1.1,
			color[text_bright],
			x_offset - checkbox_size * 0.1, y_offset);
		y_offset += checkbox_size * 1.3 + checkbox_margin * 2;


		for (auto& p_fun : p_funs)
		{
			Button button(
				checkbox_size, checkbox_size,
				x_offset, y_offset,
				&wrench_texture,
				std::bind(
					Windows::editExternalFile,
					working_directory / "settings" / "p_" + p_fun + ".csv",
					window.getSystemHandle()));
			setButtonColors(button);
			buttons.emplace_back(std::move(button));

			Checkbox checkbox(
				checkbox_size,
				x_offset + checkbox_size + checkbox_margin * 1.5, y_offset,
				p_fun, font_medium,
				true, &check_texture);
			setCheckboxColors(checkbox);

			checkboxes.emplace(p_fun, std::move(checkbox));
			y_offset += checkbox_size + checkbox_margin;
		}

		//CHECKBOXY DLA INNYCH USTAWIE—:

		y_offset += checkbox_size * 1;
		createText(
			"Ustawienia:", font_bold,
			checkbox_size * 1.1,
			color[text_bright],
			x_offset - checkbox_size * 0.1, y_offset);
		y_offset += checkbox_size * 1.3 + checkbox_margin * 2;

		std::vector<std::string> setts = {
			"Zastosuj dithering", "Zapisuj grafiki", "Wyniki procentowe", "Generuj wykresy"};
		for (auto& str : setts)
		{
			Checkbox checkbox(
				checkbox_size,
				x_offset, y_offset,
				str, font_medium,
				(str == std::string("Zastosuj dithering")) ? false : true,
				&check_texture);
			setCheckboxColors(checkbox);

			checkboxes.emplace(str, std::move(checkbox));
			y_offset += checkbox_size + checkbox_margin;
		}
	}

	//Dopisuje wskaüniki obiektÛw ktÛre moøna wyúwietliÊ i kliknπÊ do odpowiednich wektorÛw,
	//w celu pÛüniejszego ich wirtualnego wywo≥ywania przy prztwarzaniu zdarzeÒ i wyúwietlaniu.
	//UWAGA1: Wywo≥aÊ na samym koÒcu po stworzeniu innych obiektÛw by uniknπÊ uniewaønienia wskaünika
	//        na skutek realokacji pamiÍci przy powiÍkszaniu wektora buttons, checkboxes itd...
	void setVirtualPointers()
	{
		drawable_objects.push_back(&taskbar);
		for (auto& solid : solids)
		{
			drawable_objects.push_back(&solid);
		}
		for (auto& graphic : graphics)
		{
			drawable_objects.push_back(&graphic);
		}
		for (auto& checkbox : checkboxes)
		{
			drawable_objects.push_back(&checkbox.second);
			mousable_objects.push_back(&checkbox.second);
		}
		for (auto& button : buttons)
		{
			drawable_objects.push_back(&button);
			mousable_objects.push_back(&button);
		}
		for (auto& text : texts)
		{
			drawable_objects.push_back(&text);
		}
		drawable_objects.push_back(&selected_files_text);
	}

	//£aduje wszystkie potrzebne grafiki, fonty a w miÍdzy czasie wyúwietla splash screen.
	std::wstring loadAssets()
	{
		auto assets_dir = working_directory / "assets";

		if (!std::filesystem::exists(assets_dir))
		{
			return std::wstring(L"Aplikacja nie moøe zostaÊ uruchomiona.\n"
				"Nie znaleziono zasobÛw w lokalizacji: ")
				+ assets_dir.wstring();
		}
			
		//Stworzenie splash screenu:
		std::thread splash_screen_thread(
			createSplashScreen, assets_dir / "quant_studio_splash_screen.png", 3000);

		//W miÍdzy czasie zasoby sπ ≥adowane:
		font_regular.loadFromFile((assets_dir / "Roboto-Regular.ttf").generic_string());
		font_medium.loadFromFile((assets_dir / "Roboto-Medium.ttf").generic_string());
		font_bold.loadFromFile((assets_dir / "Roboto-Bold.ttf").generic_string());
		check_texture.loadFromFile((assets_dir / "check.png").generic_string());
		wrench_texture.loadFromFile((assets_dir / "wrench.png").generic_string());
		help_texture.loadFromFile((assets_dir / "help.png").generic_string());
		run_texture.loadFromFile((assets_dir / "run.png").generic_string());
		load_texture.loadFromFile((assets_dir / "load.png").generic_string());
		logo_texture.loadFromFile((assets_dir / "quant_studio_logo.png").generic_string());
		bg_polygon_texture.loadFromFile((assets_dir / "bg_polygon.jpg").generic_string());
		bg_stripes_texture.loadFromFile((assets_dir / "bg_stripes.png").generic_string());
		close_texture.loadFromFile((assets_dir / "close.png").generic_string());
		minimalize_texture.loadFromFile((assets_dir / "minimalize.png").generic_string());
		create_diagrams_texture.loadFromFile((assets_dir / "diagrams.png").generic_string());

		check_texture.setSmooth(true);
		wrench_texture.setSmooth(true);
		help_texture.setSmooth(true);
		run_texture.setSmooth(true);
		load_texture.setSmooth(true);
		logo_texture.setSmooth(true);
		bg_stripes_texture.setSmooth(true);
		bg_polygon_texture.setSmooth(true);
		close_texture.setSmooth(true);
		minimalize_texture.setSmooth(true);
		create_diagrams_texture.setSmooth(true);

		bg_stripes_texture.setRepeated(true);
		bg_polygon_texture.setRepeated(true);

		splash_screen_thread.join();

		return std::wstring();
	}

	//Tworzy okienko.
	void createWindow(unsigned int width, unsigned int height)
	{
		sf::ContextSettings settings;
		settings.antialiasingLevel = 16;
		settings.depthBits = 24;

		sf::Image icon;
		icon.loadFromFile((working_directory / "assets" / "icon.psd").generic_string());

		window.create(
			sf::VideoMode(width, height),
			"Window", sf::Style::None, settings);
		window.setTitle("QuantStudio");
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

		SetWindowLongPtrA(window.getSystemHandle(), GWL_STYLE, WS_BORDER);
		MARGINS borderless = { 1,1,1,1 };
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &borderless);

		ShowWindow(window.getSystemHandle(), SW_SHOWMINIMIZED);
	}

	//Tworzy czÍúÊ interfacu do wczytywania grafik, otwierania pomocy,
	//oraz startowania programu. KlikniÍcie w 'Wczytaj' modyfikuje zawartoúÊ
	//wektora selected_files, a klikniÍcie uruchom uruchamia funkcjÍ validateInputs,
	//i zapisuje wynik jej wykonania do flagi ready_to_run.
	//WYWO£AC PO STWORZENIU OKIENKA!
	void createInterface(unsigned int x_offset, unsigned int y_offset, float scale)
	{
		float logo_size = 0.53 * scale;
		unsigned int width = 500 * scale;

		//T£O DLA STRONY INTERFACE'U:
		sf::RectangleShape bg1;
		bg1.setSize(sf::Vector2f(width, window.getSize().y - y_offset));
		bg1.setPosition(sf::Vector2f(x_offset, y_offset));
		bg1.setFillColor(color[bg_right]);
		bg1.setTexture(&bg_polygon_texture);
		bg1.setTextureRect(sf::IntRect(0, 0, 400, 400 * bg1.getSize().y / bg1.getSize().x));
		solids.emplace_back(std::move(bg1));


		//LOGO APLIKACJI:
		y_offset += logo_texture.getSize().y * logo_size * 0.15;

		sf::RectangleShape logo;
		logo.setFillColor(sf::Color::White);
		logo.setTexture(&logo_texture);
		logo.setSize(sf::Vector2f(
			logo_texture.getSize().x * logo_size,
			logo_texture.getSize().y * logo_size));
		logo.setPosition(sf::Vector2f(
			x_offset - logo_texture.getSize().x / 2.0 * logo_size + width / 2.0,
			y_offset
		));
		graphics.emplace_back(std::move(logo));

		y_offset += logo_texture.getSize().y * logo_size;
		y_offset += logo_texture.getSize().y * logo_size * 0.15;


		//T£O WCZYTYWANIA GRAFIKI:
		sf::RectangleShape bg2;
		bg2.setSize(sf::Vector2f(
			width,
			logo_texture.getSize().y * logo_size * 0.9));
		bg2.setPosition(sf::Vector2f(x_offset, y_offset));
		bg2.setFillColor(color[bg_right_middle]);
		bg2.setTexture(&bg_stripes_texture);
		bg2.setTextureRect(sf::IntRect(0, 0, 500, 500 * bg2.getSize().y/bg2.getSize().x));
		solids.emplace_back(std::move(bg2));

		auto button_scale = logo_texture.getSize().y*logo_size / 280.f;

		y_offset += button_scale * 55.f;

		//GUZIK WCZYTYWANIA GRAFIKI:
		Button load(
			load_texture.getSize().x * button_scale,
			load_texture.getSize().y * button_scale,
			x_offset - load_texture.getSize().x * button_scale / 2.f + width / 2.f,
			y_offset,
			&load_texture);
		setButtonColors(load, true);
		load.setCallback([this](){
			//Pobranie úcieøek do grafik:
			auto handle = window.getSystemHandle();
			auto new_selected_files = Windows::getOpenFileName(handle);
			if (new_selected_files.size() >= 2)
				selected_files = std::move(new_selected_files);

			//Zaktualizowanie napisu:
			auto& text = selected_files_text;
			size_t count = (selected_files.size() <= 1)
				? 0 : selected_files.size() - 1;

			auto bounds_pre = text.getGlobalBounds();

			auto string = std::string("Wybrano ") + std::to_string(count);
			if(count == 1)
				string += " obraz.";
			else if (count >= 2 && count <= 4)
				string += " obrazy.";
			else
				string += " obrazÛw.";
			text.setString(string);

			auto bounds_post = text.getGlobalBounds();

			text.move(sf::Vector2f(
				bounds_pre.left + bounds_pre.width / 2.f - bounds_post.width / 2.f - bounds_post.left,
			0));
		});
		buttons.emplace_back(std::move(load));

		y_offset += load_texture.getSize().y * button_scale * 1.2;


		//NAPIS INFORMUJACZ O ILOSCI WCYZTANZCH OBRAZOW:
		sf::Text text("Wybrano 0 obrazÛw.", font_medium, 32.f * button_scale);
		text.setString("Wybrano 0 obrazÛw.");
		text.setFillColor(color[text_dark]);
		text.setFont(font_medium);
		text.setPosition(sf::Vector2f(
			x_offset - text.getGlobalBounds().width / 2.f + width / 2.f,
			y_offset));
		selected_files_text = std::move(text);

		y_offset += button_scale * 130.f;



		//GUZIK GENEROWANIA WYKRESOW:
		auto button_scale2 = create_diagrams_texture.getSize().y*logo_size / 75.f;

		Button diagrams(
			create_diagrams_texture.getSize().x * button_scale2,
			create_diagrams_texture.getSize().y * button_scale2,
			x_offset - create_diagrams_texture.getSize().x * button_scale2 / 2.f + width / 2.f,
			y_offset,
			&create_diagrams_texture);
		setButtonColors(diagrams, false);
		diagrams.setCallback([this]() { create_diagrams_script = true; });
		buttons.emplace_back(std::move(diagrams));

		y_offset += load_texture.getSize().y * button_scale2 * 1.2;




		auto button_margin = button_scale * 5;

		//GUZIK POMOCY:
		Button help(
			help_texture.getSize().x * button_scale,
			help_texture.getSize().y * button_scale,
			x_offset - (help_texture.getSize().x * button_scale + button_margin + run_texture.getSize().x * button_scale) / 2.f
				+ width / 2.f,
			y_offset,
			&help_texture);
		setButtonColors(help, false);
		help.setCallback([this]() {
			Windows::openExternalFile(working_directory / "resources" / "QuantStudio - Opis.pdf");
		});
		buttons.emplace_back(std::move(help));

		//GUZIK STARTU:
		Button run(
			run_texture.getSize().x * button_scale,
			run_texture.getSize().y * button_scale,
			x_offset - (help_texture.getSize().x * button_scale + button_margin + run_texture.getSize().x * button_scale) / 2.f
				+ help_texture.getSize().x * button_scale + button_margin + width / 2.f,
			y_offset,
			&run_texture);
		setButtonColors(run, true);
		run.setCallback([this]() { ready_to_run = validateInputs(); });
		buttons.emplace_back(std::move(run));


		y_offset += run_texture.getSize().y * button_scale * 1;
	}

	//Tworzy pasek z guzikiem minimalizowania i zamykania.
	void createToolbar(unsigned int height)
	{
		//T£O DLA STRONY INTERFACE'U:
		sf::RectangleShape bg;
		bg.setSize(sf::Vector2f(window.getSize().x, height));
		bg.setPosition(sf::Vector2f(0, 0));
		bg.setFillColor(color[bg_right_middle]);
		taskbar = std::move(bg);

		float button_ratio = 1.5;

		auto cl_color = sf::Color(0xe63e3edd);
		//GUZIK ZAMKNI CIA:
		Button close(
			height*button_ratio, height,
			window.getSize().x - height * button_ratio, 0,
			&close_texture);
		close.setButtonFillColors(
			sf::Color::Transparent, cl_color, cl_color
		);
		close.setButtonOutlineColors(
			sf::Color::Transparent, cl_color, cl_color
		);
		close.setOverlayColors(
			color[text_dark], sf::Color::White, sf::Color::White
		);
		close.setCallback([this]() {
			window.close();
		});
		buttons.emplace_back(std::move(close));


		auto min_color = sf::Color(0, 0, 0, 60);
		//GUZIK MINIMALIZOWANIA:
		Button minimalize(
			height*button_ratio, height,
			window.getSize().x - height * button_ratio * 2.f, 0,
			&minimalize_texture);
		minimalize.setButtonFillColors(
			sf::Color::Transparent, min_color, min_color
		);
		minimalize.setButtonOutlineColors(
			sf::Color::Transparent, min_color, min_color
		);
		minimalize.setOverlayColors(
			color[text_dark], color[text_dark], color[text_dark]
		);
		minimalize.setCallback([this]() {
			ShowWindow(window.getSystemHandle(), SW_MINIMIZE);
		});
		buttons.emplace_back(std::move(minimalize));
	}

	bool assets_found;
public:

	BatchApplication()
		: working_directory(std::filesystem::current_path()),
		temp_directory(std::filesystem::current_path() / ".temp")
	{
		//PRZYGOTOWANIE FOLDERU NA PLIKI TYMCZASOWE:
		std::filesystem::create_directories(temp_directory);
		SetFileAttributes(temp_directory.c_str(), FILE_ATTRIBUTE_HIDDEN);


		auto result = loadAssets();
		if (!result.empty())
		{
			Windows::messageBoxError(result.c_str(), L"Nie znaleziono zasobÛw.");
			assets_found = false;
			return;
		}


		//OKREåLENIE ROZMIARU APLIKACJI:
		auto screen = sf::VideoMode::getDesktopMode();
		double scale = 1;
		unsigned int screen_width = screen.width;
		unsigned int screen_height = screen.height;
		if(screen_width > screen_height)
			scale = (double)screen_width / 1920.0;
		else
			scale = (double)screen_width / 1080.0;

		if (!scale) scale = 1;


		//STWORZENIE OKIENKA I ELEMENTOW GUI:
		createWindow(900 * scale, 680 * scale);
		createToolbar(40 * scale);
		createCheckboxes(520 * scale, 35 * scale, 1.f * scale);
		createInterface(0 * scale, 40 * scale, 1.f * scale);
		setVirtualPointers();

		assets_found = true;
	}
	~BatchApplication()
	{
		//UsuniÍcie folderu plikÛw tymczasowych.
		std::filesystem::remove_all(temp_directory);
	}

	void draw()
	{
		window.clear(color[bg_left]);

		for (auto& object : drawable_objects)
			window.draw(*object);

		window.display();
	}

	operator bool()
	{
		return assets_found;
	}

protected:
	//Funkcja urchamiajπca w≥aúciwe przetwarzanie danych:
	void processImages()
	{
		//Pobranie úciezki zapisu:
		auto save_path = Windows::getFolderName();
		if (!save_path.empty())
		{
			window.setVisible(false);

			//Uruchomienie przetwarzania:
			auto result = quantization_runner.processImages(
				selected_files,
				save_path,
				working_directory / "executables" / "MATLAB_Tasker.exe",
				temp_directory,
				checkboxes["Zapisuj grafiki"],
				checkboxes["Zastosuj dithering"],
				checkboxes["Wyniki procentowe"],
				checkboxes["Generuj wykresy"]
			);

			if (!result.empty())
			{
				//WIADOMOåC PODSUMOWANIA:
				std::wstring summary =
					L"Wszystkie zadania zosta≥y zrealizowane. Podsumowanie:\n";
				summary += result;
				summary += L"\nWyúwietliÊ folder z wynikami?";

				if (Windows::messageBoxYesNo(summary.c_str(), L"Podsumowanie", GetConsoleWindow()))
					Windows::exploreExternalFolder(save_path);
			}

			window.setVisible(true);
		}
	}

	//Funkcja uruchamiajπca skrypt generowania wykresÛw.
	void createDiagramsScript()
	{
		window.setVisible(false);

		//Nawiπzanie komunikacji z MATLAB Tasker:
		auto flag1 = createSplashScreenPromise(
			working_directory / "assets" / "MATLAB_Tasker_open.png");

		MATLAB_Tasker matlab(working_directory / "executables" / "MATLAB_Tasker.exe");
		flag1.set_value(true);


		if (matlab != MATLAB_Tasker::TERMINATED)
		{
			matlab.call("script_createDiagrams");		

			auto flag2 = createSplashScreenPromise(
				working_directory / "assets" / "MATLAB_Tasker_close.png");

			matlab.close();
			flag2.set_value(true);


			window.setVisible(true);
			window.requestFocus();
		}
		else
		{
			window.setVisible(true);
			window.requestFocus();


			Windows::messageBoxError(
				L"Generowanie wykresÛw zosta≥o anulowane gdyø nie uda≥o nawiπzaÊ siÍ komunikacji z aplikacjπ MATLAB Tasker. "
				"Upewnij siÍ, iø posiadasz poprawnie zainstalowany pakiet 'MATLAB Runtime v.901':\n"
				"https://www.mathworks.com/products/compiler/matlab-runtime.html.\nUpewnij siÍ rÛwnieø, øe aplikacja MATLAB Tasker znajduje siÍ "
				"pod podanπ úcieøkπ:\n" + working_directory / "MATLAB_Tasker.exe" + " .",
				L"MATLAB Tasker - B≥πd uruchomienia.", window.getSystemHandle());
		}		
	}

public:
	void processEvents()
	{
		draw();
		ShowWindow(window.getSystemHandle(), SW_RESTORE);
		window.requestFocus();

		sf::Vector2i grabbedOffset;
		bool grabbedWindow = false;

		while (window.isOpen())
		{
			window.waitEvent(event);
			{
				switch (event.type)
				{
				case sf::Event::MouseMoved:
					for (auto& object : mousable_objects)
						object->processMoveEvent(event.mouseMove.x, event.mouseMove.y);

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
						for (auto& object : mousable_objects)
							object->processClickEvent(event.mouseButton.x, event.mouseButton.y);

						grabbedOffset = window.getPosition() - sf::Mouse::getPosition();
						if(taskbar.getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
						grabbedWindow = true;
					}
					break;

			
				case sf::Event::LostFocus:
					grabbedWindow = false;
				case sf::Event::MouseLeft:
					for (auto& object : mousable_objects)
						object->processMoveEvent(-1, -1);
					break;


				case sf::Event::Closed:
					window.close();
					break;
				}
			}
			draw();

			if (ready_to_run)
			{
				ready_to_run = false;
				processImages();
			}

			if (create_diagrams_script)
			{
				create_diagrams_script = false;
				createDiagramsScript();
			}
		}
	}
};