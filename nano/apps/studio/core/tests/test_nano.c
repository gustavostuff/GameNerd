#include "test_harness.h"

#include "retr01_studio/entities.h"
#include "retr01_studio/json_io.h"
#include "retr01_studio/project.h"
#include "retr01_studio/types.h"

#include <stdlib.h>
#include <string.h>

TEST_MAIN() {
    R01Project *p = (R01Project *)calloc(1, sizeof(R01Project));
    R01Project *p2 = (R01Project *)calloc(1, sizeof(R01Project));
    R01EntityType ent;
    int bank = -1;
    int tile = -1;
    char err[128];
    const char *path = "nano_roundtrip.r01proj";

    EXPECT(p != NULL && p2 != NULL, "alloc");
    if (!p || !p2) {
        free(p);
        free(p2);
        return 1;
    }

    /* Audio model: Nano authors one Music channel into project.bgm. */
    EXPECT(R01_BGM_CH_COUNT == 1, "bgm channel count is 1");
    EXPECT(R01_ENTITY_STATES_MAX == 4, "four entity states");
    EXPECT(R01_ENTITY_HITBOX_W == 8 && R01_ENTITY_HITBOX_H == 8, "tile-sized hitbox");

    r01_entity_type_init(&ent);
    EXPECT(ent.state_count == 1, "default one state");
    EXPECT(r01_entity_nano_set_state_tile(&ent, 0, 2, 17) == 0, "set idle tile");
    EXPECT(r01_entity_nano_get_state_tile(&ent, 0, &bank, &tile) == 0, "get idle tile");
    EXPECT(bank == 2 && tile == 17, "idle bank/tile");

    /* Multi-part / multi-frame junk collapses to Nano shape. */
    {
        R01EntityPart junk;
        memset(&junk, 0, sizeof(junk));
        junk.bank = 1;
        junk.tile_id = 3;
        junk.dx = 8;
        junk.dy = 8;
        EXPECT(r01_entity_ensure_frame(&ent, 0, 2) != NULL, "ensure extra frames");
        EXPECT(ent.states[0].frame_count == 3, "three frames before normalize");
        EXPECT(r01_entity_frame_add_part(&ent.states[0].frames[0], &junk) >= 0, "add second part");
        EXPECT(ent.states[0].frames[0].part_count == 2, "two parts before normalize");
        ent.states[0].origin_x = 4;
        ent.states[0].origin_y = 4;
        ent.states[0].frames[0].parts[0].pal = 5;
        r01_entity_nano_normalize(&ent);
        EXPECT(ent.states[0].frame_count == 1, "one frame after normalize");
        EXPECT(ent.states[0].frames[0].part_count == 1, "one part after normalize");
        EXPECT(ent.states[0].frames[0].parts[0].dx == 0, "dx cleared");
        EXPECT(ent.states[0].frames[0].parts[0].dy == 0, "dy cleared");
        EXPECT(ent.states[0].frames[0].parts[0].pal == 5, "fg color preserved");
        EXPECT(ent.states[0].origin_x == 0 && ent.states[0].origin_y == 0, "origin cleared");
        EXPECT(ent.states[0].hitbox_w == 8 && ent.states[0].hitbox_h == 8, "hitbox full tile");
    }

    EXPECT(r01_entity_nano_set_state_tile(&ent, 1, 0, 5) == 0, "set walk tile");
    EXPECT(ent.state_count == 2, "two states after set");
    EXPECT(r01_entity_nano_get_state_tile(&ent, 1, &bank, &tile) == 0, "get walk tile");
    EXPECT(bank == 0 && tile == 5, "walk bank/tile");
    EXPECT(strcmp(ent.states[1].name, "Walk") == 0, "default Walk name");

    /* Project BGM + entity JSON roundtrip stays on one audio channel. */
    r01_project_init(p, "nano");
    p->bgm.present = 1;
    p->bgm.track_count = 1;
    strncpy(p->bgm.track_name[0], "Theme", R01_BGM_NAME_MAX - 1);
    p->bgm.region_count[0][0] = 1;
    p->bgm.region[0][0][0].start = 0;
    p->bgm.region[0][0][0].len = 4;
    p->bgm.region[0][0][0].midi = 60;
    strncpy(p->bgm.region[0][0][0].tok, "C4", R01_BGM_TOK_MAX - 1);

    {
        int idx = r01_world_entity_add(&p->worlds[0]);
        EXPECT(idx == 0, "entity add");
        p->worlds[0].entities[0] = ent;
    }

    EXPECT(r01_project_save_json(p, path, err, sizeof(err)) == 0, "save json");
    EXPECT(r01_project_load_json(p2, path, err, sizeof(err)) == 0, "load json");
    EXPECT(p2->bgm.present == 1, "bgm present");
    EXPECT(p2->bgm.track_count == 1, "bgm tracks");
    EXPECT(p2->bgm.region_count[0][0] == 1, "music regions");
    EXPECT(p2->bgm.region[0][0][0].len == 4, "region len");
    EXPECT(strcmp(p2->bgm.region[0][0][0].tok, "C4") == 0, "region tok");
    EXPECT(p2->worlds[0].entity_count == 1, "entity count roundtrip");
    EXPECT(r01_entity_nano_get_state_tile(&p2->worlds[0].entities[0], 0, &bank, &tile) == 0,
           "loaded idle tile");
    EXPECT(bank == 2 && tile == 17, "loaded idle values");
    EXPECT(r01_entity_nano_get_state_tile(&p2->worlds[0].entities[0], 1, &bank, &tile) == 0,
           "loaded walk tile");
    EXPECT(bank == 0 && tile == 5, "loaded walk values");
    EXPECT(p2->worlds[0].entities[0].states[0].frame_count == 1, "loaded one frame");

    remove(path);
    free(p);
    free(p2);
    TEST_EXIT();
}
