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

    CQ_free_all(&ctx);

    return 0;
}
