# cquiry
A single header C library for interactive CLI prompts.

## Preview

<img width="726" height="438" alt="demo3" src="https://github.com/user-attachments/assets/fda0d7e4-97fc-4cb7-840e-1d7d1d5efc21" />

## Quickstart

Just download `cq.h` and drop it in your project. Done.

## Platforms

This library is fully cross-platform and can compile from C99 to C23.

## Usage
In order to use the library, include this at the top of your C file:
```c
#define CQ_IMPLEMENTATION
#include "cq.h"
```

### Context
I chose to implement a context for this library because of a trade-off between readability and manually freeing everything.

You can see the usage of the context in the example code.

## Example
Here's the code for a simple demo program from the preview:

```c
#define CQ_IMPLEMENTATION
#include "cq.h"

int main()
{
    CQ_Context ctx = CQ_DEFAULT_CONTEXT;

    char *name = CQ_text(&ctx, "What is your name?", 16);

    char *password = CQ_password(&ctx, "Enter your password:", 16);

    const char *fruits[] = {"Apple", "Banana", "Orange"};
    char *select = CQ_select(&ctx, "Select a fruit:", fruits, CQ_ARRLEN(fruits));

    const char *features[] = {"Bla bla", "Demo", "123"};
    uint64_t selected_features = CQ_checkbox(&ctx, "Select features:", features, CQ_ARRLEN(features));

    bool confirm = CQ_confirm(&ctx, "Finish demo?");

    CQ_cleanup(&ctx);

    return 0;
}
```

If you want to use the default context config, you can use the `CQ_DEFAULT_CONTEXT` macro. In case you want to customize the colors/escape sequences, you can do so.

Additionally, for checking if a checkbox is checked, you can use the `CQ_CHECKED` macro like this:

```c
if(CQ_CHECKED(selected_features, 0 /* "Bla bla" */))
{
   ...
}
```

## Functions
|Function name|Parameters|Return type|
|-------------|----------|-----------|
|CQ_text|(CQ_Context *ctx, const char *prompt, size_t max_len)|char*|
|CQ_password|(CQ_Context *ctx, const char *prompt, size_t max_len)|char*|
|CQ_select|(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)|char*|
|CQ_checkbox|(CQ_Context *ctx, const char *prompt, const char **options, size_t num_options)|uint64_t|
|CQ_confirm|(CQ_Context *ctx, const char *prompt)|bool|

## License
This project uses the [MIT License](https://github.com/NotStrahinja/cquiry/blob/main/LICENSE).

## Documentation
You can see the full documentation [here](https://github.com/NotStrahinja/cquiry/blob/main/DOCS.md).

## References
[questionary](https://github.com/tmbo/questionary) (Python)

[inquire](https://github.com/mikaelmello/inquire) (Rust)
