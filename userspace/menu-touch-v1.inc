/* SPDX-License-Identifier: Apache-2.0 */
static void tfb_touch_reset_state(
    struct tfb_touch_state *state
) {
    if (!state) {
        return;
    }

    state->slot = 0;
    state->active = 0;

    state->have_x = 0;
    state->have_y = 0;

    state->raw_x = 0;
    state->raw_y = 0;

    state->start_set = 0;
    state->start_x = 0;
    state->start_y = 0;

    state->pressed_row = -1;
    state->canceled = 0;
}


static int tfb_touch_abs_int(
    int value
) {
    return (
        value < 0
        ? -value
        : value
    );
}


static void tfb_runtime_sysfs_event_name_path(
    int index,
    char *output
) {
    static const char prefix[] =
        "/sys/class/input/event";

    static const char suffix[] =
        "/device/name";

    int position = 0;

    for (
        int i = 0;
        prefix[i] != '\0';
        i++
    ) {
        output[position++] =
            prefix[i];
    }

    if (index >= 10) {
        output[position++] = (
            '0'
            + (
                index
                / 10
            )
        );
    }

    output[position++] = (
        '0'
        + (
            index
            % 10
        )
    );

    for (
        int i = 0;
        suffix[i] != '\0';
        i++
    ) {
        output[position++] =
            suffix[i];
    }

    output[position] = '\0';
}


static int tfb_input_is_touchscreen(
    int index
) {
    char path[80];

    tfb_runtime_sysfs_event_name_path(
        index,
        path
    );

    long fd = tfb_open(
        path,
        O_RDONLY
    );

    if (fd < 0) {
        return 0;
    }

    char name[64];

    long amount = tfb_syscall3(
        SYS_READ,
        fd,
        (long) name,
        (long) (
            sizeof(name)
            - 1
        )
    );

    tfb_close(
        fd
    );

    if (amount <= 0) {
        return 0;
    }

    while (
        amount > 0
        && (
            name[amount - 1] == '\n'
            || name[amount - 1] == '\r'
        )
    ) {
        amount--;
    }

    name[amount] = '\0';

    return tfb_string_equal(
        name,
        "NVTCapacitiveTouchScreen"
    );
}



static int tfb_menu_touch_row(
    const struct tfb_menu_page *page,
    int raw_x,
    int raw_y
) {
    /*
     * TFB_TOUCH_RENDER_GEOMETRY_SYNC_V1
     *
     * Hit testing intentionally has no independent Home layout.
     *
     * The renderer is the geometry authority.  Whatever rows
     * tfb_ui_menu_geometry() gives the framebuffer are exactly the
     * rows accepted here.
     *
     * This prevents visual/input drift when Home gains or loses an
     * entry, including the Iteration-3 promotion of Boot Options from
     * the old Resume/settings-square row into its own full-width row.
     */
    if (
        !page
        || !tfb_fb_menu_active
        || !tfb_ui_landscape_supported()
        || raw_x < 0
        || raw_x >= TFB_TOUCH_RAW_WIDTH
        || raw_y < 0
        || raw_y >= TFB_TOUCH_RAW_HEIGHT
    ) {
        return -1;
    }

    struct tfb_ui_menu_geometry geometry;

    if (
        !tfb_ui_menu_geometry(
            page,
            &geometry
        )
    ) {
        return -1;
    }

    /*
     * Inverse of the framebuffer renderer's clockwise
     * logical-landscape transform.
     *
     * raw panel:  1600 x 2560
     * logical UI: 2560 x 1600
     */
    u32 logical_x = (
        TFB_UI_LOGICAL_WIDTH
        - 1U
        - (
            (u32) (
                (
                    (unsigned long) raw_y
                    * (unsigned long)
                        TFB_UI_LOGICAL_WIDTH
                )
                / (unsigned long)
                    TFB_TOUCH_RAW_HEIGHT
            )
        )
    );

    u32 logical_y = (
        (u32) (
            (
                (unsigned long) raw_x
                * (unsigned long)
                    TFB_UI_LOGICAL_HEIGHT
            )
            / (unsigned long)
                TFB_TOUCH_RAW_WIDTH
        )
    );

    if (
        logical_x < geometry.x
        || logical_x
            >= geometry.x
                + geometry.width
    ) {
        return -1;
    }

    for (
        int index = 0;
        index < geometry.item_count;
        index++
    ) {
        u32 row_y = (
            geometry.y
            + (u32) index
                * (
                    geometry.row_height
                    + geometry.row_gap
                )
        );

        /*
         * Gaps between cards are intentionally not clickable.
         */
        if (
            logical_y >= row_y
            && logical_y
                < row_y
                    + geometry.row_height
        ) {
            return index;
        }
    }

    return -1;
}


