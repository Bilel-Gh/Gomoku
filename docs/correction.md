# Guide de correction - Etape par etape

Ce fichier suit EXACTEMENT l'ordre de la grille de correction.
Pour chaque section, je te dis quoi faire et quoi dire.

---

## ETAPE 1 : Verifications preliminaires

Le correcteur verifie :
- [x] Il y a du contenu dans le repo git
- [x] Le Makefile est present et a les regles requises

**Ce que tu fais :**
```bash
make          # compile tout
make re       # recompile depuis zero
make clean    # supprime les .o
make fclean   # supprime les .o ET le binaire
make          # re-compile, verifie que ca marche
```

**Ce que tu dis :**
"Le Makefile a les 5 regles obligatoires : $(NAME), all, clean, fclean, re.
Il ne relink pas si rien n'a change."

---

## ETAPE 2 : Le programme ne crash pas

Le correcteur va essayer de faire crasher le programme. S'il crash = 0.

**Ce que tu fais :**
- Lance le jeu : `./Gomoku`
- Clique partout rapidement
- Clique en dehors du plateau
- Clique sur des pierres deja posees
- Appuie sur toutes les touches
- Joue jusqu'a la fin d'une partie puis continue a cliquer

**Ce que tu dis :**
"Le programme verifie les bornes partout. Un clic invalide est simplement ignore.
Aucune allocation dynamique risquee, le plateau est un tableau statique de 361 octets."

---

## ETAPE 3 : Regles (Yes/No)

Le correcteur verifie que les regles sont correctement implementees.

### 3.1 Poser des pierres
- Lancer `./Gomoku`
- Cliquer sur des intersections : les pierres alternent noir/blanc
- **Dire :** "Les deux joueurs alternent. Noir commence toujours."

### 3.2 Capture
- En mode PvP (appuie sur M), placer des pierres pour creer le pattern `X O O .`
- Puis jouer le X manquant pour encadrer : `X O O X`
- Les 2 pierres ennemies disparaissent, le compteur de captures augmente

**Dire :** "La capture fonctionne dans les 8 directions. On verifie le pattern
X-O-O-X dans chaque direction quand une pierre est posee. Le code est dans
board.cpp, fonction `apply_captures`."

### 3.3 Victoire par alignement
- Aligner 5 pierres -> "Black wins!" ou "White wins!"
- **Dire :** "On scanne les 4 directions depuis la pierre posee et on compte
les pierres consecutives. Si count >= 5, c'est un alignement."

### 3.4 Victoire par capture
- Capturer 5 paires (10 pierres) -> victoire instantanee
- **Dire :** "Apres chaque capture, on verifie si captures[joueur] >= 5."

### 3.5 Endgame Capture
C'est le plus dur a demontrer. En mode PvP :
- Creer une situation ou un joueur a 5 en ligne MAIS l'adversaire peut
  capturer une paire QUI FAIT PARTIE de l'alignement
- Le jeu doit CONTINUER (pas de victoire immediate)

**Dire :** "Quand un alignement de 5 est detecte, on simule TOUS les coups possibles
de l'adversaire. Si l'un d'eux capture une paire de l'alignement, le jeu continue.
Le code est dans rules.cpp, fonction `can_opponent_break_five`."

### 3.6 Double-trois interdit
En mode PvP :
- Placer des pierres pour creer une situation de double free-three
- Essayer de poser la pierre qui creerait 2 free-threes : ca ne marche pas
- Le clic est simplement ignore (mouvement illegal)

**Dire :** "Avant chaque pose, on scanne les 4 directions pour detecter les free-threes.
Si le compte >= 2, le coup est rejete. Exception : si le meme coup capture une paire,
il est autorise. Le code est dans rules.cpp, fonction `count_free_threes`."

---

## ETAPE 4 : UI et performance de l'IA (Score 0-5)

Le correcteur joue contre l'IA et note selon :
- IA met plus de 0.5s ou pas de timer = 0
- Le joueur gagne en moins de 10 tours = 0
- Le joueur gagne en 10-20 tours = 1
- Le joueur gagne apres 20+ tours = 2
- Match nul = 3
- L'IA gagne apres 20+ tours = 4
- L'IA gagne en moins de 20 tours = 5

