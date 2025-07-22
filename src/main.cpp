/**
 * SRAVZ LLC
 **/

#include "main.hpp"

int main(int ac, char *av[]) {
    try {
        int opt;
        string config_file_name;
        string sub_program_name;
        string redis_host;
        string redis_port;
        string redis_password;
        string JWT_ACCESS_TOKEN_SECRET;
        string DisableJWTTokenValidation;

        set<string> sub_program_options{"ws-vendor", "redis-uploader",
                                        "webserver"};
        set<string> log_levels{"debug", "info", "warning", "error"};
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()("version,v", "print version string")(
            "help",
            "produce help message")("config,c",
                                    po::value<string>(&config_file_name)
                                        ->default_value("app.production.cfg"),
                                    "name of a file of a configuration.")(
            "sub-program-name,s",
            po::value<string>(&sub_program_name)->default_value("ws-vendor"),
            "Module name to run. Options: ws-vendor|redis-uploader|webserver.");

        // Declare a group of options that will be
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        config.add_options()("ws-vendor-hostname", po::value<string>(),
                             "websocket vendor hostname")(
            "ws-vendor-port", po::value<string>(), "websocket vendor port")(
            "ws-vendor-greeting-str", po::value<string>(),
            "websocket vendor greeting string")("ws-vendor-endpoint",
                                                po::value<string>(),
                                                "websocket vendor endpoint")(
            "ws-vendor-no-threads", po::value<int>(),
            "websocket vendor no of threads")(
            "redis-topics", po::value<string>(),
            "comma separated list of redis topics")(
            "webserver-hostname", po::value<string>(),
            "webserver hostname or ip address to run on")(
            "webserver-port", po::value<string>(), "webserver port")(
            "webserver-root-dir", po::value<string>(), "webserver root dir")(
            "webserver-no-threads", po::value<string>(),
            "webserver no of threads")("webserver-topics", po::value<string>(),
                                       "comma separated list of topics")(
            "allowed-origins", po::value<string>(),
            "comma separated list of allowed origins")(
            "log-level", po::value<string>(), "log level");

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        // hidden.add_options()
        //     ("input-file", po::value< vector<string> >(), "input file")
        //     ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::positional_options_description p;
        // p.add("input-file", -1);

        // Calls to store, parse_command_line and notify functions
        // cause vm to contain all the options found on the command line.
        po::variables_map vm;
        store(po::command_line_parser(ac, av)
                  .options(cmdline_options)
                  .positional(p)
                  .run(),
              vm);
        notify(vm);

        if (!getConfigFile(config_file_name)) {
            BOOST_LOG_TRIVIAL(error) << "Config file not set. Exiting...: ";
            return -1;
        } else {
            BOOST_LOG_TRIVIAL(info) << "Using config file: " << config_file_name;
        }

        ifstream ifs(config_file_name.c_str());
        if (!ifs) {
            BOOST_LOG_TRIVIAL(info)
                << "can not open config file: " << config_file_name;
            return 0;
        } else {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }

        // If variables map contains help. Please help
        if (vm.count("help")) {
            BOOST_LOG_TRIVIAL(info) << visible;
            return 0;
        }

        if (vm.count("version")) {
            BOOST_LOG_TRIVIAL(info) << "backend-cpp, version 1.0";
            return 0;
        }

        // If variables map contains sub_program_name. Check it is a valid
        // sub_program.
        if (sub_program_options.find(sub_program_name) ==
            sub_program_options.end()) {
            BOOST_LOG_TRIVIAL(info) << "sub-program-name: " << sub_program_name
                                    << " invalid. should be one of: ";
            for (string const &sub_program : sub_program_options) {
                BOOST_LOG_TRIVIAL(info) << sub_program << ' ';
            }
            return 1;
        }

        std::string hostname;
        boost::program_options::options_description desc_env;
        // TODO: Add these back if we need to pass the parameters at the cli
        desc_env.add_options()(
            "redis_host",
            boost::program_options::value<std::string>(&redis_host));
        desc_env.add_options()(
            "redis_port",
            boost::program_options::value<std::string>(&redis_port));
        desc_env.add_options()(
            "redis_password",
            boost::program_options::value<std::string>(&redis_password));
        desc_env.add_options()("JWT_ACCESS_TOKEN_SECRET",
                               boost::program_options::value<std::string>(
                                   &JWT_ACCESS_TOKEN_SECRET));
        desc_env.add_options()("DisableJWTTokenValidation",
                               boost::program_options::value<std::string>(
                                   &DisableJWTTokenValidation));

        boost::program_options::variables_map vm_env;
        boost::program_options::store(
            boost::program_options::parse_environment(
                desc_env,
                [](const std::string &i_env_var) {
                    switch (get_hash(i_env_var)) {
                        case "REDIS_HOST"_: {
                            return "redis_host";
                            break;
                        }
                        case "REDIS_PORT"_:
                            return "redis_port";
                            break;
                        case "REDIS_PASSWORD"_:
                            return "redis_password";
                            break;
                        case "JWT_ACCESS_TOKEN_SECRET"_:
                            return "JWT_ACCESS_TOKEN_SECRET";
                            break;
                        case "DisableJWTTokenValidation"_:
                            return "DisableJWTTokenValidation";
                            break;
                        default:
                            return "";
                    }
                }),
            vm_env);
        boost::program_options::notify(vm_env);
        // BOOST_LOG_TRIVIAL(info) << "Environment varaibles: \n";
        // BOOST_LOG_TRIVIAL(info) << "redis_host: " <<
        // vm_env["redis_host"].as<string>() << '\n'; BOOST_LOG_TRIVIAL(info) <<
        // "redis_port: " << vm_env["redis_port"].as<string>() << '\n';
        // BOOST_LOG_TRIVIAL(info) << "redis_password: " <<
        // vm_env["redis_password"].as<string>() << '\n';
        // BOOST_LOG_TRIVIAL(info) << "JWT_ACCESS_TOKEN_SECRET: " <<
        // vm_env["JWT_ACCESS_TOKEN_SECRET"].as<string>() << '\n';

        // Init program

        switch (get_hash(vm["log-level"].as<string>())) {
            case "debug"_: {
                logging::core::get()->set_filter(logging::trivial::severity >=
                                                 logging::trivial::debug);
                break;
            }
            case "info"_:
                logging::core::get()->set_filter(logging::trivial::severity >=
                                                 logging::trivial::info);
                break;
            case "warning"_:
                logging::core::get()->set_filter(logging::trivial::severity >=
                                                 logging::trivial::warning);
                break;
            case "error"_:
                logging::core::get()->set_filter(logging::trivial::severity >=
                                                 logging::trivial::error);
                break;
            default:
                BOOST_LOG_TRIVIAL(info)
                    << "Unsupported log level: " << vm["log-level"].as<string>()
                    << endl;
                return 1;
        }

        BOOST_LOG_TRIVIAL(info) << "Boost version: " << BOOST_VERSION;
        BOOST_LOG_TRIVIAL(info) << "sub_program_name " << sub_program_name;
        switch (get_hash(sub_program_name)) {
            case "ws-vendor"_: {
                ws_vendor(vm["ws-vendor-hostname"].as<string>(),
                          vm["ws-vendor-port"].as<string>(),
                          vm["ws-vendor-greeting-str"].as<string>(),
                          vm["ws-vendor-endpoint"].as<string>(),
                          vm["ws-vendor-no-threads"].as<int>());
                break;
            }
            case "redis-uploader"_:
                redis_uploader(vm["redis-topics"].as<string>());
                break;
            case "webserver"_: {
                Aws::SDKOptions options;
                options.httpOptions.installSigPipeHandler = true;
                // options.loggingOptions.logLevel =
                // Aws::Utils::Logging::LogLevel::Trace;
                Aws::InitAPI(options);
                while (true) {
                    try {
                        // Add extra parameters to the vm_env
                        // vm_env.insert() ["allowed-origins"] =
                        // vm["allowed-origins"];
                        webserver(vm["webserver-hostname"].as<string>(),
                                  vm["webserver-port"].as<string>(),
                                  vm["webserver-root-dir"].as<string>(),
                                  vm["webserver-no-threads"].as<string>(),
                                  vm["webserver-topics"].as<string>(), vm,
                                  vm_env);

                    } catch (exception &e) {
                        BOOST_LOG_TRIVIAL(error) << e.what();
                        continue;
                    }
                }
                // Shutdown the AWS SDK for C++ when done
                Aws::ShutdownAPI(options);
                break;
            }
            default:
                BOOST_LOG_TRIVIAL(info)
                    << "Unsupported option: " << sub_program_name << endl;
        }

    } catch (exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        return 1;
    }
    return 0;
}
