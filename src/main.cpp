#include"tui/homepage.hpp"
#include<ftxui/component/screen_interactive.hpp>
int main() {
	using namespace ftxui;
	/*ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
	auto option = tui::BoardOption();
	option.board_size = 4;
	option.duration = std::chrono::milliseconds(150);
	option.move_func = ftxui::animation::easing::SineInOut;
	auto brd = tui::Board(option);
	screen.Loop(brd);*/
	/*Screen screen = Screen::Create(Dimension::Full());
	Elements rows;
	for (int i = 0; i < 10;++i){
		rows.push_back(text("          ") | bgcolor(tui::colors::color_of(1024 * (1 << i))));
	}
	Render(screen, vbox(rows));
	screen.Print();*/
	tui::HomePage page;
	page.start();
	return 0;
}