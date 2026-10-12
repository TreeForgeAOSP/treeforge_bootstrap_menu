/*
 * TFB_V2_STRING_EQUAL_FORWARD_DECL
 *
 * The V2 menu renderer now needs tfb_string_equal() before its
 * existing implementation later in this translation unit.
 *
 * Declaration only. The single canonical implementation remains
 * unchanged.
 */
static int tfb_string_equal(
    const char *,
    const char *
);


static int tfb_ui_landscape_supported(
    void
) {
    return (
        tfb_fb_menu_width
            == TFB_UI_LOGICAL_HEIGHT
        && tfb_fb_menu_height
            == TFB_UI_LOGICAL_WIDTH
    );
}


static int tfb_ui_buffer_begin_region(
    u32 x,
    u32 y,
    u32 width,
    u32 height
) {
#ifndef TFB_FB_BUFFERED_PRESENT_AVAILABLE
    (void) x;
    (void) y;
    (void) width;
    (void) height;
    return 0;
#else
    if (
        !tfb_ui_landscape_supported()
        || width == 0
        || height == 0
        || x >= TFB_UI_LOGICAL_WIDTH
        || y >= TFB_UI_LOGICAL_HEIGHT
    ) {
        return 0;
    }

    if (width > TFB_UI_LOGICAL_WIDTH - x) {
        width = TFB_UI_LOGICAL_WIDTH - x;
    }

    if (height > TFB_UI_LOGICAL_HEIGHT - y) {
        height = TFB_UI_LOGICAL_HEIGHT - y;
    }

    u32 raw_x = y;
    u32 raw_y = (
        TFB_UI_LOGICAL_WIDTH
        - x
        - width
    );

    return tfb_fb_buffer_begin_region(
        raw_x,
        raw_y,
        height,
        width
    );
#endif
}


static int tfb_fb_buffer_begin_full(void) {
#ifdef TFB_FB_BUFFERED_PRESENT_AVAILABLE
    return tfb_fb_buffer_begin_region(
        0,
        0,
        tfb_fb_menu_width,
        tfb_fb_menu_height
    );
#else
    return 0;
#endif
}


static void tfb_ui_buffer_present_if(
    int staged
) {
#ifdef TFB_FB_BUFFERED_PRESENT_AVAILABLE
    if (staged) {
        tfb_fb_buffer_present_region();
    }
#else
    (void) staged;
#endif
}


static void tfb_ui_fill_rect(
    u32 x,
    u32 y,
    u32 width,
    u32 height,
    u32 pixel
) {
    if (
        !tfb_ui_landscape_supported()
        || width == 0
        || height == 0
        || x >= TFB_UI_LOGICAL_WIDTH
        || y >= TFB_UI_LOGICAL_HEIGHT
    ) {
        return;
    }

    if (
        width
        > TFB_UI_LOGICAL_WIDTH - x
    ) {
        width =
            TFB_UI_LOGICAL_WIDTH - x;
    }

    if (
        height
        > TFB_UI_LOGICAL_HEIGHT - y
    ) {
        height =
            TFB_UI_LOGICAL_HEIGHT - y;
    }

    /*
     * Logical camera-top landscape -> native portrait scanout.
     *
     * This is a 90-degree clockwise TreeForge UI transform.
     */
    u32 raw_x =
        y;

    u32 raw_y = (
        TFB_UI_LOGICAL_WIDTH
        - x
        - width
    );

    tfb_fb_fill_rect(
        raw_x,
        raw_y,
        height,
        width,
        pixel
    );
}


static void tfb_ui_outline_rect(
    u32 x,
    u32 y,
    u32 width,
    u32 height,
    u32 thickness,
    u32 pixel
) {
    if (
        thickness == 0
        || width < thickness * 2U
        || height < thickness * 2U
    ) {
        return;
    }

    tfb_ui_fill_rect(
        x,
        y,
        width,
        thickness,
        pixel
    );

    tfb_ui_fill_rect(
        x,
        y + height - thickness,
        width,
        thickness,
        pixel
    );

    tfb_ui_fill_rect(
        x,
        y,
        thickness,
        height,
        pixel
    );

    tfb_ui_fill_rect(
        x + width - thickness,
        y,
        thickness,
        height,
        pixel
    );
}


static void tfb_ui_draw_char(
    u32 x,
    u32 y,
    char value,
    u32 scale,
    u32 pixel
) {

    /*
     * TFB_BOOT_MANAGER_V2E_GLYPH_COVERAGE_V1
     *
     * Normalize lowercase runtime strings into the built-in
     * uppercase 5x7 font and explicitly provide punctuation used by
     * kernel releases, partition names and menu/status strings.
     */
    if (
        value >= 'a'
        && value <= 'z'
    ) {
        value = (
            char
        ) (
            value - 'a' + 'A'
        );
    }

    if (value == '.') {
        tfb_ui_fill_rect(
            x + 2U * scale,
            y + 6U * scale,
            scale,
            scale,
            pixel
        );
        return;
    }

    if (value == ',') {
        tfb_ui_fill_rect(
            x + 2U * scale,
            y + 5U * scale,
            scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + scale,
            y + 6U * scale,
            scale,
            scale,
            pixel
        );
        return;
    }

    if (value == ':') {
        tfb_ui_fill_rect(
            x + 2U * scale,
            y + 2U * scale,
            scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + 2U * scale,
            y + 5U * scale,
            scale,
            scale,
            pixel
        );
        return;
    }

    if (value == '-') {
        tfb_ui_fill_rect(
            x,
            y + 3U * scale,
            5U * scale,
            scale,
            pixel
        );
        return;
    }

    if (value == '_') {
        tfb_ui_fill_rect(
            x,
            y + 6U * scale,
            5U * scale,
            scale,
            pixel
        );
        return;
    }

    if (value == '+') {
        tfb_ui_fill_rect(
            x,
            y + 3U * scale,
            5U * scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + 2U * scale,
            y + scale,
            scale,
            5U * scale,
            pixel
        );
        return;
    }

    if (value == '=') {
        tfb_ui_fill_rect(
            x,
            y + 2U * scale,
            5U * scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x,
            y + 4U * scale,
            5U * scale,
            scale,
            pixel
        );
        return;
    }

    if (value == '/') {
        for (
            u32 index = 0;
            index < 5U;
            index++
        ) {
            tfb_ui_fill_rect(
                x
                    + (
                        4U - index
                    ) * scale,
                y
                    + (
                        index + 1U
                    ) * scale,
                scale,
                scale,
                pixel
            );
        }

        return;
    }

    if (value == '%') {
        tfb_ui_fill_rect(
            x,
            y + scale,
            2U * scale,
            2U * scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + 3U * scale,
            y + 4U * scale,
            2U * scale,
            2U * scale,
            pixel
        );

        for (
            u32 index = 0;
            index < 5U;
            index++
        ) {
            tfb_ui_fill_rect(
                x
                    + (
                        4U - index
                    ) * scale,
                y
                    + (
                        index + 1U
                    ) * scale,
                scale,
                scale,
                pixel
            );
        }

        return;
    }

    if (value == '^') {
        tfb_ui_fill_rect(
            x + 2U * scale,
            y,
            scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + scale,
            y + scale,
            scale,
            scale,
            pixel
        );

        tfb_ui_fill_rect(
            x + 3U * scale,
            y + scale,
            scale,
            scale,
            pixel
        );
        return;
    }

    for (
        int row = 0;
        row < 7;
        row++
    ) {
        unsigned char bits = (
            tfb_fb_glyph_row(
                value,
                row
            )
        );

        for (
            int column = 0;
            column < 5;
            column++
        ) {
            if (
                bits
                & (
                    1U
                    << (
                        4
                        - column
                    )
                )
            ) {
                tfb_ui_fill_rect(
                    x
                        + (
                            (u32) column
                            * scale
                        ),
                    y
                        + (
                            (u32) row
                            * scale
                        ),
                    scale,
                    scale,
                    pixel
                );
            }
        }
    }
}


static void tfb_ui_draw_text(
    u32 x,
    u32 y,
    const char *value,
    u32 scale,
    u32 pixel
) {
    if (
        !value
        || scale == 0
    ) {
        return;
    }

    u32 cursor =
        x;

    for (
        usize index = 0;
        value[index] != '\0';
        index++
    ) {
        if (
            value[index]
            != ' '
        ) {
            tfb_ui_draw_char(
                cursor,
                y,
                value[index],
                scale,
                pixel
            );
        }

        cursor +=
            6U * scale;
    }
}


static void tfb_ui_draw_text_fit(
    u32 x,
    u32 y,
    const char *value,
    u32 maximum_width,
    u32 preferred_scale,
    u32 minimum_scale,
    u32 pixel
) {
    if (!value) {
        return;
    }

    u32 scale =
        preferred_scale;

    while (
        scale > minimum_scale
        && tfb_fb_text_width(
            value,
            scale
        ) > maximum_width
    ) {
        scale--;
    }

    tfb_ui_draw_text(
        x,
        y,
        value,
        scale,
        pixel
    );
}


static void tfb_ui_draw_field(
    u32 x,
    u32 y,
    u32 width,
    const char *label,
    const char *value,
    u32 label_pixel,
    u32 value_pixel
);

