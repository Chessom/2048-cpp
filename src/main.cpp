#include <ftxui/component/screen_interactive.hpp>

#include "tui/homepage.hpp"
int main() {
    using namespace ftxui;
    tui::HomePage page;
    page.start();
    return 0;
}