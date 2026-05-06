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
