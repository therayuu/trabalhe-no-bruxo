#ifndef _AUX_H_
#define _AUX_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>
#include "cores.h"


/* MACROS */
#define LEN(arr) (sizeof(arr)/sizeof(*arr))
#define UNUSED(x) (void) x


/* EVENTOS */
#define DT(...) AUX_dt(__VA_ARGS__)
int32_t AUX_dt(uint32_t antes, uint32_t* depois) {
    uint32_t agora = SDL_GetTicks();
    uint32_t delta = agora - antes;

    if (depois) *depois = agora;
    return delta;
}

bool AUX_WaitEventTimeoutCount(SDL_Event* evt, uint32_t* ms) {
    static uint32_t antes;
    if (antes == 0) antes = SDL_GetTicks();

    bool evento = SDL_WaitEventTimeout(evt, *ms);
    if (evento) {
        uint32_t delta = DT(antes, &antes);
        *ms = (delta < *ms) ? *ms - delta : 0;
    }
    return evento;
}

bool AUX_WaitEventTimeout(SDL_Event* evt, uint32_t* ms, uint32_t timeout) {
    bool evento = AUX_WaitEventTimeoutCount(evt, ms);
    if (!evento) *ms = timeout;

    return evento;
}


/* MATEMÁTICA */
void TFX_ClampRectPos(SDL_Rect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x;
    if (ret->y < win.y) ret->y = win.y;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x+win.w - ret->w;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y+win.h - ret->h;
}

void TFX_ClampRectPosF(SDL_FRect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x;
    if (ret->y < win.y) ret->y = win.y;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x+win.w - ret->w;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y+win.h - ret->h;
}

void TFX_WrapRectPos(SDL_Rect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x+win.w - ret->w;
    if (ret->y < win.y) ret->y = win.y+win.h - ret->h;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y;
}

void TFX_WrapRectPosF(SDL_FRect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x+win.w - ret->w;
    if (ret->y < win.y) ret->y = win.y+win.h - ret->h;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y;
}


/* GRÁFICOS */
SDL_Color AUX_GetRenderDrawColor(SDL_Renderer *renderer) {
    SDL_Color cor;
    SDL_GetRenderDrawColor(renderer, splat(&cor));
    return cor;
}

void AUX_SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color cor) {
    SDL_SetRenderDrawColor(renderer, splat(cor));
}

void AUX_RenderClearColor(SDL_Renderer* renderer, SDL_Color cor) {
    AUX_SetRenderDrawColor(renderer, cor);
    SDL_RenderClear(renderer);
}

//! cortar em vez de amassar a imagem
void AUX_RenderBackgroundImage(SDL_Renderer* renderer, SDL_Texture* img) {
    SDL_Rect janela = {0};
    SDL_GetRendererOutputSize(renderer, &janela.w, &janela.h);

    SDL_RenderCopy(renderer, img, NULL, &janela);
}

#endif//_AUX_H_
