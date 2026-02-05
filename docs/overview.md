# Gomoku - Vue d'ensemble

## C'est quoi le Gomoku ?

Un jeu de plateau 19x19 (comme le Go). Deux joueurs posent des pierres a tour de role.
Le premier qui aligne 5 pierres (ou plus) gagne.

## Regles speciales (imposees par le sujet)

### 1. La capture
Si tu encadres 2 pierres ennemies avec les tiennes, elles sont retirees du plateau.

```
Avant:  NOIR . BLANC BLANC . NOIR
                              ^
                        NOIR joue ici

Apres:  NOIR . [vide] [vide] NOIR NOIR
```

Le pattern c'est : `X O O X` (X = toi, O = ennemi).
Ca marche dans les 8 directions (horizontal, vertical, 2 diagonales, dans les 2 sens).

**Capture = victoire** : si tu captures 10 pierres (= 5 paires), tu gagnes instantanement.

### 2. Capture de fin de partie (Endgame Capture)
Aligner 5 pierres ne suffit PAS toujours pour gagner.

L'adversaire a UN tour pour essayer de casser ta ligne en capturant une paire
qui fait partie de l'alignement. Si il peut le faire, le jeu continue.

Aussi : si l'adversaire a deja 4 paires capturees et qu'il peut en capturer une 5eme,
c'est LUI qui gagne (par capture).

### 3. Double-trois interdit
Tu ne peux PAS poser une pierre qui cree 2 "free-threes" en meme temps.

Un free-three = 3 pierres alignees avec les 2 bouts ouverts.
```
. X X X .     <- free-three (type A, contigu)
. X X . X .   <- free-three (type B, avec un trou)
. X . X X .   <- free-three (type C, avec un trou)
```

Si ta pierre cree 2 de ces patterns sur 2 axes differents = mouvement illegal.

**Exception** : si cette meme pierre capture une paire, le double-trois est autorise.

## L'IA

L'IA utilise l'algorithme **Minimax avec elagage Alpha-Beta** :

1. Elle genere un arbre de coups possibles
2. Elle simule chaque coup, puis chaque reponse, etc.
3. A chaque feuille (fin de branche), elle evalue la position avec une **heuristique**
4. Elle remonte le meilleur score en alternant MAX (pour elle) et MIN (pour toi)
5. Alpha-Beta coupe les branches inutiles (si un coup est deja pire que le meilleur connu)

**Profondeur** : l'IA cherche a depth 10+ grace a :
- Iterative deepening (elle commence a depth 2, puis 4, 6, 8, 10...)
- Table de transposition (cache de positions deja evaluees)
- Bonne ordonnancement des coups (les meilleurs en premier)

**Temps** : l'IA repond en moins de 0.5 seconde (en moyenne). Un timer s'affiche dans l'interface.

## L'interface (Raylib)

- Plateau a gauche avec grille 19x19
- Panneau d'info a droite (tour, timer, captures, boutons)
- Clic sur une intersection pour poser une pierre
- Preview semi-transparente quand tu survoles

## Raccourcis clavier

| Touche | Action |
|--------|--------|
| N | Nouvelle partie |
| M | Changer de mode (PvAI / PvP) |
| Z | Annuler (Undo) |
| Y | Refaire (Redo) |
| H | Indice (montre le coup suggere par l'IA) |
| T | Changer de theme (clair/sombre) |
| D | Heatmap de debug (scores de l'IA sur le plateau) |

## Structure du projet

```
Gomoku/
  Makefile
  include/
    constants.hpp   <- TOUS les poids de l'IA (ouvrir en soutenance)
    gomoku.hpp      <- structs, enums, declarations
    ai.hpp          <- fonctions de l'IA
    gui.hpp         <- fonctions de l'interface
  src/
    main.cpp        <- boucle de jeu
    board.cpp       <- plateau, poses, captures, victoire
    rules.cpp       <- free-three, double-trois, endgame capture
    ai.cpp          <- minimax, alpha-beta, iterative deepening
    eval.cpp        <- heuristique (evaluation de position)
    zobrist.cpp     <- hash table + table de transposition
    gui.cpp         <- affichage raylib
```
