#ifndef _YAB_H_
#define _YAB_H_

#ifndef YAB_FREE
#define YAB_FREE free
#endif /* !YAB_FREE */

#ifndef YAB_MALLOC
#define YAB_MALLOC malloc
#endif /* !YAB_MALLOC */

#ifndef YAB_REALLOC
#define YAB_REALLOC realloc
#endif /* !YAB_REALLOC */

#ifndef YAB_ARR_INIT_CAP
#define YAB_ARR_INIT_CAP 128
#endif /* !YAB_ARR_INIT_CAP */

typedef enum
{
    YAB_INFO,
    YAB_WARNING,
    YAB_ERROR,
    YAB_NO_LOGS
} yab_log_level_t;

#ifndef YAB_MIN_LOG_LEVEL
#define YAB_MIN_LOG_LEVEL YAB_INFO
#endif /* !YAB_MIN_LOG_LEVEL */


#ifdef _WIN32

#  error "Not yet implemented"

#else

#  define _POSIX_C_SOURCE 200809L
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <unistd.h>

#endif

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef YAB_BUILD_COMMAND
#  ifdef _WIN32
#    error "Not yet defined"
#  else
#    define YAB_BUILD_COMMAND(binary_path, source_path) "cc", "-Wall", "-Wextra", "-Werror", "-std=c11", "-pedantic", "-o", binary_path, source_path
#  endif
#endif /* !YAB_BUILD_COMMAND */


#define YAB_STR(str) #str

#define yab_arr_append(arr, item)                                                               \
    do {                                                                                        \
        if ((arr)->size >= (arr)->capacity)                                                     \
        {                                                                                       \
            (arr)->capacity = ((arr)->capacity == 0 ? YAB_ARR_INIT_CAP : (arr)->capacity * 2);  \
            if ((arr)->data == NULL)                                                            \
                (arr)->data = YAB_MALLOC((arr)->capacity * sizeof(*(arr)->data));               \
            else                                                                                \
                (arr)->data = YAB_REALLOC((arr)->data, (arr)->capacity * sizeof(*(arr)->data)); \
            assert((arr)->data != NULL && "Failed to append to \"" YAB_STR(arr) "\"");          \
        }                                                                                       \
        (arr)->data[(arr)->size++] = item;                                                      \
    } while (0)

#define yab_arr_append_many(arr, items, nb)                                                     \
    do {                                                                                        \
        if ((arr)->size + (nb) > (arr)->capacity)                                               \
        {                                                                                       \
            if ((arr)->capacity == 0)                                                           \
            {                                                                                   \
                (arr)->capacity = YAB_ARR_INIT_CAP;                                             \
            }                                                                                   \
            while ((arr)->size + (nb) > (arr)->capacity)                                        \
            {                                                                                   \
                (arr)->capacity *= 2;                                                           \
            }                                                                                   \
            if ((arr)->data == NULL)                                                            \
                (arr)->data = YAB_MALLOC((arr)->capacity * sizeof(*(arr)->data));               \
            else                                                                                \
                (arr)->data = YAB_REALLOC((arr)->data, (arr)->capacity * sizeof(*(arr)->data)); \
            assert((arr)->data != NULL && "Failed to append many to \"" YAB_STR(arr) "\"");     \
        }                                                                                       \
        memcpy((arr)->data + (arr)->size, (items), (nb) * sizeof(*(arr)->data));                \
        (arr)->size += (nb);                                                                    \
    } while (0)

#define yab_arr_free(arr)                 \
    do {                                  \
        YAB_FREE((arr)->data);            \
        memset((arr), 0, sizeof(*(arr))); \
    } while (0)


void yab_log(yab_log_level_t level, const char *format, ...);

#define YAB_LOG(...) yab_log(YAB_INFO, __VA_ARGS__)
#define YAB_WARN(...) yab_log(YAB_WARNING, __VA_ARGS__)
#define YAB_ERR(...) yab_log(YAB_ERROR, __VA_ARGS__)

#if _WIN32

typedef HANDLE yab_proc_t;
#define YAB_INVALID_PROC INVALID_HANDLE_VALUE

typedef HANDLE yab_fd_t;
#define YAB_INVALID_FD INVALID_HANDLE_VALUE

#else

typedef pid_t yab_proc_t;
#define YAB_INVALID_PROC (-1)

