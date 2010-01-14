//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//               2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "request_handler.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "router.hpp"

namespace http {
    namespace server3 {

        request_handler::request_handler(std::size_t thread_pool_size, const std::string& database)
            : database_pool_(new soci::connection_pool(thread_pool_size))
        {
            // Create database connection pool.
            for (std::size_t i = 0; i < thread_pool_size; ++i)
            {
                soci::session& sql = (*database_pool_).at(i);
                sql.open(soci::mysql, database);
            }
        }

        void request_handler::handle_request(request& req, reply& rep)
        {
            // Decode url to path.
            std::string request_path;
            if (!url_decode(req.uri, request_path))
            {
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

            // Request path must be absolute and not contain "..".
            if (request_path.empty() || request_path[0] != '/'
                || request_path.find("..") != std::string::npos)
            {
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

            // Router request based upon a REST API
            req.database_pool = &(*database_pool_);

            router r(req, rep);

            if (r.exec() == declined)
            {
                rep = reply::stock_reply(reply::not_implemented);
            }
        }

        bool request_handler::url_decode(const std::string& in, std::string& out)
        {
            out.clear();
            out.reserve(in.size());
            for (std::size_t i = 0; i < in.size(); ++i)
            {
                if (in[i] == '%')
                {
                    if (i + 3 <= in.size())
                    {
                        int value = 0;
                        std::istringstream is(in.substr(i + 1, 2));
                        if (is >> std::hex >> value)
                        {
                            out += static_cast<char>(value);
                            i += 2;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else if (in[i] == '+')
                {
                    out += ' ';
                }
                else
                {
                    out += in[i];
                }
            }
            return true;
        }

    } // namespace server3
} // namespace http
