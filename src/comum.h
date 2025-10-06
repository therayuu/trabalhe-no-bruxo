#define ASSETS "../assets/"

#define W_WIDTH  1080
#define W_HEIGHT 720

#define FPS 60
#define TIMEOUT 1000/FPS

#define trans(prev, curr) (((uint16_t)prev<<8) | ((uint16_t)curr))


enum tela {
    ZERO = 0,
    MENU,
    MESA,
};