static int tfb_ui_marker_exists(
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


static int tfb_ui_read_u64(
    const char *path,
    u64 *output
) {
    char buffer[48];

    if (
        !output
        || !tfb_menu_read_state_text(
            path,
            buffer,
            sizeof(buffer)
        )
    ) {
        return 0;
    }

    u64 value = 0;
    int seen = 0;

    for (
        usize index = 0;
        buffer[index] != '\0';
        index++
    ) {
        char c =
            buffer[index];

        if (
            c < '0'
            || c > '9'
        ) {
            return 0;
        }

        seen = 1;

        value = (
            value * 10ULL
            + (u64) (
                c - '0'
            )
        );
    }

    if (!seen) {
        return 0;
    }

    *output =
        value;

    return 1;
}


static void tfb_ui_u64_text(
    u64 value,
    char *output,
    usize capacity
) {
    if (
        !output
        || capacity < 2
    ) {
        return;
    }

    char reverse[32];
    usize count = 0;

    do {
        reverse[count++] = (
            (char) (
                '0'
                + (
                    value
                    % 10ULL
                )
            )
        );

        value /=
            10ULL;

    } while (
        value > 0
        && count
            < sizeof(reverse)
    );

    usize position = 0;

    while (
        count > 0
        && position + 1
            < capacity
    ) {
        output[position++] =
            reverse[--count];
    }

    output[position] =
        '\0';
}


static void tfb_ui_append(
    char *buffer,
    usize capacity,
    const char *suffix
) {
    if (
        !buffer
        || !suffix
        || capacity < 2
    ) {
        return;
    }

    usize position =
        tfb_strlen(
            buffer
        );

    for (
        usize index = 0;
        suffix[index] != '\0'
        && position + 1
            < capacity;
        index++
    ) {
        buffer[position++] =
            suffix[index];
    }

    buffer[position] =
        '\0';
}


/*
 * TFB_BOOTCONFIG_SLOT_DETECTION_V17_1A
 *
 * Modern Pixel boot parameters live in /proc/bootconfig, e.g.
 *
 *     androidboot.slot_suffix = "_a"
 *
 * Keep /proc/cmdline as a compatibility fallback.
 */
static int tfb_ui_file_contains(
    const char *path,
    const char *needle
) {
    if (
        !path
        || !needle
        || needle[0] == '\0'
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

    usize matched = 0;
    char buffer[512];

    for (;;) {
        long amount = tfb_syscall3(
            SYS_READ,
            fd,
            (long) buffer,
            (long) sizeof(buffer)
        );

        if (amount <= 0) {
            break;
        }

        for (
            long index = 0;
            index < amount;
            index++
        ) {
            char value =
                buffer[index];

            if (
                value
                == needle[matched]
            ) {
                matched++;

                if (
                    needle[matched]
                    == '\0'
                ) {
                    tfb_close(
                        fd
                    );

                    return 1;
                }

                continue;
            }

            matched = (
                value == needle[0]
                ? 1U
                : 0U
            );
        }
    }

    tfb_close(
        fd
    );

    return 0;
}


static const char *tfb_ui_boot_slot(
    void
) {
    if (
        tfb_ui_file_contains(
            "/proc/bootconfig",
            "androidboot.slot_suffix = \"_a\""
        )
        || tfb_ui_file_contains(
            "/proc/bootconfig",
            "androidboot.slot_suffix=\"_a\""
        )
    ) {
        return "A";
    }

    if (
        tfb_ui_file_contains(
            "/proc/bootconfig",
            "androidboot.slot_suffix = \"_b\""
        )
        || tfb_ui_file_contains(
            "/proc/bootconfig",
            "androidboot.slot_suffix=\"_b\""
        )
    ) {
        return "B";
    }

    /*
     * Compatibility fallback for platforms that still expose
     * androidboot.slot_suffix directly in the kernel command line.
     */
    if (
        tfb_ui_file_contains(
            "/proc/cmdline",
            "androidboot.slot_suffix=_a"
        )
    ) {
        return "A";
    }

    if (
        tfb_ui_file_contains(
            "/proc/cmdline",
            "androidboot.slot_suffix=_b"
        )
    ) {
        return "B";
    }

    return "UNKNOWN";
}


static int tfb_ui_menu_geometry(
    const struct tfb_menu_page *page,
    struct tfb_ui_menu_geometry *geometry
) {
    if (
        !page
        || !geometry
    ) {
        return 0;
    }

    int item_count = (
        tfb_menu_visible_count(
            page
        )
    );

    if (item_count <= 0) {
        return 0;
    }

    /*
     * TFB_PIXEL_PARTITIONER_TOUCH_BACK_SYNC_V1
     *
     * Pixel Partitioner has one ordinary actionable entry: Back.
     *
     * The live-log renderer places that card at the bottom of the
     * right panel.  Put the canonical menu geometry there as well,
     * so both framebuffer rendering and tfb_menu_touch_row() use
     * exactly the same rectangle.
     */
    if (
        page->title
        && tfb_string_equal(
            page->title,
            "Pixel Partitioner"
        )
    ) {
        geometry->x =
            TFB_UI_MENU_X;

        geometry->y =
            TFB_PP_BACK_Y;

        geometry->width = (
            TFB_UI_LOGICAL_WIDTH
            - TFB_UI_MENU_X
            - TFB_UI_MENU_RIGHT_MARGIN
        );

        geometry->row_height =
            TFB_PP_BACK_HEIGHT;

        geometry->row_gap =
            0U;

        geometry->item_count =
            item_count;

        return 1;
    }


    /*
     * TFB_COMPONENT_VERSIONS_TOUCH_BACK_V3
     *
     * Component Versions is a viewer, but Back remains a normal
     * full-width touch target directly above the footer.
     */
    if (
        page->title
        && tfb_string_equal(
            page->title,
            "Component Versions"
        )
    ) {
        geometry->x =
            TFB_UI_MENU_X;

        geometry->y =
            1360U;

        geometry->width = (
            TFB_UI_LOGICAL_WIDTH
            - TFB_UI_MENU_X
            - TFB_UI_MENU_RIGHT_MARGIN
        );

        geometry->row_height =
            96U;

        geometry->row_gap =
            0U;

        geometry->item_count =
            item_count;

        return 1;
    }


    /*
     * TFB_BOOT_MANAGER_V2E_PARTITION_GEOMETRY_V1
     *
     * Partition Information is primarily a data viewer.
     * Its single actionable Back card remains fixed at the bottom.
     */
    if (
        page->title
        && tfb_string_equal(
            page->title,
            "Partition Information"
        )
    ) {
        geometry->x =
            TFB_UI_MENU_X;

        geometry->y =
            1320U;

        geometry->width = (
            TFB_UI_LOGICAL_WIDTH
            - TFB_UI_MENU_X
            - TFB_UI_MENU_RIGHT_MARGIN
        );

        geometry->row_height =
            96U;

        geometry->row_gap =
            0U;

        geometry->item_count =
            item_count;

        return 1;
    }


    /*
     * TFB_MENU_ITERATION3_EXACT_V1
     *
     * Home has five logical entries and five full-width rows.
     */
    int layout_count =
        item_count;

    u32 width = (
        TFB_UI_LOGICAL_WIDTH
        - TFB_UI_MENU_X
        - TFB_UI_MENU_RIGHT_MARGIN
    );

    u32 usable_height = (
        TFB_UI_CONTENT_BOTTOM
        - TFB_UI_CONTENT_TOP
    );

    u32 gaps = (
        layout_count > 1
        ? TFB_UI_ROW_GAP
            * (u32) (
                layout_count - 1
            )
        : 0
    );

    u32 row_height = (
        usable_height > gaps
        ? (
            usable_height
            - gaps
        ) / (u32) layout_count
        : 96U
    );

    if (row_height > 180U) {
        row_height = 180U;
    }

    if (row_height < 96U) {
        row_height = 96U;
    }

    u32 total_height = (
        row_height
        * (u32) layout_count
        + gaps
    );

    u32 y = (
        TFB_UI_CONTENT_TOP
        + (
            usable_height
                > total_height
            ? (
                usable_height
                - total_height
            ) / 2U
            : 0
        )
    );

    geometry->x =
        TFB_UI_MENU_X;

    geometry->y =
        y;

    geometry->width =
        width;

    geometry->row_height =
        row_height;

    geometry->row_gap =
        TFB_UI_ROW_GAP;

    geometry->item_count =
        item_count;

    return 1;
}


static void tfb_ui_draw_kv(
    u32 x,
    u32 y,
    const char *label,
    const char *value,
    u32 label_pixel,
    u32 value_pixel
) {
    tfb_ui_draw_text(
        x,
        y,
        label,
        3U,
        label_pixel
    );

    tfb_ui_draw_text_fit(
        x + 410U,
        y,
        value,
        550U,
        3U,
        2U,
        value_pixel
    );
}


static void tfb_ui_draw_boot_state(
    u32 x,
    u32 y,
    const char *label,
    const char *state,
    u32 state_pixel
) {
    tfb_ui_draw_text(
        x,
        y,
        label,
        3U,
        0x00dbe7efU
    );

    tfb_ui_draw_text_fit(
        x + 570U,
        y,
        state,
        390U,
        3U,
        2U,
        state_pixel
    );
}




/*
 * ================================================================
 * TFB_DIRTY_REGION_REDRAW_V17_1C
 * ================================================================
 *
 * The framebuffer bridge exposes a directly mapped scanout.
 * Repainting the complete 1600x2560 surface for every key or touch
 * event causes a visible flash.
 *
 * Full redraw remains correct for initial display and page changes.
 * Stable-page changes repaint only their affected logical regions.
 */

#define TFB_REDRAW_FULL       0x01
#define TFB_REDRAW_SELECTION  0x02
#define TFB_REDRAW_STATUS     0x04
#define TFB_REDRAW_ADB        0x08


/*
 * TFB_BOOT_MANAGER_V2_HOME_CARDS
 *
 * Home keeps the canonical menu-selection model but presents
 * Resume and Boot Options on one visual row.
 *
 * No legacy letter/icon badge box is rendered.
 */
static int tfb_ui_home_page(
    const struct tfb_menu_page *page
) {
    return (
        page
        && page->title
        && tfb_string_equal(
            page->title,
            "Home"
        )
    );
}


static void tfb_ui_draw_settings_glyph(
    u32 x,
    u32 y,
    u32 size,
    int active
) {
    const u32 accent =
        0x0000c7d9U;

    const u32 foreground =
        0x00dbe7efU;

    u32 pixel = (
        active
            ? accent
            : foreground
    );

    if (size < 72U) {
        return;
    }

    u32 cx =
        x + size / 2U;

    u32 cy =
        y + size / 2U;

    /*
     * Simple framebuffer-native settings glyph.
     *
     * This intentionally does not depend on a Unicode
     * gear character existing in the built-in font.
     */
    tfb_ui_outline_rect(
        cx - 23U,
        cy - 23U,
        46U,
        46U,
        5U,
        pixel
    );

    tfb_ui_fill_rect(
        cx - 6U,
        cy - 39U,
        12U,
        13U,
        pixel
    );

    tfb_ui_fill_rect(
        cx - 6U,
        cy + 26U,
        12U,
        13U,
        pixel
    );

    tfb_ui_fill_rect(
        cx - 39U,
        cy - 6U,
        13U,
        12U,
        pixel
    );

    tfb_ui_fill_rect(
        cx + 26U,
        cy - 6U,
        13U,
        12U,
        pixel
    );

    tfb_ui_fill_rect(
        cx - 6U,
        cy - 6U,
        12U,
        12U,
        pixel
    );
}


static void tfb_fb_render_menu_card(
    const struct tfb_menu_page *page,
    int index,
    int active
) {
    if (
        !page
        || index < 0
    ) {
        return;
    }

    struct tfb_ui_menu_geometry geometry;

    if (
        !tfb_ui_menu_geometry(
            page,
            &geometry
        )
        || index >= geometry.item_count
    ) {
        return;
    }

    const u32 card =
        0x00121f2cU;

    const u32 card_selected =
        0x00182736U;

    const u32 border =
        0x00243a4aU;

    const u32 accent =
        0x0000c7d9U;

    const u32 foreground =
        0x00dbe7efU;

    const u32 dim =
        0x008696a3U;

    u32 card_x =
        geometry.x;

    u32 card_width =
        geometry.width;

    u32 visual_row =
        (u32) index;

    /*
     * Boot Options is now a normal full-width Home row.
     * The old settings glyph branch below remains unreachable.
     */
    int settings_card = 0;

    u32 row_y = (
        geometry.y
        + visual_row
            * (
                geometry.row_height
                + geometry.row_gap
            )
    );

    tfb_ui_fill_rect(
        card_x,
        row_y,
        card_width,
        geometry.row_height,
        active
            ? card_selected
            : card
    );

    tfb_ui_outline_rect(
        card_x,
        row_y,
        card_width,
        geometry.row_height,
        active
            ? 4U
            : 2U,
        active
            ? accent
            : border
    );

    if (active) {
        tfb_ui_fill_rect(
            card_x,
            row_y,
            18U,
            geometry.row_height,
            accent
        );
    }

    /*
     * Boot Options is intentionally presented only as
     * the settings square on Home.
     */
    if (settings_card) {
        tfb_ui_draw_settings_glyph(
            card_x,
            row_y,
            geometry.row_height,
            active
        );

        return;
    }

    const struct tfb_menu_entry
        *entry = (
            tfb_menu_visible_entry(
                page,
                index
            )
        );

    if (!entry) {
        return;
    }

    const char *label = (
        tfb_menu_entry_label(
            entry
        )
    );

    const char *description = (
        entry->description
            ? entry->description
            : ""
    );

    /*
     * There is deliberately no call to
     * tfb_ui_draw_menu_icon() here.
     *
     * That removes the old letter/icon boxes.
     */
    u32 text_x =
        card_x + 48U;

    u32 text_width = (
        card_width > 96U
            ? card_width - 96U
            : card_width
    );

    tfb_ui_draw_text_fit(
        text_x,
        row_y + 38U,
        label,
        text_width,
        5U,
        3U,
        active
            ? accent
            : foreground
    );

    tfb_ui_draw_text_fit(
        text_x,
        row_y + 102U,
        description,
        text_width,
        3U,
        2U,
        dim
    );
}


static void tfb_fb_render_selection_delta(
    const struct tfb_menu_page *page,
    int old_selected,
    int new_selected
) {
    if (
        !page
        || old_selected == new_selected
    ) {
        return;
    }

    tfb_fb_render_menu_card(
        page,
        old_selected,
        0
    );

    tfb_fb_render_menu_card(
        page,
        new_selected,
        1
    );
}


static void tfb_fb_render_status_region(
    const struct tfb_menu_page *page,
    u32 elapsed_ms,
    int timeout_fired
) {
    if (!page) {
        return;
    }

    struct tfb_ui_menu_geometry
        geometry;

    if (
        !tfb_ui_menu_geometry(
            page,
            &geometry
        )
    ) {
        return;
    }

    const u32 background =
        0x00060b12U;

    const u32 dim =
        0x008ca0b0U;

    /*
     * Clear only the footer/status strip.
     */
    tfb_ui_fill_rect(
        geometry.x,
        1468U,
        geometry.width,
        76U,
        background
    );

    tfb_ui_draw_text_fit(
        geometry.x,
        1490U,
        tfb_menu_status_line(
            page,
            elapsed_ms,
            timeout_fired
        ),
        geometry.width,
        3U,
        2U,
        dim
    );
}


static void tfb_fb_render_adb_region(
    void
) {
    /*
     * TFB_MENU_SYSTEM_ADB_REGION_R3
     *
     * ADB remains a live dirty-region field. Only its final
     * position changes.
     */
    const u32 x =
        125U;

    const u32 y =
        1290U;

    const u32 width =
        250U;

    const u32 panel =
        0x000c1621U;

    const u32 dim =
        0x008ca0b0U;

    const u32 green =
        0x004ed87dU;

    const u32 warning =
        0x00ff4d5aU;

    tfb_ui_fill_rect(
        x - 8U,
        y - 10U,
        width + 16U,
        66U,
        panel
    );

    int adb_state =
        tfb_adb_display_state();

    tfb_ui_draw_field(
        x,
        y,
        width,
        "ADB",
        tfb_adb_display_text(),
        dim,
        adb_state == 1
            ? green
            : (
                adb_state >= 10
                ? warning
                : dim
            )
    );
}





/*
 * ================================================================
 * TFB_BOOT_MANAGER_V2E_INFORMATION_V1
 * ================================================================
 *
 * Read-only runtime information.
 *
 * Bootstrap owns live facts such as partition presence/size,
 * current slot and bootconfig state.
 *
 * Pixel Partitioner may later overlay installer provenance without
 * changing the viewer's runtime structure.
 */


static int tfb_ui_bootconfig_value(
    const char *key,
    char *output,
    usize capacity
) {
    if (
        !key
        || !output
        || capacity < 2
    ) {
        return 0;
    }

    char buffer[16384];

    long descriptor = tfb_open(
        "/proc/bootconfig",
        O_RDONLY
    );

    if (descriptor < 0) {
        return 0;
    }

    long amount = tfb_syscall3(
        SYS_READ,
        descriptor,
        (long) buffer,
        sizeof(buffer) - 1U
    );

    tfb_close(
        descriptor
    );

    if (amount <= 0) {
        return 0;
    }

    buffer[amount] =
        '\0';

    usize key_length = 0;

    while (
        key[key_length] != '\0'
    ) {
        key_length++;
    }

    usize cursor = 0;

    while (
        cursor < (usize) amount
    ) {
        usize line_start =
            cursor;

        usize line_end =
            line_start;

        while (
            line_end < (usize) amount
            && buffer[line_end] != '\n'
        ) {
            line_end++;
        }

        int matches = 1;

        if (
            line_end - line_start
            < key_length
        ) {
            matches = 0;
        }

        if (matches) {
            for (
                usize index = 0;
                index < key_length;
                index++
            ) {
                if (
                    buffer[
                        line_start + index
                    ] != key[index]
                ) {
                    matches = 0;
                    break;
                }
            }
        }

        if (matches) {
            usize position = (
                line_start
                + key_length
            );

            while (
                position < line_end
                && (
                    buffer[position] == ' '
                    || buffer[position] == '\t'
                )
            ) {
                position++;
            }

            if (
                position < line_end
                && buffer[position] == '='
            ) {
                position++;
            }

            while (
                position < line_end
                && (
                    buffer[position] == ' '
                    || buffer[position] == '\t'
                )
            ) {
                position++;
            }

            if (
                position < line_end
                && buffer[position] == '"'
            ) {
                position++;
            }

            usize out = 0;

            while (
                position < line_end
                && out + 1U < capacity
                && buffer[position] != '"'
                && buffer[position] != '\r'
            ) {
                output[out++] =
                    buffer[position++];
            }

            output[out] =
                '\0';

            return out > 0;
        }

        cursor = (
            line_end < (usize) amount
            ? line_end + 1U
            : line_end
        );
    }

    return 0;
}


static void tfb_ui_draw_field(
    u32 x,
    u32 y,
    u32 width,
    const char *label,
    const char *value,
    u32 label_pixel,
    u32 value_pixel
) {
    tfb_ui_draw_text_fit(
        x,
        y,
        label,
        width,
        2U,
        1U,
        label_pixel
    );

    tfb_ui_draw_text_fit(
        x,
        y + 30U,
        value,
        width,
        3U,
        1U,
        value_pixel
    );
}


struct tfb_ui_partition_row {
    const char *name;
    const char *primary_path;
    const char *fallback_path;
    const char *role;
};


static const struct
tfb_ui_partition_row
tfb_ui_partition_rows[] = {
    {
        "boot_a",
        "/dev/block/by-name/boot_a",
        0,
        "TreeForge Kernel"
    },
    {
        "init_boot_a",
        "/dev/block/by-name/init_boot_a",
        0,
        "Boot Manager"
    },
    {
        "vendor_boot_a",
        "/dev/block/by-name/vendor_boot_a",
        0,
        "Android Support"
    },
    {
        "vendor_kernel_boot_a",
        "/dev/block/by-name/vendor_kernel_boot_a",
        0,
        "Android Support"
    },
    {
        "dtbo_a",
        "/dev/block/by-name/dtbo_a",
        0,
        "Android Support"
    },
    {
        "pvmfw_a",
        "/dev/block/by-name/pvmfw_a",
        0,
        "Android Support"
    },
    {
        "vbmeta_a",
        "/dev/block/by-name/vbmeta_a",
        0,
        "AVB Root"
    },
    {
        "vbmeta_system_a",
        "/dev/block/by-name/vbmeta_system_a",
        0,
        "AVB System"
    },
    {
        "vbmeta_vendor_a",
        "/dev/block/by-name/vbmeta_vendor_a",
        0,
        "AVB Vendor"
    },
    {
        "system_a",
        "/dev/block/mapper/system_a",
        "/dev/block/by-name/system_a",
        "Android Logical"
    },
    {
        "system_dlkm_a",
        "/dev/block/mapper/system_dlkm_a",
        "/dev/block/by-name/system_dlkm_a",
        "Android Logical"
    },
    {
        "system_ext_a",
        "/dev/block/mapper/system_ext_a",
        "/dev/block/by-name/system_ext_a",
        "Android Logical"
    },
    {
        "product_a",
        "/dev/block/mapper/product_a",
        "/dev/block/by-name/product_a",
        "Android Logical"
    },
    {
        "vendor_a",
        "/dev/block/mapper/vendor_a",
        "/dev/block/by-name/vendor_a",
        "Android Logical"
    },
    {
        "vendor_dlkm_a",
        "/dev/block/mapper/vendor_dlkm_a",
        "/dev/block/by-name/vendor_dlkm_a",
        "Android Logical"
    },
    {
        "TREEFORGE OS",
        "/dev/block/sda27",
        0,
        "TreeForge Storage"
    },
};


static int tfb_ui_partition_bytes(
    const char *path,
    u64 *bytes
) {
    if (
        !path
        || !bytes
    ) {
        return 0;
    }

    long descriptor = tfb_open(
        path,
        O_RDONLY
        | O_NONBLOCK
    );

    if (descriptor < 0) {
        return 0;
    }

    long end = tfb_syscall3(
        SYS_LSEEK,
        descriptor,
        0,
        2
    );

    tfb_close(
        descriptor
    );

    if (end <= 0) {
        return 0;
    }

    *bytes = (
        (u64) end
    );

    return 1;
}


static void tfb_ui_partition_size_text(
    const struct tfb_ui_partition_row *row,
    char *output,
    usize capacity
) {
    if (
        !row
        || !output
        || capacity < 2
    ) {
        return;
    }

    u64 bytes = 0;

    int available = (
        tfb_ui_partition_bytes(
            row->primary_path,
            &bytes
        )
    );

    if (
        !available
        && row->fallback_path
    ) {
        available = (
            tfb_ui_partition_bytes(
                row->fallback_path,
                &bytes
            )
        );
    }

    /*
     * TFB_PARTITION_VIEWER_SYSFS_FALLBACK_V1
     *
     * treeforge_os can exist in the kernel block topology before
     * /dev/block/sda27 is materialized. The information viewer is
     * read-only, so sysfs is sufficient authority for presence/size.
     */
    if (
        !available
        && row->primary_path
        && tfb_string_equal(
            row->primary_path,
            "/dev/block/sda27"
        )
    ) {
        u64 sectors = 0;

        if (
            tfb_ui_read_u64(
                "/sys/class/block/sda27/size",
                &sectors
            )
            && sectors > 0
            && sectors
                <= (
                    (~0ULL)
                    / 512ULL
                )
        ) {
            bytes = (
                sectors
                * 512ULL
            );

            available = 1;
        }
    }

    if (!available) {
        static const char missing[] =
            "MISSING";

        usize index = 0;

        while (
            missing[index] != '\0'
            && index + 1U < capacity
        ) {
            output[index] =
                missing[index];

            index++;
        }

        output[index] =
            '\0';

        return;
    }

    if (
        bytes
        >= 1024ULL * 1024ULL
    ) {
        tfb_ui_u64_text(
            (
                bytes
                + 512ULL * 1024ULL
            )
            / (
                1024ULL * 1024ULL
            ),
            output,
            capacity
        );

        tfb_ui_append(
            output,
            capacity,
            " MiB"
        );

        return;
    }

    tfb_ui_u64_text(
        (
            bytes + 512ULL
        ) / 1024ULL,
        output,
        capacity
    );

    tfb_ui_append(
        output,
        capacity,
        " KiB"
    );
}


static int tfb_ui_partition_page(
    const struct tfb_menu_page *page
) {
    return (
        page
        && page->title
        && tfb_string_equal(
            page->title,
            "Partition Information"
        )
    );
}


static void tfb_ui_render_partition_information(
    const struct tfb_ui_menu_geometry *geometry
) {
    if (!geometry) {
        return;
    }

    const u32 foreground =
        0x00dbe7efU;

    const u32 dim =
        0x00758a99U;

    const u32 accent =
        0x0000c7d9U;

    tfb_ui_draw_text(
        geometry->x,
        318U,
        "ACTIVE SLOT",
        2U,
        dim
    );

    tfb_ui_draw_text(
        geometry->x + 190U,
        318U,
        tfb_ui_boot_slot(),
        3U,
        accent
    );

    tfb_ui_draw_text(
        geometry->x + 350U,
        318U,
        "READ ONLY LIVE VIEW",
        2U,
        dim
    );

    const int count = (
        (int) (
            sizeof(
                tfb_ui_partition_rows
            )
            / sizeof(
                tfb_ui_partition_rows[0]
            )
        )
    );

    const int per_column =
        8;

    u32 column_width = (
        geometry->width / 2U
    );

    for (
        int index = 0;
        index < count;
        index++
    ) {
        int column = (
            index / per_column
        );

        int row_index = (
            index % per_column
        );

        u32 x = (
            geometry->x
            + 12U
            + (
                (u32) column
                * column_width
            )
        );

        u32 y = (
            370U
            + (
                (u32) row_index
                * 108U
            )
        );

        char size_text[32];

        size_text[0] =
            '\0';

        tfb_ui_partition_size_text(
            &tfb_ui_partition_rows[
                index
            ],
            size_text,
            sizeof(size_text)
        );

        tfb_ui_draw_text_fit(
            x,
            y,
            tfb_ui_partition_rows[
                index
            ].name,
            column_width - 35U,
            3U,
            2U,
            foreground
        );

        tfb_ui_draw_text_fit(
            x,
            y + 36U,
            size_text,
            150U,
            2U,
            1U,
            accent
        );

        tfb_ui_draw_text_fit(
            x + 165U,
            y + 36U,
            tfb_ui_partition_rows[
                index
            ].role,
            column_width - 205U,
            2U,
            1U,
            dim
        );
    }
}



/*
 * TFB_COMPONENT_VERSIONS_VIEWER_V1
 *
 * Read-only live component inventory.
 *
 * Kernel-owned values come from /sys/kernel/treeforge.
 * Linux ABI remains independently visible through osrelease.
 *
 * Alternate-OS identity is discovered from the installed
 * treeforge_os filesystem. Bootstrap consumes exposed metadata;
 * it does not assign an OS identity itself.
 */
static int tfb_ui_component_versions_page(
    const struct tfb_menu_page *page
) {
    if (
        !page
        || !page->title
    ) {
        return 0;
    }

    static const char expected[] =
        "Component Versions";

    usize index = 0;

    while (
        expected[index] != '\0'
        && page->title[index]
            == expected[index]
    ) {
        index++;
    }

    return (
        expected[index] == '\0'
        && page->title[index] == '\0'
    );
}


static void tfb_ui_render_component_versions(
    const struct tfb_ui_menu_geometry *geometry,
    u32 accent,
    u32 dim,
    u32 foreground
) {
    if (!geometry) {
        return;
    }

    char release[96] =
        "NOT EXPOSED";

    char linux_base[96] =
        "NOT EXPOSED";

    char panthor[96] =
        "NOT EXPOSED";

    char panthor_backport[96] =
        "NOT EXPOSED";

    char drm_backport[96] =
        "NOT EXPOSED";

    char linux_abi[160] =
        "UNKNOWN";

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/release",
        release,
        sizeof(release)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/linux_base",
        linux_base,
        sizeof(linux_base)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/panthor",
        panthor,
        sizeof(panthor)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/panthor_backport",
        panthor_backport,
        sizeof(panthor_backport)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/drm_backport",
        drm_backport,
        sizeof(drm_backport)
    );

    tfb_menu_read_state_text(
        "/proc/sys/kernel/osrelease",
        linux_abi,
        sizeof(linux_abi)
    );

    u32 x =
        geometry->x + 40U;

    u32 y =
        390U;

    tfb_ui_draw_text(
        x,
        y,
        "TREEFORGE",
        4U,
        accent
    );

    y += 70U;

    tfb_ui_draw_kv(
        x,
        y,
        "BOOTSTRAP",
        tfb_bootstrap_version,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "BOOT MANAGER",
        tfb_version,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "RUNTIME ABI",
        tfb_runtime_abi_version,
        dim,
        foreground
    );

    y += 90U;

    tfb_ui_draw_text(
        x,
        y,
        "KERNEL",
        4U,
        accent
    );

    y += 70U;

    tfb_ui_draw_kv(
        x,
        y,
        "TREEFORGE KERNEL",
        release,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "LINUX BASE",
        linux_base,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "KERNEL ABI",
        linux_abi,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "PANTHOR",
        panthor,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "PANTHOR BACKPORT",
        panthor_backport,
        dim,
        foreground
    );

    y += 52U;

    tfb_ui_draw_kv(
        x,
        y,
        "DRM BACKPORT",
        drm_backport,
        dim,
        foreground
    );

    /*
     * ALTERNATE OS
     *
     * Installation state comes from the storage-backed probe.
     * Identity fields come from the mounted OS itself.
     */
    y += 72U;

    tfb_ui_draw_text(
        x,
        y,
        "ALTERNATE OS",
        4U,
        accent
    );

    y += 58U;

    if (
        !tfb_menu_state
            .alternate_os_configured
    ) {
        tfb_ui_draw_kv(
            x,
            y,
            "STATUS",
            "NOT INSTALLED",
            dim,
            foreground
        );
    } else {
        const char *alternate_name = (
            tfb_menu_state
                .alternate_os_detected_name[0]
            ? tfb_menu_state
                .alternate_os_detected_name
            : tfb_menu_state
                .alternate_os_name
        );

        tfb_ui_draw_kv(
            x,
            y,
            "NAME",
            alternate_name,
            dim,
            foreground
        );

        if (
            tfb_menu_state
                .alternate_os_version[0]
        ) {
            y += 46U;

            tfb_ui_draw_kv(
                x,
                y,
                "VERSION",
                tfb_menu_state
                    .alternate_os_version,
                dim,
                foreground
            );
        }

        if (
            tfb_menu_state
                .alternate_os_build[0]
        ) {
            y += 46U;

            tfb_ui_draw_kv(
                x,
                y,
                "BUILD",
                tfb_menu_state
                    .alternate_os_build,
                dim,
                foreground
            );
        }

        if (
            tfb_menu_state
                .alternate_os_milestone[0]
        ) {
            y += 46U;

            tfb_ui_draw_kv(
                x,
                y,
                "MILESTONE",
                tfb_menu_state
                    .alternate_os_milestone,
                dim,
                foreground
            );
        }

        if (
            tfb_menu_state
                .alternate_os_board[0]
        ) {
            y += 46U;

            tfb_ui_draw_kv(
                x,
                y,
                "BOARD",
                tfb_menu_state
                    .alternate_os_board,
                dim,
                foreground
            );
        }
    }

}



/*
 * TFB_DYNAMIC_BOOT_IDENTITY_V3
 *
 * Android release is observed from installed build properties.
 * Alternate-OS identity is supplied by the existing read-only
 * treeforge_os probe.
 */
static void tfb_ui_inventory_set(
    char *output,
    usize capacity,
    const char *value
) {
    if (!output || capacity < 1) {
        return;
    }

    output[0] = '\0';

    if (!value) {
        return;
    }

    usize index = 0;

    while (
        value[index] != '\0'
        && index + 1 < capacity
    ) {
        output[index] = value[index];
        index++;
    }

    output[index] = '\0';
}


static void tfb_ui_inventory_append(
    char *output,
    usize capacity,
    const char *value
) {
    if (
        !output
        || !value
        || capacity < 1
    ) {
        return;
    }

    usize out = 0;

    while (
        out < capacity
        && output[out] != '\0'
    ) {
        out++;
    }

    if (out >= capacity) {
        return;
    }

    usize input = 0;

    while (
        value[input] != '\0'
        && out + 1 < capacity
    ) {
        output[out++] =
            value[input++];
    }

    output[out] = '\0';
}


static int tfb_ui_android_release(
    char *output,
    usize capacity
) {
    if (!output || capacity < 2) {
        return 0;
    }

    output[0] = '\0';

    static const char *paths[] = {
        "/system/system/build.prop",
        "/system/build.prop",
        "/product/build.prop",
        "/vendor/build.prop",
        "/prop.default",
    };

    static const char key[] =
        "ro.build.version.release=";

    usize key_length = 0;

    while (key[key_length] != '\0') {
        key_length++;
    }

    for (
        usize path_index = 0;
        path_index
            < sizeof(paths)
                / sizeof(paths[0]);
        path_index++
    ) {
        long fd = tfb_open(
            paths[path_index],
            O_RDONLY | O_NONBLOCK
        );

        if (fd < 0) {
            continue;
        }

        char data[8192];

        long amount = tfb_syscall3(
            SYS_READ,
            fd,
            (long) data,
            sizeof(data) - 1
        );

        tfb_close(fd);

        if (amount <= 0) {
            continue;
        }

        usize length =
            (usize) amount;

        if (length >= sizeof(data)) {
            length =
                sizeof(data) - 1;
        }

        data[length] = '\0';

        usize position = 0;

        while (position < length) {
            usize begin =
                position;

            usize end =
                begin;

            while (
                end < length
                && data[end] != '\n'
                && data[end] != '\r'
            ) {
                end++;
            }

            position = end;

            while (
                position < length
                && (
                    data[position] == '\n'
                    || data[position] == '\r'
                )
            ) {
                position++;
            }

            if (
                end - begin
                <= key_length
            ) {
                continue;
            }

            usize key_index = 0;

            while (
                key_index < key_length
                && data[
                    begin + key_index
                ] == key[key_index]
            ) {
                key_index++;
            }

            if (
                key_index
                != key_length
            ) {
                continue;
            }

            usize source =
                begin + key_length;

            usize destination = 0;

            while (
                source < end
                && destination + 1
                    < capacity
            ) {
                unsigned char value =
                    (unsigned char)
                    data[source++];

                if (
                    value < 0x20
                    || value > 0x7e
                ) {
                    output[0] = '\0';
                    return 0;
                }

                output[destination++] =
                    (char) value;
            }

            output[destination] =
                '\0';

            return destination > 0;
        }
    }

    return 0;
}


static void tfb_ui_android_inventory(
    int slot_b,
    char *output,
    usize capacity
) {
    if (
        slot_b
        && !tfb_menu_state.full_ab
    ) {
        tfb_ui_inventory_set(
            output,
            capacity,
            "NOT INSTALLED"
        );

        return;
    }

    char release[32];

    if (
        tfb_ui_android_release(
            release,
            sizeof(release)
        )
    ) {
        tfb_ui_inventory_set(
            output,
            capacity,
            "ANDROID "
        );

        tfb_ui_inventory_append(
            output,
            capacity,
            release
        );

        return;
    }

    tfb_ui_inventory_set(
        output,
        capacity,
        "INSTALLED"
    );
}


static void tfb_ui_alternate_inventory(
    char *output,
    usize capacity
) {
    if (
        !tfb_menu_state
            .alternate_os_configured
    ) {
        tfb_ui_inventory_set(
            output,
            capacity,
            "NOT INSTALLED"
        );

        return;
    }

    const char *name = (
        tfb_menu_state
            .alternate_os_detected_name[0]
        ? tfb_menu_state
            .alternate_os_detected_name
        : tfb_menu_state
            .alternate_os_name
    );

    tfb_ui_inventory_set(
        output,
        capacity,
        name
    );

    if (
        tfb_menu_state
            .alternate_os_version[0]
    ) {
        tfb_ui_inventory_append(
            output,
            capacity,
            " "
        );

        tfb_ui_inventory_append(
            output,
            capacity,
            tfb_menu_state
                .alternate_os_version
        );
    }
}



/*
 * TFB_PIXEL_PARTITIONER_LIVE_LOG_PANEL_V1
 *
 * Pixel Partitioner remains an ordinary Boot Manager submenu.
 *
 * Only its right-hand content region receives a specialized
 * read-only presentation:
 *
 *     HOST
 *     LIVE LOG
 *     Back
 *
 * The left Boot Manager information panels, global header,
 * title/subtitle, normal input handling, and submenu stack
 * remain unchanged.
 *
 * The READY marker is the authorization boundary.
 *
 * The status/log files are presentation-only input and their
 * contents are never executed.
 */
static int tfb_pixel_partitioner_page(
    const struct tfb_menu_page *page
);


#define TFB_PP_LOG_PATH \
    "/dev/treeforge-bootstrap-pixel-partitioner-log"

#define TFB_PP_LOG_BUFFER_CAPACITY 4096U
#define TFB_PP_LOG_MAX_LINES 16
#define TFB_PP_LOG_LINE_CAPACITY 96U


static char tfb_pp_log_buffer[
    TFB_PP_LOG_BUFFER_CAPACITY
];


static int tfb_pp_prefix(
    const char *value,
    const char *prefix
) {
    if (
        !value
        || !prefix
    ) {
        return 0;
    }

    usize index = 0;

    while (
        prefix[index]
        != '\0'
    ) {
        if (
            value[index]
            != prefix[index]
        ) {
            return 0;
        }

        index++;
    }

    return 1;
}


static void tfb_pp_copy_log_text(
    char *output,
    usize capacity,
    const char *prefix,
    const char *message
) {
    if (
        !output
        || capacity < 2
    ) {
        return;
    }

    usize position = 0;

    if (prefix) {
        for (
            usize index = 0;
            prefix[index] != '\0'
                && position + 1 < capacity;
            index++
        ) {
            output[
                position++
            ] = prefix[index];
        }
    }

    if (message) {
        for (
            usize index = 0;
            message[index] != '\0'
                && position + 1 < capacity;
            index++
        ) {
            unsigned char value = (
                (unsigned char)
                message[index]
            );

            if (
                value == '\r'
                || value == '\n'
            ) {
                break;
            }

            output[
                position++
            ] = (
                value >= 0x20
                && value <= 0x7e
                ? (char) value
                : '?'
            );
        }
    }

    output[position] =
        '\0';
}


static u32 tfb_pp_format_log_line(
    const char *source,
    char *output,
    usize capacity,
    u32 foreground,
    u32 accent,
    u32 green,
    u32 warning
) {
    if (!source) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[INFO] ",
            ""
        );

        return foreground;
    }

    if (
        tfb_pp_prefix(
            source,
            "PASS|"
        )
    ) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[PASS] ",
            source + 5
        );

        return green;
    }

    if (
        tfb_pp_prefix(
            source,
            "RUNNING|"
        )
    ) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[RUN ] ",
            source + 8
        );

        return accent;
    }

    if (
        tfb_pp_prefix(
            source,
            "FAIL|"
        )
    ) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[FAIL] ",
            source + 5
        );

        return warning;
    }

    if (
        tfb_pp_prefix(
            source,
            "WARN|"
        )
    ) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[WARN] ",
            source + 5
        );

        return warning;
    }

    if (
        tfb_pp_prefix(
            source,
            "INFO|"
        )
    ) {
        tfb_pp_copy_log_text(
            output,
            capacity,
            "[INFO] ",
            source + 5
        );

        return foreground;
    }

    tfb_pp_copy_log_text(
        output,
        capacity,
        "",
        source
    );

    return foreground;
}


