//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the Server Side Public License, version 1,
// as published by the author.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Server Side Public License for more details.
//
// You should have received a copy of the Server Side Public License
// along with this program. If not, see
// <https://github.com/NilFoundation/dbms/blob/master/LICENSE_1_0.txt>.
//---------------------------------------------------------------------------//
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/exception/get_error_info.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#include <nil/proof-generator/aspects/prover_vanilla.hpp>

namespace std {
    template<typename CharT, typename TraitsT>
    std::basic_ostream<CharT, TraitsT> &operator<<(std::basic_ostream<CharT, TraitsT> &out,
                                                   const std::vector<std::string> &xs) {
        return out << std::accumulate(
                   std::next(xs.begin()), xs.end(), xs[0],
                   [](std::string a, const std::string &b) -> std::string { return std::move(a) + ';' + b; });
    }
}    // namespace std

namespace nil {
    namespace proof_generator {
        namespace aspects {
            prover_vanilla::prover_vanilla(boost::shared_ptr<path> aspct) : path_aspect(std::move(aspct)) {
            }

            void prover_vanilla::set_options(cli_options_type &cli) const {
                boost::program_options::options_description options("NIL Proof Generator");
                // clang-format off
                options.add_options()
                ("version,v", "Display version")
                ("proof", boost::program_options::value<std::string>(),"Output proof file")
                ("circuit,c", boost::program_options::value<std::string>(), "Circuit input file")
                ("assignment-table,t", boost::program_options::value<std::string>(), "Assignment table input file")
                ("log-level,l", boost::program_options::value<std::string>(), "Log level (trace, debug, info, warning, error, fatal)")
#ifdef PROOF_GENERATOR_MODE_MULTI_THREADED
                ("shard0-mem-scale", boost::program_options::value<int>(), "If set allocates this many times more memory for shard #0 compared to other shards.")
#endif
                ("skip-verification", "If set - skips verifiyng step of the generated proof");
                ("elliptic-curve-type,e", boost::program_options::value<std::string>(), "Native elliptic curve type (pallas, vesta, ed25519, bls12381), default: pallas");
                // clang-format on
                cli.add(options);
            }

            void prover_vanilla::set_options(cfg_options_type &cfg) const {
            }

            void prover_vanilla::initialize(configuration_type &vm) {
                if (vm.count("version")) {
#ifdef PROOF_GENERATOR_VERSION
#define xstr(s) str(s)
#define str(s) #s
        std::cout << xstr(PROOF_GENERATOR_VERSION) << std::endl;
#else
        std::cout << "undefined" << std::endl;
#endif
                    }

                std::string log_level = "info";

                if (vm.count("log-level")) {
                    log_level = vm["log-level"].as<std::string>();
                }

                std::map<std::string, boost::log::trivial::severity_level> log_options{
                    {"trace", boost::log::trivial::trace},
                    {"debug", boost::log::trivial::debug},
                    {"info", boost::log::trivial::info},
                    {"warning", boost::log::trivial::warning},
                    {"error", boost::log::trivial::error},
                    {"fatal", boost::log::trivial::fatal}
                };

                if (log_options.find(log_level) == log_options.end()) {
                    BOOST_LOG_TRIVIAL(error) << "Invalid command line argument -l (log level): " << log_level << std::endl;
                    return;
                }

                boost::log::core::get()->set_filter(boost::log::trivial::severity >= log_options[log_level]);

                if (vm.count("circuit")) {
                    if (vm["circuit"].as<std::string>().size() < PATH_MAX ||
                        vm["circuit"].as<std::string>().size() < FILENAME_MAX) {
                        if (boost::filesystem::exists(vm["circuit"].as<std::string>())) {
                            circuit_file_path = vm["circuit"].as<std::string>();
                        }
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Circuit file path is too long";
                    }
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Circuit file path not specified";
                }

                if (vm.count("assignment-table")) {
                    if (vm["assignment-table"].as<std::string>().size() < PATH_MAX ||
                        vm["assignment-table"].as<std::string>().size() < FILENAME_MAX) {
                        if (boost::filesystem::exists(vm["assignment-table"].as<std::string>())) {
                            assignment_table_file_path = vm["assignment-table"].as<std::string>();
                        }
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Assignment table file path is too long";
                    }
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Assignment table file path not specified";
                }

                if (vm.count("proof")) {
                    proof_file_path = vm["proof"].as<std::string>();
                } else {
                    proof_file_path = path_aspect->current_path() / "proof.bin";
                    BOOST_LOG_TRIVIAL(debug) << "Proof file path not specified, using default: " << proof_file_path;
                }

                if (vm.count("skip-verification")) {
                    skip_verification = true;
                }
#ifdef PROOF_GENERATOR_MODE_MULTI_THREADED
                if (vm.count("shard0-mem-scale")) {
                    shard0_mem_scale = vm["shard0-mem-scale"].as<int>();
                }
#endif
                std::string elliptic_curve = "pallas";

                if (vm.count("elliptic-curve-type")) {
                    elliptic_curve = vm["elliptic-curve-type"].as<std::string>();
                }

                std::map<std::string, int> curve_options{
                    {"pallas", detail::PALLAS},
                    {"vesta", detail::VESTA},
                    {"ed25519", detail::ED25519},
                    {"bls12381", detail::BLS12381},
                };

                if (curve_options.find(elliptic_curve) == curve_options.end()) {
                    BOOST_LOG_TRIVIAL(error) << "Invalid command line argument -e (Native elliptic curve type): " << elliptic_curve;
                } else {
                    BOOST_LOG_TRIVIAL(debug) << "Elliptic curve type not specified, using default: " << elliptic_curve;
                }
            }

            boost::filesystem::path prover_vanilla::default_config_path() const {
                return path_aspect->config_path() / "config.ini";
            }

            boost::filesystem::path  prover_vanilla::input_circuit_file_path() const {
                return circuit_file_path;
            }

            boost::filesystem::path prover_vanilla::input_assignment_file_path() const {
                return assignment_table_file_path;
            }

            boost::filesystem::path prover_vanilla::output_proof_file_path() const {
                return proof_file_path;
            }

            bool prover_vanilla::is_skip_verification_mode_on() const {
                return skip_verification;
            }

            detail::CurveType prover_vanilla::curve_type() const {
                return curve_type_;
            }

#ifdef PROOF_GENERATOR_MODE_MULTI_THREADED
            int prover_vanilla::get_shard0_mem_scale() const {
                return shard0_mem_scale;
            }
#endif
        }    // namespace aspects
    }        // namespace proof_generator
}    // namespace nil
