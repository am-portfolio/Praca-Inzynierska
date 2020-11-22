/**********************************************************************
GUI element: Button class.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once

#include "Mouseable.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional>
#include <cassert>

//Klasa pozwalaj¹ca stworzyæ checkbox wraz z powi¹zanym napisem.
class Button : public sf::Drawable, public Mouseable
{
	//OBIEKTY SFML:
	sf::RectangleShape overlay;
	sf::RectangleShape button;

	//STANY:
	enum
	{
		unpressed = 0,
		unpressed_hover,
		pressed,
		count
	};

	//PAMIÊC NA WERSJE KOLOROW:
	sf::Color button_fill_colors[count];
	sf::Color button_outline_colors[count];
	sf::Color overlay_colors[count];

	//FUNKCJA WYWO£YWANA PRZY KLIKNIÊCIU:
	std::function<void(void)> callback;

private:
	//Ustawia wszystkie kolory na dany stan.
	void toggleColors(unsigned int state)
	{
		assert(state >= 0 && state < count);
		button.setOutlineColor(button_outline_colors[state]);
		button.setFillColor(button_fill_colors[state]);
		overlay.setFillColor(overlay_colors[state]);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(button, states);

		if(overlay.getTexture())
			target.draw(overlay, states);
	}

public:
	Button(unsigned int width, unsigned int height, float x, float y,
		const sf::Texture* overlay_texture = nullptr,
		const std::function<void(void)>& callback = [](){})
		: callback(callback)
	{
		//Domyœlne ustawienia.
		setSize(width, height);
		setBoxOutlineThickness(0.1);
		setPosition(x, y);

		setButtonOutlineColors(
			sf::Color(0, 0, 0), sf::Color(0, 0, 0, 210),
			sf::Color(0, 0, 0)
		);
		setButtonFillColors(
			sf::Color(255, 255, 255, 0), sf::Color(255, 255, 255, 40),
			sf::Color(0,  0,  0, 50)
		);
		setOverlayColors(
			sf::Color(255, 255, 255, 255), sf::Color(255, 255, 255, 240),
			sf::Color(255, 255, 255, 200)
		);

		setOverlayTexture(overlay_texture);

		toggleColors(unpressed);
	}


	//---------------------------------------------------//
	//-----POZYCJONOWANIE Z KOMPENSACJ¥ OBRAMOWANIA------//
	//---------------------------------------------------//

	//Ustawia kolor obramowania oraz stosunek rozmiaru
	//ramki do ca³ego rozmiaru buttonu - wartoœci [0, 1).
	bool setBoxOutlineThickness(float thickness)
	{
		if (thickness >= 1 || thickness <= 0)
			return false;

		auto box_size = getSize();
		auto box_bounds = button.getGlobalBounds();

		button.setOutlineThickness(
			((box_size.y < box_size.x) ? box_size.y : box_size.x)
			* thickness / 2.f);
		button.setSize(sf::Vector2f(
			box_size.x - 2.f * button.getOutlineThickness(),
			box_size.y - 2.f * button.getOutlineThickness()));

		setPosition(box_bounds.left, box_bounds.top);

		return true;
	}

	//Ustawia rozmiary buttonu (sumowane z obramowaniem).
	bool setSize(unsigned int width, unsigned int height)
	{
		if (!(width*height))
			return false;

		float outline_thickness =
			(!(getSize().x * getSize().y)) ? 0 :
			(button.getOutlineThickness()*2.f) /
			((getSize().y < getSize().x) ? getSize().y : getSize().x);

		button.setSize(sf::Vector2f(width, height));
		button.setOutlineThickness(0);

		overlay.setSize(sf::Vector2f(width, height));

		setBoxOutlineThickness(outline_thickness);

		return true;
	}

	//Zwraca prostok¹t w którym mieœci button..
	auto getBoundingBox()
	{
		return button.getGlobalBounds();
	}

	//Zwraca rozmiary buttonu.
	sf::Vector2f getSize() const
	{
		return button.getSize();
	}

	//Ustawia button wraz z overlayem na podanej pozycji.
	void setPosition(float x, float y)
	{
		auto rect_size = getSize();
	
		button.setPosition(
			x + button.getOutlineThickness(),
			y + button.getOutlineThickness()
		);
		overlay.setPosition(x, y);
	}




	//---------------------------------------------------//
	//------PODSTAWOWE USTAWIENIA PRZYCISKU I AKCJI------//
	//---------------------------------------------------//

	//Ustawia texturê nak³adki graficznej.
	void setOverlayTexture(const sf::Texture* texture, bool resetRect = false)
	{
		overlay.setTexture(texture, resetRect);
	}

	//Ustawia texturê w tle przycisku.
	void setButtonTexture(const sf::Texture* texture, bool resetRect = false)
	{
		button.setTexture(texture, resetRect);
	}

	void setCallback(const std::function<void(void)>& callback)
	{
		this->callback = callback;
	}


	//---------------------------------------------------//
	//--------FUNKCJE DO USTAWINIA PALETY KOLOROW--------//
	//---------------------------------------------------//

	//Pozwala ustawiæ paletê kolorów dla wype³nienia przycisku.
	//unpressed, unpressed_hover, pressed.
	void setButtonFillColors(
		const sf::Color& upr, const sf::Color& upr_h,
		const sf::Color& pr)
	{
		button_fill_colors[unpressed] = upr;
		button_fill_colors[unpressed_hover] = upr_h;
		button_fill_colors[pressed] = pr;

		toggleColors(unpressed);
	}

	//Pozwala ustawiæ paletê kolorów dla ramki checkboxa.
	//unpressed, unpressed_hover, pressed.
	void setButtonOutlineColors(
		const sf::Color& upr, const sf::Color& upr_h,
		const sf::Color& pr)
	{
		button_outline_colors[unpressed] = upr;
		button_outline_colors[unpressed_hover] = upr_h;
		button_outline_colors[pressed] = pr;

		toggleColors(unpressed);
	}

	//Pozwala ustawiæ paletê kolorów dla nak³adki graficznej.
	//Oryginalny obraz bêdzie mno¿ony przez dany kolor.
	//unpressed, unpressed_hover, pressed.
	void setOverlayColors(
		const sf::Color& upr, const sf::Color& upr_h,
		const sf::Color& pr)
	{
		overlay_colors[unpressed] = upr;
		overlay_colors[unpressed_hover] = upr_h;
		overlay_colors[pressed] = pr;

		toggleColors(unpressed);
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
			toggleColors(unpressed_hover);
		}
		else
		{
			toggleColors(unpressed);
		}
	}

	//Sprawdza czy myszka nie kliknê³a w checkbox lub text.
	//Jeœli tak to zmienia stan check boxa i jego kolor.
	virtual void processClickEvent(int x, int y)
	{
		auto bound = getBoundingBox();
		if (bound.contains(sf::Vector2f(x, y)))
		{
			callback();
			toggleColors(pressed);
		}
	}
};


