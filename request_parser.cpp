//
// request_parser.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//               2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
#include <string>

#include "boost/lexical_cast.hpp"
#include "request_parser.hpp"
#include "request.hpp"

#define UPPER_CONTENT_LENGTH "CONTENT-LENGTH"
#define UPPER_CONTENT_TYPE   "CONTENT-TYPE"
#define UPPER_MIME_TYPE      "APPLICATION/X-WWW-FORM-URLENCODED"

namespace http {
    namespace server3 {

        request_parser::request_parser()
            : state_(method_start), cl_(0)
        {
        }

        void request_parser::reset()
        {
            state_ = method_start;
        }

        boost::tribool request_parser::consume(request& req, char input)
        {
            switch (state_)
            {
                case method_start:
                    if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        state_ = method;
                        req.method.push_back(input);
                        return boost::indeterminate;
                    }
                case method:
                    if (input == ' ')
                    {
                        state_ = uri;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.method.push_back(input);
                        return boost::indeterminate;
                    }
                case uri_start:
                    if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        state_ = uri;
                        req.uri.push_back(input);
                        return boost::indeterminate;
                    }
                case uri:
                    if (input == ' ')
                    {
                        state_ = http_version_h;
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.uri.push_back(input);
                        return boost::indeterminate;
                    }
                case http_version_h:
                    if (input == 'H')
                    {
                        state_ = http_version_t_1;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_t_1:
                    if (input == 'T')
                    {
                        state_ = http_version_t_2;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_t_2:
                    if (input == 'T')
                    {
                        state_ = http_version_p;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_p:
                    if (input == 'P')
                    {
                        state_ = http_version_slash;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_slash:
                    if (input == '/')
                    {
                        req.http_version_major = 0;
                        req.http_version_minor = 0;
                        state_ = http_version_major_start;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_major_start:
                    if (is_digit(input))
                    {
                        req.http_version_major = req.http_version_major * 10 + input - '0';
                        state_ = http_version_major;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_major:
                    if (input == '.')
                    {
                        state_ = http_version_minor_start;
                        return boost::indeterminate;
                    }
                    else if (is_digit(input))
                    {
                        req.http_version_major = req.http_version_major * 10 + input - '0';
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_minor_start:
                    if (is_digit(input))
                    {
                        req.http_version_minor = req.http_version_minor * 10 + input - '0';
                        state_ = http_version_minor;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case http_version_minor:
                    if (input == '\r')
                    {
                        state_ = expecting_newline_1;
                        return boost::indeterminate;
                    }
                    else if (is_digit(input))
                    {
                        req.http_version_minor = req.http_version_minor * 10 + input - '0';
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case expecting_newline_1:
                    if (input == '\n')
                    {
                        state_ = header_line_start;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case header_line_start:
                    if (input == '\r')
                    {
                        state_ = expecting_newline_3;
                        return boost::indeterminate;
                    }
                    else if (!req.headers.empty() && (input == ' ' || input == '\t'))
                    {
                        state_ = header_lws;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.push_back(header());
                        req.headers.back().name.push_back(input);
                        state_ = header_name;
                        return boost::indeterminate;
                    }
                case header_lws:
                    if (input == '\r')
                    {
                        state_ = expecting_newline_2;
                        return boost::indeterminate;
                    }
                    else if (input == ' ' || input == '\t')
                    {
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        state_ = header_value;
                        req.headers.back().value.push_back(input);
                        return boost::indeterminate;
                    }
                case header_name:
                    if (input == ':')
                    {
                        state_ = space_before_header_value;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.back().name.push_back(input);
                        return boost::indeterminate;
                    }
                case space_before_header_value:
                    if (input == ' ')
                    {
                        state_ = header_value;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case header_value:
                    if (input == '\r')
                    {
                        state_ = expecting_newline_2;
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.back().value.push_back(input);
                        return boost::indeterminate;
                    }
                case expecting_newline_2:
                    if (input == '\n')
                    {
                        state_ = header_line_start;
                        std::vector<header>::const_iterator cit = req.headers.begin();
                        for (; cit != req.headers.end(); ++cit)
                        {
                            std::string n = (*cit).name;
                            std::transform(n.begin(), n.end(), n.begin(), ::toupper);
							
                            if (n == UPPER_CONTENT_LENGTH)
                            {
                                try
                                {
                                    cl_ = boost::lexical_cast<int>((*cit).value);
                                }
                                catch (const boost::bad_lexical_cast& e)
                                {
                                    return false;
                                }
                            }
                            else if (n == UPPER_CONTENT_TYPE)
                            {
                                std::string m = (*cit).value;
                                std::transform(m.begin(), m.end(), m.begin(), ::toupper);

                                if (m != UPPER_MIME_TYPE)
                                {
                                    return false;
                                }
                            }
                        }
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                case expecting_newline_3:
                {
                    if (0 == cl_)
                    {
                        return (input == '\n');
                    }
                    else
                    {
                        state_ = post_data;
                        return boost::indeterminate;
                    }
                }
                case post_data:
                {
                    req.post_data.push_back(input);
                    if (0 == --cl_)
                        return true;
                    else
                        return ((cl_ < 0) ? false : boost::indeterminate);
                }
                default:
                    return false;
            }
        }

        bool request_parser::is_char(int c)
        {
            return c >= 0 && c <= 127;
        }

        bool request_parser::is_ctl(int c)
        {
            return (c >= 0 && c <= 31) || (c == 127);
        }

        bool request_parser::is_tspecial(int c)
        {
            switch (c)
            {
                case '(': case ')': case '<': case '>': case '@':
                case ',': case ';': case ':': case '\\': case '"':
                case '/': case '[': case ']': case '?': case '=':
                case '{': case '}': case ' ': case '\t':
                    return true;
                default:
                    return false;
            }
        }

        bool request_parser::is_digit(int c)
        {
            return c >= '0' && c <= '9';
        }

    } // namespace server3
} // namespace http
