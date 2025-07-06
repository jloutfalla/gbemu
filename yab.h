#ifndef _YAB_H_
#define _YAB_H_

#ifdef _WIN32

#  define _CRT_SECURE_NO_WARNINGS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  ifdef _MSC_VER
#    pragma warning(disable : 4200)
#    pragma warning(disable : 5045)
#  endif

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

#ifndef YAB_COMPILER
#  if defined ( _WIN32 )
#    if defined ( _MSC_VER )
#      define YAB_COMPILER "cl"
#    elif defined ( __clang__ )
#      define YAB_COMPILER "clang"
#    elif defined ( __MINGW32__ )
#      define YAB_COMPILER "x86_64-w64-mingw32-gcc"
#    elif defined ( __GNUC__ )
#      define YAB_COMPILER "gcc"
#    endif
#  else
#    define YAB_COMPILER "cc"
#  endif
#endif /* YAB_COMPILER */

#ifndef YAB_FREE
#  define YAB_FREE free
#endif /* !YAB_FREE */

#ifndef YAB_MALLOC
#  define YAB_MALLOC malloc
#endif /* !YAB_MALLOC */

#ifndef YAB_REALLOC
#  define YAB_REALLOC realloc
#endif /* !YAB_REALLOC */

#ifndef YAB_ARR_INIT_CAP
#  define YAB_ARR_INIT_CAP 128
#endif /* !YAB_ARR_INIT_CAP */

#ifndef YAB_MIN_LOG_LEVEL
#  define YAB_MIN_LOG_LEVEL YAB_INFO
#endif /* !YAB_MIN_LOG_LEVEL */

#define yab_shift(argc, argv) ((argc) > 0 ? ((argc)--, *(argv)++) : NULL)

#define YAB_REBUILD_SRC(argc, argv, ...) yab__try_rebuild((argc), (argv), __FILE__, __VA_ARGS__, NULL)
#define YAB_REBUILD(argc, argv) YAB_REBUILD_SRC(argc, argv, NULL)

#ifndef YAB_BUILD_COMMAND
#  if defined( _WIN32 ) && defined ( _MSC_VER )
#    define YAB_BUILD_COMMAND(binary_path, source_path) YAB_COMPILER, "/Z7", "/std:c11", "/Wall", "/WX", "/Fe:", binary_path, source_path
#  else
#    define YAB_BUILD_COMMAND(binary_path, source_path) YAB_COMPILER, "-ggdb", "-std=c99", "-Wall", "-Wextra", "-Werror", "-pedantic", "-o", binary_path, source_path
#  endif
#endif /* !YAB_BUILD_COMMAND */


#define YAB_LOG(...) yab_log(YAB_INFO, __VA_ARGS__)
#define YAB_WARN(...) yab_log(YAB_WARNING, __VA_ARGS__)
#define YAB_ERR(...) yab_log(YAB_ERROR, __VA_ARGS__)

#define YAB_STR(str) #str

#define YAB_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))


#define yab_arr_alloc(arr, nb)                                                                \
    do                                                                                        \
    {                                                                                         \
        if (*(arr) == NULL || (*(arr))->capacity < nb)                                        \
        {                                                                                     \
            *(arr) = YAB_REALLOC(*(arr), sizeof(**(arr)) + sizeof((*(arr))->data[0]) * (nb)); \
            (*(arr))->capacity = (nb);                                                        \
        }                                                                                     \
    } while (0)

#define yab_arr_append(arr, item)                       \
    do                                                  \
    {                                                   \
        if (*(arr) == NULL)                             \
        {                                               \
            yab_arr_alloc(arr, YAB_ARR_INIT_CAP);       \
            (*(arr))->size = 0;                         \
        }                                               \
        if ((*(arr))->size >= (*(arr))->capacity)       \
        {                                               \
            yab_arr_alloc(arr, (*(arr))->capacity * 2); \
        }                                               \
        (*(arr))->data[(*(arr))->size] = item;          \
        (*(arr))->size++;                               \
    } while(0)

#define yab_arr_append_many(arr, items, nb)                                                    \
    do                                                                                         \
    {                                                                                          \
        if (*(arr) == NULL)                                                                    \
        {                                                                                      \
            yab_arr_alloc((arr), YAB_ARR_INIT_CAP);                                            \
            (*(arr))->size = 0;                                                                \
        }                                                                                      \
        if ((*(arr))->size + (nb) > (*(arr))->capacity)                                        \
        {                                                                                      \
            yab_arr_alloc((arr), YAB_ARR_INIT_CAP);                                            \
            while ((*(arr))->size + (nb) > (*(arr))->capacity)                                 \
            {                                                                                  \
                (*(arr))->capacity *= 2;                                                       \
            }                                                                                  \
            yab_arr_alloc((arr), (*(arr))->capacity);                                          \
            assert((*(arr))->data != NULL && "Failed to append many to \"" YAB_STR(arr) "\""); \
        }                                                                                      \
        memcpy((*(arr))->data + (*(arr))->size, (items), (nb) * sizeof((*(arr))->data[0]));    \
        (*(arr))->size += (nb);                                                                \
    } while (0)

