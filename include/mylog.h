#pragma once

#ifndef __MY_LOG_H__
#define __MY_LOG_H__

/************************************************************************\
* ËµÃ÷                                                                 
* Õª×Ô£ºcpp_redis logger 
************************************************************************/

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <iostream>

#include "myos.h"

#define CR "\r\n" 

namespace my_logger {

	/**
	* logger_iface
	* should be inherited by any class intended to be used for logging
	*
	*/
	class logger_iface {
	public:
		/**
		* ctor
		*
		*/
		logger_iface() = default;
		/**
		* dtor
		*
		*/
		virtual ~logger_iface() = default;

		/**
		* copy ctor
		*
		*/
		logger_iface(const logger_iface&) = default;
		/**
		* assignment operator
		*
		*/
		logger_iface& operator=(const logger_iface&) = default;

	public:
		/**
		* debug logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		virtual void debug(const std::string& file, const std::string& fileline, const std::string& msg) = 0;

		/**
		* info logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		virtual void info(const std::string& file, const std::string& fileline, const std::string& msg) = 0;

		/**
		* warn logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		virtual void warn(const std::string& file, const std::string& fileline, const std::string& msg) = 0;

		/**
		* error logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		virtual void error(const std::string& file, const std::string& fileline, const std::string& msg) = 0;
	};

	/**
	* default logger class provided by the library
	*
	*/
	class logger : public logger_iface {
	public:
		/**
		* log level
		*
		*/
		enum class log_level {
			error = 0,
			warn = 1,
			info = 2,
			debug = 3
		};

	public:
		/**
		* ctor
		*
		*/
		explicit logger::logger(log_level level = log_level::debug)
			: m_level(level) {}

		/**
		* dtor
		*
		*/
		~logger() override = default;

		/**
		* copy ctor
		*
		*/
		logger(const logger&) = default;

		/**
		* assignment operator
		*
		*/
		logger& operator=(const logger&) = default;

	public:
		/**
		* debug logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		void debug(const std::string& file, const std::string& fileline, const std::string& msg) override
		{
			if (m_level >= log_level::debug) {
				std::lock_guard<std::mutex> lock(m_mutex);

				std::string folder = get_exefolder() + "log\\" + now(t_day) + "\\";
				std::string filefullpath = mkdir(folder.c_str()) ? folder + file : file;

				fstream fi;
				fi.open(filefullpath.c_str(), fstream::binary | fstream::out | fstream::app);
				std::string head =
					now(t_time) + " > debug | " + fileline;
				fi.write(head.c_str(), head.length());
				fi.write(msg.c_str(), msg.length());
				fi.write(CR, strlen(CR));
				fi.close();
			}
		}

		/**
		* info logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		void info(const std::string& file, const std::string& fileline, const std::string& msg) override
		{
			if (m_level >= log_level::info) {
				std::lock_guard<std::mutex> lock(m_mutex);

				std::string folder = get_exefolder() + "log\\" + now(t_day) + "\\";
				std::string filefullpath = mkdir(folder.c_str()) ? folder + file : file;

				fstream fi;
				fi.open(filefullpath.c_str(), fstream::binary | fstream::out | fstream::app);
				std::string head =
					now(t_time) + " > info  | " + fileline;
				fi.write(head.c_str(), head.length());
				fi.write(msg.c_str(), msg.length());
				fi.write(CR, strlen(CR));
				fi.close();
			}
		}

		/**
		* warn logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		void warn(const std::string& file, const std::string& fileline, const std::string& msg) override
		{
			if (m_level >= log_level::warn) {
				std::lock_guard<std::mutex> lock(m_mutex);

				std::string folder = get_exefolder() + "log\\" + now(t_day) + "\\";
				std::string filefullpath = mkdir(folder.c_str()) ? folder + file : file;

				fstream fi;
				fi.open(filefullpath.c_str(), fstream::binary | fstream::out | fstream::app);
				std::string head =
					now(t_time) + " > warn  | " + fileline;
				fi.write(head.c_str(), head.length());
				fi.write(msg.c_str(), msg.length());
				fi.write(CR, strlen(CR));
				fi.close();
			}
		}

		/**
		* error logging
		*
		* @param msg message to be logged
		* @param file file from which the message is coming
		* @param line line in the file of the message
		*
		*/
		void error(const std::string& file, const std::string& fileline, const std::string& msg) override
		{
			if (m_level >= log_level::error) {
				std::lock_guard<std::mutex> lock(m_mutex);

				std::string folder = get_exefolder() + "log\\" + now(t_day) + "\\";
				std::string filefullpath = mkdir(folder.c_str()) ? folder + file : file;

				fstream fi;
				fi.open(filefullpath.c_str(), fstream::binary | fstream::out | fstream::app);
				std::string head =
					now(t_time) + " > error | " + fileline;
				fi.write(head.c_str(), head.length());
				fi.write(msg.c_str(), msg.length());
				fi.write(CR, strlen(CR));
				fi.close();
			}
		}

	private:
		/**
		* current log level in use
		*
		*/
		log_level m_level;

		/**
		* mutex used to serialize logs in multi-threaded environment
		*
		*/
		std::mutex m_mutex;
	};

	static std::unique_ptr<logger_iface> active_logger = std::make_unique<logger>();

	/**
	* debug logging
	* convenience function used internally to call the logger
	*
	* @param msg message to be logged
	* @param file file from which the message is coming
	* @param line line in the file of the message
	*
	*/
	inline
	void debug(const std::string& file, const std::string& fileline, const std::string& msg) {
		if (active_logger)
			active_logger->debug(file, fileline, msg);
	}

	/**
	* info logging
	* convenience function used internally to call the logger
	*
	* @param msg message to be logged
	* @param file file from which the message is coming
	* @param line line in the file of the message
	*
	*/
	inline
	void info(const std::string& file, const std::string& fileline, const std::string& msg) {
		if (active_logger)
			active_logger->info(file, fileline, msg);
	}

	/**
	* warn logging
	* convenience function used internally to call the logger
	*
	* @param msg message to be logged
	* @param file file from which the message is coming
	* @param line line in the file of the message
	*
	*/
	inline
	void warn(const std::string& file, const std::string& fileline, const std::string& msg) {
		if (active_logger)
			active_logger->warn(file, fileline, msg);
	}

	/**
	* error logging
	* convenience function used internally to call the logger
	*
	* @param msg message to be logged
	* @param file file from which the message is coming
	* @param line line in the file of the message
	*
	*/
	inline
	void error(const std::string& file, const std::string& fileline, const std::string& msg) {
		if (active_logger)
			active_logger->error(file, fileline, msg);
	}

	inline
	void deletelog(uint16_t day_past)
	{
		std::string logfolder = get_exefolder() + "LOG";
		std::string day = now_past(t_day, day_past);
		for (const auto & entry : std::experimental::filesystem::directory_iterator(logfolder))
		{
			if(get_filename(entry.path().generic_string()) < day)
				rmdir(entry.path().generic_string().c_str());
		}
	}
}

/**
* convenience macro to log with file and line information
*
*/
#define LOG(file, level, format, ...) my_logger::level(file, get_filename(std::string(__FILE__)) + "[" + to_string(__LINE__) + "] ", vstring(format, ##__VA_ARGS__));

#define LOG_CLEAR(day_past) my_logger::deletelog(day_past)


#endif 