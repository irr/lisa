//
// router.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_ROUTER_HPP
#define HTTP_SERVER3_ROUTER_HPP

#include <boost/noncopyable.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "router.hpp"
#include "queue.hpp"

namespace http {
    namespace server3 {

/// The common router for all mapped requests.
        class router
            : private boost::noncopyable
        {
        public:
            router(const request& req, reply& rep)
                : req_(req), rep_(rep)
            {
            }

            int exec()
            {
                return queue()(req_, rep_);
            }

        private:
            const request& req_;
            reply& rep_;
        };

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_ROUTER_HPP
