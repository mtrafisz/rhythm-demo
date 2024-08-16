#include <assert.h>

#include <raylib.h>
#include <car.h>

#define color_from_hex(hex) ((Color) {(hex >> 24) & 0xff, (hex >> 16) & 0xff, (hex >> 8) & 0xff, hex & 0xff})
#define in_range(x, l, r) ((x < r) && (x > l))

#define NOTE_ACTIVE_DELTA 250
Color DAIDARK = (Color){0x18, 0x18, 0x18, 0xff};
Color KISSSHOTYELLOW = color_from_hex(0xefd683ff);

typedef uint32_t time_ms;

typedef enum {
    STATE_INACTIVE,
    STATE_ACTIVE,
    STATE_MISSED,
    STATE_CLICKED           // TODO: how clicked (BAD, GOOD, PERFECT, etc)
} NoteState;

typedef enum {
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4
} InputButton;

typedef struct {
    InputButton button;
    time_ms target_time;
    NoteState state;
} Note, *NotePtr;

void note_draw(NotePtr note, Rectangle rect) {
    DrawCircle(rect.x + rect.width / 2, rect.y + rect.height / 2, rect.width / 2, DAIDARK);
    Color rim_color = RAYWHITE;

    switch (note->state) {
    case STATE_ACTIVE:
        rim_color = GREEN;
        break;
    case STATE_MISSED:
        rim_color = RED;
        break;
    case STATE_CLICKED:
        rim_color = KISSSHOTYELLOW;
        break;
    }

    DrawCircleLines(rect.x + rect.width / 2, rect.y + rect.height / 2, rect.width / 2, rim_color);

    char* rune;
    switch (note->button) {
        case BUTTON_1:
            rune = "Z"; break;
        case BUTTON_2:
            rune = "X"; break;
        case BUTTON_3:
            rune = "C"; break;
        case BUTTON_4:
            rune = "V"; break;
        default:
            rune = "?";
    }

    DrawTextEx(GetFontDefault(), rune, (Vector2) {rect.x + rect.width * 2/7, rect.y + rect.height * 1/5}, 20, 0, WHITE);
}

typedef struct {
    time_ms global_time;
    time_ms start_time;
    time_ms duration;
    Array(Note) notes;
} NoteWindow, *NoteWindowPtr;

#define note_to_window_time_diff(note, window) (abs(note->target_time - (window->global_time - window->start_time)))

NoteWindow note_window_new(time_ms start, time_ms duration) {
    return (NoteWindow) {
        .global_time = 0,
        .duration = duration,
        .start_time = start,
        .notes = array(Note, STD_ALLOCATOR)
    };
}

void note_window_destroy(NoteWindowPtr w) {
    assert(w != NULL);
    array_destroy(&w->notes);
}

void note_window_add_note(NoteWindowPtr w, InputButton button, time_ms relative_target_time) {
    assert(w != NULL);

    Note n = {
        .button = button,
        .state = STATE_INACTIVE,
        .target_time = relative_target_time
    };

    array_append(&w->notes, &n);
}

void note_window_draw(NoteWindowPtr w, Rectangle rect) {
    float rect_mid_y = rect.y + rect.height / 2;

    DrawRectangleLinesEx((Rectangle) {rect.x - 3.0, rect.y, rect.width + 6.0, rect.height}, 3.0f, RAYWHITE);
    DrawLineEx((Vector2) {rect.x, rect_mid_y}, (Vector2) {rect.x + rect.width, rect_mid_y}, 3.0f, BLUE);

    time_ms dt = 500;
    uint32_t spaces = w->duration / dt;
    uint32_t space_px = rect.width / spaces;

    for (uint32_t i = 1; i < spaces; ++i) {
        float x = rect.x + i * space_px;
        DrawLineEx((Vector2) {x, rect_mid_y - rect.height / 15}, (Vector2) {x, rect_mid_y + rect.height / 15}, 1.0f, BLUE);
    }

    if (in_range(w->global_time, w->start_time, w->start_time + w->duration)) {
        float timer_x = (rect.x + 2.0) + (((float)(w->global_time - w->start_time) / (float)w->duration) * (rect.width - 4.0));
        DrawLineEx((Vector2) {timer_x, rect_mid_y - rect.height / 7}, (Vector2) {timer_x, rect_mid_y + rect.height / 7}, 2.0f, RED);
    }

    uint32_t note_rect_size = 30;
    ArrayIterator it = iterator(&w->notes);
    NotePtr n;
    while ((n = iterator_next(&it))) {
        note_draw(
            n,
            (Rectangle) {
                ((float)n->target_time / (float)w->duration) * (rect.width - 4.0) - note_rect_size / 2 + rect.x,
                rect_mid_y - note_rect_size / 2,
                note_rect_size,
                note_rect_size
            }
        );
    }
}

