#include <linux/input-event-codes.h>
#include <stdint.h>
#include <stdlib.h>

struct keymap_t {
    uint16_t key;
    uint16_t remap;
    char* cmd;
    uint16_t mod1;
    uint16_t mod2;
};

#define MAP(a,b) {KEY_##a, KEY_##b, NULL, 0, 0}
#define EXEC(a,b) {KEY_##a, 0, b, 0, 0}

#define MAP_MOD(a,b,c) {KEY_##a, KEY_##b, NULL, KEY_##c, 0}
#define MAP_MOD2(a,b,c,d) {KEY_##a, KEY_##b, NULL, KEY_##c, KEY_##d}

struct keymap_t map[] = {

};


/*
    How to configure:

    MAP -> Simple remapping, sends escape code for the key specified in the second argument when the trigger key (first argument) is pressed
    EXEC -> Execute command upon key press
    MAP_MOD -> Map with modifiers, first arg is trigger, second is the main key, third is the modifier (shift, ctrl, alt, etc)
    MAP_MOD2 -> Map with 2 modifiers, same as above, 4th arg is second modifier

    Example configuration

    struct keymap_t map[] = {
    MAP(F1, F13),
    MAP(F2, F14),
    MAP(F3, F15),
    MAP(F4, F16),
    MAP(F5, F17),
    MAP(F6, F18),
    MAP(F7, F19),
    MAP(F8, F20),
    MAP(F9, F21),
    MAP(F10, F22),
    MAP(F11, F23),
    MAP(F12, F24),

    EXEC(Z, "xdg-open https://youtube.com"),
    EXEC(D, "vesktop"),
    EXEC(C, "prismlauncher"),
    EXEC(T, "kitty"),

    EXEC(G, "playerctl play"),
    EXEC(H, "playerctl pause"),
    
    
    MAP_MOD(V, Q, LEFTMETA)
    };


*/