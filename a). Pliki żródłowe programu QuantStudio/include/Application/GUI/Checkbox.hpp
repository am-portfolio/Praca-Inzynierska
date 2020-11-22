/**********************************************************************
GUI element: Checkbox class.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once

#include "Mouseable.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cassert>

//Klasa pozwalaj¹ca stworzyæ checkbox wraz z powi¹zanym napisem.
class Checkbox : public sf::Drawable, public Mouseable
{
	//STA£E:
	const float box_text_space = 0.2;

	//OBIEKTY SFML:
	sf::RectangleShape check_overlay;
	sf::RectangleShape box;
	sf::Text text;

	//STANY:
	enum
	{
		unchecked = 0,
		unchecked_hover,
		checked,
		checked_hover,
		count
	};

	//PAMIÊC NA WERSJE KOLOROW:
	sf::Color text_colors[count];
	sf::Color box_fill_colors[count];
	sf::Color box_outline_colors[count];
	sf::Color check_overlay_colors[count];

	//Aktualny stan checkboxa.
	bool is_checked;

private:
	//Ustawia wszystkie kolory na dany stan.
	void toggleColors(unsigned int state)
	{
		assert(state >= 0 && state < count);
		text.setFillColor(text_colors[state]);
		box.setOutlineColor(box_outline_colors[state]);
		box.setFillColor(box_fill_colors[state]);
		check_overlay.setFillColor(check_overlay_colors[state]);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(text, states);
		target.draw(box, states);

		if(check_overlay.getTexture())
			target.draw(check_overlay, states);
	}

public:
	Checkbox() = default;

	Checkbox(unsigned int size, float x, float y, const sf::String& string,
		const sf::Font& font, bool init_state = false,
		const sf::Texture* check_texture = nullptr)
	{
		is_checked = init_state;

		text.setString(string);
		text.setFont(font);
		
		//Domyœlne ustawienia.
		setSize(size);
		setBoxOutlineThickness(0.1);
		setPosition(x, y);

		setTextColors(
			sf::Color(0, 0, 0), sf::Color(0, 0, 0, 210),
			sf::Color(0, 0, 0), sf::Color(0, 0, 0, 210)
		);
		setBoxOutlineColors(
			sf::Color(0, 0, 0), sf::Color(0, 0, 0, 210),
			sf::Color(0, 0, 0), sf::Color(0, 0, 0, 210)
		);
		setBoxFillColors(
			sf::Color(255, 255, 255, 0), sf::Color(255, 255, 255, 40),
			sf::Color(0,  0,  0, 20),  sf::Color(0,  0,  0, 20)
		);
		setCheckOverlayColors(
			sf::Color(255, 255, 255, 0), sf::Color(255, 255, 255, 0),
			sf::Color(255, 255, 255, 200), sf::Color(255, 255, 255, 160)
		);

		setCheckTexture(check_texture);
	}


	//---------------------------------------------------//
	//-----POZYCJONOWANIE Z KOMPENSACJ¥ OBRAMOWANIA------//
	//---------------------------------------------------//

	//Ustawia kolor obramowania oraz stosunek rozmiaru
	//ramki do ca³ego rozmiaru checkboxa - wartoœci [0, 1).
	bool setBoxOutlineThickness(float thickness)
	{
		if (thickness >= 1 || thickness <= 0)
			return false;

		auto box_size = getSize();
		auto box_bounds = box.getGlobalBounds();

		box.setOutlineThickness(box_size * thickness / 2.f);
		box.setSize(sf::Vector2f(
			box_size - 2.f * box.getOutlineThickness(),
			box_size - 2.f * box.getOutlineThickness()));

		setPosition(box_bounds.left, box_bounds.top);

		return true;
	}

	//Ustawia rozmiar textu oraz checkboxa wraz z obramowaniem.
	bool setSize(unsigned int size)
	{
		if (!size)
			return false;

		text.setCharacterSize(size);

		float outline_thickness =
			(!getSize()) ? 0 :
			(box.getOutlineThickness()*2.f) / getSize();

		box.setSize(sf::Vector2f(size, size));
		box.setOutlineThickness(0);

		check_overlay.setSize(sf::Vector2f(size, size));

		setBoxOutlineThickness(outline_thickness);

		return true;
	}

	//Zwraca prostok¹t w którym mieœci siê text i checkbox.
	auto getBoundingBox()
	{
		auto rect_bounds = box.getGlobalBounds();
		auto text_bounds = text.getGlobalBounds();

		if(!text.getString().isEmpty())
			rect_bounds.width += box_text_space * rect_bounds.width;

		rect_bounds.width += text_bounds.width;

		return rect_bounds;
	}

	//Zwraca rozmiar bêd¹cy rozmiarem textu oraz rozmiarem
	//checkboxa wraz z obramowaniem (pionowy rozmiar ca³oœci).
	float getSize() const
	{
		return box.getGlobalBounds().width;
	}

	//Ustawia checkbox wraz z podpisem na podanej pozycji.
	void setPosition(float x, float y)
	{
		auto rect_size = getSize();
	
		box.setPosition(
			x + box.getOutlineThickness(),
			y + box.getOutlineThickness()
		);
		text.setPosition(
			x + rect_size + rect_size * box_text_space, //Przesuniêcie i ma³y odstêp.
			y - rect_size * 0.1 //wyœrodkowanie w pionie
		);
		check_overlay.setPosition(x, y);
	}




	//---------------------------------------------------//
	//-----PODSTAWOWE USTAWIENIA CHECKBOXU I NAPISU------//
	//---------------------------------------------------//

	//Ustawia texturê zaznaczonego checkboxa.
	void setCheckTexture(const sf::Texture* texture, bool resetRect = false)
	{
		check_overlay.setTexture(texture, resetRect);
	}

	//Ustawia texturê w tle chechboxa.
	void setBoxTexture(const sf::Texture* texture, bool resetRect = false)
	{
		box.setTexture(texture, resetRect);
	}

	//Ustawia treœæ napisu obok checkboxa.
	void setString(const sf::String& string)
	{
		text.setString(string);
	}

	//Ustawia font napisu obok checkboxa.
	void setFont(const sf::Font& font)
	{
		text.setFont(font);
	}



	//---------------------------------------------------//
	//--------FUNKCJE DO USTAWINIA PALETY KOLOROW--------//
	//---------------------------------------------------//

	//Pozwala ustawiæ paletê kolorów dla textu.
	//unchecked, unchecked_hover, checked, checked_hover.
	void setTextColors(
		const sf::Color& uch, const sf::Color& uch_h,
		const sf::Color& ch, const sf::Color& ch_h)
	{
		text_colors[unchecked] = uch;
		text_colors[unchecked_hover] = uch_h;
		text_colors[checked] = ch;
		text_colors[checked_hover] = ch_h;

		toggleColors(is_checked ? checked : unchecked);
	}

	//Pozwala ustawiæ paletê kolorów dla wype³nienia checkboxa.
	//unchecked, unchecked_hover, checked, checked_hover.
	void setBoxFillColors(
		const sf::Color& uch, const sf::Color& uch_h,
		const sf::Color& ch, const sf::Color& ch_h)
	{
		box_fill_colors[unchecked] = uch;
		box_fill_colors[unchecked_hover] = uch_h;
		box_fill_colors[checked] = ch;
		box_fill_colors[checked_hover] = ch_h;

		toggleColors(is_checked ? checked : unchecked);
	}

	//Pozwala ustawiæ paletê kolorów dla ramki checkboxa.
	//unchecked, unchecked_hover, checked, checked_hover.
	void setBoxOutlineColors(
		const sf::Color& uch, const sf::Color& uch_h,
		const sf::Color& ch, const sf::Color& ch_h)
	{
		box_outline_colors[unchecked] = uch;
		box_outline_colors[unchecked_hover] = uch_h;
		box_outline_colors[checked] = ch;
		box_outline_colors[checked_hover] = ch_h;

		toggleColors(is_checked ? checked : unchecked);
	}

	//Pozwala ustawiæ paletê kolorów dla grafiki zaznaczenia.
	//Oryginalny obraz bêdzie mno¿ony przez dany kolor.
	//unchecked, unchecked_hover, checked, checked_hover.
	void setCheckOverlayColors(
		const sf::Color& uch, const sf::Color& uch_h,
		const sf::Color& ch, const sf::Color& ch_h)
	{
		check_overlay_colors[unchecked] = uch;
		check_overlay_colors[unchecked_hover] = uch_h;
		check_overlay_colors[checked] = ch;
		check_overlay_colors[checked_hover] = ch_h;

		toggleColors(is_checked ? checked : unchecked);
	}



	//---------------------------------------------------//
	//--------------PRZETWARZANIE ZDARZEÑ----------------//
	//---------------------------------------------------//

	//Sprawdza po³o¿onie myszki i ustawia adekwatny kolor najechania.
	virtual void processMoveEvent(int x, int y)
	{
		auto bound = getBoundingBox();
		if (bound.contains(sf::Vector2f(x, y)))
		{
			if (is_checked)
				toggleColors(checked_hover);
			else
				toggleColors(unchecked_hover);
		}
		else
		{
			if (is_checked)
				toggleColors(checked);
			else
				toggleColors(unchecked);
		}
	}

	//Sprawdza czy myszka nie kliknê³a w checkbox lub text.
	//Jeœli tak to zmienia stan check boxa i jego kolor.
	virtual void processClickEvent(int x, int y)
	{
		auto bound = getBoundingBox();
		if (bound.contains(sf::Vector2f(x, y)))
		{
			if (is_checked)
			{
				toggleColors(unchecked_hover);
				is_checked = false;
			}
			else
			{
				toggleColors(checked_hover);
				is_checked = true;
			}
		}
	}



	//---------------------------------------------------//
	//------------ZWRACANIE STANU CHECKBOXA--------------//
	//---------------------------------------------------//

	//Zwraca stan checkboxa.
	bool isChecked() const
	{
		return is_checked;
	}

	//Zwraca stan checkboxa.
	operator bool() const
	{
		return isChecked();
	}
};


