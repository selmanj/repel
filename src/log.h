/*
 * log.h
 *
 *  Created on: Jun 17, 2011
 *      Author: joe
 */

#ifndef LOG_H_
#define LOG_H_

#include <sstream>
#include <cstdio>
#include <ctime>

enum LOG_LEVEL {LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG};

class FilePolicy {
public:
	static FILE*& stream() {
		static FILE* fStream = stderr;
		return fStream;
	}
	static void output(const std::string& str) {
		FILE* fStream = stream();
		if (!fStream) return;

		fprintf(fStream, "%s", str.c_str());
		fflush(fStream);
	}
private:
	FILE* file_;
};


template <class LogPolicy>
class Log {
public:
	Log(bool printStdOut=false)
	  : stream_(), printStdOut_(printStdOut), logLevel_(LOG_INFO) {};
	virtual ~Log();

	static LOG_LEVEL& globalLogLevel();
	static std::string toString(LOG_LEVEL level);

	std::ostringstream& get(LOG_LEVEL level);
private:
	std::string currentTime() const;

	static LOG_LEVEL globalLogLevel_;

	std::ostringstream stream_;
	bool printStdOut_;
	LOG_LEVEL logLevel_;

};

template <class LogPolicy>
Log<LogPolicy>::~Log() {
	std::ostringstream toPrint;
	toPrint << "- " << currentTime();
	toPrint << " " << Log::toString(logLevel_) << ": ";
	stream_ << std::endl;
	toPrint << stream_.str();
	LogPolicy::output(toPrint.str());
	if (printStdOut_ && LogPolicy::stream() != stdout) {
		fprintf(stdout, "%s", stream_.str().c_str());
	}
}

template <class T>
LOG_LEVEL& Log<T>::globalLogLevel() {
	static LOG_LEVEL level = LOG_INFO;

	return level;
}

template <class T>
std::string Log<T>::toString(LOG_LEVEL level) {
	switch (level) {
	case LOG_ERROR:
		return "ERROR";
	case LOG_WARN:
		return "WARN";
	case LOG_INFO:
		return "INFO";
	case LOG_DEBUG:
		return "DEBUG";
	default:
		return "UNKNOWN";
	}
}

template <class T>
std::ostringstream& Log<T>::get(LOG_LEVEL level) {
	logLevel_ = level;
	return stream_;
}

template <class T>
std::string Log<T>::currentTime() const {
	time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	std::string timeStr(asctime(timeinfo));

	return timeStr.substr(11,8);	// just get the hour:min:sec
}

typedef Log<FilePolicy> FileLog;
#define LOG(level) \
	if (level > FileLog::globalLogLevel()) ; \
	else FileLog().get(level)

#define LOG_PRINT(level) \
	(level > FileLog::globalLogLevel() ? std::cout : FileLog(true).get(level))

#endif /* LOG_H_ */
