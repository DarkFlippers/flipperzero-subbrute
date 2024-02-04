#include "subbrute_main_view.h"
#include "../subbrute_i.h"
#include "../helpers/gui_top_buttons.h"

#include <input/input.h>
#include <gui/elements.h>

#define STATUS_BAR_Y_SHIFT 14
#define TAG "SubBruteMainView"

#define ITEMS_ON_SCREEN 3
#define ITEMS_INTERVAL 1
#define ITEM_WIDTH 14
#define ITEM_Y 27
#define ITEM_HEIGHT 13
#define TEXT_X 6
#define TEXT_Y 37
#define TEXT_INTERVAL 3
#define TEXT_WIDTH 12
#define ITEM_FRAME_RADIUS 2

struct SubBruteMainView {
    View* view;
    SubBruteMainViewCallback callback;
    void* context;
    uint8_t index;
    bool is_select_byte;
    bool two_bytes;
    uint64_t key_from_file;
    uint8_t repeat_values[SubBruteAttackTotalCount];
    uint8_t window_position;
};

typedef struct {
    uint8_t index;
    uint8_t repeat_values[SubBruteAttackTotalCount];
    uint8_t window_position;
    bool is_select_byte;
    bool two_bytes;
    uint64_t key_from_file;
} SubBruteMainViewModel;

void subbrute_main_view_set_callback(
    SubBruteMainView* instance,
    SubBruteMainViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void subbrute_main_view_center_displayed_key(
    Canvas* canvas,
    uint64_t key,
    uint8_t index,
    bool two_bytes) {
    uint8_t text_x = TEXT_X;
    uint8_t item_x = TEXT_X - ITEMS_INTERVAL;
    canvas_set_font(canvas, FontSecondary);

    for(int i = 0; i < 8; i++) {
        char current_value[3] = {0};
        uint8_t byte_value = (uint8_t)(key >> 8 * (7 - i)) & 0xFF;
        snprintf(current_value, sizeof(current_value), "%02X", byte_value);

        // For two bytes we need to select prev location
        if(!two_bytes && i == index) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rbox(
                canvas, item_x - 1, ITEM_Y, ITEM_WIDTH + 1, ITEM_HEIGHT, ITEM_FRAME_RADIUS);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str(canvas, text_x, TEXT_Y, current_value);
        } else if(two_bytes && (i == index || i == index - 1)) {
            if(i == index) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_rbox(
                    canvas,
                    item_x - ITEMS_INTERVAL - ITEM_WIDTH - 1,
                    ITEM_Y,
                    ITEM_WIDTH * 2 + ITEMS_INTERVAL * 2 + 1,
                    ITEM_HEIGHT,
                    ITEM_FRAME_RADIUS);

                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str(canvas, text_x, TEXT_Y, current_value);

                // Redraw prev element with white
                memset(current_value, 0, sizeof(current_value));
                byte_value = (uint8_t)(key >> 8 * (7 - i + 1)) & 0xFF;
                snprintf(current_value, sizeof(current_value), "%02X", byte_value);
                canvas_draw_str(
                    canvas, text_x - (TEXT_WIDTH + TEXT_INTERVAL), TEXT_Y, current_value);
            } else {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str(canvas, text_x, TEXT_Y, current_value);
            }
        } else {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_str(canvas, text_x, TEXT_Y, current_value);
        }
        text_x = text_x + TEXT_WIDTH + TEXT_INTERVAL;
        item_x = item_x + ITEM_WIDTH + ITEMS_INTERVAL;
    }

    // Return normal color
    canvas_set_color(canvas, ColorBlack);
}

void subbrute_main_view_draw_is_byte_selected(Canvas* canvas, SubBruteMainViewModel* model) {
#ifdef FURI_DEBUG
    //FURI_LOG_D(TAG, "key_from_file: %s", model->key_from_file);
#endif
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 17, AlignCenter, AlignTop, "Please select values to calc:");

    subbrute_main_view_center_displayed_key(
        canvas, model->key_from_file, model->index, model->two_bytes);

    elements_button_center(canvas, "Select");
    if(model->index > 0) {
        elements_button_left(canvas, " ");
    }
    if(model->index < 7) {
        elements_button_right(canvas, " ");
    }

    // Switch to another mode
    if(model->two_bytes) {
        elements_button_top_left(canvas, "One byte");
    } else {
        elements_button_top_left(canvas, "Two bytes");
    }
}

void subbrute_main_view_draw_is_ordinary_selected(Canvas* canvas, SubBruteMainViewModel* model) {
    uint16_t screen_width = canvas_width(canvas);
    uint16_t screen_height = canvas_height(canvas);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), STATUS_BAR_Y_SHIFT);
    canvas_invert_color(canvas);
    canvas_draw_str_aligned(canvas, 64, 3, AlignCenter, AlignTop, SUBBRUTEFORCER_VER);
    canvas_invert_color(canvas);

    // Menu
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    const uint8_t item_height = 16;
    const uint8_t string_height_offset = 9;

