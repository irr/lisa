//
// lisa.cpp
// ~~~~~~~~
//
// Copyright (c) 2010 Ivan Ribeiro Rocha (ivanribeiro at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include "server.hpp"
#include "logger.hpp"

#include <pthread.h>
#include <signal.h>

#define DEFAULT_DATABASE "db=<db> user=<user> password=<pwd>"
#define DEFAULT_ADDRESS  "0.0.0.0"
#define DEFAULT_PORT      1972
#define DEFAULT_THREADS    42
#define DEFAULT_SAMPLE1  "./lisa -d \"db=lisa user=root password=irr\""
#define DEFAULT_SAMPLE2  "./lisa -d \"db=lisa user=root password=irr\" -a localhost"
#define DEFAULT_SAMPLE3  "./lisa -d \"db=lisa user=root password=irr\" -a 127.0.0.1 -p 1972 -t 10"
#define MAX_PORT         65535
#define MAX_THREADS       100

#define HELP "\nLISA 1.0 beta (http://github.com/irr/lisa)\n\
This is free software, and you are welcome to redistribute it and/or modify\n\
it under the terms of the Boost Software License - Version 1.0.\n\
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)\n\
LISA comes with ABSOLUTELY NO WARRANTY.\n\nAllowed Options"

http::server3::logger g_logger;

void help(boost::program_options::options_description& desc)
{
    std::cout << desc << std::endl << "samples: " << DEFAULT_SAMPLE1 << " or " << std::endl
              << "         " << DEFAULT_SAMPLE2 << std::endl
              << "         " << DEFAULT_SAMPLE3 << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
    try
    {
        // Parse command line arguments.
        namespace po = boost::program_options;

        std::string database;
        std::string address;
        int port, threads;

        std::stringstream smaxport, smaxthreads;
        smaxport << "port [1," << MAX_PORT << "] (optional)";
        smaxthreads << "threads [1," << MAX_THREADS << "] (optional)";

        po::options_description desc(HELP);
        desc.add_options()
            ("help,h", "help message")
            ("database,d", po::value<std::string>(&database)->default_value(DEFAULT_DATABASE), "dsn (mandatory)")
            ("address,a", po::value<std::string>(&address)->default_value(DEFAULT_ADDRESS), "interface (optional)")
            ("port,p", po::value<int>(&port)->default_value(DEFAULT_PORT), smaxport.str().c_str())
            ("threads,t", po::value<int>(&threads)->default_value(DEFAULT_THREADS), smaxthreads.str().c_str());

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        // Check command line arguments.
        if (((vm.count("help")) || (database == DEFAULT_DATABASE)) ||
            (((port <= 0) || (port > MAX_PORT)) ||
             ((threads < 1) || (threads > MAX_THREADS))))
        {
            help(desc);
            return 1;
        }

        std::stringstream sport;
        sport << port;

        // Block all signals for background thread.
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

        // Run server in background thread.
        std::size_t num_threads = boost::lexical_cast<std::size_t>(threads);
        http::server3::server s(database, address, sport.str(), num_threads);
        boost::thread t(boost::bind(&http::server3::server::run, &s));

        // Restore previous signals.
        pthread_sigmask(SIG_SETMASK, &old_mask, 0);

        // Wait for signal indicating time to shut down.
        sigset_t wait_mask;
        sigemptyset(&wait_mask);
        sigaddset(&wait_mask, SIGINT);
        sigaddset(&wait_mask, SIGQUIT);
        sigaddset(&wait_mask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
        int sig = 0;
        sigwait(&wait_mask, &sig);

        // Stop the server.
        s.stop();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        LIERR(e.what());
    }

    log4cpp::Category::shutdown();

    return 0;
}

