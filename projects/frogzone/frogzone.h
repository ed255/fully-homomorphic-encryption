#ifndef FROGZONE_H
#define FROGZONE_H

#define NUM_OBSTACLES 100
#define NUM_PLAYERS 4
#define NUM_ITEMS 2

#define HEIGHT 64
#define WIDTH 64

struct Coord {
    unsigned char x;
    unsigned char y;

    bool operator==(Coord rhs) {
        return (x == rhs.x) && (y == rhs.y);
    }
};

enum Direction {
    Up,
    Down,
    Left,
    Right,
};

struct Obstacles100 {
    Coord values[NUM_OBSTACLES];
};

struct PlayerData {
    Coord loc;
    unsigned char hp;
    unsigned char atk;
};


struct PlayerWithId {
    unsigned char id;
    PlayerData data;
};

struct ItemData {
    Coord loc;
    unsigned char hp;
    unsigned char atk;
    bool is_consumed;
};

struct ItemWithId {
    unsigned char id;
    ItemData data;
};

struct Items {
    ItemData values[NUM_ITEMS];
};

struct ItemsWithId {
    ItemWithId values[NUM_ITEMS];
};

struct ApplyMoveOut {
    PlayerData player_data;
    Items items;
};

struct PlayersWithId {
    PlayerWithId values[NUM_PLAYERS];
};

enum EntityType {
    None,
    Player,
    Item,
    Monster,
    Invalid,
};

struct CellData {
    EntityType entity_type;
    unsigned char entity_id;
    unsigned char hp;
    unsigned char atk;

    CellData() {
        entity_type = None;
        entity_id = 0;
        hp = 0;
        atk = 0;
    }
};

ApplyMoveOut apply_move(
                        PlayerData player_data,
                        Direction direction,
                        Obstacles100 obstacles,
                        Items items);

CellData get_cell(
                  Coord player_coord,
                  Coord query_coord,
                  ItemsWithId items,
                  PlayersWithId players
                  );

#endif  // FROGZONE_H
