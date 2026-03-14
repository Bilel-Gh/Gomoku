Le jeu de base
Un plateau 19x19 (comme le Go). Deux joueurs posent des pierres à tour de rôle. Noir commence toujours. Le but : aligner 5 pierres (horizontal, vertical ou diagonal).

Les règles spéciales du sujet 42
1. La capture
Si tu encadres 2 pierres ennemies entre 2 des tiennes, elles sont retirées :


NOIR  BLANC  BLANC  [NOIR joue ici]
  X     O      O        X

→ les 2 blanches disparaissent
Ça marche dans les 8 directions. Si tu captures 5 paires (10 pierres), tu gagnes direct.

2. L'endgame capture (la règle piège)
Aligner 5 ne suffit pas toujours. L'adversaire a un tour pour essayer de casser ta ligne en capturant une paire qui fait partie de ton alignement. S'il peut, le jeu continue.

Aussi : si l'adversaire a déjà 4 paires et que ton coup lui permet d'en capturer une 5ème, c'est lui qui gagne.

→ C'est la règle la plus dure à comprendre. En soutenance, montre-la en mode PvP.

3. Double-trois interdit
Tu ne peux pas poser une pierre qui crée 2 free-threes en même temps.

Un free-three = 3 pierres alignées avec les 2 bouts ouverts :


. X X X .       ← free-three classique
. X X . X .     ← free-three avec trou
. X . X X .     ← free-three avec trou
Si ta pierre crée 2 de ces patterns sur 2 axes différents → coup illégal, le clic est ignoré.

Exception : si ce même coup capture une paire, c'est autorisé.

Ton IA
Algorithme : Minimax + Alpha-Beta

L'IA simule un arbre de coups : "si je joue là, il joue là, puis je joue là..."
À chaque niveau, elle alterne :
MAX (son tour) → elle choisit le meilleur score
MIN (ton tour) → elle suppose que tu choisis le pire pour elle
Alpha-Beta coupe les branches inutiles : si une branche est déjà pire que ce qu'on a, on l'ignore
Iterative Deepening : elle cherche d'abord à profondeur 2, puis 4, 6, 8, 10... et s'arrête quand le temps approche 0.45s. Ça garantit qu'elle a toujours une réponse même si le temps est court.

Table de transposition (Zobrist) : un cache de positions déjà évaluées. Si la même position revient (par un ordre de coups différent), elle réutilise le résultat au lieu de recalculer.

Génération des candidats : elle ne teste pas les 361 cases, seulement celles dans un rayon de 2 autour des pierres existantes (20-60 coups au lieu de 300+).

L'heuristique (comment l'IA évalue une position)
Elle scanne toutes les pierres dans les 4 directions et classe les patterns :

Pattern	Signification	Score
FIVE	5 en ligne = victoire	100M
OPEN_FOUR	4 avec 2 bouts ouverts = imbloquable	10M
HALF_OPEN_FOUR	4 avec 1 bout ouvert	500K
OPEN_THREE	3 avec 2 bouts ouverts	50K
HALF_OPEN_THREE	3 avec 1 bout ouvert	5K
OPEN_TWO	2 avec 2 bouts ouverts	3K
Open = 2 bouts libres (très dangereux, dur à bloquer)
Half-open = 1 bout bloqué (plus facile à gérer)
Fermé = 2 bouts bloqués = vaut 0, inoffensif

Les captures ont un score escaladant : plus t'approches de 5 paires, plus ça vaut cher.

Les bonus (features en plus)
Touche	Action
N	Nouvelle partie
M	Toggle PvAI / PvP
Z	Undo (annuler)
Y	Redo (refaire)
H	Hint (l'IA te suggère un coup)
T	Changer de thème (clair/sombre)
D	Heatmap debug (scores de l'IA sur le plateau)
En résumé pour la correction
Règles : capture, endgame capture, double-trois → tout dans board.cpp et rules.cpp
IA : Minimax + Alpha-Beta + Iterative Deepening → ai.cpp
Évaluation : patterns + captures + multiplicateurs de tour → eval.cpp
Poids : tout centralisé dans constants.hpp (le fichier à ouvrir en premier en soutenance)
Timer : l'IA répond en < 0.5s, affiché dans le panneau