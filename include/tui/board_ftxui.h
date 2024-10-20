#pragma once
#include"board_2048.hpp"
#include"solver.hpp"
#include<format>
#include<ftxui/screen/color.hpp>
#include<ftxui/dom/elements.hpp>
#include<ftxui/component/event.hpp>
#include<ftxui/component/component.hpp>
namespace tui {
	using namespace ftxui::literals;
	namespace colors {
		inline ftxui::Color color_of(int tile_number) {
			using namespace ftxui;
			switch (tile_number)
			{
			case 2: return 0xeee4da_rgb;
			case 4: return 0xeee1c9_rgb;
			case 8: return 0xf3b27a_rgb;
			case 16: return 0xf69664_rgb;
			case 32: return 0xf77c5f_rgb;
			case 64: return 0xf75f3b_rgb;
			case 128: return 0xedd073_rgb;
			case 256: return 0xedcc62_rgb;
			case 512: return 0xedc950_rgb;
			case 1024: return Color::RGB(237, 197, 63);
			case 2048: return Color::RGB(237, 194, 46);
			default:
				srand(tile_number);
				int baseRed = 255;
				int baseGreen = 180;
				int baseBlue = 0;

				double factor1 = std::log2(tile_number * rand() / RAND_MAX) / std::log2(2048);
				factor1 = std::abs(std::tanh(factor1));
				double factor2 = std::log2(tile_number * rand() / RAND_MAX) / std::log2(2048);
				factor2 = std::abs(std::tanh(factor2));

				int r = static_cast<int>(baseRed * factor1);
				int g = static_cast<int>(baseGreen * factor2);
				int b = static_cast<int>(0);

				return Color::RGB(r, g, b);
				break;
			}
		}
		inline ftxui::Color sep_col = 0xbbada0_rgb;
		inline ftxui::Color num_col = 0x776e65_rgb;
		inline ftxui::Color zero_col = 0xcdc1b4_rgb;
	}
	
	inline ftxui::Element board_view_2048(const core::board_2048& brd, int cell_size) {
		using namespace ftxui;
		Elements rows;
		auto cell_size_style = ftxui::size(HEIGHT, EQUAL, cell_size) | ftxui::size(WIDTH, EQUAL, cell_size * 2);
		auto bgcol = colors::sep_col;
		auto numcol = colors::num_col;
		auto zero_col = colors::zero_col;
		auto sep = std::string((cell_size * 2 + 2) * brd.size() - cell_size, ' ');
		for (int x = 0; x < brd.size(); ++x) {
			Elements row;
			for (int y = 0; y < brd.size(); ++y)
			{
				auto tile_n = brd.get_tile(x, y);
				if (tile_n != 0) {
					row.push_back(text(std::to_string(tile_n))
						| center | cell_size_style | bgcolor(colors::color_of(tile_n)) | color(numcol));
				}
				else {
					row.push_back(text(" ")
						| cell_size_style | bgcolor(zero_col));
				}
				if (y != brd.size() - 1) {
					row.push_back(text("  ")
						| ftxui::size(HEIGHT, EQUAL, cell_size) | ftxui::size(WIDTH, EQUAL, 2) | bgcolor(bgcol) | color(numcol));
				}
			}
			rows.push_back(hbox(row));
			if (x != brd.size() - 1) {
				rows.push_back(
					ftxui::text(sep) | bgcolor(bgcol) | size(WIDTH, EQUAL, int(sep.size()))
				);
			}
		}
		return vbox(rows) | borderRounded | bgcolor(bgcol) /*| size(WIDTH, EQUAL, sep.size() + 3) | size(HEIGHT, EQUAL, (cell_size + 1) * brd.size() - 1)*/;
	}
	struct BoardOption {
		int board_size = 4;
		int cell_size = 5;
		int sep_size = 1;
		ftxui::animation::easing::Function move_func = ftxui::animation::easing::Linear;
		ftxui::animation::Duration duration = std::chrono::milliseconds(250);
	};
	struct TileBase :ftxui::Node {
		explicit TileBase(int number, int cell_size, ftxui::Color num_col)
			:n(number),
			cell_size(cell_size),
			num_col(num_col)
		{
		}
		void ComputeRequirement() override {
			requirement_.min_x = cell_size * 2;
			requirement_.min_y = cell_size;
		}
		void Render(ftxui::Screen& screen) override {
			if (box_.x_max < box_.x_min) {
				return;
			}
			for (int x = box_.x_min; x <= box_.x_max; ++x) {
				for (int y = box_.y_min; y <= box_.y_max; ++y) {
					screen.PixelAt(x, y).background_color = colors::color_of(n);
				}
			}
			int midy = (box_.y_min + box_.y_max) / 2;
			std::string nstr = std::format("{0:^{1}}", n, box_.x_max - box_.x_min + 1);
			auto it = nstr.begin();
			for (int x = box_.x_min; x <= box_.x_max; ++x) {
				auto& pixel = screen.PixelAt(x, midy);
				pixel.character = *it++;
				pixel.foreground_color = num_col;
			}
		}
	private:
		int n;
		int cell_size;
		ftxui::Color num_col;
	};
	struct TileRowBase :ftxui::Node {
		explicit TileRowBase(std::vector<float> tiles_pos, std::vector<int> tile_numbers, int row_len, int tile_size, int sep_size = 1)
			:tile_start(std::move(tiles_pos)), tiles(std::move(tile_numbers)), tile_size(tile_size), length(row_len), sep_size(sep_size) {
			using namespace ftxui;
			auto zero_e = [this] {
				return text(" ")
				| ftxui::size(HEIGHT, EQUAL, this->tile_size)
				| ftxui::size(WIDTH, EQUAL, this->tile_size * 2)
				| bgcolor(colors::zero_col);
			};
			auto sep_e = [this] {
				return text(" ")
					| ftxui::size(HEIGHT, EQUAL, this->tile_size) | ftxui::size(WIDTH, EQUAL, this->sep_size * 2) | bgcolor(colors::sep_col);
				};
			Elements es;
			es.reserve(2 * length - 1);
			es.push_back(zero_e());
			for (int i = 1; i < length; ++i) {
				es.push_back(sep_e());
				es.push_back(zero_e());
			}
			children_.push_back(hbox(es));
		}

