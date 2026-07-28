#ifndef CQ_H_
#define CQ_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define THEME_COLOR "\x1b[38;2;255;157;0m"
#define Q_COLOR "\x1b[38;2;92;124;151m"
#define ERR_COLOR "\x1b[31m"
#define ARROW ">"

#define CQ_DEFAULT_CONTEXT { NULL, 0, 0, THEME_COLOR, Q_COLOR, ERR_COLOR, ARROW }

#define CQ_CHECKED(result, i) ((result) & (1ULL << (i)))
#define CQ_ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct
{
    char** ptrs;
    size_t count;
    size_t capacity;
    const char* theme_color;
    const char* q_color;
    const char* err_color;
    const char* arrow;
} CQ_Context;

char* CQ_alloc(CQ_Context* ctx, size_t size);
void CQ_cleanup(CQ_Context* ctx);
char* CQ_text(CQ_Context* ctx, const char* prompt, size_t max_len);
char* CQ_password(CQ_Context* ctx, const char* prompt, size_t max_len);
char* CQ_select(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options);
bool CQ_confirm(CQ_Context* ctx, const char* prompt);
uint64_t CQ_checkbox(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options);
int CQ_getch();

#ifdef CQ_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define CONFIRM '\r'
#else
#include <unistd.h>
#include <termios.h>
#define CONFIRM '\n'
#endif

#ifndef _WIN32
int CQ_getch()
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int c = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}
#else
int CQ_getch()
{
    return _getch();
}
#endif

char* CQ_alloc(CQ_Context* ctx, size_t size)
{
    char* ptr = (char*)malloc(size);
    if (!ptr)
        return NULL;

    if (ctx->count == ctx->capacity)
    {
        size_t new_cap = ctx->capacity ? ctx->capacity * 2 : 8;
        char** new_ptrs = (char**)realloc(ctx->ptrs, new_cap * sizeof(char*));
        if (!new_ptrs)
        {
            free(ptr);
            return NULL;
        }
        ctx->ptrs = new_ptrs;
        ctx->capacity = new_cap;
    }

    ctx->ptrs[ctx->count++] = ptr;
    return ptr;
}

void CQ_free_last(CQ_Context* ctx)
{
    if (ctx->count == 0)
        return;

    free(ctx->ptrs[--ctx->count]);
}

static void CQ_secure_zero(void* ptr, size_t len)
{
    if (!ptr || len == 0)
        return;
#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#elif defined(__STDC_LIB_EXT1__)
    memset_s(ptr, len, 0, len);
#else
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--)
        *p++ = 0;
#endif
}

bool CQ_free_secure(CQ_Context* ctx, char* buf)
{
    if (!buf)
        return false;

    for (size_t i = 0; i < ctx->count; ++i)
    {
        if (ctx->ptrs[i] == buf)
        {
            CQ_secure_zero(buf, strlen(buf) + 1);
#ifdef _WIN32
            VirtualUnlock(buf, strlen(buf) + 1);
#else
            munlock(buf, strlen(buf) + 1);
#endif
            free(buf);

            ctx->ptrs[i] = ctx->ptrs[ctx->count - 1];
            ctx->count--;
            return true;
        }
    }

    return false;
}

void CQ_cleanup(CQ_Context* ctx)
{
    for (size_t i = 0; i < ctx->count; ++i)
        free(ctx->ptrs[i]);
    free(ctx->ptrs);
    ctx->ptrs = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
}

char* CQ_text(CQ_Context* ctx, const char* prompt, size_t max_len)
{
    if (max_len == 0)
        return NULL;
    char* buf = CQ_alloc(ctx, max_len);
    if (!buf)
        return NULL;
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    if (!fgets(buf, max_len, stdin))
        return NULL;
    buf[strcspn(buf, "\n")] = '\0';
    printf("\x1b[0m");
    return buf;
}

char* CQ_password(CQ_Context* ctx, const char* prompt, size_t max_len)
{
    if (max_len == 0)
        return NULL;

    printf("\x1b[0m");

    char* buf = CQ_alloc(ctx, max_len);
    if (!buf)
        return NULL;
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);

#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        return NULL;
    DWORD original;
    if (!GetConsoleMode(hStdin, &original))
        return NULL;
    DWORD hidden = original & ~ENABLE_ECHO_INPUT;
    if (!SetConsoleMode(hStdin, hidden))
        return NULL;
    if (!fgets(buf, (int)max_len, stdin))
    {
        SetConsoleMode(hStdin, original);
        return NULL;
    }
    SetConsoleMode(hStdin, original);
#else
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return NULL;
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        return NULL;
    if (!fgets(buf, max_len, stdin))
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return NULL;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fputc('\n', stdout);
#endif
    buf[strcspn(buf, "\n")] = '\0';

    printf("\x1b[0m%s?\x1b[0m Confirm: \x1b[1m%s", ctx->q_color, ctx->theme_color);
    char* confirm = CQ_alloc(ctx, sizeof(char) * max_len);
    if (!confirm)
        return NULL;

#ifdef _WIN32
    if (!GetConsoleMode(hStdin, &original))
        return NULL;
    if (!SetConsoleMode(hStdin, hidden))
        return NULL;
    if (!fgets(confirm, (int)max_len, stdin))
    {
        SetConsoleMode(hStdin, original);
        return NULL;
    }
    SetConsoleMode(hStdin, original);
#else
    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return NULL;
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        return NULL;
    if (!fgets(confirm, max_len, stdin))
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return NULL;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fputc('\n', stdout);
#endif
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strcmp(buf, confirm) != 0)
    {
        printf("\x1b[0m%sX\x1b[0m Passwords do not match.\n", ctx->err_color);
        return NULL;
    }

    CQ_free_last(ctx);

    printf("\x1b[1A\x1b[2K");
    printf("\x1b[1A\x1b[2K");
    printf("\x1b[0m%s?\x1b[0m %s \x1b[1m%s********", ctx->q_color, prompt, ctx->theme_color);
    printf("\x1b[0m\n");

    return buf;
}

