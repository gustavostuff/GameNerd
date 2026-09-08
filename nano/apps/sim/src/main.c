#include "retr01_nano_sim/app.h"

#ifdef R01NS_DEFAULT_CART
#define DEFAULT_CART R01NS_DEFAULT_CART
#else
#define DEFAULT_CART "../../output/nano/test.r01nano"
#endif

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : DEFAULT_CART;
    return r01ns_app_run(path);
}
