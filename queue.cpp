//
// queue.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <boost/lexical_cast.hpp>
#include "queue.hpp"
#include "request_handler.hpp"
#include "soci.h"
#include "soci-mysql.h"

namespace http {
    namespace server3 {

        boost::mutex queue::dequeue_mutex_;

        int queue::operator() (const request& req, reply& rep)
        {
            try
            {
                // Check for valid requests.
                std::string action((req.uri.size() > 1) ? req.uri.substr(1) : "");

                if ((!action.empty()) &&
                    (action != "spy") && (action != "size") && (action != "count"))
                {
                    if (req.method == "GET")
                    {
                        rep = reply::stock_reply(reply::bad_request);
                        return request_handler::finished;
                    }
                }

                // Check for queries size/count/spy or dequeue(default)
                if (req.method == "GET")
                {
                    if (action == "size" || action == "count")
                    {
                        int count;
                        (*req.sql) << "SELECT COUNT(*) FROM q", soci::into(count);

                        std::stringstream scount;
                        scount << count;

                        rep.content = scount.str();

                        content(req, rep);
                        return request_handler::finished;
                    }

                    // Retrieve data and k
                    // URI must be: /spy or / (dequeue)
                    unsigned long k;
                    std::string d;

                    boost::mutex::scoped_lock lock(dequeue_mutex_);

                    (*req.sql).begin();

                    (*req.sql) << "SELECT k, d FROM q ORDER BY p DESC,k LIMIT 1 FOR UPDATE", soci::into(k), soci::into(d);

                    rep.content = d;

                    // Dequeue item if URI = "/"
                    if (action.empty())
                    {
                        soci::statement st = ((*req.sql).prepare << "DELETE FROM q where k = :k",
                                              soci::use(k));

                        st.execute(true);
                    }

                    (*req.sql).commit();

                    // Check for 404 (empty/no data) or 200
                    if (d.empty())
                        rep = reply::stock_reply(reply::not_found);
                    else
                        content(req, rep);

                    return request_handler::finished;
                }
                else if (req.method == "POST")
                {
                    if (req.post_data.size() > 2)
                    {
                        std::string uri((req.uri.size() > 1) ? req.uri.substr(1) : "0");

                        std::string d = req.post_data.substr(2);

                        int p = boost::lexical_cast<int>(uri);

                        soci::statement st = ((*req.sql).prepare << "INSERT INTO q(d, p) VALUES (:d, :p)",
                                              soci::use(d),
                                              soci::use(p));

                        st.execute(true);

                        content(req, rep);
                    }
                    else
                    {
                        rep = reply::stock_reply(reply::bad_request);
                    }

                    return request_handler::finished;
                }
            }
            catch (std::exception const &e)
            {
                rep = reply::stock_reply(reply::internal_server_error);

                LIERR(e.what());

                return request_handler::finished;
            }

            return request_handler::declined;
        }

        void queue::content(const request& req, reply& rep)
        {
            header hcl, hct;

            hcl.name = CONTENT_LENGTH;
            hcl.value = boost::lexical_cast<std::string>(rep.content.size());
            rep.headers.push_back(hcl);

            hct.name = CONTENT_TYPE;
            hct.value = MIME_TYPE;
            rep.headers.push_back(hct);

            rep.status = reply::ok;
        }

    } // namespace server3
} // namespace http
