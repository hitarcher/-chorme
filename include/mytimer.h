#pragma once

#ifndef __MY_TIMER__H__
#define __MY_TIMER__H__

#include <string>
#include <mutex>
using namespace std;

#include "json.hpp"
using json = nlohmann::json;

/************************************************************************/
/*                               定时器                                 */
/************************************************************************/

// 支持定时执行一次或轮询执行
// 注意：单线程使用，多次调用setTimeout或其他方法会造成紊乱！
class easyTimer
{
public:
	easyTimer()
	{
		this->mutex_thread_work.lock();
	};
	~easyTimer()
	{
		this->mutex_thread_work.unlock();
		this->mutex_thread_exit.try_lock_for(std::chrono::seconds(10));
	};

private:
	// 替代sleep，可以中途退出
	std::timed_mutex mutex_thread_work;
	// 等待线程安全退出
	std::timed_mutex mutex_thread_exit;

public:
	template<typename Function>
	void setTimeout(Function function, int delay)
	{
		this->mutex_thread_exit.try_lock();
		std::thread t([=]() {
			// 替代sleep
			this->mutex_thread_work.try_lock();
			bool blocked = this->mutex_thread_work.try_lock_for(std::chrono::milliseconds(delay));
			if (blocked == true) {
				return this->mutex_thread_exit.unlock();
			}
			function();
			this->mutex_thread_exit.unlock();
		});
		t.detach();
	}

	template<typename Function>
	void setInterval(Function function, int interval)
	{
		this->mutex_thread_exit.try_lock();
		std::thread t([=]() {
			while (true)
			{
				// 替代sleep
				this->mutex_thread_work.try_lock();
				bool blocked = this->mutex_thread_work.try_lock_for(std::chrono::milliseconds(interval));
				if (blocked == true) {
					return this->mutex_thread_exit.unlock();
				}
				function();
			}
		});
		t.detach();
	}

	template<typename Function>
	void setInterval2(Function function, json param, int interval)
	{
		this->mutex_thread_exit.try_lock();
		std::thread t([=](json th_param) {
			while (true)
			{
				// 替代sleep
				this->mutex_thread_work.try_lock();
				bool blocked = this->mutex_thread_work.try_lock_for(std::chrono::milliseconds(interval));
				if (blocked == true) {
					return this->mutex_thread_exit.unlock();
				}
				function(th_param);
			}
		}, param);
		t.detach();
	}
};

// 支持自定义定时器
// 注意：单线程，只有上一个完成才能执行下一个，强行关闭会导致线程不安全
class myTimer {
public:
	myTimer()
	{
		// 初始化
		mutex_waiting.lock();
	};
	~myTimer()
	{
		// 释放主线程
		m_exit = true;
		mutex_waiting.unlock();

		// 释放所有子线程
		threadpoll_clear();

		// 等待线程退出
		while (!threadpoll_isexited())
		{
			Sleep(2);
		}
	};

private:
	// 主线程退出
	bool m_exit = false;
	// 主线程等待信号
	std::timed_mutex mutex_waiting;
	// 当前执行的timer id
	int m_idevent = NULL;

private:
	/************************************************************************/
	/*                                线程池管理                            */
	/************************************************************************/
	// 所有设置过的timer poll
	//   [%id][active:bool]	
	//   [%id][param:json]
	//   [%id][mutexs:array:uint64_t] 存在一个timer idevent多个线程
	//   [poll:array:%id]
	json m_timerpoll;
	// 给json加锁，防止冲突
	std::timed_mutex mutex_poll;

	// 添加到线程池
	void threadpoll_join(int idevent, void *pmutex, json param)
	{
		mutex_poll.lock();
		// [%id]
		m_timerpoll[to_string(idevent).c_str()]["active"] = true;
		m_timerpoll[to_string(idevent).c_str()]["param"] = param;
		m_timerpoll[to_string(idevent).c_str()]["mutexs"].push_back((uint64_t)pmutex);

		// [poll]
		bool already = false;
		for (json::iterator it = m_timerpoll["poll"].begin();
			it != m_timerpoll["poll"].end(); it++)
		{
			if ((*it).get<std::int32_t>() == idevent) {
				already = true;
				break;
			}
		}
		if (already == false) m_timerpoll["poll"].push_back(idevent);
		mutex_poll.unlock();
	}

	// 匹配pmutex（单个）线程退出：非阻赛
	void threadpoll_exit(int idevent, void *pmutex)
	{
		mutex_poll.lock();
		try
		{
#ifdef DEBUG
			int size0 = m_timerpoll[to_string(idevent).c_str()]["mutexs"].size();
#endif 
			// [%id]
			//   [%id][mutexs:array:uint64_t]
			for (json::iterator it = m_timerpoll[to_string(idevent).c_str()]["mutexs"].begin();
				it != m_timerpoll[to_string(idevent).c_str()]["mutexs"].end(); it++) {
				// 找到并删除
				if ((*it).get<std::uint64_t>() == (uint64_t)pmutex) {
					m_timerpoll[to_string(idevent).c_str()]["mutexs"].erase(it);
					break;
				}
			}
#ifdef DEBUG
			std::string tmp = m_timerpoll.dump();
			int size = m_timerpoll[to_string(idevent).c_str()]["mutexs"].size();
#endif 
			// [poll]
			// do nothing
		}
		catch (const std::exception&)
		{
			// do nothing
		}
		mutex_poll.unlock();
	}