#ifdef FURI_DEBUG
    //FURI_LOG_D(TAG, "window_position: %d, index: %d", model->window_position, model->index);
#endif
    for(size_t position = 0; position < SubBruteAttackTotalCount; ++position) {
        uint8_t item_position = position - model->window_position;

        if(item_position < ITEMS_ON_SCREEN) {
            if(model->index == position) {
                canvas_draw_str_aligned(
                    canvas,
                    3,
                    string_height_offset + (item_position * item_height) + STATUS_BAR_Y_SHIFT,
                    AlignLeft,
                    AlignCenter,
                    subbrute_protocol_name(position));

                elements_frame(
                    canvas, 1, 1 + (item_position * item_height) + STATUS_BAR_Y_SHIFT, 124, 15);
            } else {
                canvas_draw_str_aligned(
                    canvas,
                    4,
                    string_height_offset + (item_position * item_height) + STATUS_BAR_Y_SHIFT,
                    AlignLeft,
                    AlignCenter,
                    subbrute_protocol_name(position));
            }

            uint8_t current_repeat_count = model->repeat_values[position];
            uint8_t min_repeat_count = subbrute_protocol_repeats_count(position);

            if(current_repeat_count > min_repeat_count) {
#ifdef FW_ORIGIN_Official
                canvas_set_font(canvas, FontSecondary);
#else
                canvas_set_font(canvas, FontBatteryPercent);
#endif
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "x%d", current_repeat_count);
                uint8_t temp_x_offset_repeats =
                    current_repeat_count <= SUBBRUTE_PROTOCOL_MAX_REPEATS ? 15 : 18;

                canvas_draw_str_aligned(
                    canvas,
                    screen_width - temp_x_offset_repeats,
                    string_height_offset + (item_position * item_height) + STATUS_BAR_Y_SHIFT,
                    AlignLeft,
                    AlignCenter,
                    buffer);
                canvas_set_font(canvas, FontSecondary);
            }
        }
    }

    elements_scrollbar_pos(
        canvas,
        screen_width,
        STATUS_BAR_Y_SHIFT + 2,
        screen_height - STATUS_BAR_Y_SHIFT,
        model->index,
        SubBruteAttackTotalCount);
}

void subbrute_main_view_draw(Canvas* canvas, SubBruteMainViewModel* model) {
    if(model->is_select_byte) {
        subbrute_main_view_draw_is_byte_selected(canvas, model);
    } else {
        subbrute_main_view_draw_is_ordinary_selected(canvas, model);
    }
}

bool subbrute_main_view_input_file_protocol(InputEvent* event, SubBruteMainView* instance) {
    bool updated = false;
    if(event->key == InputKeyLeft) {
        if((instance->index > 0 && !instance->two_bytes) ||
           (instance->two_bytes && instance->index > 1)) {
            instance->index--;
        }
        updated = true;
    } else if(event->key == InputKeyRight) {
        if(instance->index < 7) {
            instance->index++;
        }
        updated = true;
    } else if(event->key == InputKeyUp) {
        instance->two_bytes = !instance->two_bytes;
        // Because index is changing
        if(instance->two_bytes && instance->index < 7) {
            instance->index++;
        }
        // instance->callback(
        //     instance->two_bytes ? SubBruteCustomEventTypeChangeStepUp :
        //                           SubBruteCustomEventTypeChangeStepDown,
        //     instance->context);

        updated = true;
    } else if(event->key == InputKeyOk) {
        instance->callback(SubBruteCustomEventTypeIndexSelected, instance->context);
        updated = true;
    }
    return updated;
}

bool subbrute_main_view_input_ordinary_protocol(
    InputEvent* event,
    SubBruteMainView* instance,
    bool is_short) {
    const uint8_t min_value = 0;
    const uint8_t correct_total = SubBruteAttackTotalCount - 1;
    uint8_t index = instance->index;
    uint8_t min_repeats = subbrute_protocol_repeats_count(index);
    uint8_t max_repeats = min_repeats * 3;
    uint8_t current_repeats = instance->repeat_values[index];

    bool updated = false;
    if(event->key == InputKeyUp && is_short) {
        if(index == min_value) {
            instance->index = correct_total;
        } else {
            instance->index = CLAMP(index - 1, correct_total, min_value);
        }
        //instance->repeat_values = 0;
        updated = true;
    } else if(event->key == InputKeyDown && is_short) {
        if(index == correct_total) {
            instance->index = min_value;
        } else {
            instance->index = CLAMP(index + 1, correct_total, min_value);
        }
        //instance->repeat_values = 0;
        updated = true;
    } else if(event->key == InputKeyLeft && is_short) {
        instance->repeat_values[index] = CLAMP(current_repeats - 1, max_repeats, min_repeats);

        updated = true;
    } else if(event->key == InputKeyRight && is_short) {
        instance->repeat_values[index] = CLAMP(current_repeats + 1, max_repeats, min_repeats);

        updated = true;
    } else if(event->key == InputKeyOk && is_short) {
        if(index == SubBruteAttackLoadFile) {
            instance->callback(SubBruteCustomEventTypeLoadFile, instance->context);
        } else {
            instance->callback(SubBruteCustomEventTypeMenuSelected, instance->context);
        }
        updated = true;
    }

    if(updated) {
        instance->window_position = instance->index;
        if(instance->window_position > 0) {
            instance->window_position -= 1;
        }

        if(SubBruteAttackTotalCount <= ITEMS_ON_SCREEN) {
            instance->window_position = 0;
        } else {
            if(instance->window_position >= (SubBruteAttackTotalCount - ITEMS_ON_SCREEN)) {
                instance->window_position = (SubBruteAttackTotalCount - ITEMS_ON_SCREEN);
            }
        }
    }

    return updated;
}

