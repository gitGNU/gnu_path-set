// Copyright (C) 2016  Stefan Vargyas
// 
// This file is part of Path-Set.
// 
// Path-Set is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Path-Set is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Path-Set.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"

#include <stdio.h>

#include "common.h"
#include "ptr-traits.h"
#include "set-intf.h"

const char program[] = STRINGIFY(PROGRAM);
const char verdate[] = "0.6 -- 2016-08-19 09:20"; // $ date +'%F %R'

const char license[] =
"Copyright (C) 2016  Stefan Vargyas.\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

static void set_intf_nop(
    const struct set_intf_t* set UNUSED,
    FILE* file UNUSED)
{
    // stev: nop
}

static void exec_set(const struct options_t* opt)
{
    typedef void (*action_func_t)(
        const struct set_intf_t*,
        FILE*);
    static const action_func_t actions[] = {
        [options_load_only_action]   = set_intf_nop,
        [options_print_set_action]   = set_intf_print_set,
        [options_print_elems_action] = set_intf_print_elems,
    };
    struct set_intf_t* set;
    action_func_t act;

    set = set_intf_create(opt);
    set_intf_load(set, stdin, opt->verbose);

    act = ARRAY_NULL_ELEM(actions, opt->action);
    ASSERT(act != NULL);
    act(set, stdout);

    if (opt->print_stats) {
        if (opt->action != options_load_only_action &&
            !set_intf_is_empty(set))
            fputc('\n', stdout);

        set_intf_print_stats(set, stdout);
    }

    set_intf_destroy(set);
}

int main(int argc, char* argv[])
{
    const struct options_t* opt = options(argc, argv);

    if (opt->action == options_print_config_action)
        print_config(stdout);
    else
    if (opt->action == options_print_sizes_action)
        print_sizes(opt, stdout);
    else
    if (opt->action == options_print_names_action)
        print_names(opt, stdout);
    else
        exec_set(opt);

    return 0;
}


