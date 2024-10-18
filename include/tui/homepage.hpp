#pragma once
#include<ftxui/component/component.hpp>
#include<ftxui/component/screen_interactive.hpp>
#include"board_ftxui.h"
namespace tui {
	struct HomePage {
		void start() {
			using namespace ftxui;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			auto brd = tui::Board(option);
			screen.Loop(brd);
		}
		ftxui::Component ResetButton() {
			using namespace ftxui;
			return Button("Reset", [] {
				auto p = ScreenInteractive::Active();
				if (p != nullptr) {
					p->PostEvent(Event::Special("reset_board"));
				}
				}, ButtonOption::Animated(0xeee4da_rgb, 0x776e65_rgb));
		}
		std::string board_size;
		int best_score;
		int score;
		BoardOption option;
	};
}