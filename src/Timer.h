#include <atomic>
#include <functional>
#include <thread>

class Timer {
public:
	Timer(int initial_ms);

	void setInterval(int new_ms);
	void setCallback(std::function<void()> callback);

	void start();
	void stop();

private:
	std::atomic<bool> m_running;
	std::atomic<int> m_interval_ms;
	std::thread m_timerThread;
	std::function<void()> m_callback;
};
