//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2022 Aleksei Moskvin <alalmoskvin@nil.foundation>
// Copyright (c) 2022-2023 Ilia Shirobokov <i.shirobokov@nil.foundation>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//---------------------------------------------------------------------------//

#ifndef PROOF_VERIFIER_DETAIL_UTILS_HPP
#define PROOF_VERIFIER_DETAIL_UTILS_HPP

#include <fstream>

#include <nil/marshalling/endianness.hpp>
#include <nil/marshalling/status_type.hpp>
#include <nil/crypto3/marshalling/zk/types/placeholder/proof.hpp>


namespace nil {
    namespace proof_verifier {

        template<typename Endianness, typename Proof, typename CommitmentParamsType>
        Proof read_proof(const CommitmentParamsType &params, boost::filesystem::path input_file) {
            using TTypeBase = nil::marshalling::field_type<Endianness>;

            using proof_marshalling_type = nil::crypto3::marshalling::types::placeholder_proof<TTypeBase, Proof>;

            std::ifstream in;
            in.open(input_file.c_str(), std::ios_base::binary | std::ios_base::in);
            if (!in.is_open())
            {
                std::cerr << "Cannot open input file " << input_file.string()<< std::endl;
                exit(-1);
            }

            in.seekg(0, std::ios_base::end);
            auto size = in.tellg();
            in.seekg(0, std::ios_base::beg);

            std::vector<std::uint8_t> cv(size);
            if (!in.read(reinterpret_cast<char*>(cv.data()), size))
            {
                std::cerr << "Cannot parse input file " << input_file.string()<< std::endl;
                exit(-1);
            }
            in.close();

            proof_marshalling_type val_read;
            auto read_iter = cv.begin();
            auto status = val_read.read(read_iter, cv.size());
            Proof constructed_val_read = nil::crypto3::marshalling::types::make_placeholder_proof<Endianness, Proof>(val_read);

            return constructed_val_read;

        }

    }    // namespace proof_verifier
}    // namespace nil

#endif    // PROOF_VERIFIER_DETAIL_UTILS_HPP
