#include "frogzone.h"

void test_apply_move() {
  PlayerData player_data;
  Direction direction;
  Obstacles100 obstacles;
  Items items;
  apply_move(player_data, direction, obstacles, items);
}

void test_get_cell() {
  Coord player_coord;
  Coord query_coord;
  ItemsWithId items;
  PlayersWithId players;
  get_cell(player_coord, query_coord, items, players);
}

int main() {
  test_apply_move();
  return 0;
}
