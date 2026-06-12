#ifndef CQ_H_
#define CQ_H_

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
#define FUNC _getch()
#define CONFIRM '\r'
#endif
    
#define THEME_COLOR "\x1b[38;2;255;157;0m"
#define Q_COLOR "\x1b[38;2;92;124;151m"
#define ERR_COLOR "\x1b[31m"
#define ARROW ">"

#define CQ_DEFAULT_CONTEXT {NULL, 0, 0,                  \
    THEME_COLOR,                              \
    Q_COLOR,                             \
    ERR_COLOR,                                          \
    ARROW}

#define CQ_CHECKED(result, i) ((result) & (1ULL << (i)))
#define CQ_ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#define FUNC own_getch()
#define CONFIRM '\n'
#endif

typedef struct {
    char **ptrs;
    size_t count;
    size_t capacity;
    const char *theme_color;
    const char *q_color;
    const char *err_color;
    const char *arrow;
} CQ_Context;

char *CQ_alloc(CQ_Context *ctx, size_t size);
void CQ_cleanup(CQ_Context *ctx);
char* CQ_text(CQ_Context *ctx, const char *prompt, size_t max_len);
char* CQ_password(CQ_Context *ctx, const char *prompt, size_t max_len);
char* CQ_select(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options);
bool CQ_confirm(CQ_Context *ctx, const char *prompt);
uint64_t CQ_checkbox(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options);

#ifdef CQ_IMPLEMENTATION

#ifndef _WIN32
int own_getch()
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
#endif

