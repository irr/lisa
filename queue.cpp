//
// queue.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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

        int queue::operator() (const request& req, reply& rep) const
        {
            if ((req.method != "GET") && (req.method != "POST"))
            {
                rep = reply::stock_reply(reply::method_not_allowed);
                return request_handler::finished;
            }

            std::string action((req.uri.size() > 1) ? req.uri.substr(1) : "");

            try
            {
                // Check for valid requests.
                if ((!action.empty()) &&
                    (action != "spy") && (action != "size") && (action != "count"))
                {
                    if (req.method == "GET")
                    {
                        rep = reply::stock_reply(reply::bad_request);
                        return request_handler::finished;
                    }
                }
            }
            catch (std::exception const &exc)
            {
                LIERR(exc.what());
            }

            soci::session sql(*req.database_pool);
            bool rollback = false;

            try
            {
                // Check for queries size/count/spy or dequeue(default)
                if (req.method == "GET")
                {
                    if (action == "size" || action == "count")
                    {
                        int count;
                        sql << "SELECT COUNT(*) FROM q", soci::into(count);

                        std::stringstream scount;
                        scount << count;

                        rep.content = scount.str();

                        content(req, rep);
                        return request_handler::finished;
                    }

                    sql.begin();
                    rollback = true;

                    // Retrieve data and k
                    // URI must be: /spy or / (dequeue)
                    std::string d;
                    soci::indicator ind;

                    soci::statement st = (sql.prepare << "SELECT p(:r)",
                                          soci::use((action.empty() ? 1 : 0)),
                                          soci::into(d, ind));
                    st.execute(true);

                    if (sql.got_data())
                    {
                        switch (ind)
                        {
                            case soci::i_ok:
                                rep.content = d;
                                content(req, rep);
                                break;
                            case soci::i_null:
                                rep = reply::stock_reply(reply::not_found);
                                break;
                            default:
                                LIERR("soci: error retrieving data from SELECT p(:r)");
                                rep = reply::stock_reply(reply::internal_server_error);
                                return request_handler::finished;
                        }
                    }
                    else
                    {
                        LIERR("soci: no data from SELECT p(:r)");
                        rep = reply::stock_reply(reply::internal_server_error);
                        sql.rollback();
                        return request_handler::finished;
                    }

                    sql.commit();

                    return request_handler::finished;
                }
                else if (req.method == "POST")
                {
                    if (req.post_data.size() > 2)
                    {
                        std::string uri((req.uri.size() > 1) ? req.uri.substr(1) : "0");

                        std::string d = req.post_data.substr(2);

                        int p(boost::lexical_cast<int>(uri));

                        sql.begin();
                        rollback = true;

                        soci::statement st = (sql.prepare << "INSERT INTO q(d, p) VALUES (:d, :p)",
                                              soci::use(d), soci::use(p));
                        st.execute(true);

                        content(req, rep);

                        sql.commit();
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
                if (rollback)
                {
                    try
                    {
                        sql.rollback();
                    }
                    catch (std::exception const &ex)
                    {
                        LIERR(ex.what());
                    }
                }

                rep = reply::stock_reply(reply::internal_server_error);

                LIERR(e.what());

                return request_handler::finished;
            }

            return request_handler::declined;
        }

        void queue::content(const request& req, reply& rep) const
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
