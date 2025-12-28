#ifndef BUTTON_H
#define BUTTON_H

#define BTN_HIGHLIGHT_COLOR 0xFFFF

#define BUTTON_SFX_VOLUME 154 // 60% of MM_FULL_VOLUME

typedef void (*OnButtonPressedFunc)(void);
typedef bool (*ButtonCanBePressedFunc)(void);

// TODO: Document
typedef struct
{
    int border_pal_idx;
    int button_pal_idx;
    OnButtonPressedFunc on_pressed;
    ButtonCanBePressedFunc can_be_pressed;
} Button;

// TODO: Document
void button_set_highlight(Button* button, bool highlight);
void button_press(Button* button);

#endif // BUTTON_H