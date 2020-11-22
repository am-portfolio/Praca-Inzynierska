/**********************************************************************
Multi threaded task managing class.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <array>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <functional>

#include "AsyncVariable.hpp"

// Za pomoca tej klasy mozna w proty sposob podzielic wykonanie
// wielu funkcji na kilka watkow (tyle ile rdzeni ma dany komputer).
// Kolejnos wykonywania zadan przez watki w tej implementacji to LIFO.
class AsyncTasker
{
	//Ogranicza ilosc watkow do max 32.
	using Flag = uint32_t;

	std::vector<std::function<void()>> tasks;
	std::vector<std::thread> threads;
	std::condition_variable no_more_tasks;
	std::condition_variable new_tasks;
	std::mutex mutex;

	size_t thread_count = 0;

	Flag running_flags	   = 0u;
	bool terminate_threads = false;
	

	// Funkcja watkow wykonujacych powierzone zadania.
	// Gdy wszystkie dostepne zadania zostana juz wykonane
	// zostanie wyslane o tym powiadomienie a nastepnie
	// watki beda czekaly na powiadomienie o nowych zadaniach.
	void taskRunner(Flag thread_flag)
	{
		while (true)
		{
			std::unique_lock<std::mutex> locker(mutex);
			new_tasks.wait(locker, [this, thread_flag]
			{
				switch (tasks.empty()) 
				{
				[[unlikely]] case true:
					running_flags &= ~thread_flag;

					if (!running_flags)
						no_more_tasks.notify_all();

					if (terminate_threads)
						[[unlikely]] return true;
					else
						[[likely]]   return false;

				[[likely]] case false:
					running_flags |= thread_flag;
					return true;
				}
			});
			if (terminate_threads) [[unlikely]] return;

			auto task = std::move(tasks.back());
			tasks.pop_back();
			locker.unlock();

			task();
		}
	}

	// Tworzy tyle watkow ile rdzeni obsluguje dany komputer.
	void createThreads()
	{
		/// Ograniczenie ilosci obslugiwanych watkow by zoptymalizowac
		/// implementacje (liczby jako flagi zamiast bitsetu).
		constexpr size_t bits_per_byte = 8;
		constexpr size_t max_threads   = sizeof(Flag) * bits_per_byte;

		thread_count = (size_t)std::thread::hardware_concurrency();
		thread_count = (thread_count > max_threads) ? max_threads : thread_count;
		threads.reserve(thread_count);

		for (size_t i = 0; i < thread_count; i++)
			threads.emplace_back(&AsyncTasker::taskRunner, this, pow(2, i));
	}

public:

	AsyncTasker()
	{
		createThreads();
	}

	// Czeka az wszystkie zadania zostana zakonczone, powiadamia watki by wrocily
	// a nastepnie wywoluje na kazdym watku join w celu bezpiecznego zamkniecia.
	~AsyncTasker()
	{
		waitForThreads();

		std::unique_lock<std::mutex> locker(mutex);
		terminate_threads = true;
		locker.unlock();

		notifyAll();

		for (auto& t : threads) t.join();
	}

	// Za pomoca tej metody mozna dodac nowe wywolanie funkcji do listy zadan.
	// Jesli lista zadan nie byla pusta to zostanie wykonane gdy przyjdzie jego pora.
	// Jesli lista zadan byla pusta (wszystkie watki czekaja) to by zo zadanie oraz
	// inne dodane wczesniej zostalo wykonane trzeba bedzie wyslac powiadomienie.
	template<typename Foo, typename... Args>
	void addTask(Foo&& function, Args&&... args)
	{
		std::lock_guard<std::mutex> locker(mutex);
		tasks.emplace_back(std::bind(function, args...));
	}

	// Za pomoca tej metody mozna dodac nowe wywolanie funkcji robi¹cej coœ po zakresie
	//do listy zadan. Zakres zostanie podzielony na tyle czêœci ile jest dostêpnych w¹tków.
	//Jesli lista zadan nie byla pusta to zostania zostanie wykonane gdy przyjdzie jego pora.
	//Jesli lista zadan byla pusta (wszystkie watki czekaja) to by zo zadania oraz
	//inne dodane wczesniej zostalo wykonane trzeba bedzie wyslac powiadomienie.
	template<typename Foo, typename Iter>
	void addRangeTask(Foo&& function, Iter beg, Iter end)
	{
		assert(end > begin);

		size_t n_thread  = hardwareConcurrency();
		double increment = (end - beg) / (double)n_thread;

		size_t beg_offset = 0;
		size_t end_offset = 0;

		for (unsigned int i = 0; i < n_thread; i++)
		{
			beg_offset = i * increment;
			end_offset = (i < n_thread - 1) ? ((i + 1) * increment) : (end - beg);

			addTask(function, beg + beg_offset, beg + end_offset);
		}
	}

	// Za pomoca tej metody mozna dodac nowe wywolanie funkcji robi¹cej coœ 2 zakresach
	//do listy zadan. Zakresy zostan¹ podzielone na tyle czêœci ile jest dostêpnych w¹tków.
	//Oba zakresy musz¹ byæ tego samego rozmiaru inaczej do funkcji zostan¹ wys³ane z³e wska¿niki.
	//Jesli lista zadan nie byla pusta to zostania zostanie wykonane gdy przyjdzie jego pora.
	//Jesli lista zadan byla pusta (wszystkie watki czekaja) to by zo zadania oraz
	//inne dodane wczesniej zostalo wykonane trzeba bedzie wyslac powiadomienie.
	template<typename Foo, typename IterA, typename IterB>
	void addRangeTask(Foo&& function, IterA beg_a, IterA end_a, IterB beg_b, IterB end_b)
	{
		assert(end_b - beg_b == end_a - beg_a);
		assert(end_b > beg_b);
		assert(end_a > beg_a);

		size_t n_thread = hardwareConcurrency();
		double increment = (end_a - beg_a) / (double)n_thread;

		size_t beg_offset = 0;
		size_t end_offset = 0;

		for (unsigned int i = 0; i < n_thread; i++)
		{
			beg_offset = i * increment;
			end_offset = (i < n_thread - 1) ? ((i + 1) * increment) : (end_a - beg_a);

			addTask(function, beg_a + beg_offset, beg_a + end_offset,
				beg_b + beg_offset, beg_b + end_offset);
		}
	}


	// Za pomoca tej funkcji mozna powiadomic wszystkie oczekujace watki 
	// ze lista z zadaniami przestala juz byc pusta i mozna zaczac prace.
	void notifyAll()
	{
		new_tasks.notify_all();
	}

	// Za pomoca tej funkcji mozna powiadomic jeden oczekujacy watek 
	// ze lista z zadaniami przestala juz byc pusta i mozna zaczac prace.
	void notifyOne()
	{
		new_tasks.notify_one();
	}

	// Watek wywolujacy ta funkcje zostanie uspiony az do momentu gdy
	// asynchroniczne watki wykonaja wszystkie powierzone zadania. 
	void waitForThreads()
	{
		std::unique_lock<std::mutex> locker(mutex);
		no_more_tasks.wait(locker, [&]
			{ return tasks.empty() && !running_flags; });
	}

	// Za pomoca tej funkcji mozna powiadomic wszystkie oczekujace watki 
	// ze lista z zadaniami przestala juz byc pusta i mozna zaczac prace.
	// Watek wywolujacy ta funkcje zostanie uspiony az do momentu gdy
	// asynchroniczne watki wykonaja wszystkie powierzone zadania. 
	void notifyAndWait()
	{
		notifyAll();
		waitForThreads();
	}

	// Zwraca ilosc pracujacych watkow.
	auto threadCount() const
	{
		return thread_count;
	}

	//Zwraca iloœæ rdzeni którymi dysponuje dany procesor.
	static auto hardwareConcurrency()
	{
		return std::thread::hardware_concurrency();
	}
};

//Globalny obiekt do zarz¹dzania w¹tkami.
AsyncTasker __async_tasker;
#define ASYNC __async_tasker