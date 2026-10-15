/* SPDX-License-Identifier: Apache-2.0 */
#define TFB_ACTION_NONE 0
#define TFB_ACTION_BOOT_ANDROID_SLOT_A 1
#define TFB_ACTION_BOOT_ANDROID_SLOT_B 2
#define TFB_ACTION_BOOT_ALTERNATE_OS 3
#define TFB_ACTION_BOOT_ROOTED_ANDROID 4
#define TFB_ACTION_INSTALL_ROOT 5
#define TFB_ACTION_UNINSTALL_ROOT 6
#define TFB_ACTION_TOGGLE_ROOT_PERSISTENCE 7
#define TFB_ACTION_REBOOT_BOOTLOADER 8
#define TFB_ACTION_REBOOT_RECOVERY 9
#define TFB_ACTION_BACK 10
#define TFB_ACTION_SET_BOOT_TARGET_SLOT_A 11
#define TFB_ACTION_SET_BOOT_TARGET_SLOT_B 12
#define TFB_ACTION_LIVE_CONSOLE 13
#define TFB_ACTION_RESTART_ADB_USB 14
#define TFB_ACTION_NOT_IMPLEMENTED 15
#define TFB_ACTION_BOOT_DIAGNOSTICS 16
#define TFB_ACTION_REBOOT_SYSTEM 17

#define TFB_CONDITION_ALWAYS 0
#define TFB_CONDITION_FULL_AB 1
#define TFB_CONDITION_ALTERNATE_OS_CONFIGURED 2
#define TFB_CONDITION_ROOT_INSTALLED 3
#define TFB_CONDITION_ROOT_NOT_INSTALLED 4

#define TFB_LABEL_STATE_NONE 0
#define TFB_LABEL_STATE_ROOT_PERSISTENCE 1
#define TFB_LABEL_STATE_ALTERNATE_OS_NAME 2

#define TFB_MENU_ICON_NONE 0
#define TFB_MENU_ICON_ANDROID 1
#define TFB_MENU_ICON_RECOVERY 2
#define TFB_MENU_ICON_MAINTENANCE 3
#define TFB_MENU_ICON_ALTERNATE_OS 4
#define TFB_MENU_ICON_ROOT 5
#define TFB_MENU_ICON_PIXEL_PARTITIONER 6
#define TFB_MENU_ICON_BOOTLOADER 7
#define TFB_MENU_ICON_BACK 8

#define TFB_MENU_STACK_DEPTH 4


struct tfb_menu_page;


struct tfb_menu_entry {
    const char *label;
    const char *description;
    int icon;
    int action;
    const struct tfb_menu_page *submenu;
    int condition;
    int label_state;
};


struct tfb_menu_page {
    const char *title;
    const char *subtitle;
    u32 timeout_ms;
    int timeout_action;
    int default_entry_index;
    const struct tfb_menu_entry *entries;
    int entry_count;
};


struct tfb_menu_runtime_state {
    int full_ab;
    int alternate_os_configured;

    /*
     * Presentation label retained for the menu contract.
     */
    char alternate_os_name[64];

    /*
     * Identity discovered directly from treeforge_os.
     *
     * These fields are read-only runtime observations. They are not
     * installation authority and are never persisted by Bootstrap.
     */
    char alternate_os_detected_name[96];
    char alternate_os_version[96];
    char alternate_os_build[96];
    char alternate_os_milestone[64];
    char alternate_os_board[96];

    int root_installed;
    int root_persistence_enabled;
};


static struct tfb_menu_runtime_state
tfb_menu_state = {
    0,
    0,
    "Alternate OS",
    "",
    "",
    "",
    "",
    "",
    0,
    0,
};


static const char *tfb_menu_status = 0;

static char **tfb_runtime_envp = 0;


__TREEFORGE_MENU_PROFILE__



