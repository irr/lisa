//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//               2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

#include "soci.h"
#include "soci-mysql.h"

namespace http {
    namespace server3 {

        struct reply;
        struct request;

/// The common handler for all incoming requests.
        class request_handler
            : private boost::noncopyable
        {
        public:
            enum { finished, declined };

            /// Construct with a directory containing files to be served.
            explicit request_handler(std::size_t thread_pool_size, const std::string& database);

            /// Handle a request and produce a reply.
            void handle_request(request& req, reply& rep);

        private:
            /// SQLite3 connection pool.
            const std::auto_ptr<soci::connection_pool> database_pool_;

            /// Perform URL-decoding on a string. Returns false if the encoding was
            /// invalid.
            static bool url_decode(const std::string& in, std::string& out);
        };

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP
