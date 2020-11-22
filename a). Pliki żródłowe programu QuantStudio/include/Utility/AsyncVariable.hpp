/**********************************************************************
Thread safe variable wrapper class.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once
#include <mutex>
#include <thread>
#include <type_traits>


//Pozwala ochroniæ jakikolwiek dostêp do dowolnego typu mutexem.
//Dla typów integralnych lepiej u¿yæ std::atomic.
template<typename T>
class AsyncVariable
{
	static_assert(!std::is_integral<T>::value,
		"If integral use std::atomic.");

	T value;
	std::mutex mutex;

public:

	AsyncVariable() = default;
	AsyncVariable(const T& value)
		: value(value) {}
	AsyncVariable(T&& value)
		: value(std::move(value)) {}

	T& operator=(const T& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value = value;
		return this->value;
	}
	T& operator=(T&& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value = std::move(value);
		return this->value;
	}

	operator T()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return value;
	}
	operator const T() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return value;
	}



	template<typename K>
	T& operator+=(const K& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value += value;
		return this->value;
	}

	template<typename K>
	T& operator-=(const K& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value -= value;
		return this->value;
	}

	template<typename K>
	T& operator/=(const K& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value /= value;
		return this->value;
	}

	template<typename K>
	T& operator*=(const K& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->value *= value;
		return this->value;
	}
};