#include <chrono>

#include "Timer.h"

Timer::Timer(int initial_ms)
	: m_running(false)
	, m_interval_ms(initial_ms)
{
}

void Timer::setInterval(int new_ms)
{
	m_interval_ms.store(new_ms);
}

void Timer::setCallback(std::function<void()> callback)
{
	m_callback = std::move(callback);
}

void Timer::start()
{
	if (m_running) {
		return;
	}
	m_running = true;

	m_timerThread = std::thread([this]() {
		while (m_running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(m_interval_ms.load()));
			if (m_running) {
				m_callback();
			}
		}
	});
}

void Timer::stop()
{
	m_running = false;
	if (m_timerThread.joinable()) {
		m_timerThread.join();
	}
}
