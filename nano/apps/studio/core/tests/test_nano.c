#include "test_harness.h"

#include "retr01_studio/json_io.h"
#include "retr01_studio/project.h"
#include "retr01_studio/types.h"

#include <stdlib.h>
#include <string.h>

TEST_MAIN() {
    R01Project *p = (R01Project *)calloc(1, sizeof(R01Project));
    R01Project *p2 = (R01Project *)calloc(1, sizeof(R01Project));
    char err[128];
    const char *path = "nano_roundtrip.r01proj";

    EXPECT(p != NULL && p2 != NULL, "alloc");
    if (!p || !p2) {
        free(p);
        free(p2);
        return 1;
    }

    EXPECT(R01_BGM_CH_COUNT == 1, "bgm channel count is 1");
    EXPECT(R01_SCREEN_PX_H == 96, "nano playfield height");
    EXPECT(R01_MAX_PRESENT_SCREENS == 16, "present screen cap");

    r01_project_init(p, "nano");
    p->bgm.present = 1;
    p->bgm.track_count = 1;
    strncpy(p->bgm.track_name[0], "Theme", R01_BGM_NAME_MAX - 1);
    p->bgm.region_count[0][0] = 1;
    p->bgm.region[0][0][0].start = 0;
    p->bgm.region[0][0][0].len = 4;
    p->bgm.region[0][0][0].midi = 60;
    strncpy(p->bgm.region[0][0][0].tok, "C4", R01_BGM_TOK_MAX - 1);

    EXPECT(r01_project_save_json(p, path, err, sizeof(err)) == 0, "save json");
    EXPECT(r01_project_load_json(p2, path, err, sizeof(err)) == 0, "load json");
    EXPECT(p2->bgm.present == 1, "bgm present");
    EXPECT(p2->bgm.track_count == 1, "bgm tracks");
    EXPECT(p2->bgm.region_count[0][0] == 1, "music regions");
    EXPECT(p2->bgm.region[0][0][0].len == 4, "region len");
    EXPECT(strcmp(p2->bgm.region[0][0][0].tok, "C4") == 0, "region tok");

    remove(path);
    free(p);
    free(p2);
    TEST_EXIT();
}
