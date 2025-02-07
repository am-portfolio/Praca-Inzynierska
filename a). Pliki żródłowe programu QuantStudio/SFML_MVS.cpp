/**********************************************************************
QuantStudio - application entry point.
Author: Aron Mandrella
@2018
**********************************************************************/

#include <Application/QuantStudio_GUI.hpp>


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//BatchApplication app;
	//if(app)
	//	app.processEvents();

	Windows::createConsole();

	auto path = Windows::getOpenFileName();

	long double time_a = 0, time_b = 0;
	long double quality_a = 0, quality_b = 0;

#define COUNT 20

	Clock clock;
	for (int i = 0; i < COUNT; i++)
	{
		clock.restart();
		ImageU8 image(path[0] / path[1]);
		Palette p = Palette::kMeans(image, 256, Palette::InitEnum::init_WU);
		auto after = image.applyPalette(p);
		time_a += clock.getTime();

		if (i == COUNT - 1)
		{
			quality_a = ImageU8::DELTAE_76(image, after);
			after.saveToFile(path[0] / "_dwa.png");
		}
		std::cout << i << std::endl;
	}
	time_a /= (long double)COUNT;

	for (int i = 0; i < COUNT; i++)
	{
		clock.restart();
		ImageU8 image(path[0] / path[1]);
		auto new_image = ImageU8::resizeNearestNeighbour(image, 1.0 / 50.0);
		Palette p = Palette::kMeans(new_image, 256, Palette::InitEnum::init_WU);
		auto after = image.applyPalette(p);
		time_b += clock.getTime();

		if (i == COUNT - 1)
		{
			quality_b = ImageU8::DELTAE_76(image, after);
			after.saveToFile(path[0] / "_troj.png");
		}
		std::cout << i << std::endl;
	}
	time_b /= (long double)COUNT;

	std::cout << "\nFPS A: " << 1.0 / time_a;

	std::cout << "\nDE76 A: " << quality_a << std::endl;

	std::cout << "\nFPS B: " << 1.0 / time_b;

	std::cout << "\nDE76 B: " << quality_b << std::endl;



	system("pause");

	return 0;
}