bool subbrute_main_view_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SubBruteMainView* instance = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
#ifdef FURI_DEBUG
        FURI_LOG_I(TAG, "InputKey: BACK");
#endif
        instance->callback(SubBruteCustomEventTypeBackPressed, instance->context);
        return false;
    }

#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG,
        "InputKey: %d, extra_repeats: %d",
        event->key,
        instance->repeat_values[instance->index]);
#endif

    bool updated = false;
    bool is_short = (event->type == InputTypeShort) || (event->type == InputTypeRepeat);

    if(instance->is_select_byte) {
        if(is_short) {
            updated = subbrute_main_view_input_file_protocol(event, instance);
        }
    } else {
        updated = subbrute_main_view_input_ordinary_protocol(event, instance, is_short);
    }

    if(updated) {
        with_view_model(
            instance->view,
            SubBruteMainViewModel * model,
            {
                model->index = instance->index;
                model->window_position = instance->window_position;
                model->key_from_file = instance->key_from_file;
                model->is_select_byte = instance->is_select_byte;
                model->two_bytes = instance->two_bytes;
                model->repeat_values[model->index] = instance->repeat_values[instance->index];
            },
            true);
    }

    return updated;
}

void subbrute_main_view_enter(void* context) {
    furi_assert(context);
}

void subbrute_main_view_exit(void* context) {
    furi_assert(context);
}

SubBruteMainView* subbrute_main_view_alloc() {
    SubBruteMainView* instance = malloc(sizeof(SubBruteMainView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubBruteMainViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subbrute_main_view_draw);
    view_set_input_callback(instance->view, subbrute_main_view_input);
    view_set_enter_callback(instance->view, subbrute_main_view_enter);
    view_set_exit_callback(instance->view, subbrute_main_view_exit);

    instance->index = 0;
    instance->window_position = 0;
    instance->key_from_file = 0;
    instance->is_select_byte = false;
    instance->two_bytes = false;

    with_view_model(
        instance->view,
        SubBruteMainViewModel * model,
        {
            model->index = instance->index;
            model->window_position = instance->window_position;
            model->key_from_file = instance->key_from_file;
            model->is_select_byte = instance->is_select_byte;
            model->two_bytes = instance->two_bytes;
        },
        true);

    return instance;
}

void subbrute_main_view_free(SubBruteMainView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* subbrute_main_view_get_view(SubBruteMainView* instance) {
    furi_assert(instance);
    return instance->view;
}

void subbrute_main_view_set_index(
    SubBruteMainView* instance,
    uint8_t idx,
    const uint8_t* repeats,
    bool is_select_byte,
    bool two_bytes,
    uint64_t key_from_file) {
    furi_assert(instance);
    furi_assert(idx < SubBruteAttackTotalCount);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Set index: %d, is_select_byte: %d", idx, is_select_byte);
#endif
    for(size_t i = 0; i < SubBruteAttackTotalCount; i++) {
        instance->repeat_values[i] = repeats[i];
    }
    instance->is_select_byte = is_select_byte;
    instance->two_bytes = two_bytes;
    instance->key_from_file = key_from_file;
    instance->index = idx;
    instance->window_position = idx;

    if(!is_select_byte) {
        if(instance->window_position > 0) {
            instance->window_position -= 1;
        }

        if(instance->window_position >= (SubBruteAttackTotalCount - ITEMS_ON_SCREEN)) {
            instance->window_position = (SubBruteAttackTotalCount - ITEMS_ON_SCREEN);
        }
    }

    with_view_model(
        instance->view,
        SubBruteMainViewModel * model,
        {
            model->index = instance->index;
            model->window_position = instance->window_position;
            model->key_from_file = instance->key_from_file;
            model->is_select_byte = instance->is_select_byte;
            model->two_bytes = instance->two_bytes;
            for(size_t i = 0; i < SubBruteAttackTotalCount; i++) {
                model->repeat_values[i] = repeats[i];
            }
        },
        true);
}

SubBruteAttacks subbrute_main_view_get_index(SubBruteMainView* instance) {
    furi_assert(instance);
    return instance->index;
}

const uint8_t* subbrute_main_view_get_repeats(SubBruteMainView* instance) {
    furi_assert(instance);
    return instance->repeat_values;
}

bool subbrute_main_view_get_two_bytes(SubBruteMainView* instance) {
    furi_assert(instance);
    return instance->two_bytes;
}
