#define YAB_IMPLEMENTATION
#include "yab.h"

#define BINARY_NAME "gbemu"
#define COMMON_FLAGS "-Wall", "-Wextra", "-pedantic", "-std=c11", "-Werror", "-ggdb"

int main(int argc, const char *argv[])
{
    YAB_REBUILD_SRC(argc, argv, "yab.h");

    size_t c_src_size = 0;
    yab_files_t sources = {0};
    yab_arr_append(&sources, "src/gbemu.c");
    yab_arr_append(&sources, "src/bus.c");
    yab_arr_append(&sources, "src/emu.c");
    yab_arr_append(&sources, "src/cart.c");
    yab_arr_append(&sources, "src/cpu.c");

    c_src_size = sources.size;
    yab_arr_append(&sources, "src/types.h");
    yab_arr_append(&sources, "src/utils.h");
    yab_arr_append(&sources, "src/bus.h");
    yab_arr_append(&sources, "src/emu.h");
    yab_arr_append(&sources, "src/cart.h");
    yab_arr_append(&sources, "src/cpu.h");

    if (yab_need_rebuild(BINARY_NAME, sources.data, sources.size) == 1)
    {
        yab_command_t cmd = {0};
        yab_command_append(&cmd, "cc", COMMON_FLAGS, "-o", BINARY_NAME);
        yab_arr_append_many(&cmd, sources.data, c_src_size);
        return (yab_exec_sync(cmd) == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    else
    {
        YAB_LOG("Nothing to do");
    }

    return 0;
}
