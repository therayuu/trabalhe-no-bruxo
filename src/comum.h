#ifndef _COMUM_H_
#define _COMUM_H_

#define ASSETS "../assets/"

#define W_WIDTH  1080
#define W_HEIGHT 720

#define FPS 60
#define TIMEOUT 1000/FPS

#define trans(prev, curr) (((uint16_t)prev<<8) | ((uint16_t)curr))
#define prox_diff(prev, curr) (((prev) != (curr)) ? curr : ZERO)
#define curr_diff(prev, curr) (((prev) != (curr)) ? prev : ZERO)

#define maior(prev, curr) (prev>curr ? prev : curr)
#define menor(prev, curr) (prev<curr ? prev : curr)
#define par(prev, curr) trans(menor(prev, curr), maior(prev, curr))

enum tela {
    ZERO = 0,
    MENU,
    MESA,
    DIALOGO,
};

typedef enum {
    AUX_NOEVENT = AUX_FIRSTUSEREVENT,
} UserEventType;

#endif//_COMUM_H_