static int tfb_menu_read_state_text(
    const char *path,
    char *buffer,
    usize capacity
) {
    if (
        !path
        || !buffer
        || capacity < 2
    ) {
        return 0;
    }

    long fd = tfb_open(
        path,
        O_RDONLY
        | O_NONBLOCK
    );

    if (fd < 0) {
        return 0;
    }

    long amount = tfb_syscall3(
        SYS_READ,
        fd,
        (long) buffer,
        (long) (
            capacity - 1
        )
    );

    tfb_close(
        fd
    );

    if (amount <= 0) {
        return 0;
    }

    usize length = (
        (usize) amount
    );

    if (length >= capacity) {
        length = capacity - 1;
    }

    buffer[length] = '\0';

    for (
        usize index = 0;
        index < length;
        index++
    ) {
        unsigned char value = (
            (unsigned char)
            buffer[index]
        );

        if (
            value == '\n'
            || value == '\r'
        ) {
            buffer[index] = '\0';
            length = index;
            break;
        }

        if (
            value < 0x20
            || value > 0x7e
        ) {
            buffer[0] = '\0';
            return 0;
        }
    }

    return length > 0;
}


/*
 * TFB_FULL_AB_GEOMETRY_DETECTION_V17_1A
 *
 * Current release support is explicitly tangorpro. Its canonical
 * factory GPT places super at sda25 with 16,662,528 512-byte
 * sectors. TreeForge Full A/B grows that physical partition.
 *
 * Keep the metadata marker as an explicit affirmative contract,
 * but do not require it for a converted device.
 */
#define TFB_TANGORPRO_STOCK_SUPER_SECTORS 16662528ULL


static int tfb_ui_read_u64(
    const char *path,
    u64 *output
);


/*
 * TFB_ALTROOT_INSTALLED_PROBE_V1
 *
 * Implemented with the accepted storage-backed alternate-root
 * helpers later in this translation unit.
 */
static int tfb_altroot_installed(
    void
);


static void tfb_menu_load_runtime_state(
    void
) {
    int full_ab_by_marker = 0;

    long full_ab = tfb_open(
        "/metadata/treeforge-bootstrap/full-ab",
        O_RDONLY
        | O_NONBLOCK
    );

    if (full_ab >= 0) {
        full_ab_by_marker = 1;

        tfb_close(
            full_ab
        );
    }

    u64 super_sectors = 0;

    int full_ab_by_geometry = (
        tfb_ui_read_u64(
            "/sys/class/block/sda25/size",
            &super_sectors
        )
        && super_sectors
            > TFB_TANGORPRO_STOCK_SUPER_SECTORS
    );

    tfb_menu_state.full_ab = (
        full_ab_by_marker
        || full_ab_by_geometry
    );

    /*
     * TFB_ALTROOT_DYNAMIC_INVENTORY_V1
     *
     * Installation state belongs to the storage-backed OS itself.
     * The metadata name is optional presentation metadata and must
     * never be the installed/not-installed authority.
     */
    tfb_menu_state
        .alternate_os_configured =
        tfb_altroot_installed();

    char alternate_name[64];

    if (
        tfb_menu_read_state_text(
            "/metadata/treeforge-bootstrap/"
            "alternate-os-name",
            alternate_name,
            sizeof(alternate_name)
        )
    ) {
        usize index = 0;

        while (
            alternate_name[index] != '\0'
            && index
                < sizeof(
                    tfb_menu_state
                    .alternate_os_name
                ) - 1
        ) {
            tfb_menu_state
                .alternate_os_name[index] =
                alternate_name[index];

            index++;
        }

        tfb_menu_state
            .alternate_os_name[index] =
            '\0';
    } else {
        static const char fallback[] =
            "Alternate OS";

        usize index = 0;

        while (
            fallback[index] != '\0'
            && index
                < sizeof(
                    tfb_menu_state
                    .alternate_os_name
                ) - 1
        ) {
            tfb_menu_state
                .alternate_os_name[index] =
                fallback[index];

            index++;
        }

        tfb_menu_state
            .alternate_os_name[index] =
            '\0';
    }

}


static int tfb_menu_condition_visible(
    int condition
) {
    if (
        condition
        == TFB_CONDITION_ALWAYS
    ) {
        return 1;
    }

    if (
        condition
        == TFB_CONDITION_FULL_AB
    ) {
        return (
            tfb_menu_state
            .full_ab
            != 0
        );
    }

    if (
        condition
        == TFB_CONDITION_ALTERNATE_OS_CONFIGURED
    ) {
        return (
            tfb_menu_state
            .alternate_os_configured
            != 0
        );
    }

    if (
        condition
        == TFB_CONDITION_ROOT_INSTALLED
    ) {
        return (
            tfb_menu_state
            .root_installed
            != 0
        );
    }

    if (
        condition
        == TFB_CONDITION_ROOT_NOT_INSTALLED
    ) {
        return (
            tfb_menu_state
            .root_installed
            == 0
        );
    }

    return 0;
}



