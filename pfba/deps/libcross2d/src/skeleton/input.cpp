//
// Created by cpasjuste on 13/01/17.
//

#include "input.h"

int Input::Clear(int player) {
    while (true) {
        Player p = Update()[player];
        if (!p.enabled || p.state <= 0
            || p.state & EV_REFRESH || p.state & EV_QUIT) {
            break;
        }
    }
}

void Input::SetJoystickMapping(int player, int *mapping, int deadzone) {
    for (int i = 0; i < KEY_COUNT; i++) {
        players[player].mapping[i] = mapping[i];
    }
    players[player].dead_zone = deadzone;
}

void Input::SetKeyboardMapping(int *mapping) {
    for (int i = 0; i < KEY_COUNT; i++) {
        keyboard.mapping[i] = mapping[i];
    }
}
