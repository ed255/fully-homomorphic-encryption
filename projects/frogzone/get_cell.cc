#include "frogzone.h"

CellData get_cell_no_check(
                           Coord coord,
                           ItemsWithId items,
                           PlayersWithId players) {
  CellData cell = CellData();

  #pragma hls_unroll yes
  for (int i = 0; i < NUM_ITEMS; i++) {
    ItemWithId item = items.values[i];
    if ((coord == item.data.loc) && (!item.data.is_consumed)) {
      cell.entity_type = Item;
      cell.entity_id = item.id;
      cell.hp = item.data.hp;
      cell.atk = item.data.atk;
    }
  }

  #pragma hls_unroll yes
  for (int i = 0; i < NUM_PLAYERS; i++) {
    PlayerWithId player = players.values[i];
    if (coord == player.data.loc) {
      cell.entity_type = Player;
      cell.entity_id = player.id;
      cell.hp = player.data.hp;
      cell.atk = player.data.atk;
    }
  }

  return cell;
}

#pragma hls_top
CellData get_cell(
                  Coord player_coord,
                  Coord query_coord,
                  ItemsWithId items,
                  PlayersWithId players
                  ) {
  unsigned char abs_delta_x = player_coord.x > query_coord.x ?
    player_coord.x - query_coord.x : query_coord.x - player_coord.x;
  unsigned char abs_delta_y = player_coord.y > query_coord.y ?
    player_coord.y - query_coord.y : query_coord.y - player_coord.y;

  if ((abs_delta_x > 2) || (abs_delta_y > 2)) {
    CellData ret = CellData();
    ret.entity_type = Invalid;
    return ret;
  }

  return get_cell_no_check(query_coord, items, players);
}

