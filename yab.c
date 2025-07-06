#define YAB_IMPLEMENTATION
#include "yab.h"

#define STRCMP(A, B) strncmp((A), (B), sizeof(B))

#if defined ( _WIN32 )
#  define BINARY_NAME "gbemu.exe"
#else
#  define BINARY_NAME "gbemu"
#endif

#if defined( _WIN32 ) && defined ( _MSC_VER )
#  define COMMON_FLAGS "/std:c11", "/Wall", "/WX", "/D_CRT_SECURE_NO_WARNINGS"
#  define DEBUG_FLAGS "/Z7", "/Od"
#  define RELEASE_FLAGS "/DNDEBUG", "/O2", "/wd4710", "/wd4711"
#else
#  define COMMON_FLAGS "-Wall", "-Wextra", "-pedantic", "-std=c11", "-Werror"
#  if defined(__MINGW32__) && defined(__GNUC__)
#    define DEBUG_FLAGS "-ggdb", "-Og"
#  else
#    define DEBUG_FLAGS "-ggdb", "-Og", "-fsanitize=address", "-fsanitize=undefined"
#  endif
#  define RELEASE_FLAGS "-DNDEBUG", "-O3"
#endif


typedef struct
{
    size_t size;
    const char *data;
} string_t;

#define YAB_SIZE_STR(str) (YAB_SIZE(str) - sizeof(str[0]))
#define YAB_ENSURE_CLIT_STR(str) ("" str "")
#define cstr(str) ((string_t){.size = YAB_SIZE_STR(YAB_ENSURE_CLIT_STR(str)), .data = (str) })

string_t mkstring(const char *str)
{
    return (string_t){ .size = strlen(str), .data = str};
}

typedef enum ConfType
{
    CFG_DEBUG = 0,
    CFG_RELEASE,
    CFG_COUNT
} CType;

typedef struct BuildConfig
{
    CType type;
} BConfig;

typedef void (*ConfigFunc)(BConfig *, int *, const char ***);

typedef struct BuildOption
{
    const char *op[2];
    const char *desc;
    ConfigFunc func;
} BOption;


static void build_usage(BConfig *config, int *argc, const char ***argv);
static void build_config(BConfig *config, int *argc, const char ***argv);

static void usage(FILE *steam);

static const BOption OPTIONS[] = {
    { .op = { "-h", "--help" }, .desc = "Display this help", .func = build_usage },
    { .op = { "-c", "--config" }, .desc = "Set the build config, either Debug or Release", .func = build_config },
};

typedef struct string_frag
{
    string_t str;
    struct string_frag *prev, *next;
} string_frag;

typedef struct
{
    size_t size;
    string_frag *start;
    string_frag *current;
} path_builder_t;

void clear_path_builder(path_builder_t *pb)
{
    string_frag *iter = pb->start;
    while (iter)
    {
        string_frag *next = iter->next;
        free(iter);
        iter = next;
    }
    *pb = (path_builder_t){0};
}

void append_path_builder(path_builder_t *pb, string_frag f)
{
    string_frag *frag = malloc(sizeof(*frag));
    *frag = f;

    pb->size += frag->str.size + 1;
    if (pb->current)
    {
        pb->current->next = frag;
        pb->current = frag;
    }
    else
    {
        pb->start = frag;
        pb->current = frag;
    }
}

path_builder_t construct_path_builder(string_t path)
{
    path_builder_t pb = {0};

    size_t start_pos = 0;
    const char *start = path.data;
    for (size_t i = 0; i < path.size; ++i)
    {
        if (path.data[i] == '/')
        {
            string_frag frag = {.str = {.size = i - start_pos, .data = start}, .prev = pb.current};
            append_path_builder(&pb, frag);

            start_pos = i + 1;
            start = path.data + start_pos;
        }
    }
    if (start_pos != path.size)
    {
        string_frag frag = {.str = {.size = path.size - start_pos, .data = start}, .prev = pb.current};
        append_path_builder(&pb, frag);
    }

    return pb;
}

int create_dir_recursive_impl(string_t path)
{
    path_builder_t pb = construct_path_builder(path);

    string_frag *iter = pb.start;
    while (iter)
    {
        string_frag *remove = NULL;
        string_frag *next = iter->next;
        if (strncmp(iter->str.data, "..", 2) == 0)
        {
            assert(iter->prev != NULL);
            string_frag *prev = iter->prev->prev;
            assert(prev != NULL);

            pb.size -= (prev->next->str.size + 1);
            free(prev->next);

            prev->next = next;
            next->prev = prev;

            remove = iter;
            iter = next;
        }
        else if (strncmp(iter->str.data, ".", 1) == 0)
        {
            if (iter->prev)
                iter->prev->next = next;
            next->prev = iter->prev;

            remove = iter;
            iter = next;
        }

        iter = next;
        if (remove != NULL)
        {
            pb.size -= (remove->str.size + 1);
            free(remove);
        }
    }

    char *buff = malloc(pb.size + 1);
    size_t pos = 0;
    iter = pb.start;
    while (iter)
    {
        int result = snprintf(buff + pos, pb.size - pos, "%.*s/", (int)iter->str.size, iter->str.data);
        assert((size_t)result != iter->str.size);

        pos += iter->str.size + 1;
        yab_create_dir(buff);
        iter = iter->next;
    }
    clear_path_builder(&pb);
    free(buff);

    return 0;
}

