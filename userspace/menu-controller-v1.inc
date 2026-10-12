static void tfb_menu_activate_selected(
    const struct tfb_menu_page **page,
    const struct tfb_menu_page **page_stack,
    int *stack_depth,
    int *selected,
    u32 *elapsed_ms,
    int *timeout_fired,
    int *redraw,
    long *inputs,
    int created_input_directory,
    int *created_input_nodes
) {
    if (
        !page
        || !*page
        || !page_stack
        || !stack_depth
        || !selected
        || !elapsed_ms
        || !timeout_fired
        || !redraw
    ) {
        return;
    }

    const struct tfb_menu_entry
        *entry = (
            tfb_menu_visible_entry(
                *page,
                *selected
            )
        );

    if (!entry) {
        tfb_menu_status =
            "INVALID SELECTION";

        *redraw |=
            TFB_REDRAW_STATUS;

        return;
    }

    if (
        entry->submenu
    ) {
        if (
            *stack_depth
            >= TFB_MENU_STACK_DEPTH
        ) {
            tfb_menu_status =
                "SUBMENU DEPTH LIMIT";

            *redraw |=
                TFB_REDRAW_STATUS;

            return;
        }

        /*
         * TFB_PIXEL_PARTITIONER_MENU_SCOPE_V1
         */
        if (
            tfb_pixel_partitioner_page(
                entry->submenu
            )
            && !tfb_pixel_partitioner_activate()
        ) {
            tfb_menu_status =
                "PIXEL PARTITIONER UNAVAILABLE";

            *redraw |=
                TFB_REDRAW_STATUS;

            return;
        }

        page_stack[
            *stack_depth
        ] = entry->submenu;

        (*stack_depth)++;

        *page =
            entry->submenu;

        *selected = (
            tfb_menu_default_selection(
                *page
            )
        );

        tfb_menu_status = (
            tfb_pixel_partitioner_page(
                *page
            )
            ? "PIXEL PARTITIONER READY"
            : 0
        );

        *elapsed_ms = 0;
        *timeout_fired = 0;

        tfb_log(
            "early-menu submenu=enter"
        );

        *redraw |=
            TFB_REDRAW_FULL;

        return;
    }

    if (
        entry->action
        == TFB_ACTION_BACK
    ) {
        if (
            *stack_depth > 1
        ) {
            if (
                tfb_pixel_partitioner_page(
                    *page
                )
            ) {
                tfb_pixel_partitioner_deactivate();
            }

            (*stack_depth)--;

            *page = (
                page_stack[
                    *stack_depth - 1
                ]
            );

            *selected = (
                tfb_menu_default_selection(
                    *page
                )
            );

            tfb_menu_status = 0;

            tfb_log(
                "early-menu submenu=back"
            );

            *redraw |=
                TFB_REDRAW_FULL;
        } else {
            tfb_menu_status =
                "ALREADY AT MAIN MENU";

            *redraw |=
                TFB_REDRAW_STATUS;
        }

        return;
    }

    tfb_menu_execute_action(
        entry->action,
        inputs,
        created_input_directory,
        created_input_nodes
    );

    /*
     * Transition actions never return.  Live Console does return, but
     * it owns the full framebuffer while active and therefore requires
     * a complete menu repaint.
     */
    if (
        entry->action
        == TFB_ACTION_LIVE_CONSOLE
    ) {
        *redraw |=
            TFB_REDRAW_FULL;
    } else {
        *redraw |=
            TFB_REDRAW_STATUS;
    }
}


