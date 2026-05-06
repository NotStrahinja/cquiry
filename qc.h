#ifndef QC_H_
#define QC_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define FUNC _getch()
#define CONFIRM '\r'
#endif
    
#define THEME_COLOR "\x1b[38;2;255;157;0m"
#define Q_COLOR "\x1b[38;2;92;124;151m"
#define ERR_COLOR "\x1b[31m"
#define ARROW "\b>"

#define QC_DEFAULT_CONTEXT {NULL, 0, 0,                  \
    THEME_COLOR,                              \
    Q_COLOR,                             \
    ERR_COLOR,                                          \
    ARROW}

#define QC_CHECKED(result, i) ((result) & (1ULL << (i)))
#define QC_ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

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
} QC_Context;

char *QC_alloc(QC_Context *ctx, size_t size);
void QC_free_all(QC_Context *ctx);
char* QC_text(QC_Context *ctx, const char *prompt, size_t max_len);
char* QC_password(QC_Context *ctx, const char *prompt, size_t max_len);
char* QC_select(QC_Context *ctx, const char *prompt, const char **options, size_t num_options);
bool QC_confirm(QC_Context *ctx, const char *prompt);
uint64_t QC_checkbox(QC_Context *ctx, const char *prompt, const char **options, size_t num_options);

#ifdef QC_IMPLEMENTATION
char *QC_alloc(QC_Context *ctx, size_t size)
{
    char *ptr = malloc(size);
    if(!ptr) return NULL;

    if(ctx->count == ctx->capacity)
    {
        size_t new_cap = ctx->capacity ? ctx->capacity * 2 : 8;
        char **new_ptrs = realloc(ctx->ptrs, new_cap * sizeof(char*));
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

void QC_free_all(QC_Context *ctx)
{
    for(size_t i = 0; i < ctx->count; ++i)
        free(ctx->ptrs[i]);
    free(ctx->ptrs);
    ctx->ptrs = NULL;
    ctx->count = 0;
    ctx->capacity = 0;
}

char* QC_text(QC_Context *ctx, const char *prompt, size_t max_len)
{
    char *buf = QC_alloc(ctx, max_len);
    if(!buf) return NULL;
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    fgets(buf, max_len, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    printf("\x1b[0m");
    return buf;
}

char* QC_password(QC_Context *ctx, const char *prompt, size_t max_len)
{
    char *buf = QC_alloc(ctx, max_len);
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

    printf("%s?\x1b[0m Confirm: \x1b[1m%s", ctx->q_color, ctx->theme_color);
    char *confirm = calloc(max_len, sizeof(char));
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
    printf("%s?\x1b[0m %s \x1b[1m%s********", ctx->q_color, prompt, ctx->theme_color);
    printf("\x1b[0m\n");

    return buf;
}

char* QC_select(QC_Context *ctx, const char *prompt, const char **options, size_t num_options)
{
    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);

    printf("\x1b[?25l");

    int selected_i = 0;
    int key;
    do
    {
        for(int i = 0; i < num_options; ++i)
            printf("  %s %s\n", (i == selected_i ? ctx->arrow : ""), options[i]);
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
            printf("\x1b[1A\x1b[2K");
    } while(key != CONFIRM);

    char *selected = QC_alloc(ctx, strlen(options[selected_i]) + 1);
    strcpy(selected, options[selected_i]);

    printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s%s\n", ctx->q_color, prompt, ctx->theme_color, selected);

    printf("\x1b[?25h");

    return selected;
}

bool QC_confirm(QC_Context *ctx, const char *prompt)
{
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

uint64_t QC_checkbox(QC_Context *ctx, const char *prompt, const char **options, size_t num_options)
{
    uint64_t selected = 0;

    printf("%s?\x1b[0m %s\n", ctx->q_color, prompt);

    printf("\x1b[?25l");

    int selected_i = 0;
    bool all_selected[num_options];
    for(int i = 0; i < num_options; ++i)
        all_selected[i] = false;
    int key;
    do
    {
        for(int i = 0; i < num_options; ++i)
        {
            printf("  ");
            if(i == selected_i)
                printf("%s", ctx->arrow);
            if(all_selected[i])
                printf(" \x1b[32m[X]\x1b[0m");
            else printf(" [ ]");
            printf(" %s\n", options[i]);
        }
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
        if(key == ' ')
        {
            all_selected[selected_i] = !all_selected[selected_i];
        }
        if(selected_i < 0) selected_i = num_options - 1;
        if(selected_i >= num_options) selected_i = 0;
        for(int i = 0; i < num_options; ++i)
            printf("\x1b[1A\x1b[2K");
    } while(key != CONFIRM);
    
    for(int i = 0; i < num_options; ++i)
    {
        if(all_selected[i])
            selected |= (1ULL << i);
    }

    printf("\x1b[1A\x1b[2K");
    printf("%s?\x1b[0m %s \x1b[1m%s", ctx->q_color, prompt, ctx->theme_color);
    for(int i = 0; i < num_options; ++i)
        if(selected & (1ULL << i))
            printf("%s ", options[i]);

    printf("\x1b[?25h\n");

    return selected;
}
#endif

#endif
