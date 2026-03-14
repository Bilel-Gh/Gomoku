# Carte du code - Fichier par fichier

Ce fichier te permet de retrouver rapidement OU est chaque fonctionnalite dans le code.
En soutenance, si on te demande "montre-moi le code de X", tu sais ou aller.

---

## include/constants.hpp
**LE fichier a ouvrir en premier pendant la soutenance.**

Contient TOUS les poids de l'IA, ranges par categorie :
- Lignes 7-15 : Scores des patterns (SCORE_FIVE, SCORE_OPEN_FOUR, etc.)
- Lignes 17-18 : Multiplicateurs de tour
- Lignes 20-26 : Scores des captures (escaladants)
- Lignes 28-32 : Priorites du tri des coups
- Lignes 34-39 : Parametres de recherche (taille TT, limites de temps, distance)
- Lignes 41-42 : Bonus de tempo, score de victoire
- Lignes 44-50 : Constantes GUI (taille cellule, marges, rayon des pierres)
- Lignes 52-57 : Vecteurs de direction (les 4 axes)

---

## include/gomoku.hpp
Le "header central" du projet. Tout le monde l'inclut.

- `enum Cell` : EMPTY=0, STONE_BLACK=1, STONE_WHITE=2
- `struct GameState` : le plateau (board[361]), captures, tour, hash, etc.
- `struct Move` : position + score (pour le tri)
- `struct MoveUndo` : sauvegarde pour annuler un coup dans la recherche IA
- `struct TTEntry` : entree de la table de transposition
- `struct Snapshot` : copie complete d'un etat (pour undo/redo)
- Fonctions utilitaires : `idx()`, `row_of()`, `col_of()`, `in_bounds()`, `opponent()`
- Declarations de toutes les fonctions du projet

---

## src/board.cpp - Plateau et regles de base

| Fonction | Ce qu'elle fait |
|----------|----------------|
| `init_game()` | Initialise le plateau a vide, noir commence |
| `check_captures()` | Compte les paires capturables dans les 8 dirs (sans les retirer) |
| `apply_captures()` | Retire les paires capturees et enregistre dans MoveUndo |
| `check_five()` | Verifie s'il y a 5+ en ligne a travers une position |
| `get_five_positions()` | Recupere les positions des pierres formant le 5 |
| `check_win()` | Check victoire (5 en ligne + endgame capture + capture win + nul) |
| `place_stone()` | Pose une pierre : validation, capture, victoire, changement de tour |
| `make_snapshot()` | Copie l'etat complet (pour undo) |
| `restore_snapshot()` | Restaure un etat sauvegarde |

**Ligne a montrer au correcteur :**
- `apply_captures` (vers ligne 35) : le pattern de capture X-O-O-X
- `check_five` (vers ligne 55) : le comptage dans 4 directions

---

## src/rules.cpp - Regles avancees

| Fonction | Ce qu'elle fait |
|----------|----------------|
| `is_free_three()` | Detecte un free-three sur un axe apres avoir pose une pierre |
| `count_free_threes()` | Compte les free-threes dans les 4 directions |
| `is_legal_move()` | Verifie si un coup est legal (vide + pas de double-trois) |
| `can_opponent_break_five()` | Verifie si l'adversaire peut casser un 5 par capture |

**Comment `is_free_three` marche :**
1. Construit un tableau `line[9]` : les 9 cellules autour de la position sur un axe
2. La pierre pas encore posee est simulee dans le tableau
3. Cherche les patterns `.XXX.` (Type A) et `.XX.X.`/`.X.XX.` (Types B/C)
4. Verifie que les deux bouts du pattern sont EMPTY

**`can_opponent_break_five` :**
1. Recupere les positions du 5-en-ligne
2. Pour chaque case vide du plateau, simule un coup adverse
3. Si ce coup capture une paire dont une pierre est dans le 5 -> return true
4. Si ce coup donne la 5eme paire a l'adversaire -> return true

---

## src/ai.cpp - Le cerveau de l'IA

