#define ASSETS "../assets/"

#define W_WIDTH  1080
#define W_HEIGHT 720

#define FPS 60
#define TIMEOUT 1000/FPS

#define trans(prev, curr) (((uint16_t)prev<<8) | ((uint16_t)curr))
#define prox_diff(prev, curr) (((prev) != (curr)) ? curr : ZERO)
#define curr_diff(prev, curr) (((prev) != (curr)) ? prev : ZERO)


enum tela {
    ZERO = 0,
    MENU,
    MESA,
};

