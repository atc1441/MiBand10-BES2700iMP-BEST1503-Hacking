// Link stubs for the best1503 (best1306-substituted) build.
//
// The config forces IBRT_UI/IBRT = 0 (no TWS — correct for a single MiBand9
// device) in config/common.mk, yet config/best1503/target.mk still adds
// -DIBRT_UI to the compiler flags, so a handful of IBRT/SBC symbols are
// *referenced* by dead (never-taken) code paths but never *built*.  These weak
// stubs satisfy the linker so the firmware links + boots; none are reached on
// the BLE bring-up path we care about.
//
// Symbol linkage was taken from the actual relocations:
//   nm: 5 are extern "C" (unmangled), 1 is C++
//   (_Z32app_ibrt_search_ui_handle_key_v2P8bdaddr_tP14APP_KEY_STATUSPv).

extern "C" {
    void app_bt_callback_register(void) {}
    void app_bt_manager_ibrt_role_process(void) {}
    void app_ibrt_enter_limited_mode(void) {}
    void app_ibrt_search_ui_init(void) {}
    // referenced as a 256-entry SBC CRC table by audioplayers/plc_utils.
    // NOTE: must be non-const — a C++ `const` global has *internal* linkage
    // (even inside extern "C"), which would not satisfy the external reference.
    unsigned char sbc_crc_tbl[256] = { 0 };
}

// C++ symbol: forward-declared struct names reproduce the exact mangling
// P8bdaddr_t / P14APP_KEY_STATUS / Pv.
struct bdaddr_t;
struct APP_KEY_STATUS;
void app_ibrt_search_ui_handle_key_v2(bdaddr_t *, APP_KEY_STATUS *, void *) {}
