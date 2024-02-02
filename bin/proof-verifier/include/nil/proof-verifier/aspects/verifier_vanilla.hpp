//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2022 Aleksei Moskvin <alalmoskvin@nil.foundation>
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

#ifndef PROOF_VERIFIER_VERIFIER_VANILLA_ASPECT_HPP
#define PROOF_VERIFIER_VERIFIER_VANILLA_ASPECT_HPP

#include <iostream>
#include <numeric>

#include <nil/proof-verifier/aspects/path.hpp>
#include <nil/proof-verifier/detail/configurable.hpp>

#include <nil/dbms/plugin/options_description.hpp>
#include <nil/dbms/plugin/variables_map.hpp>

namespace nil {
    namespace proof_verifier {
        namespace detail {
            enum CurveType {
                PALLAS,
                VESTA,
                ED25519,
                BLS12381
            };
        } // namespace detail

        namespace aspects {
            struct verifier_vanilla
                : public detail::configurable<dbms::plugin::variables_map, dbms::plugin::cli_options_description,
                                              dbms::plugin::cfg_options_description> {
                typedef detail::configurable<dbms::plugin::variables_map, dbms::plugin::cli_options_description,
                                             dbms::plugin::cfg_options_description>
                    policy_type;

                typedef typename policy_type::options_types options_type;
                typedef typename policy_type::configuration_type configuration_type;

                typedef typename std::tuple_element<0, options_type>::type cli_options_type;
                typedef typename std::tuple_element<1, options_type>::type cfg_options_type;

                verifier_vanilla(boost::shared_ptr<path> aspct);

                virtual void set_options(cli_options_type &cli) const override;
                virtual void set_options(cfg_options_type &cfg) const override;

                virtual void initialize(configuration_type &vm) override;

                boost::filesystem::path input_circuit_file_path() const;
                boost::filesystem::path input_assignment_file_path() const;
                boost::filesystem::path output_proof_file_path() const;

                detail::CurveType curve_type() const;

                boost::filesystem::path default_config_path() const;

            protected:
                boost::shared_ptr<path> path_aspect;

                boost::filesystem::path circuit_file_path;
                boost::filesystem::path assignment_table_file_path;
                boost::filesystem::path proof_file_path;
                detail::CurveType curve_type_;

            };
        }    // namespace aspects
    }        // namespace proof_verifier
}    // namespace nil

#endif    // PROOF_VERIFIER_VERIFIER_VANILLA_ASPECT_HPP
