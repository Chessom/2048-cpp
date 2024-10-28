#pragma once
#include<ftxui/component/component.hpp>
#include<ftxui/component/screen_interactive.hpp>
#include"board_ftxui.h"
namespace tui {
	struct HomePage {
		HomePage() {
			using namespace ftxui;
			reset_handler = [this] {
				try {
					brd->option.board_size = std::stoi(board_size);
					brd->option.cell_size = std::max(1, std::stoi(cell_size));
				}
				catch (const std::exception&) {}
				brd->OnEvent(Event::Special("reset_board"));
				show_modal = false;
				score = 0;
				brd->TakeFocus();
				};
		}
		void start() {
			using namespace ftxui;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			option.move_func = animation::easing::SineInOut;
			option.duration = std::chrono::milliseconds(125);
			option.board_size = 4;
			brd = tui::Board(board, option);
			Component layout =
				Container::Horizontal(
					{
						brd,
						Renderer([] {return separatorEmpty(); }),
						Container::Vertical({
							Container::Horizontal(
							{
								ScoreRecord("Score"),
								Renderer([] {return separatorEmpty(); }),
								BestScoreRecord("Best"),
								Renderer([] {return separatorEmpty(); }),
								ResetButton(),
							}),
							Ele(separatorEmpty()),
							Container::Horizontal({
								BoardSizeInput() | borderEmpty | bgcolor(colors::zero_col) | color(colors::num_col),
								Ele(separatorEmpty()),
								CellSizeInput() | borderEmpty | bgcolor(colors::zero_col) | color(colors::num_col),
							}) | hcenter,
							AnimationDurationAdjust(),
							Ele(separatorEmpty()),
							AnimationEasingAdjust(),
							Ele(separatorEmpty()),
							AutomaticMove() | vcenter,
							Ele(separatorEmpty()),
							Container::Horizontal(
							{
								SearchDepth() | vcenter,
								Renderer([] {return separatorEmpty(); }),
								Text("Negative: automatic") | borderEmpty | vcenter
							}),
							Ele(separatorEmpty()),
							Button("      Quit      ",screen.ExitLoopClosure(),ButtonOption::Animated(
									colors::zero_col,colors::num_col,
									Color::Red,colors::num_col)
							) | hcenter
						}),
					}
				)
				| Modal(ModalDialog("Over!"), &show_modal)
				| CatchEvent([this](Event e) {
				if (e == Event::Special("gameover")) {
					show_modal = true;
					score = board.get_score();
					if (score > best_score) {
						best_score = score;
					}
					return true;
				}
				return false;
					});
			screen.Loop(layout | center);
		}

		ftxui::Component ModalDialog(std::string msg) {
			using namespace ftxui;
			return Container::Vertical({
				Text("                "),
				Text(msg) | hcenter ,
				Text(" "),
				Button("  OK  ",[this] {show_modal = false; },ButtonOption::Animated(0xffcb80_rgb)) | hcenter
				}) | borderRounded | bgcolor(0x877d74_rgb);
		}

		ftxui::Component Ele(ftxui::Element e) const {
			return ftxui::Renderer([e] {return e; });
		}

		ftxui::Component Text(std::string str) const {
			return ftxui::Renderer([str] {return ftxui::text(str); });
		}

		ftxui::Component ResetButton() {
			using namespace ftxui;
			return Button("Reset", reset_handler, ButtonOption::Animated(0xeee4da_rgb, 0x776e65_rgb));
		}

		ftxui::Component ScoreRecord(std::string title) {
			using namespace ftxui;
			return Renderer(
				[this, title] {
					return
						vbox(
							text(title) | color(0xeae0d5_rgb) | center,
							text(std::to_string(board.get_score())) | color(Color::White) | center
						)
						| bgcolor(colors::sep_col)
						| size(WIDTH, GREATER_THAN, 11)
						| size(HEIGHT, LESS_THAN, 6)
						| flex_grow;
				}
			);
		}

		ftxui::Component BestScoreRecord(std::string title) {
			using namespace ftxui;
			return Renderer(
				[this, title] {
					return
						vbox(
							text(title) | color(0xeae0d5_rgb) | center,
							text(std::to_string(best_score)) | color(Color::White) | center
						)
						| bgcolor(colors::sep_col)
						| size(WIDTH, GREATER_THAN, 11)
						| size(HEIGHT, LESS_THAN, 6)
						| flex_grow;
				}
			);
		}

		ftxui::Component BoardSizeInput() {
			using namespace ftxui;
			auto option = InputOption::Default();
			option.on_enter = reset_handler;
			option.multiline = false;
			Component input = Input(&board_size, "Input Board Size", option)
				| size(WIDTH, GREATER_THAN, 4);
			// Filter out non-digit characters.
			input |= CatchEvent([&](Event event) {
				return event.is_character() && !std::isdigit(event.character()[0]);
				});
			return Container::Horizontal({ Text("Board size:  ") | vcenter,input | vcenter });
		}

