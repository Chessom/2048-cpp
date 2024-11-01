#pragma once
#include<utility>
namespace core {
namespace direction {
    enum {
        left = 0,
        down,
        right,
        up
    };
} // namespace direction
template <typename T>
struct coord_t {
    coord_t()
        = default;
    coord_t(T x, T y)
        : x(x)
        , y(y)
    {
    }
    coord_t(const std::pair<int, int>& p)
        : x(p.first)
        , y(p.second) { };
    void shift(int dir)
    {
        switch (dir) {
        case direction::up:
            --y;
            break;
        case direction::down:
            ++y;
            break;
        case direction::left:
            --x;
            break;
        case direction::right:
            ++x;
            break;
        default:
            break;
        }
    }
    T x = 0, y = 0;
};
using coord = coord_t<int>;
}
