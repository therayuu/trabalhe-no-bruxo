---
title: Trabalhe no Bruxó
subtitle: Avanços 2
author: Rafaela Grillo & Theo Albuquerque & Yuri Sacksida
...

# Estado do jogo

## Menu

![Menu atual do jogo](img/menu-avanços-2.png)

## Elementos

![Cartas organizadas](img/cartas-organizadas-avanços-2.png)

## Elementos

![Cartas desorganizadas](img/cartas-desorganizadas-avanços-2.png)

## Diálogos

Da história central e de duas rotas que o jogador pode seguir.

![Diálogo de introdução](img/diálogo-intro-avanços-2.png)
![Diálogo com um cliente](img/diálogo-pedro-avanços-2.png)

A princípio o diálogo e rotas seriam estruturados por uma árvore encadeada, mas foi decidido que o texto será adaptado para Markdown e um parser o transformará nessa estrutura de dados (que o nosso programa interpretaria).

## Arte

Cada carta é feita com 2 sprites, um que é um fundo igual para todas as cartas e outro que é semi-transparente com a arte específica dos elementos.

As artes foram feitas no aseprite\*

![Fundo das cartas](../assets/fundo_carta.png)
![Carta do elemento de água](../assets/carta_agua.png)
![Carta do elemento de fogo](../assets/carta_fogo.png)
![Carta do elemento de ar](../assets/carta_ar.png)
![Carta do elemento de terra](../assets/carta_terra.png)

\*(compilado na mão).

# Demonstração

# Código

## Geral

O programa principal é uma máquina de estados de "telas".

Cada tela precisa fornecer 3 funções:
- [tela]_setup(renderer\*)
- [tela]_loop(renderer\*, event)
- [tela]_free(void)

Internamente, cada tela teria um struct com o seu estado.\*

## Geral

Atualmente temos 2 telas:
- MENU
- MESA

## Geral

\center \small
```c
int main() { /* ... */
    enum tela tela = ZERO;
    uint32_t falta = TIMEOUT;
    for (SDL_Event evt; evt.type != SDL_QUIT; ) {
        AUX_NextEvent(&evt, &falta, TIMEOUT);
        enum tela prox;
        switch (tela) {
            case ZERO: prox = MENU; break;
            case MENU: prox = menu_loop(ren, evt); break;
            case MESA: prox = mesa_loop(ren, evt); break;
        }
        switch (prox_diff(tela, prox)) {
            case ZERO: break;
            case MENU: menu_setup(ren); break;
            case MESA: mesa_setup(ren); break;
        }
        /* ... */
        tela = prox;
    }
    /* ... */
}
```

## Geral

\center \small
```c
int main() { /* ... */
    enum tela tela = ZERO;
    uint32_t falta = TIMEOUT;
    for (SDL_Event evt; evt.type != SDL_QUIT; ) {
        AUX_NextEvent(&evt, &falta, TIMEOUT);
        /* ... */
        switch (curr_diff(tela, prox)) {
            case ZERO: break;
            case MENU: menu_free(); break;
            case MESA: mesa_free(); break;
        }
        switch (trans(tela, prox)) {
            case trans(MENU, ZERO): evt.type = SDL_QUIT; break;
        }
        /* ... */
    }
    /* ... */
}
```

## Menu

## Mesa

# Máquinas de Estados

## Menu

![Máquina de Estados do Menu](img/máquina-menu-avanços-2.png)

## Mesa / Cartas

![Drag&Drop](img/máquina-drag&drop-avanços-2.png)