#define yab_arr_free(arr) \
    do {                  \
        YAB_FREE(*(arr)); \
        *arr = NULL;      \
    } while (0)


#define yab_command_append(cmd, ...)                     \
    yab_arr_append_many((cmd),                           \
                        ((const char *[]){__VA_ARGS__}), \
                        (sizeof((const char *[]){__VA_ARGS__})/sizeof(const char *)))



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const char *program;

typedef enum
{
    YAB_INFO,
    YAB_WARNING,
    YAB_ERROR,
    YAB_NO_LOGS
} yab_log_level_t;


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
    size_t capacity;
    size_t size;
    yab_fd_t data[];
} yab_fds_t;

typedef struct
{
    size_t capacity;
    size_t size;
    const char *data[];
} yab_files_t;


typedef struct
{
    size_t capacity;
    size_t size;
    yab_proc_t data[];
} yab_procs_t;

typedef struct
{
    size_t capacity;
    size_t size;
    const char *data[];
} yab_command_t;


void yab_log(yab_log_level_t level, const char *format, ...);

char *yab_sprintf(const char *format, ...);

int yab_check_endianness(void); /* 1 if big endian, 0 otherwise */

int yab_exec_sync(yab_command_t *cmd);
yab_proc_t yab_exec_async(yab_command_t *cmd);

int yab_proc_wait(yab_proc_t proc);
int yab_procs_wait(yab_procs_t *procs);

int yab_dir_exist(const char *dir);
int yab_file_exist(const char *file);

int yab_create_dir(const char *dir);
int yab_create_dir_recursive(const char *dir);

int yab_need_rebuild(const char *binary_name, const char **sources, size_t nb_sources);

void yab__try_rebuild(int argc, const char **argv, const char *source, ...);


#ifdef YAB_IMPLEMENTATION

#include <errno.h>
#include <stdint.h>

#ifdef _WIN32

#else
#  include <dirent.h>
#endif

const char *program = NULL;

#ifdef _WIN32
char *yab_get_last_error(int error_code)
{
    char *err = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&err, 0, NULL);
    return err;
}

void yab_free_last_error(char *err)
{
    LocalFree(err);
}
#endif

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

        case YAB_NO_LOGS:
            return;

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

int yab_check_endianness(void)
{
    uint32_t val = 0x01234567;
    return ((*(uint8_t*)&val) == 0x01); /* 1 if big endian, 0 otherwise */
}

int yab_exec_sync(yab_command_t *cmd)
{
    if (cmd == NULL)
        return -1;
    yab_proc_t p = yab_exec_async(cmd);
    if (p == YAB_INVALID_PROC)
        return -1;
    return yab_proc_wait(p);
}