| Fonction | Ce qu'elle fait |
|----------|----------------|
| `ai_apply_move()` | Pose une pierre rapidement (pour la recherche, pas le GUI) |
| `ai_undo_move()` | Annule une pose rapidement (restaure captures + hash) |
| `generate_candidates()` | Genere les coups dans un rayon de 2 |
| `order_moves()` | Trie les coups (gagnants > TT > captures > heuristique) |
| `minimax()` | Minimax recursif avec alpha-beta |
| `ai_get_move()` | Point d'entree : iterative deepening + gestion du temps |
| `ai_suggest_move()` | Version pour le hint (utilise une copie de l'etat) |

**La boucle d'iterative deepening (dans `ai_get_move`) :**
```
for depth = 2, 4, 6, 8, 10, 12...20 :
    lance minimax a cette profondeur
    si timeout -> break, garde resultat precedent
    si victoire trouvee -> break
    si temps > 0.40s -> break (pas assez pour la prochaine)
```

**La coupure alpha-beta (dans `minimax`) :**
```cpp
if (beta <= alpha)
    break;   // on coupe cette branche
```

**Controle du temps :**
```cpp
if ((node_count & 1023) == 0) {  // check toutes les 1024 noeuds
    if (temps > 0.45s) timed_out = true;
}
```

---

## src/eval.cpp - L'heuristique

| Fonction | Ce qu'elle fait |
|----------|----------------|
| `pattern_score()` | Retourne le score d'un type de pattern |
| `classify_pattern()` | Classe un pattern selon (count, open_ends, gaps) |
| `scan_line()` | Scanne les pierres sur un axe depuis une position |
| `evaluate()` | Evaluation complete du plateau |
| `count_capturable_pairs()` | Compte les paires capturables |
| `quick_eval_move()` | Evaluation rapide d'un coup (pour le tri) |

**La fonction `evaluate` fait :**
1. Pour chaque pierre du plateau, dans chaque direction (4 axes)
2. Si c'est le debut d'un groupe (pas de meme couleur avant)
3. Scanne le pattern, le classe, calcule le score
4. Applique le multiplicateur de tour si c'est le tour de ce joueur
5. Additionne les patterns de l'IA, soustrait ceux de l'adversaire
6. Ajoute le score des captures (escaladant)
7. Bonus de tempo (+1000 pour le joueur qui a le trait)

---

## src/zobrist.cpp - Hashing et cache

| Element | Description |
|---------|-------------|
| `zobrist_table[361][3]` | Nombres aleatoires 64-bit pour chaque (position, couleur) |
| `tt[1048576]` | Table de transposition, 1M entrees |
| `tt_generation` | Compteur de generation (pour remplacer les vieilles entrees) |
| `init_zobrist()` | Remplit la table avec un RNG a seed fixe (42) |
| `tt_lookup()` | Cherche une position dans le cache |
| `tt_store()` | Stocke un resultat dans le cache |
| `tt_clear()` | Vide le cache (utilise a chaque nouvelle partie) |

---

## src/gui.cpp - Interface SDL2

| Element | Description |
|---------|-------------|
| `themes[]` | Les 2 themes (clair et sombre) |
| `hoshi[]` | Les 9 points etoiles traditionnels du plateau |
| `draw_board()` | Dessine grille + pierres + indicateurs |
| `draw_heatmap()` | Overlay des scores sur les cases candidates |
| `draw_panel()` | Panneau de droite (infos + boutons) |
| `gui_draw()` | Appelle tout le rendu |
| `gui_handle_input()` | Gere le clic sur le plateau |
| `gui_poll_events()` | Gere les events SDL (clavier, souris, quit) |

---

## src/main.cpp - Boucle de jeu

La boucle principale :
```
while (!quit) :
    gui_poll_events()  <- events SDL + raccourcis clavier
    si c'est le tour du humain -> traiter le clic
    si c'est le tour de l'IA -> lancer ai_get_move(), poser le coup
    gui_draw()
```

**Les raccourcis :**
- N = nouvelle partie
- M = toggle PvAI/PvP
- T = toggle theme
- H = toggle hint
- D = toggle heatmap
- Z = undo (2 coups en PvAI, 1 en PvP)
- Y = redo

---

## Schema des appels pendant un tour IA

```
main.cpp: "c'est le tour de l'IA"
    |
    v
ai_get_move()                    <- src/ai.cpp
    |
    |-- generate_candidates()     <- genere 20-60 coups candidats
    |-- order_moves()             <- trie par priorite
    |-- for depth = 2, 4, 6...
    |       |
    |       v
    |   minimax()                 <- recursif, alpha-beta
    |       |
    |       |-- tt_lookup()       <- check le cache
    |       |-- generate_candidates()
    |       |-- order_moves()
    |       |-- for chaque coup:
    |       |       ai_apply_move()
    |       |       minimax(depth-1)  <- recursion
    |       |       ai_undo_move()
    |       |-- tt_store()        <- sauve dans le cache
    |       |
    |       v (aux feuilles, depth=0)
    |   evaluate()                <- src/eval.cpp
    |       |-- scan_line() x N   <- detecte les patterns
    |       |-- pattern_score()   <- score de chaque pattern
    |       |-- capture_score     <- score des captures
    |
    v
place_stone()                     <- src/board.cpp
    |-- apply_captures()
    |-- check_win()
```