	// 匹配idevent（一组）线程移除：非阻赛
	void threadpoll_remove(int idevent)
	{
		mutex_poll.lock();
		try
		{
			// [%id]
			//   [%id][active:bool]	
			m_timerpoll[to_string(idevent).c_str()]["active"] = false;

			//   [%id][mutexs:array:uint64_t]
			for (json::iterator it = m_timerpoll[to_string(idevent).c_str()]["mutexs"].begin();
				it != m_timerpoll[to_string(idevent).c_str()]["mutexs"].end(); it++) {
				// 移除
				std::timed_mutex *pmutex_elapse =
					(std::timed_mutex *)(void*)((*it).get<std::uint64_t>());
				pmutex_elapse->unlock();
			}

			// [poll]
			// do nothing
		}
		catch (const std::exception&)
		{
			// do nothing
		}
		mutex_poll.unlock();
	}

	// 匹配idevent获取param
	json threadpoll_param(int idevent)
	{
		json param;
		mutex_poll.lock();
		try
		{
			param = m_timerpoll[to_string(idevent).c_str()]["param"];
		}
		catch (const std::exception&)
		{
			// do nothing
		}
		mutex_poll.unlock();
		return param;
	}

	// 匹配idevent查看timer的激活状态，kill就是false
	bool threadpoll_active(int idevent)
	{
		bool active = false;
		mutex_poll.lock();
		try
		{
			active = m_timerpoll[to_string(idevent).c_str()]["active"];
		}
		catch (const std::exception&)
		{
			// do nothing
		}
		mutex_poll.unlock();
		return active;
	}

	// 清空所有线程池
	void threadpoll_clear()
	{
		mutex_poll.lock();
		//   [poll:array:%id]
		for (json::iterator itid = m_timerpoll["poll"].begin();
			itid != m_timerpoll["poll"].end(); itid++) {
			//   [%id][mutexs:array:uint64_t] 
			json jidmutexs = m_timerpoll[to_string((*itid).get<std::int32_t>()).c_str()];
			for (json::iterator itmutex = jidmutexs["mutexs"].begin();
				itmutex != jidmutexs["mutexs"].end(); itmutex++) {
				// 移除
				std::timed_mutex *pmutex_elapse =
					(std::timed_mutex *)(void*)((*itmutex).get<std::uint64_t>());
				pmutex_elapse->unlock();
			}
			m_timerpoll[to_string((*itid).get<std::int32_t>()).c_str()]["active"] = false;
		}
		mutex_poll.unlock();
	}

	// 是否全部释放
	bool threadpoll_isexited()
	{
		bool exit = true;
		mutex_poll.lock();
		try
		{
#ifdef DEBUG
			std::string tmp = m_timerpoll.dump();
#endif 
			//   [poll:array:%id]
			for (json::iterator itid = m_timerpoll["poll"].begin();
				itid != m_timerpoll["poll"].end(); itid++) {
				//   [%id][mutexs:array:uint64_t] 
				exit &= !(m_timerpoll[to_string((*itid).get<std::int32_t>()).c_str()]["mutexs"].size());
			}
		}
		catch (const std::exception&)
		{
			// do nothing
		}
		mutex_poll.unlock();
		return exit;
	}

public:
	// 自定义timer函数
	template<typename Function>
	void onTimer(Function function)
	{
		std::thread t([=]() {
			std::timed_mutex mutex_main; mutex_main.lock();
			threadpoll_join(-1, &mutex_main, "");
			while (!m_exit)
			{
				// 等待timer过来
				mutex_waiting.lock();

				// 主线程退出，此线程退出
				if (m_exit) break;

				// 执行.....
				function(this->m_idevent, threadpoll_param(this->m_idevent));
			}
			threadpoll_exit(-1, &mutex_main);
		});
		t.detach();
	}

	// 定时执行：只执行一次
	void setTimer(int idevent, int elapse, json param)
	{
		// 定时
		std::thread t([=]() {
			// 使用mutex代替sleep
			std::timed_mutex mutex_elapse; mutex_elapse.lock();

			// 添加到线程池
			threadpoll_join(idevent, &mutex_elapse, param);

			// 开始计时......
			bool blocked = mutex_elapse.try_lock_for(std::chrono::milliseconds(elapse));

			// 中断：主线程退出，不再执行定时任务
			if (blocked == true) return threadpoll_exit(idevent, &mutex_elapse);

			// 中断：定时器被kill
			if (threadpoll_active(idevent) == false)
				return threadpoll_exit(idevent, &mutex_elapse);

			// 执行......
			//   注：若此时ontimer正在执行，那么此此消息被忽略
			this->m_idevent = idevent;
			mutex_waiting.unlock();

			// 标记此线程已经退出
			threadpoll_exit(idevent, &mutex_elapse);
		});
		t.detach();
	}

	// 定时执行：轮询
	void setInterval(int idevent, int elapse, json param)
	{
		killTimer(idevent);

		std::thread t([=]() {
			// 使用mutex代替sleep
			std::timed_mutex mutex_elapse; mutex_elapse.lock();

			// 添加到线程池
			threadpoll_join(idevent, &mutex_elapse, param);

			// 定时
			while (true)
			{
				// set timer
				setTimer(idevent, elapse, param);

				// 开始计时......
				//   +10是为了防止上一个还没执行完就执行了下一条
				bool blocked = mutex_elapse.try_lock_for(std::chrono::milliseconds(elapse + 10));

				// 中断：主线程退出，不再执行定时任务
				if (blocked == true) return threadpoll_exit(idevent, &mutex_elapse);

				// 中断：定时器被kill
				if (threadpoll_active(idevent) == false)
					return threadpoll_exit(idevent, &mutex_elapse);
			}
		});
		t.detach();
	}

	// 提前执行之前杀掉定时器
	void killTimer(int idevent)
	{
		// 移除所有的线程
		threadpoll_remove(idevent);
	}

	// 清空所有消息
	void clear()
	{
		threadpoll_clear();
	}
};

#endif