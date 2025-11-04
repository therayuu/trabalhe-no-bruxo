---
title: Trabalhe no Bruxó
subtitle: Avanços 2
author: Rafaela Grillo & Theo Albuquerque & Yuri Sacksida
...

# Estado do jogo

## Menu

![menu atual do jogo](img/menu-avanços-2.png)

## Elementos

![cartas organizadas](img/cartas-organizadas-avanços-2.png)

## Elementos

![cartas desorganizadas](img/cartas-desorganizadas-avanços-2.png)

## Diálogos

Criamos os diálogos da história central e duas rotas que o jogador pode seguir.

![](img/intro-dialogo.png "Dialogo de introdução")
![](img/pedro-dialogo.png "Dialogo com um cliente")

A princípio o diálogo e rotas seriam estruturados por uma árvore encadeada, mas foi decidido que o texto será adaptado para Markdown e um interpretador o transformará nessa estrutura de dados.

## Arte
A arte das cartas tem sprites, um que é um fundo igual para todas as cartas e outro que é um sprite transparente com a arte dos elementos.

As artes foram feitas no aseprite.

![](../assets/carta-agua.png "Carta do elemento de água")
![](../assets/carta-fogo.png "Carta do elemento de fogo")
![](../assets/carta-ar.png "Carta do elemento de ar")
![](../assets/carta-terra.png "Carta do elemento de terra")

# Demonstração

# Código

## Geral

O programa principal é uma máquina de estados de "telas".

Cada tela precisa fornecer 3 funções:
- [tela]_setup(renderer*)
- [tela]_loop(renderer*, event)
- [tela]_free(void)

Internamente, cada tela tem um struct com [partes d]o seu estado.

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

![](maquina-menu.png "Máquina de Estados do Menu")

## Mesa / Cartas

![drag&drop](img/drag&drop-avanços-2.png)
