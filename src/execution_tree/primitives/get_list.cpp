//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/get_list.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::int64_t get_int(primitive_argument_type const& p) {
        switch(p.index()) {
            case 2:
                return util::get<2>(p);
            case 4:
                return (std::int64_t)util::get<4>(p)[0];
            default:
                ;
        }
        return 0;
    }
    primitive create_get_list(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("get_list");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const get_list::match_data =
    {
        hpx::util::make_tuple("get_list",
            std::vector<std::string>{"get_list(_1,_2)", "'(_1,_2)"},
            &create_get_list, &create_primitive<get_list>)
    };

    ///////////////////////////////////////////////////////////////////////////
    get_list::get_list(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> get_list::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        bool arguments_valid = true;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type> && args)
            ->  primitive_argument_type
            {
                if(args.size()==2) {
                    if(args[1].index() == 2 && args[0].index() == 7) {
                        auto n = util::get<2>(args[1]);
                        auto value = (util::get<7>(args[0]).get())[n];
                        return primitive_argument_type{std::move(value)};
                    } else if(args[1].index() == 4 && args[0].index() == 7) {
                        int n = (int)(util::get<4>(args[1])[0]);
                        auto value = (util::get<7>(args[0]).get())[n];
                        return primitive_argument_type{std::move(value)};
                    } else if(args[1].index() == 2 && args[0].index() == 4) {
                        auto n = util::get<2>(args[1]);
                        auto value = (util::get<4>(args[0]))[n];
                        return primitive_argument_type{std::move(value)};
                    } else if(args[1].index() == 4 && args[0].index() == 4) {
                        int n = (int)(util::get<4>(args[1])[0]);
                        auto value = (util::get<4>(args[0]))[n];
                        return primitive_argument_type{std::move(value)};
                    }
                }
                return 0;
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> get_list::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