static int tfb_menu_visible_count(
    const struct tfb_menu_page *page
) {
    if (!page) {
        return 0;
    }

    int count = 0;

    for (
        int index = 0;
        index < page->entry_count;
        index++
    ) {
        if (
            tfb_menu_condition_visible(
                page->entries[
                    index
                ].condition
            )
        ) {
            count++;
        }
    }

    return count;
}


static const struct tfb_menu_entry *
tfb_menu_visible_entry(
    const struct tfb_menu_page *page,
    int visible_index
) {
    if (
        !page
        || visible_index < 0
    ) {
        return 0;
    }

    int visible = 0;

    for (
        int index = 0;
        index < page->entry_count;
        index++
    ) {
        const struct tfb_menu_entry *entry = (
            &page->entries[
                index
            ]
        );

        if (
            !tfb_menu_condition_visible(
                entry->condition
            )
        ) {
            continue;
        }

        if (
            visible
            == visible_index
        ) {
            return entry;
        }

        visible++;
    }

    return 0;
}


static int tfb_menu_default_selection(
    const struct tfb_menu_page *page
) {
    if (!page) {
        return 0;
    }

    int visible = 0;

    for (
        int index = 0;
        index < page->entry_count;
        index++
    ) {
        const struct tfb_menu_entry *entry = (
            &page->entries[
                index
            ]
        );

        if (
            !tfb_menu_condition_visible(
                entry->condition
            )
        ) {
            continue;
        }

        if (
            index
            == page->default_entry_index
        ) {
            return visible;
        }

        visible++;
    }

    return 0;
}


static int tfb_menu_move_selection(
    const struct tfb_menu_page *page,
    int selected,
    int delta
) {
    int count = (
        tfb_menu_visible_count(
            page
        )
    );

    if (count <= 0) {
        return 0;
    }

    if (
        selected < 0
        || selected >= count
    ) {
        selected = 0;
    }

    selected += delta;

    while (selected < 0) {
        selected += count;
    }

    while (selected >= count) {
        selected -= count;
    }

    return selected;
}


static const char *tfb_menu_entry_label(
    const struct tfb_menu_entry *entry
) {

    if (
        entry->label_state
        == TFB_LABEL_STATE_ALTERNATE_OS_NAME
    ) {
        return (
            tfb_menu_state
            .alternate_os_name
        );
    }


    if (!entry) {
        return "";
    }

    if (
        entry->label_state
        != TFB_LABEL_STATE_ROOT_PERSISTENCE
    ) {
        return entry->label;
    }

    static char state_label[96];

    usize position = 0;

    while (
        entry->label[
            position
        ] != '\0'
        && position < 88
    ) {
        state_label[
            position
        ] = entry->label[
            position
        ];

        position++;
    }

    const char *suffix = (
        tfb_menu_state
        .root_persistence_enabled
        ? " [ON]"
        : " [OFF]"
    );

    usize suffix_index = 0;

    while (
        suffix[
            suffix_index
        ] != '\0'
        && position < 95
    ) {
        state_label[
            position
        ] = suffix[
            suffix_index
        ];

        position++;
        suffix_index++;
    }

    state_label[
        position
    ] = '\0';

    return state_label;
}