static const char *tfb_pp_log_tail(
    int *line_count
) {
    if (line_count) {
        *line_count = 0;
    }

    if (
        !tfb_pp_read_status_text(
            TFB_PP_LOG_PATH,
            tfb_pp_log_buffer,
            sizeof(
                tfb_pp_log_buffer
            )
        )
    ) {
        return 0;
    }

    int total = 1;

    for (
        usize index = 0;
        tfb_pp_log_buffer[index]
            != '\0';
        index++
    ) {
        if (
            tfb_pp_log_buffer[index]
            == '\n'
        ) {
            total++;
        }
    }

    int skip = (
        total
        > TFB_PP_LOG_MAX_LINES
        ? total
            - TFB_PP_LOG_MAX_LINES
        : 0
    );

    char *start =
        tfb_pp_log_buffer;

    while (
        *start
        && skip > 0
    ) {
        if (*start == '\n') {
            skip--;
        }

        start++;
    }

    int visible = 0;

    for (
        char *cursor = start;
        *cursor;
        cursor++
    ) {
        if (*cursor == '\n') {
            visible++;
        }
    }

    if (*start) {
        visible++;
    }

    if (
        visible
        > TFB_PP_LOG_MAX_LINES
    ) {
        visible =
            TFB_PP_LOG_MAX_LINES;
    }

    if (line_count) {
        *line_count =
            visible;
    }

    return start;
}


