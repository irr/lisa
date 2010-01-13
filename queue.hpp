//
// queue.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_QUEUE_HPP
#define HTTP_SERVER3_QUEUE_HPP

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "globals.hpp"
#include "reply.hpp"
#include "request.hpp"

namespace http {
    namespace server3 {

/// The priority-queue service.
        class queue
            : private boost::noncopyable
        {
        public:
            int operator() (const request& req, reply& rep);
        private:
            void content(const request& req, reply& rep);

			static boost::mutex dequeue_mutex_;
        };

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_QUEUE_HPP
