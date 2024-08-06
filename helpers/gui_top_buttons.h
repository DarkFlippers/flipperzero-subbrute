#pragma once

#include <subghz_bruteforcer_icons.h>
#include <input/input.h>
#include <gui/elements.h>
#include <gui/icon.h>
#include <gui/icon_animation.h>

/**
 * @brief This function draws a button in the top left corner of the canvas with icon and string.
 *
 * The design and layout of the button is defined within this function.
 *
 *
 * @param[in] canvas This is a pointer to the @c Canvas structure where the button will be drawn.
 * @param[in] str This is a pointer to the character string that will be drawn within the button.
 *
 * @note Thanks to the author of metronome @see https://github.com/panki27/Metronome
 *
 */
void elements_button_top_left(Canvas* canvas, const char* str);

/**
 * @brief This function draws a button in the top right corner of the canvas with icon and string.
 *
 * The design and layout of the button is defined within this function.
 *
 *
 * @param[in] canvas This is a pointer to the @c Canvas structure where the button will be drawn.
 * @param[in] str This is a pointer to the character string that will be drawn within the button.
 *
 * @note Thanks to the author of metronome @see https://github.com/panki27/Metronome
 *
 */
void elements_button_top_right(Canvas* canvas, const char* str);
