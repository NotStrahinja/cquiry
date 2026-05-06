# cquiry
A single header C library for interactive CLI prompts.

## Preview

<img width="544" height="365" alt="demo" src="https://github.com/user-attachments/assets/c71a7e9b-02b4-4b99-bb34-d1484ba633dd" />

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
#include <stdio.h>

#define QC_IMPLEMENTATION
#include "qc.h"

int main()
{
    QC_Context ctx = {0};

    char *name = QC_text(&ctx, "What is your name?", 16);

    char *password = QC_password(&ctx, "Enter your password:", 16);

    const char *fruits[] = {"Apple", "Banana", "Orange"};
    size_t num_fruits = sizeof(fruits)/sizeof(fruits[0]);
    char *select = QC_select(&ctx, "Select a fruit:", fruits, num_fruits);

    const char *features[] = {"Bla bla", "Demo", "123"};
    size_t num_features = sizeof(features)/sizeof(features[0]);
    uint64_t selected_features = QC_checkbox("Select features:", features, num_features);

    bool confirm = QC_confirm("Finish demo?");

    QC_free_all(&ctx);

    return 0;
}
```

## Functions
|Function name|Parameters|Return type|
|-------------|----------|-----------|
|QC_text|(QC_Context *ctx, const char *prompt, size_t max_len)|char*|
|QC_password|(QC_Context *ctx, const char *prompt, size_t max_len)|char*|
|QC_select|(QC_Context *ctx, const char *prompt, const char **options, size_t num_options)|char*|
|QC_checkbox|(const char *prompt, const char **options, size_t num_options)|uint64_t|
|QC_confirm|(const char *prompt)|bool|

## License
This project uses the [MIT License](https://github.com/NotStrahinja/cquiry/blob/main/LICENSE).

## References
[questionary](https://github.com/tmbo/questionary) (Python)

[inquire](https://github.com/mikaelmello/inquire) (Rust)