typedef int yab_fd_t;
#define YAB_INVALID_FD (-1)

#endif

typedef struct
{
    yab_fd_t *data;
    size_t capacity;
    size_t size;
} yab_fds_t;

typedef struct
{
    const char **data;
    size_t capacity;
    size_t size;
} yab_files_t;


typedef struct
{
    yab_proc_t *data;
    size_t capacity;
    size_t size;
} yab_procs_t;

typedef struct
{
    const char **data;
    size_t capacity;
    size_t size;
} yab_command_t;

#define yab_command_append(cmd, ...)                     \
    yab_arr_append_many(cmd,                             \
                        ((const char *[]){__VA_ARGS__}), \
                        (sizeof((const char *[]){__VA_ARGS__})/sizeof(const char *)))


#define yab_shift(argc, argv) (assert(argc > 0), argc > 0 ? ((argc)--, *(argv)++) : NULL)

char *yab_sprintf(const char *format, ...);

int yab_exec_sync(yab_command_t cmd);

int yab_exec_async(yab_command_t cmd);
int yab_proc_wait(yab_proc_t proc);
int yab_procs_wait(yab_procs_t procs);

int yab_need_rebuild(const char *binary_name, const char **sources, size_t nb_sources);

void yab__try_rebuild(int argc, const char **argv, const char *source, ...);

#define YAB_REBUILD_SRC(argc, argv, ...) yab__try_rebuild((argc), (argv), __FILE__, __VA_ARGS__, NULL)
#define YAB_REBUILD(argc, argv) YAB_REBUILD_SRC(argc, argv, NULL)

#ifdef YAB_IMPLEMENTATION

#include <errno.h>