static void tfb_ui_render_pixel_partitioner(
    const struct tfb_ui_menu_geometry *geometry,
    int selected,
    u32 background,
    u32 panel,
    u32 foreground,
    u32 dim,
    u32 border,
    u32 accent,
    u32 green,
    u32 warning
) {
    if (!geometry) {
        return;
    }

    /*
     * HOST row.
     *
     * This is Bootstrap's own ADB observation, not a value
     * supplied by Pixel Partitioner.
     *
     * TFB_PP_HOST_ROW_DIRTY_REFRESH_V1
     *
     * Clear only this small row before repainting.  This prevents
     * stale characters when the ADB state changes from a longer
     * label to a shorter one.
     */
    tfb_ui_fill_rect(
        geometry->x,
        315U,
        geometry->width,
        60U,
        background
    );

    const char *host_state =
        tfb_adb_display_text();

    u32 host_pixel = (
        tfb_string_equal(
            host_state,
            "CONNECTED"
        )
        ? green
        : warning
    );

    tfb_ui_draw_text(
        geometry->x,
        330U,
        "HOST",
        3U,
        dim
    );

    tfb_ui_draw_text_fit(
        geometry->x + 150U,
        330U,
        host_state,
        geometry->width - 150U,
        3U,
        2U,
        host_pixel
    );


    /*
     * Main log box.
     *
     * Leave only enough room below it for the Back card.
     */
    const u32 log_y =
        390U;

    const u32 log_height =
        810U;

    const u32 log_inner_x =
        geometry->x + 34U;

    const u32 log_inner_width = (
        geometry->width > 68U
        ? geometry->width - 68U
        : geometry->width
    );

    tfb_ui_fill_rect(
        geometry->x,
        log_y,
        geometry->width,
        log_height,
        panel
    );

    tfb_ui_outline_rect(
        geometry->x,
        log_y,
        geometry->width,
        log_height,
        4U,
        border
    );

    tfb_ui_draw_text(
        log_inner_x,
        log_y + 28U,
        "LIVE LOG",
        3U,
        accent
    );


    int visible_lines = 0;

    const char *cursor =
        tfb_pp_log_tail(
            &visible_lines
        );

    u32 line_y =
        log_y + 88U;

    if (
        !cursor
        || visible_lines == 0
    ) {
        tfb_ui_draw_text_fit(
            log_inner_x,
            line_y,
            "Waiting for Pixel Partitioner...",
            log_inner_width,
            2U,
            2U,
            dim
        );
    } else {
        int rendered = 0;

        while (
            *cursor
            && rendered
                < TFB_PP_LOG_MAX_LINES
        ) {
            char source[
                TFB_PP_LOG_LINE_CAPACITY
            ];

            usize source_length = 0;

            while (
                cursor[source_length]
                    != '\0'
                && cursor[source_length]
                    != '\n'
                && source_length + 1
                    < sizeof(source)
            ) {
                source[
                    source_length
                ] = cursor[
                    source_length
                ];

                source_length++;
            }

            source[
                source_length
            ] = '\0';


            char display[
                TFB_PP_LOG_LINE_CAPACITY
            ];

            u32 line_pixel = (
                tfb_pp_format_log_line(
                    source,
                    display,
                    sizeof(display),
                    foreground,
                    accent,
                    green,
                    warning
                )
            );

            tfb_ui_draw_text_fit(
                log_inner_x,
                line_y,
                display,
                log_inner_width,
                2U,
                2U,
                line_pixel
            );

            line_y +=
                43U;

            rendered++;


            /*
             * Advance through the entire source line even if
             * its presentation was truncated to our bounded
             * display buffer.
             */
            while (
                *cursor
                && *cursor != '\n'
            ) {
                cursor++;
            }

            if (*cursor == '\n') {
                cursor++;
            }
        }
    }


    /*
     * Back remains the single ordinary menu action.
     *
     * Input handling is unchanged; selected == 0 maps to the
     * profile's existing action:"back".
     */
    const u32 back_y =
        geometry->y;

    const u32 back_height =
        geometry->row_height;

    const u32 back_fill = (
        selected == 0
        ? 0x00142836U
        : panel
    );

    const u32 back_border = (
        selected == 0
        ? accent
        : border
    );

    tfb_ui_fill_rect(
        geometry->x,
        back_y,
        geometry->width,
        back_height,
        back_fill
    );

    tfb_ui_outline_rect(
        geometry->x,
        back_y,
        geometry->width,
        back_height,
        selected == 0
            ? 5U
            : 3U,
        back_border
    );

    tfb_ui_draw_text(
        geometry->x + 35U,
        back_y + 34U,
        "Back",
        4U,
        foreground
    );

    tfb_ui_draw_text_fit(
        geometry->x + 35U,
        back_y + 94U,
        "End authorization and return Home",
        geometry->width - 70U,
        2U,
        2U,
        dim
    );
}