static const char *tfb_menu_action_name(
    int action
) {
    if (
        action
        == TFB_ACTION_BOOT_ANDROID_SLOT_A
    ) {
        return "boot_android_slot_a";
    }

    if (
        action
        == TFB_ACTION_BOOT_ANDROID_SLOT_B
    ) {
        return "boot_android_slot_b";
    }

    if (
        action
        == TFB_ACTION_BOOT_ALTERNATE_OS
    ) {
        return "boot_alternate_os";
    }

    if (
        action
        == TFB_ACTION_BOOT_DIAGNOSTICS
    ) {
        return "boot_diagnostics";
    }

    if (
        action
        == TFB_ACTION_BOOT_ROOTED_ANDROID
    ) {
        return "boot_rooted_android";
    }

    if (
        action
        == TFB_ACTION_INSTALL_ROOT
    ) {
        return "install_root";
    }

    if (
        action
        == TFB_ACTION_UNINSTALL_ROOT
    ) {
        return "uninstall_root";
    }

    if (
        action
        == TFB_ACTION_TOGGLE_ROOT_PERSISTENCE
    ) {
        return "toggle_root_persistence";
    }

    if (
        action
        == TFB_ACTION_REBOOT_SYSTEM
    ) {
        return "reboot_system";
    }

    if (
        action
        == TFB_ACTION_REBOOT_BOOTLOADER
    ) {
        return "reboot_bootloader";
    }

    if (
        action
        == TFB_ACTION_REBOOT_RECOVERY
    ) {
        return "reboot_recovery";
    }

    if (
        action
        == TFB_ACTION_SET_BOOT_TARGET_SLOT_A
    ) {
        return "set_boot_target_slot_a";
    }

    if (
        action
        == TFB_ACTION_SET_BOOT_TARGET_SLOT_B
    ) {
        return "set_boot_target_slot_b";
    }

    if (
        action
        == TFB_ACTION_LIVE_CONSOLE
    ) {
        return "live_console";
    }

    if (
        action
        == TFB_ACTION_RESTART_ADB_USB
    ) {
        return "restart_adb_usb";
    }

    if (
        action
        == TFB_ACTION_NOT_IMPLEMENTED
    ) {
        return "not_implemented";
    }

    if (
        action
        == TFB_ACTION_BACK
    ) {
        return "back";
    }

    return "none";
}




static int tfb_adb_connected(
    void
) {
    long ready = tfb_open(
        "/dev/treeforge-bootstrap-adb-ready",
        O_RDONLY
        | O_NONBLOCK
    );

    if (ready < 0) {
        return 0;
    }

    tfb_close(
        ready
    );

    long fd = tfb_open(
        "/sys/class/udc/"
        "11210000.dwc3/state",
        O_RDONLY
        | O_NONBLOCK
    );

    if (fd < 0) {
        return 0;
    }

    char state[32];

    long amount = tfb_syscall3(
        SYS_READ,
        fd,
        (long) state,
        (long) (
            sizeof(state) - 1
        )
    );

    tfb_close(
        fd
    );

    if (amount <= 0) {
        return 0;
    }

    state[amount] = '\0';

    static const char configured[] =
        "configured";

    for (
        usize index = 0;
        configured[index] != '\0';
        index++
    ) {
        if (
            index >= (usize) amount
            || state[index]
                != configured[index]
        ) {
            return 0;
        }
    }

    return 1;
}


/*
 * TFB_ADB_MANUAL_RESET_V1
 *
 * Retained early-userspace control/status markers shared by the
 * menu dispatcher and standalone ADB service.
 */
#define TFB_ADB_RESET_REQUEST \
    "/dev/treeforge-bootstrap-adb-reset"

#define TFB_ADB_RESTARTING \
    "/dev/treeforge-bootstrap-adb-restarting"

#define TFB_ADB_ERROR_B_SESSION \
    "/dev/treeforge-bootstrap-adb-error-b-session"

#define TFB_ADB_ERROR_UDC \
    "/dev/treeforge-bootstrap-adb-error-udc"

#define TFB_ADB_ERROR_CONFIGFS \
    "/dev/treeforge-bootstrap-adb-error-configfs"

#define TFB_ADB_ERROR_FFS \
    "/dev/treeforge-bootstrap-adb-error-ffs"

#define TFB_ADB_ERROR_ADBD \
    "/dev/treeforge-bootstrap-adb-error-adbd"

#define TFB_ADB_ERROR_LINK \
    "/dev/treeforge-bootstrap-adb-error-link"

#define TFB_ADB_ERROR_BIND \
    "/dev/treeforge-bootstrap-adb-error-bind"

#define TFB_ADB_ERROR_UNKNOWN \
    "/dev/treeforge-bootstrap-adb-error-unknown"


static int tfb_adb_marker_exists(
    const char *path
) {
    long fd = tfb_open(
        path,
        O_RDONLY
        | O_NONBLOCK
    );

    if (fd < 0) {
        return 0;
    }

    tfb_close(
        fd
    );

    return 1;
}