void yab_log(yab_log_level_t level, const char *format, ...)
{
    va_list args;

    if (level < YAB_MIN_LOG_LEVEL)
        return;

    switch (level) {
        case YAB_INFO:
            fprintf(stderr, "[INFO] ");
            break;

        case YAB_WARNING:
            fprintf(stderr, "[WARN] ");
            break;

        case YAB_ERROR:
            fprintf(stderr, "[ERROR] ");
            break;

        default:
            assert(0 && "UNREACHABLE");
            break;
    }

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

char *yab_sprintf(const char *format, ...)
{
    int n;
    size_t size;
    char *str;
    va_list args;

    va_start(args, format);
    n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (n < 0)
        return NULL;

    size = n + 1;
    str = YAB_MALLOC(size * sizeof(char));
    if (str == NULL)
    {
        YAB_ERR("Failed to alloc string: %s", strerror(errno));
        return NULL;
    }

    va_start(args, format);
    n = vsnprintf(str, size, format, args);
    va_end(args);

    if (n < 0)
    {
        YAB_FREE(str);
        YAB_ERR("Failed to print formatted string: %s", strerror(errno));
        return NULL;
    }

    return str;
}

int yab_exec_sync(yab_command_t cmd)
{
    yab_proc_t p = yab_exec_async(cmd);
    if (p == YAB_INVALID_PROC)
        return -1;
    return yab_proc_wait(p);
}

int yab_proc_wait(yab_proc_t proc)
{
#ifdef _WIN32
    assert(0 && "NOT IMPLEMENTED");
    return -1;
#else
    int wstatus, estatus;

    if (proc == YAB_INVALID_PROC)
    {
        YAB_ERR("Invalid process provided");
        return -1;
    }

    while (1)
    {
        if (waitpid(proc, &wstatus, 0) < 0)
        {
            YAB_ERR("Failed to wait on child (pid %d): %s", proc, strerror(errno));
            return -1;
        }

        if (WIFEXITED(wstatus))
        {
            estatus = WEXITSTATUS(wstatus);
            if (estatus != 0)
            {
                YAB_ERR("Command exited with code %d", estatus);
                return -1;
            }
            break;
        }

        if (WIFSIGNALED(wstatus))
        {
            YAB_ERR("Command was terminated by signal %s", strsignal(WTERMSIG(wstatus)));
            return -1;
        }
    }

    return 0;
#endif
}

int yab_procs_wait(yab_procs_t procs)
{
    int success = 0;

    if (procs.size == 0)
    {
        YAB_ERR("Invalid number of processes provided");
        return -1;
    }

    for (size_t i = 0; i < procs.size; ++i)
        success = ( success == 0 ? yab_proc_wait(procs.data[i]) : -1);

    return success;
}

yab_proc_t yab_exec_async(yab_command_t cmd)
{
#ifdef _WIN32
    assert(0 && "NOT IMPLEMENTED");
    return YAB_INVALID_PROC;
#else
    yab_proc_t proc;
    char *str = NULL;

    assert(cmd.data != NULL && cmd.size > 0 && "Invalid command");
    if (cmd.size == 0)
    {
        YAB_ERR("Invalid command provided");
        return YAB_INVALID_PROC;
    }

    for (size_t i = 0; i < cmd.size; ++i)
    {
        if (str == NULL)
        {
            str = yab_sprintf("%s", cmd.data[i]);
        }
        else
        {
            char *tmp = yab_sprintf("%s %s", str, cmd.data[i]);
            YAB_FREE(str);
            str = tmp;
        }
    }
    YAB_LOG("Command: %s", str);
    YAB_FREE(str);

    proc = fork();
    if (proc == YAB_INVALID_PROC)
    {
        YAB_ERR("Failed to create subprocess: %s", strerror(errno));
        return YAB_INVALID_PROC;
    }

    if (proc == 0)
    {
        yab_command_t cmd_null = {0};
        yab_arr_append_many(&cmd_null, cmd.data, cmd.size);
        yab_arr_append(&cmd_null, NULL);
        if (execvp(cmd_null.data[0], (char * const*)cmd_null.data) == -1)
        {
            YAB_ERR("Failed to exec command: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        assert(0 && "UNREACHABLE");
    }

    return proc;
#endif
}

int yab_need_rebuild(const char *binary_name, const char **sources, size_t nb_sources)
{
#ifdef _WIN32
    assert(0 && "NOT IMPLEMENTED");
    return -1;
#else
    time_t binary_date = 0, source_date = 0;
    struct stat statbuf = {0};

    if (binary_name == NULL)
    {
        YAB_ERR("The executable path is NULL\n");
        return -1;
    }

    if (stat(binary_name, &statbuf) < 0)
    {
        if (errno == ENOENT)
        {
            YAB_LOG("Binary file is not found");
            return 1;
        }
        else
        {
            YAB_ERR("Can't get \"%s\" file stats: %s", binary_name, strerror(errno));
            return -1;
        }
    }
    binary_date = statbuf.st_mtime;

    for (size_t i = 0; i < nb_sources; ++i)
    {
        if (stat(sources[i], &statbuf) < 0)
        {
            YAB_ERR("Can't get \"%s\" file stats: %s", sources[i], strerror(errno));
            return -1;
        }
        if (source_date < statbuf.st_mtime)
            source_date = statbuf.st_mtime;
    }

    return (binary_date < source_date ? 1 : 0);
#endif
}

void yab__try_rebuild(int argc, const char **argv, const char *source, ...)
{
    va_list args;
    int rebuild;
    yab_files_t files = {0};
    yab_command_t command = {0};
    char *backup_name;
    const char *bin_name = yab_shift(argc, argv);

    yab_arr_append(&files, source);

    va_start(args, source);
    for (;;)
    {
        const char *file = va_arg(args, const char *);
        if (file == NULL)
            break;
        yab_arr_append(&files, file);
    }
    va_end(args);

    rebuild = yab_need_rebuild(bin_name, files.data, files.size);
    if (rebuild < 0)
        exit(EXIT_FAILURE);

    if (rebuild == 0)
    {
        yab_arr_free(&files);
        return;
    }

    backup_name = yab_sprintf("%s.old", bin_name);
    if (backup_name == NULL)
        exit(EXIT_FAILURE);

    if (rename(bin_name, backup_name) < 0)
    {
        YAB_ERR("Failed to backup old binary: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    yab_command_append(&command, YAB_BUILD_COMMAND(bin_name, source));
    if (yab_exec_sync(command) < 0)
    {
        rename(backup_name, bin_name);
        exit(EXIT_FAILURE);
    }

    command.size = 0;
    yab_command_append(&command, bin_name);
    if (yab_exec_sync(command) < 0)
    {
        rename(backup_name, bin_name);
        exit(EXIT_FAILURE);
    }

#ifndef _WIN32
    remove(backup_name);
#endif /* !_WIN32 */

    exit(EXIT_SUCCESS);
}

#endif /* YAB_IMPLEMENTATION */

#endif /* !_YAB_H_ */