/*
 * TFB_PP_LIVE_PANEL_REFRESH_V1
 *
 * Refresh only the Pixel Partitioner right-panel dynamic region.
 *
 * The page title/subtitle and all left-side Boot Manager information
 * remain untouched.  The log reader is presentation-only and does
 * not participate in transaction authorization.
 */
static void tfb_fb_refresh_pixel_partitioner(
    const struct tfb_menu_page *page,
    int selected
) {
    if (
        !tfb_fb_menu_active
        || tfb_fb_menu_mapped_address < 0
        || !tfb_pixel_partitioner_page(
            page
        )
        || !tfb_ui_landscape_supported()
    ) {
        return;
    }

    struct tfb_ui_menu_geometry geometry;

    if (
        !tfb_ui_menu_geometry(
            page,
            &geometry
        )
    ) {
        return;
    }

    tfb_ui_render_pixel_partitioner(
        &geometry,
        selected,
        0x00060b12U,
        0x000c1621U,
        0x00f4f8fbU,
        0x008ca0b0U,
        0x00283d4dU,
        0x0000d4eeU,
        0x004ed87dU,
        0x00ff4d5aU
    );
}


static void tfb_fb_render_menu(
    const struct tfb_menu_page *page,
    int selected,
    u32 elapsed_ms,
    int timeout_fired
) {
    if (
        !tfb_fb_menu_active
        || tfb_fb_menu_mapped_address < 0
        || !page
    ) {
        return;
    }

    const u32 background =
        0x00060b12U;

    const u32 panel =
        0x000c1621U;

    const u32 foreground =
        0x00f4f8fbU;

    const u32 dim =
        0x008ca0b0U;

    const u32 border =
        0x00283d4dU;

    const u32 accent =
        0x0000d4eeU;

    const u32 green =
        0x004ed87dU;

    const u32 warning =
        0x00ff4d5aU;

    /*
     * Keep an explicit degraded fallback rather than ever attempting
     * a guessed transform on an unknown framebuffer geometry.
     */
    if (
        !tfb_ui_landscape_supported()
    ) {
        tfb_fb_fill_rect(
            0,
            0,
            tfb_fb_menu_width,
            tfb_fb_menu_height,
            0x00000000U
        );

        tfb_fb_draw_text(
            24U,
            24U,
            "TREEFORGE LANDSCAPE GEOMETRY UNSUPPORTED",
            3U,
            0x00ffffffU
        );

        return;
    }

    tfb_fb_fill_rect(
        0,
        0,
        tfb_fb_menu_width,
        tfb_fb_menu_height,
        background
    );

    /*
     * Header.
     */
    tfb_ui_draw_text(
        110U,
        72U,
        "TREEFORGE",
        8U,
        accent
    );

    tfb_ui_draw_text(
        110U,
        150U,
        "BOOT MANAGER",
        5U,
        foreground
    );


    /*
     * TFB_TOP_INPUT_HINTS_RESTORED_V3
     */
    tfb_ui_draw_text(
        1780U,
        92U,
        "VOL -",
        3U,
        foreground
    );

    tfb_ui_draw_text(
        1970U,
        92U,
        "VOL +",
        3U,
        foreground
    );

    tfb_ui_draw_text(
        2230U,
        92U,
        "POWER",
        3U,
        foreground
    );

    /*
     * TFB_FINAL_MENU_CONSOLIDATION_R3
     *
     * Final normal-menu information layout:
     *
     *   BOOT OPTIONS
     *   KERNEL STACK
     *   SYSTEM
     *
     * Detailed component information belongs in
     * Maintenance -> Component Versions.
     */
    const u32 left_x =
        80U;

    const u32 left_width =
        1180U;

    const u32 inner_x =
        left_x + 45U;


    /*
     * ------------------------------------------------------------
     * BOOT OPTIONS
     * ------------------------------------------------------------
     */
    const u32 boot_box_y =
        220U;

    const u32 boot_box_height =
        360U;

    tfb_ui_fill_rect(
        left_x,
        boot_box_y,
        left_width,
        boot_box_height,
        panel
    );

    tfb_ui_outline_rect(
        left_x,
        boot_box_y,
        left_width,
        boot_box_height,
        4U,
        border
    );

    tfb_ui_draw_text(
        inner_x,
        boot_box_y + 30U,
        "BOOT OPTIONS",
        4U,
        accent
    );

    u32 boot_state_y =
        boot_box_y + 95U;

    char android_a_state[64];
    char android_b_state[64];
    char alternate_os_state[192];

    tfb_ui_android_inventory(
        0,
        android_a_state,
        sizeof(android_a_state)
    );

    tfb_ui_android_inventory(
        1,
        android_b_state,
        sizeof(android_b_state)
    );

    tfb_ui_alternate_inventory(
        alternate_os_state,
        sizeof(alternate_os_state)
    );


    tfb_ui_draw_boot_state(
        inner_x,
        boot_state_y,
        "ANDROID A",
        android_a_state,
        green
    );

    boot_state_y += 50U;

    tfb_ui_draw_boot_state(
        inner_x,
        boot_state_y,
        "ANDROID B",
        android_b_state,
        tfb_menu_state.full_ab
            ? green
            : dim
    );

    boot_state_y += 50U;

    tfb_ui_draw_boot_state(
        inner_x,
        boot_state_y,
        "ALTERNATE OS",
        alternate_os_state,
        tfb_menu_state
            .alternate_os_configured
            ? green
            : dim
    );

    boot_state_y += 50U;

    tfb_ui_draw_boot_state(
        inner_x,
        boot_state_y,
        "RECOVERY",
        "NOT INSTALLED",
        dim
    );

    boot_state_y += 50U;

    tfb_ui_draw_boot_state(
        inner_x,
        boot_state_y,
        "ROOT",
        tfb_menu_state.root_installed
            ? "INSTALLED"
            : "NOT INSTALLED",
        tfb_menu_state.root_installed
            ? green
            : dim
    );


    /*
     * ------------------------------------------------------------
     * KERNEL STACK
     * ------------------------------------------------------------
     *
     * These are live runtime values supplied by the kernel.
     *
     * Until the new release kernel is installed they intentionally
     * report NOT EXPOSED instead of displaying expected/hardcoded
     * versions.
     */
    const u32 kernel_box_y =
        600U;

    const u32 kernel_box_height =
        380U;

    tfb_ui_fill_rect(
        left_x,
        kernel_box_y,
        left_width,
        kernel_box_height,
        panel
    );

    tfb_ui_outline_rect(
        left_x,
        kernel_box_y,
        left_width,
        kernel_box_height,
        4U,
        border
    );

    tfb_ui_draw_text(
        inner_x,
        kernel_box_y + 30U,
        "KERNEL STACK",
        4U,
        accent
    );

    char kernel_release[96] =
        "NOT EXPOSED";

    char kernel_linux_base[96] =
        "NOT EXPOSED";

    char kernel_panthor[96] =
        "NOT EXPOSED";

    char kernel_panthor_backport[96] =
        "NOT EXPOSED";

    char kernel_drm_backport[96] =
        "NOT EXPOSED";

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/release",
        kernel_release,
        sizeof(kernel_release)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/linux_base",
        kernel_linux_base,
        sizeof(kernel_linux_base)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/panthor",
        kernel_panthor,
        sizeof(kernel_panthor)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/panthor_backport",
        kernel_panthor_backport,
        sizeof(kernel_panthor_backport)
    );

    tfb_menu_read_state_text(
        "/sys/kernel/treeforge/drm_backport",
        kernel_drm_backport,
        sizeof(kernel_drm_backport)
    );

    u32 kernel_y =
        kernel_box_y + 92U;

    tfb_ui_draw_kv(
        inner_x,
        kernel_y,
        "RELEASE",
        kernel_release,
        dim,
        foreground
    );

    kernel_y += 48U;

    tfb_ui_draw_kv(
        inner_x,
        kernel_y,
        "LINUX BASE",
        kernel_linux_base,
        dim,
        foreground
    );

    kernel_y += 48U;

    tfb_ui_draw_kv(
        inner_x,
        kernel_y,
        "PANTHOR",
        kernel_panthor,
        dim,
        foreground
    );

    kernel_y += 48U;

    tfb_ui_draw_kv(
        inner_x,
        kernel_y,
        "PANTHOR BACKPORT",
        kernel_panthor_backport,
        dim,
        foreground
    );

    kernel_y += 48U;

    tfb_ui_draw_kv(
        inner_x,
        kernel_y,
        "DRM BACKPORT",
        kernel_drm_backport,
        dim,
        foreground
    );


    /*
     * ------------------------------------------------------------
     * SYSTEM
     * ------------------------------------------------------------
     */
    const u32 system_box_y =
        1000U;

    /*
     * TFB_SYSTEM_BOTTOM_ALIGNMENT_V1
     *
     * Keep the bottom edge aligned with the menu content baseline:
     *
     *   system_box_y  = 1000
     *   content bottom = 1430
     *   height = 430
     */
    const u32 system_box_height =
        430U;

    tfb_ui_fill_rect(
        left_x,
        system_box_y,
        left_width,
        system_box_height,
        panel
    );

    tfb_ui_outline_rect(
        left_x,
        system_box_y,
        left_width,
        system_box_height,
        4U,
        border
    );

    tfb_ui_draw_text(
        inner_x,
        system_box_y + 30U,
        "SYSTEM",
        4U,
        accent
    );

    const u32 info_width =
        250U;

    const u32 info_gap =
        22U;

    const u32 c0 =
        inner_x;

    const u32 c1 =
        c0
        + info_width
        + info_gap;

    const u32 c2 =
        c1
        + info_width
        + info_gap;

    const u32 c3 =
        c2
        + info_width
        + info_gap;

    const u32 version_y =
        system_box_y + 95U;

    tfb_ui_draw_field(
        c0,
        version_y,
        info_width,
        "BOOTSTRAP",
        tfb_bootstrap_version,
        dim,
        foreground
    );

    tfb_ui_draw_field(
        c1,
        version_y,
        info_width,
        "BOOT MANAGER",
        tfb_version,
        dim,
        foreground
    );

    tfb_ui_draw_field(
        c2,
        version_y,
        info_width,
        "RUNTIME ABI",
        tfb_runtime_abi_version,
        dim,
        foreground
    );

    tfb_ui_draw_field(
        c3,
        version_y,
        info_width,
        "A/B TYPE",
        tfb_menu_state.full_ab
            ? "FULL A/B"
            : "VIRTUAL A/B",
        dim,
        foreground
    );


    /*
     * Live status row.
     *
     * ADB retains the existing dirty-region redraw behavior.
     */
    const u32 live_y =
        1290U;

    tfb_fb_render_adb_region();

    tfb_ui_draw_field(
        c1,
        live_y,
        info_width,
        "TOUCH",
        tfb_ui_marker_exists(
            "/dev/treeforge-bootstrap-touchscreen-ready"
        )
            ? "WORKING"
            : "UNAVAILABLE",
        dim,
        tfb_ui_marker_exists(
            "/dev/treeforge-bootstrap-touchscreen-ready"
        )
            ? green
            : warning
    );


    /*
     * Battery percentage and charging state in one field.
     *
     * Examples:
     *
     *   51% / Charging
     *   82% / Discharging
     *   100% / Full
     */
    char battery_value[64] =
        "UNKNOWN";

    u64 battery_capacity = 0;

    if (
        tfb_ui_read_u64(
            "/sys/class/power_supply/battery/capacity",
            &battery_capacity
        )
    ) {
        battery_value[0] =
            '\0';

        tfb_ui_u64_text(
            battery_capacity,
            battery_value,
            sizeof(battery_value)
        );

        tfb_ui_append(
            battery_value,
            sizeof(battery_value),
            "%"
        );

        char battery_state[32];

        if (
            tfb_menu_read_state_text(
                "/sys/class/power_supply/battery/status",
                battery_state,
                sizeof(battery_state)
            )
        ) {
            tfb_ui_append(
                battery_value,
                sizeof(battery_value),
                " / "
            );

            tfb_ui_append(
                battery_value,
                sizeof(battery_value),
                battery_state
            );
        }
    }

    tfb_ui_draw_field(
        c2,
        live_y,
        (
            info_width
            * 2U
            + info_gap
        ),
        "BATTERY",
        battery_value,
        dim,
        foreground
    );


    /*
     * Right menu panel.
     */
    struct tfb_ui_menu_geometry geometry;

    if (
        !tfb_ui_menu_geometry(
            page,
            &geometry
        )
    ) {
        tfb_ui_draw_text(
            TFB_UI_MENU_X,
            420U,
            "NO MENU ENTRIES",
            5U,
            warning
        );

        return;
    }

    tfb_ui_draw_text_fit(
        geometry.x,
        205U,
        page->title,
        geometry.width,
        6U,
        3U,
        foreground
    );

    /*
     * TFB_HOME_SUBTITLE_SUPPRESSED_V1
     *
     * The profile schema requires a non-empty subtitle, but Home
     * intentionally renders only its page title.
     */
    if (
        !tfb_ui_home_page(
            page
        )
    ) {
        tfb_ui_draw_text_fit(
            geometry.x,
            260U,
            page->subtitle,
            geometry.width,
            3U,
            2U,
            dim
        );
    }

    if (
        tfb_ui_partition_page(
            page
        )
    ) {
        tfb_ui_render_partition_information(
            &geometry
        );
    }


    if (
        tfb_ui_component_versions_page(
            page
        )
    ) {
        tfb_ui_render_component_versions(
            &geometry,
            accent,
            dim,
            foreground
        );

        tfb_fb_render_status_region(
            page,
            elapsed_ms,
            timeout_fired
        );

        return;
    }


    /*
     * TFB_PIXEL_PARTITIONER_LIVE_LOG_PANEL_HOOK_V1
     *
     * Pixel Partitioner remains a normal submenu.
     *
     * Replace only its ordinary list-of-cards body with the
     * host indicator + live log + bottom Back card.
     */
    if (
        tfb_pixel_partitioner_page(
            page
        )
    ) {
        tfb_ui_render_pixel_partitioner(
            &geometry,
            selected,
            background,
            panel,
            foreground,
            dim,
            border,
            accent,
            green,
            warning
        );

        tfb_fb_render_status_region(
            page,
            elapsed_ms,
            timeout_fired
        );

        return;
    }



    for (
        int index = 0;
        index < geometry.item_count;
        index++
    ) {
        tfb_fb_render_menu_card(
            page,
            index,
            index == selected
        );
    }


    tfb_fb_render_status_region(
        page,
        elapsed_ms,
        timeout_fired
    );



}