__attribute__((noreturn))
static void tfb_run_menu(void) {
    int created_input_directory = 0;

    int created_input_nodes[
        INPUT_COUNT
    ];

    int sysfs_devices_seen = 0;

    int last_adb_display_state = -1;

    tfb_realize_input_nodes(
        &created_input_directory,
        created_input_nodes,
        &sysfs_devices_seen
    );

    long inputs[
        INPUT_COUNT
    ];

    int touch_inputs[
        INPUT_COUNT
    ];

    struct tfb_touch_state
        touch_states[
            INPUT_COUNT
        ];

    for (
        int index = 0;
        index < INPUT_COUNT;
        index++
    ) {
        inputs[index] = -1;
        touch_inputs[index] = 0;

        tfb_touch_reset_state(
            &touch_states[index]
        );
    }

    long marker = tfb_syscall4(
        SYS_OPENAT,
        AT_FDCWD,
        (long)
            "/dev/treeforge-bootstrap-early-menu",
        O_WRONLY
        | O_CREAT
        | O_TRUNC,
        0644
    );

    if (marker >= 0) {
        tfb_write_all(
            marker,
            "treeforge-bootstrap-early-menu-v1\n"
        );

        tfb_close(
            marker
        );
    }

    tfb_probe_display_surfaces();

    if (
        sysfs_devices_seen
        > 0
    ) {
        tfb_log(
            "early-menu input-sysfs=present"
        );
    } else {
        tfb_log(
            "early-menu input-sysfs=absent"
        );
    }

    for (
        int index = 0;
        index < INPUT_COUNT;
        index++
    ) {
        char event_path[32];

        tfb_event_path(
            index,
            event_path
        );

        inputs[index] = tfb_open(
            event_path,
            O_RDONLY
            | O_NONBLOCK
        );

        if (
            inputs[index]
            < 0
        ) {
            continue;
        }

        touch_inputs[index] = (
            tfb_input_is_touchscreen(
                index
            )
        );

        tfb_touch_reset_state(
            &touch_states[index]
        );

        /*
         * Drain stale key state before displaying the live menu.
         */
        for (;;) {
            struct tfb_input_event
                event;

            long amount = (
                tfb_syscall3(
                    SYS_READ,
                    inputs[index],
                    (long) &event,
                    (long) sizeof(event)
                )
            );

            if (
                amount
                != (long)
                    sizeof(event)
            ) {
                break;
            }
        }
    }

    const struct tfb_menu_page
        *page_stack[
            TFB_MENU_STACK_DEPTH
        ];

    int stack_depth = 1;

    page_stack[0] = (
        tfb_menu_root_page()
    );

    const struct tfb_menu_page
        *page = page_stack[0];

    int selected = (
        tfb_menu_default_selection(
            page
        )
    );

    u32 elapsed_ms = 0;
    int timeout_fired = 0;

    /*
     * TFB_PP_REFRESH_CLOCK_V1
     *
     * Independent from page timeout bookkeeping. Pixel Partitioner
     * has timeout_ms == 0, but its host/log display still needs a
     * bounded periodic refresh while the page remains resident.
     */
    u32 pp_refresh_elapsed_ms = 0;

    /*
     * TFB_AUTOBOOT_CANCEL_ON_INTERACTION_V1
     *
     * Once the user deliberately interacts with the boot manager,
     * automatic boot is disabled for the remainder of this menu
     * session.
     *
     * This state is intentionally independent from timeout_fired.
     * Submenu navigation may reset page-local timeout bookkeeping;
     * it must never re-arm autoboot after user interaction.
     */
    int autoboot_canceled = 0;

    tfb_menu_status = 0;

    tfb_log(
        tfb_menu_profile_marker
    );

    tfb_log(
        tfb_menu_profile_id
    );

    tfb_log(
        "early-menu begin"
    );

    tfb_render_buffered(
        page,
        selected,
        elapsed_ms,
        timeout_fired
    );

    int rendered_selected =
        selected;

    last_adb_display_state =
        tfb_adb_display_state();

    tfb_log(
        "TFB_DIRTY_REGION_REDRAW_V17_1C"
    );

    for (;;) {
        int redraw = 0;

        for (
            int index = 0;
            index < INPUT_COUNT;
            index++
        ) {
            if (
                inputs[index]
                < 0
            ) {
                char event_path[32];

                tfb_event_path(
                    index,
                    event_path
                );

                inputs[index] = (
                    tfb_open(
                        event_path,
                        O_RDONLY
                        | O_NONBLOCK
                    )
                );

                if (
                    inputs[index]
                    >= 0
                ) {
                    touch_inputs[index] = (
                        tfb_input_is_touchscreen(
                            index
                        )
                    );

                    tfb_touch_reset_state(
                        &touch_states[index]
                    );

                    for (;;) {
                        struct tfb_input_event
                            event;

                        long amount = (
                            tfb_syscall3(
                                SYS_READ,
                                inputs[index],
                                (long) &event,
                                (long)
                                    sizeof(event)
                            )
                        );

                        if (
                            amount
                            != (long)
                                sizeof(event)
                        ) {
                            break;
                        }
                    }

                    continue;
                }
            }

            if (
                inputs[index]
                < 0
            ) {
                continue;
            }

            for (;;) {
                struct tfb_input_event
                    event;

                long amount = (
                    tfb_syscall3(
                        SYS_READ,
                        inputs[index],
                        (long) &event,
                        (long) sizeof(event)
                    )
                );

                if (
                    amount
                    != (long)
                        sizeof(event)
                ) {
                    if (
                        amount < 0
                        && amount != -11
                    ) {
                        tfb_close(
                            inputs[index]
                        );

                        inputs[index] =
                            -1;
                    }

                    break;
                }

                if (
                    touch_inputs[index]
                    && event.type == EV_ABS
                ) {
                    if (
                        event.code
                            == ABS_MT_TRACKING_ID
                        && event.value >= 0
                    ) {
                        if (!autoboot_canceled) {
                            autoboot_canceled = 1;

                            tfb_log(
                                "early-menu autoboot=canceled input=touch"
                            );
                        }

                        elapsed_ms = 0;
                        tfb_menu_status = 0;

                        redraw |=
                            TFB_REDRAW_STATUS;
                    }

                    int touch_result = (
                        tfb_touch_process_event(
                            &touch_states[index],
                            page,
                            &event,
                            &selected
                        )
                    );

                    if (touch_result != 0) {
                        elapsed_ms = 0;
                        tfb_menu_status = 0;
                    }

                    if (touch_result == 2) {
                        tfb_log(
                            "early-menu touch=select"
                        );

                        redraw |= (
                            TFB_REDRAW_SELECTION
                            | TFB_REDRAW_STATUS
                        );

                        continue;
                    }

                    if (touch_result == 3) {
                        tfb_log(
                            "early-menu touch=activate"
                        );

                        tfb_menu_activate_selected(
                            &page,
                            page_stack,
                            &stack_depth,
                            &selected,
                            &elapsed_ms,
                            &timeout_fired,
                            &redraw,
                            inputs,
                            created_input_directory,
                            created_input_nodes
                        );

                        continue;
                    }

                    if (touch_result != 0) {
                        continue;
                    }
                }

                if (
                    event.type != EV_KEY
                    || event.value != 1
                ) {
                    continue;
                }

                if (
                    event.code == KEY_VOLUMEUP
                    || event.code == KEY_VOLUMEDOWN
                    || event.code == KEY_POWER
                ) {
                    if (!autoboot_canceled) {
                        autoboot_canceled = 1;

                        tfb_log(
                            "early-menu autoboot=canceled input=button"
                        );
                    }

                    elapsed_ms = 0;
                    tfb_menu_status = 0;

                    redraw |=
                        TFB_REDRAW_STATUS;
                }

                if (
                    event.code
                    == KEY_VOLUMEUP
                ) {
                    tfb_log(
                        "early-menu key=volume_up"
                    );

                    selected = (
                        tfb_menu_move_selection(
                            page,
                            selected,
                            -1
                        )
                    );

                    redraw |= (
                        TFB_REDRAW_SELECTION
                        | TFB_REDRAW_STATUS
                    );

                    continue;
                }

                if (
                    event.code
                    == KEY_VOLUMEDOWN
                ) {
                    tfb_log(
                        "early-menu key=volume_down"
                    );

                    selected = (
                        tfb_menu_move_selection(
                            page,
                            selected,
                            1
                        )
                    );

                    redraw |= (
                        TFB_REDRAW_SELECTION
                        | TFB_REDRAW_STATUS
                    );

                    continue;
                }

                if (
                    event.code
                    != KEY_POWER
                ) {
                    continue;
                }

                tfb_log(
                    "early-menu key=power"
                );

                tfb_menu_activate_selected(
                    &page,
                    page_stack,
                    &stack_depth,
                    &selected,
                    &elapsed_ms,
                    &timeout_fired,
                    &redraw,
                    inputs,
                    created_input_directory,
                    created_input_nodes
                );
            }
        }

        if (
            page->timeout_ms != 0
            && !timeout_fired
            && !autoboot_canceled
        ) {
            if (
                elapsed_ms
                + POLL_INTERVAL_MS
                >= page->timeout_ms
            ) {
                elapsed_ms =
                    page->timeout_ms;

                timeout_fired = 1;

                tfb_log(
                    "early-menu timeout"
                );

                tfb_menu_execute_action(
                    page->timeout_action,
                    inputs,
                    created_input_directory,
                    created_input_nodes
                );

                redraw |=
                    TFB_REDRAW_STATUS;
            } else {
                u32 previous_second = (
                    elapsed_ms
                    / 1000U
                );

                elapsed_ms +=
                    POLL_INTERVAL_MS;

                if (
                    elapsed_ms
                    / 1000U
                    != previous_second
                ) {
                    redraw |=
                        TFB_REDRAW_STATUS;
                }
            }
        }

        tfb_pixel_partitioner_service(
            page,
            &redraw
        );

        int adb_display_state =
            tfb_adb_display_state();

        if (
            adb_display_state
            != last_adb_display_state
        ) {
            last_adb_display_state =
                adb_display_state;

            redraw |=
                TFB_REDRAW_ADB;
        }

        if (
            redraw
            & TFB_REDRAW_FULL
        ) {
            tfb_render_buffered(
                page,
                selected,
                elapsed_ms,
                (
                    timeout_fired
                    || autoboot_canceled
                )
            );

            rendered_selected =
                selected;
        } else {
            if (
                redraw
                & TFB_REDRAW_SELECTION
            ) {
                tfb_fb_render_selection_delta_buffered(
                    page,
                    rendered_selected,
                    selected
                );

                rendered_selected =
                    selected;
            }

            if (
                redraw
                & TFB_REDRAW_STATUS
            ) {
                tfb_fb_render_status_region_buffered(
                    page,
                    elapsed_ms,
                    (
                        timeout_fired
                        || autoboot_canceled
                    )
                );
            }

            if (
                redraw
                & TFB_REDRAW_ADB
            ) {
                tfb_fb_render_adb_region_buffered();
            }
        }

        /*
         * TFB_PP_PERIODIC_PANEL_REFRESH_V1
         *
         * The ordinary dirty-region system knows about selection,
         * footer/status, and the left-side ADB field.  Pixel
         * Partitioner additionally consumes host-written status/log
         * files, so refresh its own right-side panel at 4 Hz.
         *
         * No full-screen repaint is performed here.
         */
        if (
            tfb_pixel_partitioner_page(
                page
            )
        ) {
            pp_refresh_elapsed_ms +=
                POLL_INTERVAL_MS;

            if (
                pp_refresh_elapsed_ms
                >= TFB_PP_REFRESH_MS
            ) {
                tfb_fb_refresh_pixel_partitioner_buffered(
                    page,
                    selected
                );

                pp_refresh_elapsed_ms =
                    0;
            }
        } else {
            pp_refresh_elapsed_ms =
                0;
        }

        tfb_sleep_ms(
            POLL_INTERVAL_MS
        );
    }
}


__attribute__((noreturn))