		ftxui::Component CellSizeInput() {
			using namespace ftxui;
			auto option = InputOption::Default();
			option.on_enter = [this] {
				try {
					brd->option.cell_size = std::max(1, std::stoi(cell_size));
				}
				catch (const std::exception&) {}
				brd->TakeFocus();
				};;
			option.multiline = false;
			Component input = Input(&cell_size, "Input Cell Size", option)
				| size(WIDTH, GREATER_THAN, 4);
			// Filter out non-digit characters.
			input |= CatchEvent([&](Event event) {
				return event.is_character() && !std::isdigit(event.character()[0]);
				});
			return Container::Horizontal({ Text("Cell size:  ") | vcenter,input | vcenter });
		}

		ftxui::Component SearchDepth() {
			using namespace ftxui;
			Color bgc = 0xeee4da_rgb, fgc = 0x776e65_rgb;
			auto option = InputOption::Spacious();
			option.on_change = [this] {
				int depth = -3;
				try {
					depth = std::stoi(search_depth);
					brd->solver.set_depth(depth);
				}
				catch (const std::exception&) {
					brd->solver.set_depth(-3);
				}
				};
			option.multiline = false;
			Component input = Input(&search_depth, "Depth", option)
				| size(WIDTH, GREATER_THAN, 4);
			input |= CatchEvent([&](Event event) {
				return event.is_character() && event.character()[0] != '-' && !std::isdigit(event.character()[0]);
				});
			return Container::Horizontal({ Text("Search Depth:") | vcenter,input | vcenter })/* | bgcolor(0xeee4da_rgb) | color(0x776e65_rgb)*/;
		}

		ftxui::Component AnimationDurationAdjust() {
			using namespace ftxui;
			auto option = InputOption::Spacious();
			option.on_change = [this] {
				if (!animation_duration.empty()) {
					duration_ms = std::stoi(animation_duration);
				}
				else {
					duration_ms = 0;
				}
				brd->option.duration = std::chrono::milliseconds(duration_ms);
				};
			option.multiline = false;
			Component input = Input(&animation_duration, "duration(ms)", option);
			// Filter out non-digit characters.
			input |= CatchEvent([&](Event event) {
				return event.is_character() && !std::isdigit(event.character()[0]);
				});
			Component slider = Slider("", &duration_ms, 0, 500)
				| CatchEvent([this, input](Event e) {
				if (e.is_mouse()) {
					int now = std::stoi(animation_duration);
					if (now != duration_ms) {
						animation_duration = std::to_string(duration_ms);
						brd->option.duration = std::chrono::milliseconds(duration_ms);
					}
				}
				return false;
					});

			return
				Container::Vertical({
					Container::Horizontal({
						Text("Animation duration(ms):") | vcenter,
						input | vcenter,
					}) ,
					slider | vcenter
					});
		}

		ftxui::Component AnimationEasingAdjust() {
			using namespace ftxui;
			using namespace animation::easing;
			easing_name = {
				"Linear",
				"Sine",
				"Quadratic",
				"Cubic",
				"Quartic",
				"Quintic",
				"Circular",
				"Exponential",
				"Elastic",
				"Back",
				"Bounce"
			};
			easing_func = {
				Linear,
				SineInOut,
				QuadraticInOut,
				CubicInOut,
				QuarticInOut,
				QuinticInOut,
				CircularInOut,
				ExponentialInOut,
				ElasticInOut,
				BackInOut,
				BounceInOut
			};
			Component drop = Dropdown(&easing_name, &selected_easing)
				| size(HEIGHT, LESS_THAN, 10)
				| CatchEvent([this](Event e) {
				if (e.is_mouse() && e.mouse().motion == Mouse::Pressed) {
					ScreenInteractive::Active()->Post([this] {
						brd->option.move_func = easing_func[selected_easing];
						});
				}
				return false;
					});
			return
				Container::Vertical
				({
					Container::Horizontal
					({
						Text("Animation Style: ") | borderEmpty,
						drop
					}) | bgcolor(0xeee4da_rgb) | color(0x776e65_rgb)
					});
		}

		ftxui::Component AutomaticMove() {
			using namespace ftxui;
			return Button("Automatic Play Switch", [this] {
				if (!board.is_over()) {
					brd->automatic_move = !brd->automatic_move;
					brd->TakeFocus();
					ScreenInteractive::Active()->PostEvent(Event::Special("automatic_move"));
				}
				else
				{
					brd->automatic_move = false;
				}
				}, ButtonOption::Animated(0xeee4da_rgb, 0x776e65_rgb));
		}

		std::vector<std::string> easing_name;
		std::vector<ftxui::animation::easing::Function> easing_func;
		std::function<void()> reset_handler;
		int selected_easing = 1;
		BoardCom brd;
		core::board_2048 board;
		std::string board_size = "4";
		std::string cell_size = "5";
		std::string search_depth = "-3";
		std::string animation_duration = "125";
		int duration_ms = 125;
		int best_score = 0;
		int score = 0;
		bool show_modal = false;
		BoardOption option;
	};
};