static int tfb_adb_display_state(
    void
) {
    if (
        tfb_adb_marker_exists(
            TFB_ADB_RESTARTING
        )
    ) {
        return 2;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_B_SESSION
        )
    ) {
        return 10;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_UDC
        )
    ) {
        return 11;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_CONFIGFS
        )
    ) {
        return 12;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_FFS
        )
    ) {
        return 13;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_ADBD
        )
    ) {
        return 14;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_LINK
        )
    ) {
        return 15;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_BIND
        )
    ) {
        return 16;
    }

    if (
        tfb_adb_marker_exists(
            TFB_ADB_ERROR_UNKNOWN
        )
    ) {
        return 17;
    }

    /*
     * READY remains tied to the existing complete connection
     * predicate: TreeForge ready marker plus configured UDC.
     */
    if (tfb_adb_connected()) {
        return 1;
    }

    return 0;
}


static const char *tfb_adb_display_text(
    void
) {
    /*
     * TFB_RUNTIME_HEALTH_LABELS_V3
     */
    int state =
        tfb_adb_display_state();

    if (state == 1) {
        return "CONNECTED";
    }

    if (
        state >= 10
        && state <= 17
    ) {
        return "UNAVAILABLE";
    }

    return "DISCONNECTED";
}



static char tfb_pp_live_status[
    256
];


static int tfb_pp_read_status_text(
    const char *path,
    char *buffer,
    unsigned long capacity
) {
    long fd;
    long count;

    if (
        !path
        || !buffer
        || capacity < 2
    ) {
        return 0;
    }

    buffer[0] = 0;

    fd = tfb_open(
        path,
        O_RDONLY
        | O_NONBLOCK
    );

    if (fd < 0) {
        return 0;
    }

    count = tfb_syscall3(
        SYS_READ,
        fd,
        (long) buffer,
        (long) (
            capacity - 1
        )
    );

    tfb_close(
        fd
    );

    if (count <= 0) {
        buffer[0] = 0;
        return 0;
    }

    if (
        (unsigned long) count
        >= capacity
    ) {
        count = (
            (long) capacity
            - 1
        );
    }

    buffer[count] = 0;

    while (
        count > 0
        && (
            buffer[count - 1] == '\n'
            || buffer[count - 1] == '\r'
        )
    ) {
        count--;
        buffer[count] = 0;
    }

    return (
        count > 0
    );
}


static const char *tfb_menu_status_line(
    const struct tfb_menu_page *page,
    u32 elapsed_ms,
    int timeout_fired
) {

    if (
        tfb_pp_read_status_text(
            "/dev/treeforge-bootstrap-pixel-partitioner-status",
            tfb_pp_live_status,
            sizeof(
                tfb_pp_live_status
            )
        )
    ) {
        return tfb_pp_live_status;
    }

    if (tfb_menu_status) {
        return tfb_menu_status;
    }

    if (
        !page
        || page->timeout_ms == 0
        || timeout_fired
    ) {
        return "WAITING FOR SELECTION";
    }

    static char line[48];

    static const char prefix[] =
        "AUTO ACTION IN ";

    usize position = 0;

    while (
        prefix[position] != '\0'
        && position < 40
    ) {
        line[position] =
            prefix[position];

        position++;
    }

    u32 remaining_ms = (
        page->timeout_ms
        > elapsed_ms
        ? page->timeout_ms
            - elapsed_ms
        : 0
    );

    u32 seconds = (
        remaining_ms
        + 999U
    ) / 1000U;

    if (seconds > 99U) {
        seconds = 99U;
    }

    if (seconds >= 10U) {
        line[position++] = (
            (char) (
                '0'
                + (
                    seconds / 10U
                )
            )
        );
    }

    line[position++] = (
        (char) (
            '0'
            + (
                seconds % 10U
            )
        )
    );

    line[position++] = 'S';
    line[position] = '\0';

    return line;
}



/*
 * TFB_LANDSCAPE_MENU_RENDERER_V17
 *
 * Everything below is a TreeForge-owned software presentation layer.
 * tfb_fb_fill_rect() remains the raw 1600x2560 framebuffer primitive.
 */

struct tfb_ui_menu_geometry {
    u32 x;
    u32 y;
    u32 width;
    u32 row_height;
    u32 row_gap;
    int item_count;
};


