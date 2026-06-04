#include "core/Application.h"

int main()
{
    app::Application app;
    if (!app.Run()) {
        return 1;
    }
    return 0;
}