char* CQ_select(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options)
{
    printf("\x1b[0m");

    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);

    printf("\x1b[?25l");

    size_t selected_i = 0;
    int key;
    char frame[4096];
    for (size_t i = 0; i < num_options; ++i)
        printf(" %s %s\n", (i == selected_i ? ctx->arrow : " "), options[i]);
    do
    {
        size_t n = 0;
        key = CQ_getch();
#ifdef _WIN32
        if (key == 0 || key == 224)
        {
            int key2 = CQ_getch();
            switch (key2)
            {
            case 72:
                selected_i = (selected_i + num_options - 1) % num_options;
                break;
            case 80:
                selected_i = (selected_i + 1) % num_options;
                break;
            }
        }
#else
        if (key == 27)
        {
            int key1 = CQ_getch();
            int key2 = CQ_getch();

            if (key1 == '[')
            {
                switch (key2)
                {
                case 'A':
                    selected_i = (selected_i + num_options - 1) % num_options;
                    break;
                case 'B':
                    selected_i = (selected_i + 1) % num_options;
                    break;
                }
            }
        }
#endif
        for (size_t i = 0; i < num_options; ++i)
        {
            if (n < sizeof(frame))
                n += snprintf(frame + n, sizeof(frame) - n, "\x1b[1A\x1b[2K");
        }
        for (size_t i = 0; i < num_options; ++i)
        {
            if (n < sizeof(frame))
                n += snprintf(frame + n, sizeof(frame) - n, " %s %s\n", (i == selected_i ? ctx->arrow : " "), options[i]);
        }
        fwrite(frame, 1, n, stdout);
        fflush(stdout);
    } while (key != CONFIRM);

    char* selected = CQ_alloc(ctx, strlen(options[selected_i]) + 1);
    if (!selected)
        return NULL;
    memcpy(selected, options[selected_i], strlen(options[selected_i]) + 1);

    printf("\x1b[0m");

    for (int i = 0; i < num_options + 1; ++i)
        printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s%s\n", ctx->q_color, prompt, ctx->theme_color, selected);

    printf("\x1b[?25h");

    return selected;
}

bool CQ_confirm(CQ_Context* ctx, const char* prompt)
{
    printf("\x1b[0m");

    printf("%s?\x1b[0m %s (Y/n) \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);

    bool confirmed;
    int key = CQ_getch();
    if (toupper(key) == 'Y' || key == CONFIRM)
    {
        confirmed = true;
        printf("Yes\x1b[0m\n");
    }
    else
    {
        confirmed = false;
        printf("No\x1b[0m\n");
    }

    return confirmed;
}

uint64_t CQ_checkbox(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options)
{
    uint64_t selected = 0;

    assert(num_options <= 64);

    if (num_options > 64)
        return UINT64_MAX;

    printf("\x1b[0m");
    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);
    printf("\x1b[?25l");

    int selected_i = 0;
    bool* all_selected = (bool*)CQ_alloc(ctx, num_options * sizeof(bool));
    if (!all_selected)
        return UINT64_MAX;
    for (int i = 0; i < num_options; ++i)
        all_selected[i] = false;

    for (int i = 0; i < num_options; ++i)
        printf(" %s %s %s\n",
               (i == selected_i ? ctx->arrow : " "),
               (all_selected[i] ? "\x1b[32m[X]\x1b[0m" : "[ ]"),
               options[i]);

    int key;
    char frame[4096];
    do
    {
        int n = 0;
        key = CQ_getch();
#ifdef _WIN32
        if (key == 0 || key == 224)
        {
            int key2 = CQ_getch();
            switch (key2)
            {
            case 72:
                selected_i = (selected_i + num_options - 1) % num_options;
                break;
            case 80:
                selected_i = (selected_i + 1) % num_options;
                break;
            }
        }
#else
        if (key == 27)
        {
            int key1 = CQ_getch();
            int key2 = CQ_getch();
            if (key1 == '[')
            {
                switch (key2)
                {
                case 'A':
                    selected_i = (selected_i + num_options - 1) % num_options;
                    break;
                case 'B':
                    selected_i = (selected_i + 1) % num_options;
                    break;
                }
            }
        }
#endif
        if (key == ' ')
            all_selected[selected_i] = !all_selected[selected_i];

        for (int i = 0; i < num_options; ++i)
        {
            if (n < sizeof(frame))
                n += snprintf(frame + n, sizeof(frame) - n, "\x1b[1A\x1b[2K");
        }
        for (int i = 0; i < num_options; ++i)
        {
            if (n < sizeof(frame))
                n += snprintf(frame + n, sizeof(frame) - n, " %s %s %s\n", (i == selected_i ? ctx->arrow : " "), (all_selected[i] ? "\x1b[32m[X]\x1b[0m" : "[ ]"), options[i]);
        }

        fwrite(frame, 1, n, stdout);
        fflush(stdout);
    } while (key != CONFIRM);

    int feature_count = 0;

    for (int i = 0; i < num_options; ++i)
        if (all_selected[i])
            selected |= (1ULL << i), feature_count++;

    printf("\x1b[0m");

    for (int i = 0; i < num_options + 1; ++i)
        printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    for (int i = 0; i < num_options; ++i)
        if (selected & (1ULL << i))
            printf("%s ", options[i]);
    if (feature_count == 0)
        printf("None");

    printf("\x1b[?25h\n\x1b[0m");

    return selected;
}
#endif

#endif