		void ComputeRequirement() override {
			Node::ComputeRequirement();
			requirement_ = children_[0]->requirement();
		}
		void SetBox(ftxui::Box box) override {
			Node::SetBox(box);
			children_[0]->SetBox(
				ftxui::Box{
					.x_min = box_.x_min,
					.x_max = box_.x_min + requirement_.min_x - 1,
					.y_min = box_.y_min,
					.y_max = box_.y_min + requirement_.min_y - 1,
				}
			);
		}
		void Render(ftxui::Screen& screen) override {
			using namespace ftxui;
			Node::Render(screen);
			if (box_.x_max < box_.x_min) {
				return;
			}
			for (int i = 0; i < tiles.size(); ++i) {
				auto tile = tile_element(tiles[i]);
				int start_x = box_.x_min + int(std::round(tile_start[i])), start_y = box_.y_min;
				tile->SetBox(
					Box{
						.x_min = start_x,
						.x_max = start_x + tile_size * 2 - 1,
						.y_min = start_y,
						.y_max = start_y + tile_size - 1
					}
				);
				tile->Render(screen);
			}
		}
		
	private:
		ftxui::Element tile_element(int tile_number) const {
			using namespace ftxui;
			return Make<TileBase>(tile_number, tile_size, colors::num_col);
		}
		std::vector<float> tile_start;
		std::vector<int> tiles;
		int length;
		int tile_size;
		int sep_size;
	};
	struct TileColBase :ftxui::Node {
		explicit TileColBase(std::vector<float> tiles_pos, std::vector<int> tile_numbers, int row_len, int tile_size, int sep_size = 1)
			:tile_start(std::move(tiles_pos)), tiles(std::move(tile_numbers)), tile_size(tile_size), length(row_len), sep_size(sep_size) {
			using namespace ftxui;
			auto zero_e = [this] {
				return text(" ")
					| ftxui::size(HEIGHT, EQUAL, this->tile_size)
					| ftxui::size(WIDTH, EQUAL, this->tile_size * 2)
					| bgcolor(colors::zero_col);
				};
			auto sep_e = [this] {
				return text(" ")
					| ftxui::size(HEIGHT, EQUAL, this->sep_size) | ftxui::size(WIDTH, EQUAL, this->tile_size * 2) | bgcolor(colors::sep_col);
				};
			Elements es;
			es.reserve(2 * length - 1);
			es.push_back(zero_e());
			for (int i = 1; i < length; ++i) {
				es.push_back(sep_e());
				es.push_back(zero_e());
			}
			children_.push_back(vbox(es));
		}

