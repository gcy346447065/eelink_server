#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <log4cpp/PropertyConfigurator.hh>

using namespace std;
using namespace log4cpp;

#define AppenderName "rootAppender"

extern log4cpp::Category *logger_;

#define LOG_DEBUG \
    LOG4CPP_DEBUG_S((*logger_))<<__FILE__<<":"<<__LINE__
#define LOG_INFO \
    LOG4CPP_INFO_S((*logger_))<<__FILE__<<":"<<__LINE__
#define LOG_WARN \
    LOG4CPP_WARN_S((*logger_))<<__FILE__<<":"<<__LINE__
#define LOG_ERROR \
    LOG4CPP_ERROR_S((*logger_))<<__FILE__<<":"<<__LINE__
#define LOG_FATAL \
    LOG4CPP_FATAL_S((*logger_))<<__FILE__<<":"<<__LINE__

void InitLog(const char *conf_file);
string mformat(const char *fmt, ...);

#endif /*_LOGGER_H*/

