# cquiry

![Static Badge](https://img.shields.io/badge/Version-1.6-blue) ![Static Badge](https://img.shields.io/badge/License-MIT%20License-green) 

A single header C library for interactive CLI prompts.

## Preview

<img width="726" height="440" alt="demo3" src="https://github.com/user-attachments/assets/5fd6a408-753f-4010-b951-fb9fd8feba3e" />

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
    CQ_free_secure(&ctx, password); // Wipes the memory first then frees

    const char *fruits[] = { "Apple", "Banana", "Orange" };
    char *select = CQ_select(&ctx, "Select a fruit:", fruits, CQ_ARRLEN(fruits));

    const char *features[] = { "Bla bla", "Demo", "123" };
    uint64_t selected_features = CQ_checkbox(&ctx, "Select features:", features, CQ_ARRLEN(features));

    if (CQ_CHECKED(selected_features, 0 /* Bla Bla */))
    {
        printf("[+] Bla Bla checked!\n");
    }

    bool confirm = CQ_confirm(&ctx, "Finish demo?");

    CQ_cleanup(&ctx);

    return 0;
}
```

If you want to use the default context config, you can use the `CQ_DEFAULT_CONTEXT` macro. In case you want to customize the colors/escape sequences, you can do so.

Additionally, for checking if a checkbox is checked, you can use the `CQ_CHECKED` macro like this:

```c
if (CQ_CHECKED(selected_features, 0 /* "Bla bla" */))
{
   printf("[+] Bla Bla checked!\n");
}
```

## Functions
|Function name|Parameters|Return type|Description|
|-------------|----------|-----------|-----------|
|CQ_text|(CQ_Context* ctx, const char* prompt, size_t max_len)|char*|A text prompt|
|CQ_password|(CQ_Context* ctx, const char* prompt, size_t max_len)|char*|A hidden password prompt|
|CQ_select|(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options)|char*|A prompt with multiple options|
|CQ_checkbox|(CQ_Context* ctx, const char* prompt, const char** options, size_t num_options)|uint64_t|A prompt with multiple checkboxes|
|CQ_confirm|(CQ_Context* ctx, const char* prompt)|bool|A simple confirmation prompt|
|CQ_alloc|(CQ_Context* ctx, size_t size)|char*|Allocate on the arena|
|CQ_free_last|(CQ_Context* ctx)|void|Free the last arena allocation|
|CQ_free_secure|(CQ_Context* ctx, char* buf)|void|Free specific allocation + secure zero|
|CQ_cleanup|(CQ_Context* ctx)|void|Free and clean up the entire arena|

## License
This project uses the [MIT License](https://github.com/NotStrahinja/cquiry/blob/main/LICENSE).

## Documentation
You can see the full documentation [here](https://github.com/NotStrahinja/cquiry/blob/main/DOCS.md).

## References
[questionary](https://github.com/tmbo/questionary) (Python)

[inquire](https://github.com/mikaelmello/inquire) (Rust)