/*
 * TFB_BUFFERED_MENU_PRESENT_V1
 *
 * Dirty-region decisions remain owned by the Boot Manager.  The
 * framebuffer provider only stages the selected raw scanout region and
 * publishes it after the logical redraw is complete.
 */
static void tfb_fb_render_selection_delta_buffered(
    const struct tfb_menu_page *page,
    int old_selected,
    int new_selected
) {
    struct tfb_ui_menu_geometry geometry;
    int staged = 0;

    if (
        tfb_ui_menu_geometry(page, &geometry)
        && old_selected >= 0
        && new_selected >= 0
        && old_selected < geometry.item_count
        && new_selected < geometry.item_count
    ) {
        u32 first = (u32) (
            old_selected < new_selected
                ? old_selected
                : new_selected
        );

        u32 last = (u32) (
            old_selected > new_selected
                ? old_selected
                : new_selected
        );

        u32 row_span = (
            geometry.row_height
            + geometry.row_gap
        );

        u32 region_y = (
            geometry.y
            + first * row_span
        );

        u32 region_height = (
            (last - first) * row_span
            + geometry.row_height
        );

        staged = tfb_ui_buffer_begin_region(
            geometry.x,
            region_y,
            geometry.width,
            region_height
        );
    }

    tfb_fb_render_selection_delta(
        page,
        old_selected,
        new_selected
    );

    tfb_ui_buffer_present_if(staged);
}


