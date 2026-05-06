# Documentation

## Controls

Arrow keys - move selection

Space - check a box

Enter - select

## Context
The library requires a context to be passed to each function (`CQ_Context`).

It can be initialized with `CQ_DEFAULT_CONTEXT` for basic usage, or you can initialize it yourself and fully customize it.

### Struct

```c
typedef struct {
    char **ptrs;
    size_t count;
    size_t capacity;
    const char *theme_color;
    const char *q_color;
    const char *err_color;
    const char *arrow;
} CQ_Context;
```

For customizing the colors, I'd strongly recommend using [this link](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797) as reference.

`ptrs` is a dynamic array of pointers that get stored inside function calls (e.g., `CQ_text`) when allocating a buffer is required.

`count` and `capacity` are used for the dynamic array reallocation.

`theme_color` is an ANSI escape sequence for any color of your choosing (default value: `"\x1b[38;2;255;157;0m"`).

`q_color` is used to alter the color of the question mark at the beginning of each prompt (default value: `"\x1b[38;2;92;124;151m"`).

`err_color` is used to alter the color of the X in front of error messages (default value: `"\x1b[31m"`).

`arrow` is used for the selection screens (default value: `">"`).

## Functions

### CQ_text
A text prompt

__Arguments__: (CQ_Context *ctx, const char *prompt, size_t max_len)

__Return type__: char*

__Example usage__:

```c
CQ_Context ctx = CQ_DEFAULT_CONTEXT;
char *name = CQ_text(&ctx, "What is your name?", 16);
...
CQ_free_all(&ctx);
```

### CQ_password
A password prompt

__Arguments__: (CQ_Context *ctx, const char *prompt, size_t max_len)

__Return type__: char*

__Example usage__:

```c
CQ_Context ctx = CQ_DEFAULT_CONTEXT;
char *password = CQ_password(&ctx, "Enter your password:", 16);
...
CQ_free_all(&ctx);
```

### CQ_select
A selection prompt

__Arguments__: (CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)

__Return type__: char*

__Example usage__:

```c
CQ_Context ctx = CQ_DEFAULT_CONTEXT;
const char *options[] = {"fizz", "buzz", "fizzbuzz"};
char *selected = CQ_select(&ctx, "Select an option:", options, CQ_ARRLEN(options));
...
CQ_free_all(&ctx);
```

### CQ_checkbox
A multi-selection prompt

__Arguments__: (CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)

__Return type__: uint64_t

__Example usage__:

```c
CQ_Context ctx = CQ_DEFAULT_CONTEXT;
const char *options[] = {"fizz", "buzz", "fizzbuzz"};
uint64_t selected = CQ_checkbox(&ctx, "Select options:", options, QC_ARRLEN(options));
...
CQ_free_all(&ctx);
```

And checking for selected options:

```c
if(CQ_CHECKED(selected, 0))
{
  printf("fizz selected\n");
}
...
```

### CQ_confirm
A simple confirmation prompt

__Arguments__: (CQ_Context *ctx, const char *prompt)

__Return type__: bool

__Example usage__:

```c
CQ_Context ctx = CQ_DEFAULT_CONTEXT;
bool start = CQ_confirm(&ctx, "Do you want to start?");
...
CQ_free_all(&ctx);
```
