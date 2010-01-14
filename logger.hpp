//
// logger.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_LOGGER_HPP
#define HTTP_SERVER3_LOGGER_HPP

#include <iostream>
#include "SyslogAppender.hh"
#include "Category.hh"
#include "CategoryStream.hh"

#include <boost/noncopyable.hpp>

#define LINFO(MESSAGE) g_logger.get().log(log4cpp::Priority::INFO, MESSAGE); std::cout << MESSAGE << std::endl
#define LIERR(MESSAGE) g_logger.get().log(log4cpp::Priority::ERROR, MESSAGE); std::cerr << MESSAGE << std::endl

namespace http {
    namespace server3 {

/// The logger service.
        class logger
            : private boost::noncopyable
        {
        public:
            logger() : app_(new log4cpp::SyslogAppender("SyslogAppender", "lisa")),
					   layout_(new log4cpp::BasicLayout()),					   
					   cat_(log4cpp::Category::getInstance("lisa"))
            {
                app_->setLayout(layout_);
                cat_.setAdditivity(false);
                cat_.setAppender(app_);
                cat_.setPriority(log4cpp::Priority::ERROR);
            }

			log4cpp::Category& get()
			{
				return cat_;
			}
        private:
			log4cpp::Appender* app_;
			log4cpp::Layout* layout_;
			log4cpp::Category& cat_;
        };

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_LOGGER_HPP