int create_dir_recursive(const char *path)
{
    return create_dir_recursive_impl(mkstring(path));
}

#define create_dir_recursive_cstr(str) create_dir_recursive_impl(cstr(str))

int main(int argc, const char **argv)
{
    YAB_REBUILD_SRC(argc, argv, "yab.h");

    int result = EXIT_SUCCESS;
    BConfig config = { .type = CFG_DEBUG };

    while (argc > 0)
    {
        const char *arg = yab_shift(argc, argv);
        for (size_t i = 0; i < YAB_SIZE(OPTIONS); ++i)
        {
            BOption o = OPTIONS[i];
            if (strncmp(arg, o.op[0], 3) == 0 || strncmp(arg, o.op[1], strlen(o.op[1])) == 0)
            {
                o.func(&config, &argc, &argv);
            }
        }
    }

    size_t c_src_size = 0;
    yab_files_t *sources = {0};
    yab_arr_append(&sources, "src/gbemu.c");
    yab_arr_append(&sources, "src/bus.c");
    yab_arr_append(&sources, "src/emu.c");
    yab_arr_append(&sources, "src/cart.c");
    yab_arr_append(&sources, "src/cpu.c");
    c_src_size = sources->size;

    yab_arr_append(&sources, "src/types.h");
    yab_arr_append(&sources, "src/utils.h");
    yab_arr_append(&sources, "src/bus.h");
    yab_arr_append(&sources, "src/emu.h");
    yab_arr_append(&sources, "src/cart.h");
    yab_arr_append(&sources, "src/cpu.h");

    if (yab_need_rebuild(BINARY_NAME, sources->data, sources->size) == 1)
    {
        yab_command_t *cmd = {0};

        yab_command_append(&cmd, YAB_COMPILER, COMMON_FLAGS);

        switch (config.type)
        {
            case CFG_DEBUG:
                yab_command_append(&cmd, DEBUG_FLAGS);
                break;

            case CFG_RELEASE:
                yab_command_append(&cmd, RELEASE_FLAGS);
                break;

            case CFG_COUNT:
            default:
                break;
        }

        yab_arr_append_many(&cmd, sources->data, c_src_size);

#ifdef _MSC_VER
        yab_command_append(&cmd, "/link", "/out:"BINARY_NAME);
#else
        yab_command_append(&cmd, "-o", BINARY_NAME);
#endif

        result = (yab_exec_sync(cmd) == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
        yab_arr_free(&cmd);
    }
    else
    {
        YAB_LOG("Nothing to do");
    }

    yab_arr_free(&sources);
    return result;
}


void usage(FILE *stream)
{
    const unsigned int padding = 20;
    fprintf(stream, "Usage: %s [options]\n\n", program);
    fprintf(stream, "Options:\n");

    for (size_t i = 0; i < YAB_SIZE(OPTIONS); ++i)
    {
        BOption o = OPTIONS[i];
        const char *options = yab_sprintf("%s, %s", o.op[0], o.op[1]);
        fprintf(stream, "\t%-*s%s\n", padding, options, o.desc);
    }
}

void build_usage(BConfig *config, int *argc, const char ***argv)
{
    (void)config;
    (void)argc;
    (void)argv;

    usage(stdout);
    exit(EXIT_SUCCESS);
}

void build_config(BConfig *config, int *argc, const char ***argv)
{
    static int called = 0;

    const char *type;
    if (called != 0)
    {
        fprintf(stderr, "Trying to set multiple times the build config\n");
    }

    called++;

    type = yab_shift(*argc, *argv);
    if (type == NULL)
    {
        fprintf(stderr, "The type of config is missing\n");
        exit(EXIT_FAILURE);
    }

    if (STRCMP(type, "Debug") == 0)
    {
        config->type = CFG_DEBUG;
        YAB_LOG("Using Debug configuration");
    }
    else if (STRCMP(type, "Release") == 0)
    {
        config->type = CFG_RELEASE;
        YAB_LOG("Using Release configuration");
    }
    else
    {
        fprintf(stderr, "Unrecognized config type '%s'\n", type);
        exit(EXIT_FAILURE);
    }
}