static void tfb_fb_render_status_region_buffered(
    const struct tfb_menu_page *page,
    u32 elapsed_ms,
    int timeout_fired
) {
    struct tfb_ui_menu_geometry geometry;
    int staged = 0;

    if (tfb_ui_menu_geometry(page, &geometry)) {
        staged = tfb_ui_buffer_begin_region(
            geometry.x,
            1468U,
            geometry.width,
            76U
        );
    }

    tfb_fb_render_status_region(
        page,
        elapsed_ms,
        timeout_fired
    );

    tfb_ui_buffer_present_if(staged);
}


static void tfb_fb_render_adb_region_buffered(void) {
    int staged = tfb_ui_buffer_begin_region(
        117U,
        1280U,
        266U,
        66U
    );

    tfb_fb_render_adb_region();

    tfb_ui_buffer_present_if(staged);
}


static void tfb_fb_refresh_pixel_partitioner_buffered(
    const struct tfb_menu_page *page,
    int selected
) {
    struct tfb_ui_menu_geometry geometry;
    int staged = 0;

    if (tfb_ui_menu_geometry(page, &geometry)) {
        u32 bottom = (
            TFB_PP_BACK_Y
            + TFB_PP_BACK_HEIGHT
        );

        if (bottom > 315U) {
            staged = tfb_ui_buffer_begin_region(
                geometry.x,
                315U,
                geometry.width,
                bottom - 315U
            );
        }
    }

    tfb_fb_refresh_pixel_partitioner(
        page,
        selected
    );

    tfb_ui_buffer_present_if(staged);
}


