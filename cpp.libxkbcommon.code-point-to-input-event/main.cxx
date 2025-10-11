import standard;

// unicode_to_input_events.cpp
// g++ -std=c++17 -lxkbcommon
#include <xkbcommon/xkbcommon.h>
#include <linux/input.h>    // input_event, KEY_*, EV_KEY, EV_SYN, SYN_REPORT
#include <sys/time.h>
#include <vector>
#include <cstring>
#include <iostream>

// Heuristic mapping from XKB modifier name to an evdev KEY_* code.
// This is a best-effort mapping; adjust for your environment if needed.
static int mod_name_to_evdev_key(const char *modname) {
    if (!modname) return -1;
    // prefer substring checks because mod names can be "Shift", "Control", "Mod1", "ISO_Level3_Shift", etc.
    if (strstr(modname, "Shift"))        return KEY_LEFTSHIFT;
    if (strstr(modname, "Control") || strstr(modname, "Ctrl")) return KEY_LEFTCTRL;
    if (strstr(modname, "Alt") && !strstr(modname, "AltGr"))  return KEY_LEFTALT;
    if (strcmp(modname, "Mod1") == 0)    return KEY_LEFTALT;   // common convention
    if (strcmp(modname, "Mod4") == 0)    return KEY_LEFTMETA;  // common (Super/Win)
    if (strstr(modname, "Super") || strstr(modname, "Meta"))  return KEY_LEFTMETA;
    // Level3/Mode_switch/ISO_Level3_Shift usually correspond to AltGr / Right Alt
    if (strstr(modname, "Level3") || strstr(modname, "Mode_switch") || strstr(modname, "ISO_Level3")) return KEY_RIGHTALT;
    if (strstr(modname, "Hyper"))        return KEY_LEFTMETA;  // fallback
    if (strstr(modname, "Lock") || strstr(modname, "Caps"))   return KEY_CAPSLOCK;
    // unknown mapping
    return -1;
}

// Main function: given a compiled xkb_keymap and a Unicode codepoint (UTF-32),
// return a vector of input_event that, when injected to the kernel input stream,
// will synthesize the corresponding keypress (modifiers + key + releases + SYN).
std::vector<input_event> unicode_to_input_events(struct xkb_keymap *keymap, uint32_t ucs) {
    std::vector<input_event> out;

    if (!keymap) return out;

    // 1) convert unicode -> keysym
    xkb_keysym_t keysym = xkb_utf32_to_keysym(ucs);
    if (keysym == XKB_KEY_NoSymbol) {
        // no corresponding keysym
        return out;
    }

    // get time for events
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    // 2) search for a keycode + layout + level producing this keysym
    xkb_keycode_t min_k = xkb_keymap_min_keycode(keymap);
    xkb_keycode_t max_k = xkb_keymap_max_keycode(keymap);

    bool found = false;
    xkb_keycode_t found_key = 0;
    xkb_layout_index_t found_layout = 0;
    xkb_level_index_t found_level = 0;
    xkb_mod_mask_t masks[16];
    size_t nmasks = 0;

    for (xkb_keycode_t kc = min_k; kc <= max_k && !found; ++kc) {
        xkb_layout_index_t nlayouts = xkb_keymap_num_layouts_for_key(keymap, kc);
        for (xkb_layout_index_t layout = 0; layout < nlayouts && !found; ++layout) {
            xkb_level_index_t nlevels = xkb_keymap_num_levels_for_key(keymap, kc, layout);
            for (xkb_level_index_t level = 0; level < nlevels && !found; ++level) {
                const xkb_keysym_t *syms_out = nullptr;
                int nsyms = xkb_keymap_key_get_syms_by_level(keymap, kc, layout, level, &syms_out);
                if (nsyms <= 0 || !syms_out) continue;
                for (int i = 0; i < nsyms; ++i) {
                    if (syms_out[i] == keysym) {
                        // found
                        found = true;
                        found_key = kc;
                        found_layout = layout;
                        found_level = level;
                        // get modifier masks that produce this level (may be multiple combinations)
                        nmasks = xkb_keymap_key_get_mods_for_level(keymap, kc, layout, level, masks, sizeof(masks)/sizeof(masks[0]));
                        break;
                    }
                }
            }
        }
    }

    if (!found) {
        // Couldn't find a key that produces the keysym in this keymap.
        return out;
    }

    // Pick a mask (if any). If no mask, treat as "no modifiers required".
    xkb_mod_mask_t chosen_mask = (nmasks > 0) ? masks[0] : 0;

    // expand chosen_mask into modifier key evdev codes (heuristic)
    std::vector<int> modifier_evdev_keys;
    xkb_mod_index_t nmods = xkb_keymap_num_mods(keymap);
    for (xkb_mod_index_t midx = 0; midx < nmods; ++midx) {
        if (chosen_mask & ( (xkb_mod_mask_t)1 << midx )) {
            const char *mname = xkb_keymap_mod_get_name(keymap, midx);
            int ev = mod_name_to_evdev_key(mname);
            if (ev >= 0) modifier_evdev_keys.push_back(ev);
            // if ev == -1 we simply skip (we cannot synthesize unknown modifier)
        }
    }

    // Helper to make input_event with current timestamp
    auto make_ev = [&](__u16 type, __u16 code, __s32 value) {
        input_event ev{};
        ev.input_event_sec = tv.tv_sec;
        ev.input_event_usec = tv.tv_usec;
        ev.type = type;
        ev.code = code;
        ev.value = value;
        return ev;
    };

    // 3) emit modifier presses (value = 1)
    for (int m : modifier_evdev_keys) {
        out.push_back(make_ev(EV_KEY, static_cast<__u16>(m), 1));
    }

    // 4) emit main key press/release
    // Convert xkb keycode -> evdev keycode: usually xkb keycode == evdev KEY_* + 8 (X11 offset).
    // So subtract 8 if possible. This mirrors common usage in libxkbcommon docs.
    const int XKB_X11_OFFSET = 8;
    int evdev_code = static_cast<int>(found_key);
    if (evdev_code >= XKB_X11_OFFSET) evdev_code -= XKB_X11_OFFSET;

    if (evdev_code <= 0 || evdev_code > 0xFFFF) {
        // invalid evdev code (shouldn't normally happen)
        // clean up by releasing modifiers and return
        for (auto it = modifier_evdev_keys.rbegin(); it != modifier_evdev_keys.rend(); ++it) {
            out.push_back(make_ev(EV_KEY, static_cast<__u16>(*it), 0));
        }
        out.push_back(make_ev(EV_SYN, SYN_REPORT, 0));
        return out;
    }

    // press main key
    out.push_back(make_ev(EV_KEY, static_cast<__u16>(evdev_code), 1));
    // release main key
    out.push_back(make_ev(EV_KEY, static_cast<__u16>(evdev_code), 0));

    // 5) release modifiers (reverse order)
    for (auto it = modifier_evdev_keys.rbegin(); it != modifier_evdev_keys.rend(); ++it) {
        out.push_back(make_ev(EV_KEY, static_cast<__u16>(*it), 0));
    }

    // 6) SYN
    out.push_back(make_ev(EV_SYN, SYN_REPORT, 0));

    return out;
}


// Small usage example (not injecting into /dev/uinput here)
int main() {
    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(ctx, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);

    uint32_t codepoint = 'A'; // U+0041
    auto evs = unicode_to_input_events(keymap, codepoint);
    for (const auto &e : evs) {
        std::cout << "type=" << e.type << " code=" << e.code << " value=" << e.value << "\n";
    }

    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    return 0;
}

