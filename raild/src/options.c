#include "raild.h"

options_t options;

void parse_options(int argc, char **argv) {
    bool sim = false;
    int  lua_init = 0;

    for(int i = 1; i < argc; ++i) {
        char* arg = argv[i];
        if(arg[0] == '-') {
            switch(arg[1]) {
                case 's':
                    sim = true;
                    break;
            }
        } else {
            if(lua_init) {
                printf("Usage: raild <lua_init> [OPTIONS]\n");
                exit(1);
            } else {
                lua_init = i;
            }
        }
    }

    options_t opts = { sim, lua_init };
    options = opts;
}