		void ComputeRequirement() override {
			Node::ComputeRequirement();
			requirement_ = children_[0]->requirement();
		}

		void SetBox(ftxui::Box box) override {
			Node::SetBox(box);
			children_[0]->SetBox(
				ftxui::Box{
					.x_min = box_.x_min,
					.x_max = box_.x_min + requirement_.min_x - 1,
					.y_min = box_.y_min,
					.y_max = box_.y_min + requirement_.min_y - 1,
				}
				);
		}
		void Render(ftxui::Screen& screen) override {
			using namespace ftxui;
			Node::Render(screen);
			if (box_.y_max < box_.y_min) {
				return;
			}
			for (int i = 0; i < tiles.size(); ++i) {
				auto tile = tile_element(tiles[i]);
				int start_y = box_.y_min + int(std::round(tile_start[i])), start_x = box_.x_min;
				tile->SetBox(
					Box{
						.x_min = start_x,
						.x_max = start_x + tile_size * 2 - 1,
						.y_min = start_y,
						.y_max = start_y + tile_size - 1
					}
				);
				tile->Render(screen);
			}
		}
	private:
		ftxui::Element tile_element(int tile_number) const {
			using namespace ftxui;
			return Make<TileBase>(tile_number, tile_size, colors::num_col);
			/*static auto cell_size_style = ftxui::size(HEIGHT, EQUAL, tile_size) | ftxui::size(WIDTH, EQUAL, tile_size * 2);
			return text(std::to_string(tile_number)) | cell_size_style | color(num_col) | bgcolor(colors::color_of(tile_number));*/
		}
		std::vector<float> tile_start;
		std::vector<int> tiles;
		int length;
		int tile_size;
		int sep_size;
	};
	class BoardBase :public ftxui::ComponentBase {
	public:
		explicit BoardBase(core::board_2048& board_ref, const BoardOption& options)
			:option(options),board(board_ref){
			board.brd_size = option.board_size;
			pre_board = board;
			animate_value_and_target.resize(options.board_size);
		};

		void OnAnimation(ftxui::animation::Params& params) override {
			if (animator_main.to() != 0.0f) {
				animator_main.OnAnimation(params);
			}
		}

		bool OnEvent(ftxui::Event e) override {
			using namespace ftxui;
			if (e.is_mouse() && e.mouse().motion == Mouse::Pressed) {
				if (box_.Contain(e.mouse().x, e.mouse().y)) {
					TakeFocus();
				}
			}
			int dir = -1;
			if (e == Event::ArrowUp || e == Event::w) {
				dir = core::direction::up;
			}
			else if (e == Event::ArrowDown || e == Event::s)
			{
				dir = core::direction::down;
			}
			else if (e == Event::ArrowLeft || e == Event::a)
			{
				dir = core::direction::left;
			}
			else if (e == Event::ArrowRight || e == Event::d)
			{
				dir = core::direction::right;
			}
			else if (e == Event::Special("reset_board")) {
				board = core::board_2048(option.board_size);
				animate_value_and_target.clear();
				animate_value_and_target.resize(option.board_size);
				return true;
			}
			else if (e == Event::Special("automatic_move")) {
				if (automatic_move) {
					dir = solver.get_best_move(board);
				}
				else {
					return true;
				}
			}

			if (dir != -1) {
				if (animation_progress == 1.0f) {
					if (board.valid_move(dir)) {
						animation_progress = 0.0f;
						pre_board = board;
						board.move_record(dir);
						animate_direction = dir;
						UpdateAnimationTarget(animate_direction);
						board.add_random_tile();
					}
					if (board.is_over()) {
						ScreenInteractive::Active()->PostEvent(Event::Special("gameover"));
						automatic_move = false;
					}
				}
				return true;
			}
			else {
				return false;
			}
		}