/*
 * Return values:
 *
 *   0 = event is not handled by touch
 *   1 = touch event consumed, no UI change
 *   2 = row selected/highlight changed
 *   3 = completed tap should activate selection
 */
static int tfb_touch_process_event(
    struct tfb_touch_state *state,
    const struct tfb_menu_page *page,
    const struct tfb_input_event *event,
    int *selected
) {
    if (
        !state
        || !event
        || !selected
        || event->type != EV_ABS
    ) {
        return 0;
    }

    if (
        event->code
        == ABS_MT_SLOT
    ) {
        state->slot =
            event->value;

        return 1;
    }

    /*
     * The menu intentionally consumes only the primary contact.
     * Additional fingers never activate menu entries.
     */
    if (state->slot != 0) {
        return 1;
    }

    if (
        event->code
        == ABS_MT_TRACKING_ID
    ) {
        if (event->value >= 0) {
            state->active = 1;

            state->have_x = 0;
            state->have_y = 0;

            state->start_set = 0;
            state->pressed_row = -1;
            state->canceled = 0;

            return 1;
        }

        if (!state->active) {
            return 1;
        }

        int release_row = -1;

        if (
            state->have_x
            && state->have_y
        ) {
            release_row = (
                tfb_menu_touch_row(
                    page,
                    state->raw_x,
                    state->raw_y
                )
            );
        }

        int activate = (
            state->start_set
            && !state->canceled
            && state->pressed_row >= 0
            && release_row
                == state->pressed_row
            && tfb_touch_abs_int(
                state->raw_x
                - state->start_x
            )
                <= TFB_TOUCH_TAP_SLOP
            && tfb_touch_abs_int(
                state->raw_y
                - state->start_y
            )
                <= TFB_TOUCH_TAP_SLOP
        );

        state->active = 0;
        state->have_x = 0;
        state->have_y = 0;
        state->start_set = 0;
        state->pressed_row = -1;
        state->canceled = 0;

        return (
            activate
            ? 3
            : 1
        );
    }

    if (!state->active) {
        return 1;
    }

    if (
        event->code
        == ABS_MT_POSITION_X
    ) {
        state->raw_x =
            event->value;

        state->have_x = 1;
    } else if (
        event->code
        == ABS_MT_POSITION_Y
    ) {
        state->raw_y =
            event->value;

        state->have_y = 1;
    } else {
        return 1;
    }

    if (
        state->have_x
        && state->have_y
        && !state->start_set
    ) {
        state->start_x =
            state->raw_x;

        state->start_y =
            state->raw_y;

        state->start_set = 1;

        state->pressed_row = (
            tfb_menu_touch_row(
                page,
                state->raw_x,
                state->raw_y
            )
        );

        if (
            state->pressed_row
            >= 0
        ) {
            *selected =
                state->pressed_row;

            return 2;
        }

        /*
         * A contact that begins outside a menu row can never become
         * an activating tap merely by moving onto one later.
         */
        state->canceled = 1;

        return 1;
    }

    if (
        state->have_x
        && state->have_y
        && state->start_set
        && !state->canceled
    ) {
        int current_row = (
            tfb_menu_touch_row(
                page,
                state->raw_x,
                state->raw_y
            )
        );

        if (
            tfb_touch_abs_int(
                state->raw_x
                - state->start_x
            )
                > TFB_TOUCH_TAP_SLOP
            || tfb_touch_abs_int(
                state->raw_y
                - state->start_y
            )
                > TFB_TOUCH_TAP_SLOP
            || current_row
                != state->pressed_row
        ) {
            /*
             * Cancellation is sticky for this contact. Returning to
             * the original row before release must not reactivate it.
             */
            state->canceled = 1;
        }
    }

    return 1;
}


/*
 * One canonical menu-selection execution path.
 *
 * KEY_POWER and a completed touchscreen tap both arrive here.
 */

/*
 * ================================================================
 * TFB_PIXEL_PARTITIONER_BLOCK_RUNTIME_V1
 * ================================================================
 *
 * Reuse the existing Bootstrap device-node infrastructure:
 *
 *   tfb_runtime_parse_device_number()
 *   tfb_runtime_makedev()
 *   SYS_MKNODAT
 *   SYS_SYMLINKAT
 *
 * BusyBox remains completely outside device realization.
 */