**Ce que tu fais :**
- Lance `./Gomoku` (mode PvAI par defaut)
- Montre le timer dans le panneau de droite ("AI time: 0.XXXs")
- Laisse l'IA jouer, montre qu'elle repond vite
- **Le timer est dans le panneau droite, affiche apres chaque coup de l'IA**

**Ce que tu dis :**
"Le timer mesure le temps reel de calcul de l'IA. Il s'affiche dans le panneau.
En moyenne c'est bien en dessous de 0.5 seconde grace a l'iterative deepening
et l'elagage alpha-beta."

**Pour le mode PvP :**
- Appuie sur M pour passer en mode Human vs Human
- Montre que les 2 joueurs jouent au clic

---

## ETAPE 5 : Algorithme Minimax (Score 0-5)

**CRUCIAL : Si tu ne peux pas expliquer en detail, c'est 0.**

Le correcteur veut entendre :

### Script de ce que tu dis :

"Mon IA utilise Minimax avec elagage Alpha-Beta.

**Minimax** : c'est un algorithme de recherche dans un arbre de jeu.
A chaque noeud, on alterne entre MAX (mon tour, je choisis le meilleur score)
et MIN (tour adverse, il choisit le pire score pour moi).
Aux feuilles de l'arbre, on evalue la position avec l'heuristique.

**Alpha-Beta** : c'est une optimisation. On garde deux bornes, alpha et beta.
Alpha = le meilleur score que MAX peut garantir.
Beta = le meilleur score que MIN peut garantir.
Si beta <= alpha, on arrete de chercher dans cette branche car elle ne peut pas
donner un meilleur resultat. Ca coupe enormement de noeuds.

En pratique, avec un bon tri des coups, alpha-beta reduit le nombre de noeuds
de b^d a environ b^(d/2), ou b est le nombre de coups possibles et d la profondeur.

Le code est dans `src/ai.cpp`, fonction `minimax`.
La coupure c'est le `if (beta <= alpha) break;` a la fin de la boucle."

**Si on te demande de montrer le code :**
Ouvre `src/ai.cpp` et montre la fonction `minimax` (vers la ligne 65-110).
Pointe :
- Les parametres `alpha`, `beta`, `maximizing`
- Le `break` quand `beta <= alpha`
- L'appel recursif avec `!maximizing`

Pour le score : Alpha-Beta pruning = **5/5**

---

## ETAPE 6 : Profondeur de recherche (Score 0-5)

**Ce que tu dis :**
"J'utilise l'iterative deepening. L'IA commence a profondeur 2,
puis 4, 6, 8, 10, 12... et s'arrete quand le temps approche 0.45 secondes.
En pratique, elle atteint regulierement 10 ou plus.

La profondeur effective est encore plus grande grace a la table de transposition
qui cache les resultats des iterations precedentes."

**Si le correcteur veut une preuve :**
Tu peux ajouter un `std::cerr << "depth=" << depth << std::endl;`
dans la boucle d'iterative deepening et montrer la sortie.

Pour le score : 10+ niveaux = **5/5**

---

## ETAPE 7 : Espace de recherche (Score 0-5)

**Ce que tu dis :**
"Je ne cherche pas sur tout le plateau. Je genere uniquement les coups
dans un rayon de 2 cases autour de chaque pierre existante.
C'est-a-dire que pour chaque pierre sur le plateau, toutes les cases vides
dans un carre 5x5 autour sont des candidats.

Ca cree plusieurs fenetres qui englobent les pierres en minimisant l'espace gaspille.
En milieu de partie ca donne 20 a 60 candidats au lieu de 300+.

Le code est dans `src/ai.cpp`, fonction `generate_candidates`.
Le parametre `CANDIDATE_DISTANCE = 2` est dans `include/constants.hpp`."

Pour le score : Multiple windows minimizing waste = **5/5**

---

## ETAPE 8 : Heuristique (section Yes/No)

**CRUCIAL : Si tu ne peux pas expliquer en detail, c'est 0.**

### 8.1 Alignements (Yes)
"L'heuristique detecte tous les alignements de 2, 3, 4, 5+ pierres
dans les 4 directions. Chaque type a un score different.
Code : `src/eval.cpp`, fonction `evaluate` qui appelle `scan_line`."

