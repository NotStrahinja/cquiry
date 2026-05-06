# cquiry
A single header C library for interactive CLI prompts.

## Preview

<img width="544" height="365" alt="demo" src="https://github.com/user-attachments/assets/6eff3cda-bdce-4fbb-b388-6a22912b857c" />

## Usage
In order to use the library, include this at the top of your C file:
```c
#define QC_IMPLEMENTATION
#include "qc.h"
```

### Context
I chose to implement a context for this library because of a trade-off between readability and manually freeing everything.

You can see the usage of the context in the example code.

## Example
Here's the code for a simple demo program from the preview:

```c
#define QC_IMPLEMENTATION
#include "qc.h"

int main()
{
    QC_Context ctx = QC_DEFAULT_CONTEXT;

    char *name = QC_text(&ctx, "What is your name?", 16);

    char *password = QC_password(&ctx, "Enter your password:", 16);

    const char *fruits[] = {"Apple", "Banana", "Orange"};
    char *select = QC_select(&ctx, "Select a fruit:", fruits, QC_ARRLEN(fruits));

    const char *features[] = {"Bla bla", "Demo", "123"};
    uint64_t selected_features = QC_checkbox(&ctx, "Select features:", features, QC_ARRLEN(features));

    bool confirm = QC_confirm(&ctx, "Finish demo?");

    QC_free_all(&ctx);

    return 0;
}
```

If you want to use the default context config, you can use the `QC_DEFAULT_CONTEXT` macro. In case you want to customize the colors/escape sequences, you can do so.

Additionally, for checking if a checkbox is checked, you can use the `QC_CHECKED` macro like this:

```c
if(QC_CHECKED(selected_features, 0 /* "Bla bla" */))
{
   ...
}
```

## Functions
|Function name|Parameters|Return type|
|-------------|----------|-----------|
|QC_text|(QC_Context *ctx, const char *prompt, size_t max_len)|char*|
|QC_password|(QC_Context *ctx, const char *prompt, size_t max_len)|char*|
|QC_select|(QC_Context *ctx, const char *prompt, const char **options, size_t num_options)|char*|
|QC_checkbox|(QC_Context *ctx, const char *prompt, const char **options, size_t num_options)|uint64_t|
|QC_confirm|(QC_Context *ctx, const char *prompt)|bool|

## License
This project uses the [MIT License](https://github.com/NotStrahinja/cquiry/blob/main/LICENSE).

## References
[questionary](https://github.com/tmbo/questionary) (Python)

[inquire](https://github.com/mikaelmello/inquire) (Rust)