void note_window_update(NoteWindowPtr w, time_ms dt) {
    assert(w != NULL);
    
    w->global_time += dt;

    if (w->global_time < w->start_time) return;

    ArrayIterator note_it = iterator(&w->notes);
    NotePtr curr_n;
    while ((curr_n = iterator_next(&note_it))) {
        if (note_to_window_time_diff(curr_n, w) <= NOTE_ACTIVE_DELTA && curr_n->state == STATE_INACTIVE) {
            curr_n->state = STATE_ACTIVE;
        } else if (note_to_window_time_diff(curr_n, w) > NOTE_ACTIVE_DELTA && curr_n->state == STATE_ACTIVE) {
            curr_n->state = STATE_MISSED;
        }
    }
}

void note_window_reset(NoteWindowPtr w) {
    w->global_time = 0;

    ArrayIterator note_it = iterator(&w->notes);
    NotePtr curr_n;
    while ((curr_n = iterator_next(&note_it))) {
        curr_n->state = STATE_INACTIVE;
    }
}

typedef struct {
    union {
        struct {
            uint8_t a : 1;
            uint8_t b : 1;
            uint8_t c : 1;
            uint8_t d : 1;
            uint8_t r : 1;
            // unused:
            uint8_t btnZ : 1;
            uint8_t btnX : 1;
            uint8_t btnC : 1;
        } btn;      // why can't I just bitshift bitfield? I hate C standard smh...
        uint8_t i;  // I must use smelly unions :( (I could convert by hand, but I wont)
        // TODO: Someone on stackoverflow wrote, that union bitfields aren't portable. Verify.
    };
} InputField;

void note_window_input(NoteWindowPtr w, InputField input) {
    if (input.btn.r) {
        note_window_reset(w);
        return;
    }

    ArrayIterator note_it = iterator(&w->notes);
    NotePtr curr_n;
    NotePtr active = NULL;
    time_ms min_diff = UINT32_MAX;
    while ((curr_n = iterator_next(&note_it))) {
        time_ms d = note_to_window_time_diff(curr_n, w);
        if (d < min_diff && curr_n->state == STATE_ACTIVE) {
            active = curr_n;
            min_diff = d;
        }
    }

    if (active == NULL) return;

    if ((input.i >> active->button) & 0b00000001) {
        active->state = STATE_CLICKED;
    }
}

int main(void) {
    InitWindow(800, 600, "Super Ultra Hyper Miracle Romantic Window :3");
    InitAudioDevice();

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    NoteWindow w = note_window_new(1000, 4000);
    NoteWindow w2 = note_window_new(5300, 5000);
    note_window_add_note(&w, BUTTON_1, 1000);
    note_window_add_note(&w, BUTTON_4, 1300);
    note_window_add_note(&w, BUTTON_2, 2400);
    note_window_add_note(&w, BUTTON_3, 3750);
    note_window_add_note(&w2, BUTTON_1, 2137);
    note_window_add_note(&w2, BUTTON_4, 4269);

    while (!WindowShouldClose()) {
        InputField field = {
            .btn = {
                .a = IsKeyPressed(KEY_Z),
                .b = IsKeyPressed(KEY_X),
                .c = IsKeyPressed(KEY_C),
                .d = IsKeyPressed(KEY_V),
                .r = IsKeyPressed(KEY_R),
            }
        };

        note_window_input(&w, field);
        note_window_input(&w2, field);

        time_ms dt = GetFrameTime() * 1000;
        note_window_update(&w, dt);
        note_window_update(&w2, dt);

        BeginDrawing();
            ClearBackground(DAIDARK);
            note_window_draw(&w, (Rectangle) {25.0f, 25.0f, 400.0f, 150.0f});
            note_window_draw(&w2, (Rectangle) {25.0f, 250.0f, 400.0f, 150.0f});
        EndDrawing();
    }

    note_window_destroy(&w);
    note_window_destroy(&w2);

    CloseAudioDevice();
    CloseWindow();
}