### 8.2 Potentiel de victoire par alignement (Yes)
"On regarde si l'alignement a assez d'espace pour devenir un 5.
C'est le concept d'open/half-open : on verifie si les bouts sont libres.
Un alignement avec les 2 bouts bloques vaut 0 (il ne deviendra jamais un 5)."

### 8.3 Liberte / Freedom (Yes)
"Chaque pattern est classe selon sa liberte :
- OPEN (2 bouts libres) = le plus dangereux
- HALF_OPEN (1 bout libre) = moyennement dangereux
- Flanked/bloque (0 bout libre) = score 0

Par exemple OPEN_THREE vaut 50 000 mais HALF_OPEN_THREE vaut seulement 5 000.
Les scores sont dans `include/constants.hpp`."

### 8.4 Captures potentielles (Yes)
"La fonction `count_capturable_pairs` dans eval.cpp scanne le plateau pour trouver
les paires qui peuvent etre capturees en un seul coup. Quand un joueur a 4 paires,
chaque paire capturable donne un bonus de 1 000 000 points."

### 8.5 Captures actuelles (Yes)
"Les captures deja realisees sont dans `captures[0]` et `captures[1]`.
L'heuristique leur donne un score escaladant :
0 -> 5K -> 15K -> 50K -> 200K -> victoire.
Plus tu approches de 5 paires, plus le score augmente vite."

### 8.6 Figures / combinaisons (Yes)
"Le move ordering detecte les combinaisons avantageuses :
coups qui gagnent, coups qui capturent, coups qui creent des menaces doubles.
L'evaluation combine les scores des 4 directions pour detecter les double-menaces."

### 8.7 Les deux joueurs (Yes)
"L'evaluation est symetrique. On evalue les patterns des DEUX joueurs.
Les patterns de l'IA sont positifs, ceux de l'adversaire sont negatifs.
C'est dans `src/eval.cpp` : `if (color == ai_color) score += ps; else score -= ps;`"

### 8.8 Partie dynamique (Yes)
"L'heuristique prend en compte l'historique via :
- Les captures passees (stockees dans GameState)
- La table de transposition qui conserve les meilleurs coups des recherches precedentes
- Le `last_move` pour contextualiser
- Les multiplicateurs de tour (un three sur ton tour vaut plus car tu agis dessus)"

---

## ETAPE 9 : Bonuses (Score 0-5)

On a 5 bonuses, 1 point chacun :

### Bonus 1 : Undo/Redo
- Appuie sur Z pour annuler, Y pour refaire
- En mode PvAI, ca annule 2 coups (le tien + celui de l'IA)
- **Dire :** "On sauvegarde un snapshot complet de l'etat avant chaque coup.
  Undo pop le dernier snapshot, Redo le restaure."

### Bonus 2 : Hint (indice)
- Appuie sur H : un cercle vert apparait sur le coup suggere par l'IA
- **Dire :** "On lance une recherche IA rapide pour suggerer le meilleur coup."

### Bonus 3 : Toggle PvP/PvAI
- Appuie sur M pour changer de mode
- **Dire :** "Un simple toggle entre les deux modes. Reset la partie."

### Bonus 4 : Theme
- Appuie sur T : le plateau passe en mode sombre/clair
- **Dire :** "Deux themes : clair (bois) et sombre. Les couleurs sont dans une struct Theme."

### Bonus 5 : Heatmap de debug
- Appuie sur D : des carres colores apparaissent sur les cases candidates
- Rouge a vert selon le score. Le nombre affiche = score / 1000
- **Dire :** "Ca affiche le score heuristique de chaque case candidate.
  Utile pour comprendre le raisonnement de l'IA."

Pour le score : 5 bonuses = **5/5**

---

## Resume des scores attendus

| Section | Score attendu | Justification |
|---------|---------------|---------------|
| Regles | Yes | Toutes implementees |
| UI + AI perf | 4-5 | IA joue bien, timer affiche |
| Algorithme | 5/5 | Alpha-Beta pruning |
| Profondeur | 5/5 | 10+ niveaux (iterative deepening) |
| Espace recherche | 5/5 | Fenetres multiples (distance 2) |
| Heuristique | Tous Yes | 8/8 criteres |
| Bonuses | 5/5 | 5 bonuses |

**Total possible : 125%**