		ftxui::Element Render() override {
			using namespace ftxui;
			Element ret;
			if (animation_progress == 1.0f) {
				ret = board_view_2048(board, option.cell_size);
				if (automatic_move) {
					ScreenInteractive::Active()->PostEvent(Event::Special("automatic_move"));
				}
			}
			else {
				if (animate_direction % 2 == 0) {
					Elements rows;
					std::vector<float> tile_pos;
					std::vector<int> tile_numbers;
					for (auto& vec : animate_value_and_target) {
						for (auto& tp : vec) {
							tile_numbers.push_back(std::get<0>(tp));
							tile_pos.push_back(
								(option.cell_size + option.sep_size) * 2 *
								std::lerp(std::get<1>(tp), std::get<2>(tp), animation_progress)
							);
						}
						rows.push_back(Make<TileRowBase>(tile_pos, tile_numbers, option.board_size, option.cell_size, option.sep_size));
						rows.push_back(separatorEmpty() | size(HEIGHT, EQUAL, option.sep_size) | bgcolor(colors::sep_col));
						tile_numbers.clear();
						tile_pos.clear();
					}
					rows.pop_back();
					ret = vbox(rows)
						| borderRounded | bgcolor(colors::sep_col);
				}
				else {
					Elements cols;
					std::vector<float> tile_pos;
					std::vector<int> tile_numbers;
					for (auto& vec : animate_value_and_target) {
						for (auto& tp : vec) {
							tile_numbers.push_back(std::get<0>(tp));
							tile_pos.push_back(
								(option.cell_size + option.sep_size) *
								std::lerp(std::get<1>(tp), std::get<2>(tp), animation_progress)
							);
						}
						cols.push_back(Make<TileColBase>(tile_pos, tile_numbers, option.board_size, option.cell_size, option.sep_size));
						cols.push_back(separatorEmpty() | size(WIDTH, EQUAL, option.sep_size * 2) | bgcolor(colors::sep_col));
						tile_pos.clear();
						tile_numbers.clear();
					}
					cols.pop_back();
					ret = hbox(cols) | borderRounded | bgcolor(colors::sep_col);
				}
			}
			int col_size = (option.cell_size + option.sep_size) * option.board_size - option.sep_size;
			return ret
				| size(WIDTH, EQUAL, col_size * 2 + 2)
				| size(HEIGHT, EQUAL, col_size + 2)|reflect(box_);
		}

		int op_dir(int dir)const { return (dir + 2) & 0b11; }

		void UpdateAnimationTarget(int dir) {
			animator_main = ftxui::animation::Animator(&animation_progress, 1, option.duration, option.move_func);
			using namespace ftxui;
			
			auto brd_sz = board.size();
			for (int i = 0; i < animate_value_and_target.size(); ++i) {
				animate_value_and_target[i].clear();
			}
			for (int x = 0; x < brd_sz; ++x) {
				for (int y = 0; y < brd_sz; ++y) {
					auto& p = board.records[x * brd_sz + y];
					if (p.first != 0) {
						if (dir == core::direction::left) {
							int ay = p.second % brd_sz;
							animate_value_and_target[x].push_back({ p.first,y,ay });
						}
						else if (dir == core::direction::right) {
							int ay = brd_sz - 1 - p.second % brd_sz;
							animate_value_and_target[brd_sz - 1 - x].push_back({ p.first,brd_sz - 1 - y,ay });
						}
						else if (dir == core::direction::down) {
							int ay = brd_sz - 1 - p.second % brd_sz;
							animate_value_and_target[x].push_back({ p.first,brd_sz - 1 - y,ay });
						}
						else if (dir == core::direction::up) {
							int ay = p.second % brd_sz;
							animate_value_and_target[brd_sz - 1 - x].push_back({ p.first,y,ay });
						}
					}
					
				}
			}
		}
		BoardOption option;
		bool automatic_move = false;
		core::solver solver;
	private:
		ftxui::Box box_;
		core::board_2048& board;
		core::board_2048 pre_board;
		int animate_direction = core::direction::left;
		float animation_progress = 1.0f;
		ftxui::animation::Animator animator_main = ftxui::animation::Animator(&animation_progress);
		std::vector<std::vector<std::tuple<int, float, float>>> animate_value_and_target;
	};
	using BoardCom = std::shared_ptr<BoardBase>;
	inline auto Board(core::board_2048& brd_ref, BoardOption option = BoardOption{}) {
		return ftxui::Make<BoardBase>(brd_ref, option);
	}
}