int yab_proc_wait(yab_proc_t proc)
{
#ifdef _WIN32
    DWORD result = WaitForSingleObject(proc, INFINITE);
    if (result == WAIT_FAILED)
    {
        char *error = yab_get_last_error(GetLastError());
        YAB_ERR("Failed to wait on child: %s", error);
        yab_free_last_error(error);
        return -1;
    }

    DWORD exit_code;
    if (GetExitCodeProcess(proc, (LPDWORD)&exit_code) == 0)
    {
        char *error = yab_get_last_error(GetLastError());
        YAB_ERR("Failed to get child exit code: %s", error);
        yab_free_last_error(error);
        return -1;
    }

    CloseHandle(proc);

    return exit_code;
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

int yab_procs_wait(yab_procs_t *procs)
{
    int success = 0;

    if (procs == NULL || procs->size == 0)
    {
        YAB_ERR("Invalid number of processes provided");
        return -1;
    }

    for (size_t i = 0; i < procs->size; ++i)
        success = ( success == 0 ? yab_proc_wait(procs->data[i]) : -1);

    return success;
}

int yab_dir_exist(const char *dir)
{
#ifdef _WIN32
    DWORD attribs = GetFileAttributes(dir);
    if (attribs == INVALID_FILE_ATTRIBUTES)
        return 0;
    return (attribs & FILE_ATTRIBUTE_DIRECTORY);
#else
    assert(strlen(dir) > 0);
    DIR *d = opendir(dir);
    if (d == NULL)
    {
        if (errno == ENOENT)
        {
            YAB_LOG("The directory \"%s\" does not exist", dir);
            return 0;
        }

        YAB_ERR("Can't get \"%s\" directory informations: %s", dir, strerror(errno));
        return -1;
    }
    closedir(d);
    return 1;
#endif
}

int yab_file_exist(const char *file)
{
#ifdef _WIN32
    DWORD attribs = GetFileAttributes(file);
    if (attribs == INVALID_FILE_ATTRIBUTES)
        return 0;
    return !(attribs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat statbuf = {0};

    if (stat(file, &statbuf) < 0)
    {
        if (errno == ENOENT)
        {
            YAB_LOG("File \"%s\" does not exist", file);
            return 0;
        }

        YAB_ERR("Can't get \"%s\" file stats: %s", file, strerror(errno));
        return -1;
    }

    return 1;
#endif
}

int yab_create_dir(const char *dir)
{
#ifdef _WIN32
    if (CreateDirectory(dir, NULL) == 0)
    {
        char *error = yab_get_last_error(GetLastError());
        YAB_ERR("Failed to create directory \"%s\": %s", dir, error);
        yab_free_last_error(error);
        return -1;
    }
    return 0;
#else
    if (mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
    {
        if (errno != EEXIST)
        {
            YAB_ERR("Failed to create directory \"%s\": %s", dir, strerror(errno));
            return -1;
        }
    }
    return 0;
#endif
}

int yab_create_dir_recursive(const char *dir)
{
#ifdef _WIN32
    (void)dir;
    assert(0 && "NOT IMPLEMENTED");
    return -1;
#else
    (void)dir;
    assert(0 && "NOT IMPLEMENTED");
    return -1;
#endif
}

yab_proc_t yab_exec_async(yab_command_t *cmd)
{
    char *str = NULL;

    assert(cmd != NULL && cmd->size > 0 && "Invalid command");
    if (cmd->size == 0)
    {
        YAB_ERR("Invalid command provided");
        return YAB_INVALID_PROC;
    }

    for (size_t i = 0; i < cmd->size; ++i)
    {
        if (str == NULL)
        {
            str = yab_sprintf("%s", cmd->data[i]);
        }
        else
        {
            char *tmp = yab_sprintf("%s %s", str, cmd->data[i]);
            YAB_FREE(str);
            str = tmp;
        }
    }
    YAB_LOG("Executing: %s", str);

#ifndef _WIN32
    YAB_FREE(str);
#endif

#ifdef _WIN32
    STARTUPINFO si = { .cb = sizeof(si) };
    PROCESS_INFORMATION proc_infos;

    BOOL success = CreateProcess(NULL, str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &proc_infos);
    YAB_FREE(str);

    if (!success)
    {
        char *error = yab_get_last_error(GetLastError());
        YAB_ERR("Failed to create subprocess: %s", error);
        yab_free_last_error(error);
        return YAB_INVALID_PROC;
    }

    CloseHandle(proc_infos.hThread);

    return proc_infos.hProcess;
#else
    yab_proc_t proc = fork();
    if (proc == YAB_INVALID_PROC)
    {
        YAB_ERR("Failed to create subprocess: %s", strerror(errno));
        return YAB_INVALID_PROC;
    }

    if (proc == 0)
    {
        yab_command_t *cmd_null = {0};
        yab_arr_append_many(&cmd_null, cmd->data, cmd->size);
        yab_arr_append(&cmd_null, NULL);
        if (execvp(cmd_null->data[0], (char * const*)cmd_null->data) == -1)
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
    DWORD error_code;
    HANDLE hFile;
    FILETIME binary_date = {0}, source_date = {0};

    hFile = CreateFile(binary_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE && (error_code = GetLastError()) != ERROR_FILE_NOT_FOUND)
    {
        char *error = yab_get_last_error(error_code);
        YAB_ERR("Can't open \"%s\" file: %s", binary_name, error);
        yab_free_last_error(error);
        return -1;
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        GetFileTime(hFile, &binary_date, NULL, NULL);
        CloseHandle(hFile);
    }

    for (size_t i = 0; i < nb_sources; ++i)
    {
        FILETIME temp_date;
        hFile = CreateFile(sources[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            char *error = yab_get_last_error(GetLastError());
            YAB_ERR("Can't open \"%s\" file: %s", binary_name, error);
            yab_free_last_error(error);
            return -1;
        }
        GetFileTime(hFile, NULL, NULL, &temp_date);
        CloseHandle(hFile);

        if (CompareFileTime(&source_date, &temp_date) < 0)
            source_date = temp_date;
    }

    if (CompareFileTime(&binary_date, &source_date) < 0)
        return 1;
    return 0;
#else
    time_t binary_date = 0, source_date = 0;
    struct stat statbuf = {0};

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
    yab_files_t *files = {0};
    yab_command_t *command = {0};
    char *backup_name;

    program = yab_shift(argc, argv);

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

    rebuild = yab_need_rebuild(program, files->data, files->size);
    if (rebuild < 0)
        exit(EXIT_FAILURE);

    if (rebuild == 0)
    {
        yab_arr_free(&files);
        return;
    }

    backup_name = yab_sprintf("%s.old", program);
    if (backup_name == NULL)
        exit(EXIT_FAILURE);

    if (yab_file_exist(backup_name) < 0 && rename(program, backup_name) < 0)
    {
        YAB_ERR("Failed to backup old binary: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    yab_command_append(&command, YAB_BUILD_COMMAND(program, source));
    if (yab_exec_sync(command) != 0)
    {
        rename(backup_name, program);
        exit(EXIT_FAILURE);
    }

    command->size = 0;
    yab_command_append(&command, program);
    yab_arr_append_many(&command, argv, argc);
    if (yab_exec_sync(command) != 0)
    {
        rename(backup_name, program);
        exit(EXIT_FAILURE);
    }

#ifndef _WIN32
    remove(backup_name);
#endif /* !_WIN32 */

    exit(EXIT_SUCCESS);
}

#endif /* YAB_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_YAB_H_ */
