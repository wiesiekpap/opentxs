// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/program_options.hpp>
#include <cassert>
#include <iostream>
#include <memory>

#include "opentxs/Types.hpp"

namespace ot = opentxs;
namespace po = boost::program_options;

namespace
{
class ArgumentParser
{
public:
    bool show_help_;

    auto options() const noexcept -> const po::options_description&
    {
        return options_;
    }

    auto parse(int argc, char** argv, ot::ArgList& args) noexcept -> void
    {
        try {
            po::store(po::parse_command_line(argc, argv, options_), variables_);
            po::notify(variables_);
        } catch (po::error& e) {
            std::cerr << "ERROR: " << e.what() << "\n\n"
                      << options_ << std::endl;

            return;
        }

        for (const auto& [name, value] : variables_) {
            if (name == help_) {
                show_help_ = true;
            } else if (name == log_) {
                auto& set = args[OPENTXS_ARG_LOGLEVEL];
                set.clear();

                try {
                    set.emplace(std::to_string(value.as<int>()));
                } catch (...) {
                    args.erase(OPENTXS_ARG_LOGLEVEL);
                }
            }
        }
    }

    ArgumentParser() noexcept
        : show_help_(false)
        , variables_p_(std::make_unique<po::variables_map>())
        , options_p_(std::make_unique<po::options_description>())
        , variables_(*variables_p_)
        , options_(*options_p_)
    {
        options_.add_options()(help_, "Display this message");
        options_.add_options()(
            OPENTXS_ARG_LOGLEVEL,
            po::value<int>(),
            "Log verbosity. Valid values are -1 through 5. Higher numbers are "
            "more verbose. Default value is 0");
    }

private:
    static constexpr auto help_{"help"};
    static constexpr auto log_{OPENTXS_ARG_LOGLEVEL};

    std::unique_ptr<po::variables_map> variables_p_;
    std::unique_ptr<po::options_description> options_p_;
    po::variables_map& variables_;
    po::options_description& options_;
};
}  // namespace