char *CQ_alloc(CQ_Context *ctx, size_t size)
{
    char *ptr = (char*)malloc(size);
    if(!ptr) return NULL;

    if(ctx->count == ctx->capacity)
    {
        size_t new_cap = ctx->capacity ? ctx->capacity * 2 : 8;
        char **new_ptrs = (char**)realloc(ctx->ptrs, new_cap * sizeof(char*));
        if(!new_ptrs)
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

void CQ_cleanup(CQ_Context *ctx)
{
    for(size_t i = 0; i < ctx->count; ++i)
        free(ctx->ptrs[i]);
    free(ctx->ptrs);
    ctx->ptrs = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
}

char* CQ_text(CQ_Context *ctx, const char *prompt, size_t max_len)
{
    char *buf = CQ_alloc(ctx, max_len);
    if(!buf) return NULL;
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    fgets(buf, max_len, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    printf("\x1b[0m");
    return buf;
}

char* CQ_password(CQ_Context *ctx, const char *prompt, size_t max_len)
{
    printf("\x1b[0m");

    char *buf = CQ_alloc(ctx, max_len);
    if(!buf) return NULL;
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    if(max_len == 0)
        return NULL;

#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if(hStdin == INVALID_HANDLE_VALUE) { return NULL; }
    DWORD original;
    if(!GetConsoleMode(hStdin, &original)) { return NULL; }
    DWORD hidden = original & ~ENABLE_ECHO_INPUT;
    if(!SetConsoleMode(hStdin, hidden)) { return NULL; }
    if(!fgets(buf, (int)max_len, stdin))
    {
        SetConsoleMode(hStdin, original);
        return NULL;
    }
    SetConsoleMode(hStdin, original);
#else
    struct termios oldt, newt;
    if(tcgetattr(STDIN_FILENO, &oldt) != 0) { return NULL; }
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    if(tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) { return NULL; }
    if(!fgets(buf, max_len, stdin))
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return NULL;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fputc('\n', stdout);
#endif
    buf[strcspn(buf, "\n")] = '\0';

    printf("\x1b[0m%s?\x1b[0m Confirm: \x1b[1m%s", ctx->q_color, ctx->theme_color);
    char *confirm = (char*)calloc(max_len, sizeof(char));
    if(!confirm) { return NULL; }

#ifdef _WIN32
    if(!GetConsoleMode(hStdin, &original))
        { free(confirm); return NULL; }
    if(!SetConsoleMode(hStdin, hidden))
        { free(confirm); return NULL; }
    if(!fgets(confirm, (int)max_len, stdin))
    {
        SetConsoleMode(hStdin, original);
        free(confirm); return NULL;
    }
    SetConsoleMode(hStdin, original);
#else
    if(tcgetattr(STDIN_FILENO, &oldt) != 0)
        { free(confirm); return NULL; }
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    if(tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        { free(confirm); return NULL; }
    if(!fgets(confirm, max_len, stdin))
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        free(confirm); return NULL;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fputc('\n', stdout);
#endif
    confirm[strcspn(confirm, "\n")] = '\0';

    if(strcmp(buf, confirm) != 0)
    {
        printf("\x1b[0m%sX\x1b[0m Passwords do not match.\n", ctx->err_color);
        free(confirm);
        return NULL;
    }

    free(confirm);

    printf("\x1b[1A\x1b[2K");
    printf("\x1b[1A\x1b[2K");
    printf("\x1b[0m%s?\x1b[0m %s \x1b[1m%s********", ctx->q_color, prompt, ctx->theme_color);
    printf("\x1b[0m\n");

    return buf;
}

char* CQ_select(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)
{
    assert(num_options <= 64);
    
    printf("\x1b[0m");

    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);

    printf("\x1b[?25l");

    int selected_i = 0;
    int key;
    char frame[4096];
    for(int i = 0; i < num_options; ++i)
        printf(" %s %s\n", (i == selected_i ? ctx->arrow : " "), options[i]);
    do
    {
        int n = 0;
        key = FUNC;
#ifdef _WIN32
        if(key == 0 || key == 224)
        {
            int key2 = FUNC;
            switch(key2)
            {
                case 72:
                    selected_i--;
                    break;
                case 80:
                    selected_i++;
                    break;
            }
        }
#else
        if(key == 27)
        {
            int key1 = FUNC;
            int key2 = FUNC;

            if(key1 == '[')
            {
                switch(key2)
                {
                    case 'A':
                        selected_i--;
                        break;
                    case 'B':
                        selected_i++;
                        break;
                }
            }
        }
#endif
        if(selected_i < 0) selected_i = num_options - 1;
        if(selected_i >= num_options) selected_i = 0;

        for(int i = 0; i < num_options; ++i)
            n += snprintf(frame + n, sizeof(frame) - n, "\x1b[1A\x1b[2K");
        for(int i = 0; i < num_options; ++i)
            n += snprintf(frame + n, sizeof(frame) - n,
                    " %s %s\n",
                    (i == selected_i ? ctx->arrow : " "),
                    options[i]);
        fwrite(frame, 1, n, stdout);
        fflush(stdout);
    } while(key != CONFIRM);

    char *selected = CQ_alloc(ctx, strlen(options[selected_i]) + 1);
    strcpy(selected, options[selected_i]);

    printf("\x1b[0m");

    for(int i = 0; i < num_options + 1; ++i)
        printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s%s\n", ctx->q_color, prompt, ctx->theme_color, selected);

    printf("\x1b[?25h");

    return selected;
}

bool CQ_confirm(CQ_Context *ctx, const char *prompt)
{
    printf("\x1b[0m");

    printf("%s?\x1b[0m %s (Y/n) \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);

    bool confirmed;
    int key = FUNC;
    if(toupper(key) == 'Y' || key == CONFIRM)
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

uint64_t CQ_checkbox(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)
{
    assert(num_options <= 64);
    
    uint64_t selected = 0;

    printf("\x1b[0m");
    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);
    printf("\x1b[?25l");

    int selected_i = 0;
    assert(num_options <= 64);
    bool *all_selected = (bool*)calloc(num_options, sizeof(bool));
    for(int i = 0; i < num_options; ++i)
        all_selected[i] = false;

    for(int i = 0; i < num_options; ++i)
        printf(" %s %s %s\n",
            (i == selected_i ? ctx->arrow : " "),
            (all_selected[i] ? "\x1b[32m[X]\x1b[0m" : "[ ]"),
            options[i]);

    int key;
    char frame[4096];
    do
    {
        int n = 0;
        key = FUNC;
#ifdef _WIN32
        if(key == 0 || key == 224)
        {
            int key2 = FUNC;
            switch(key2)
            {
                case 72: selected_i--; break;
                case 80: selected_i++; break;
            }
        }
#else
        if(key == 27)
        {
            int key1 = FUNC;
            int key2 = FUNC;
            if(key1 == '[')
            {
                switch(key2)
                {
                    case 'A': selected_i--; break;
                    case 'B': selected_i++; break;
                }
            }
        }
#endif
        if(key == ' ')
            all_selected[selected_i] = !all_selected[selected_i];

        if(selected_i < 0) selected_i = num_options - 1;
        if(selected_i >= num_options) selected_i = 0;

        for(int i = 0; i < num_options; ++i)
            n += snprintf(frame + n, sizeof(frame) - n, "\x1b[1A\x1b[2K");
        for(int i = 0; i < num_options; ++i)
            n += snprintf(frame + n, sizeof(frame) - n,
                " %s %s %s\n",
                (i == selected_i ? ctx->arrow : " "),
                (all_selected[i] ? "\x1b[32m[X]\x1b[0m" : "[ ]"),
                options[i]);

        fwrite(frame, 1, n, stdout);
        fflush(stdout);
    } while(key != CONFIRM);

    int feature_count = 0;

    for(int i = 0; i < num_options; ++i)
        if(all_selected[i])
            selected |= (1ULL << i), feature_count++;

    printf("\x1b[0m");

    for(int i = 0; i < num_options + 1; ++i)
        printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    for(int i = 0; i < num_options; ++i)
        if(selected & (1ULL << i))
            printf("%s ", options[i]);
    if(feature_count == 0)
        printf("None");

    printf("\x1b[?25h\n\x1b[0m");

    return selected;
}
#endif

#endif
