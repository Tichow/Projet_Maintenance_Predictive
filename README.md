# Maintenance Prédictive par IA Embarquée sur STM32L4R9

## Table des matières

1. [Contexte et objectif](#1-contexte-et-objectif)
2. [Structure du dépôt](#2-structure-du-dépôt)
3. [Installation et utilisation](#3-installation-et-utilisation)

---

## 1. Contexte et objectif

En industrie, on a généralement trois façons de gérer la maintenance d'une machine : attendre qu'elle casse (maintenance corrective, coûteux et potentiellement dangereux), la réviser à intervalles fixes (maintenance préventive, on remplace des pièces qui auraient pu durer encore longtemps), ou analyser les données de ses capteurs pour détecter les signes avant-coureurs d'une panne et intervenir au bon moment (maintenance prédictive). C'est cette troisième approche qu'on met en oeuvre ici.

L'idée du projet, c'est de prendre un dataset de capteurs industriels (températures, couple, vitesse de rotation, usure d'outil), d'entraîner un réseau de neurones à classifier l'état d'une machine parmi 5 catégories (fonctionnelle + 4 types de pannes), puis de déployer ce modèle sur un microcontrôleur STM32L4R9. 

Ce déploiement sert à montrer que l'inférence tourne en embarqué, avec 23 Ko de Flash et 2.8 Ko de RAM, via l'outil X-CUBE-AI de STMicroelectronics.

Le schéma ci-dessous résume le pipeline complet du projet, du dataset brut jusqu'au test d'inférence sur cible :

![Pipeline du projet](images/pipeline_projet.png)

---

## 2. Structure du dépôt

```
Projet_Maintenance_Predictive/
│
├── notebook/
│   └── TP_IA_EMBARQUEE.ipynb      # Notebook principal : analyse, entraînement, export
│
├── data/
│   └── ai4i2020.csv               # Dataset AI4I 2020 (10 000 échantillons)
│
├── modele/
│   ├── modele_maintenance.tflite   # Modèle entraîné, prêt à déployer (15.6 KB)
│   ├── x_test.npy                  # Données de test (1996 × 8 features normalisées)
│   └── y_test.npy                  # Labels one-hot (1996 × 5 classes)
│
├── scripts/
│   └── communication.py            # Script UART pour tester l'inférence sur cible
│
├── stm32/
│   ├── AI4I2020/                   # Projet STM32CubeIDE complet
│   │   ├── Core/Src/main.c         # Initialisation carte (UART, horloges, etc.)
│   │   ├── X-CUBE-AI/App/
│   │   │   └── app_x-cube-ai.c    # Code utilisateur : réception UART + inférence + envoi
│   │   └── AI4I2020.ioc            # Configuration CubeMX (périphériques, X-CUBE-AI)
│   ├── rapport-analyse.txt         # Rapport d'analyse mémoire du modèle
│   └── rapport-validation-desktop.txt  # Rapport de validation desktop (cross-accuracy)
│
├── images/                         # Graphiques et captures d'écran pour ce README
├── requirements.txt                # Dépendances Python
└── README.md                       # Ce fichier
```

**Fichiers clés à regarder en priorité :**
- Le notebook `notebook/TP_IA_EMBARQUEE.ipynb` contient toute la logique d'entraînement, de l'analyse des données à l'export du modèle.
- Le fichier `stm32/AI4I2020/X-CUBE-AI/App/app_x-cube-ai.c` contient le code que j'ai écrit pour faire communiquer la carte avec le PC et lancer l'inférence.
- Le script `scripts/communication.py` est ce qu'on exécute côté PC pour envoyer les données de test et récupérer les prédictions.

---

## 3. Installation et utilisation

### Prérequis

- Python 3.10+
- STM32CubeIDE (pour compiler et flasher le firmware)
- X-CUBE-AI v10.x (installé via le gestionnaire de packs dans CubeMX)
- Une carte STM32L4R9I-Discovery branchée en USB via le ST-Link

### Installation des dépendances Python

```bash
pip install -r requirements.txt
```

### Reproduire l'entraînement

Ouvrir `notebook/TP_IA_EMBARQUEE.ipynb` et exécuter toutes les cellules. Le notebook va :
1. Charger et analyser le dataset `data/ai4i2020.csv`
2. Entraîner deux modèles (avec et sans SMOTE)
3. Exporter le modèle final en `.tflite` dans `modele/`
4. Sauvegarder les données de test (`x_test.npy`, `y_test.npy`)

### Compiler et flasher le firmware

1. Ouvrir le projet `stm32/AI4I2020/` dans STM32CubeIDE
2. Build (Ctrl+B)
3. Flash via le ST-Link (Run → Debug ou Run)

### Tester l'inférence sur cible

Une fois le firmware flashé et la carte branchée :

```bash
python scripts/communication.py
```

Le script va se synchroniser avec la carte, envoyer les 1996 échantillons de test un par un, et afficher l'accuracy finale. Il faut vérifier que le port COM est le bon (par défaut `COM3`, visible dans le Gestionnaire de périphériques Windows sous "Ports (COM et LPT)").

---

## 4. Analyse du dataset AI4I 2020

Le dataset utilisé est le [AI4I 2020 Predictive Maintenance Dataset](https://archive.ics.uci.edu/ml/datasets/AI4I+2020+Predictive+Maintenance+Dataset). Il contient 10 000 échantillons de données de capteurs industriels simulés. Chaque ligne représente l'état d'une machine à un instant donné, avec des mesures physiques (températures, vitesse de rotation, couple, usure de l'outil) et des étiquettes indiquant si une panne est survenue, et de quel type.

### Déséquilibre des classes

Le premier truc qu'on remarque quand on regarde les données, c'est que la très grande majorité des machines fonctionnent normalement : environ 96.6% sont étiquetées "No Failure", et seulement 3.4% sont en panne.

![Distribution des pannes](images/distribution_pannes.png)

Ce déséquilibre est tout à fait réaliste (heureusement qu'il y a plus de machines qui marchent que de machines cassées), mais ça pose un vrai problème pour l'entraînement. Un modèle peut très bien atteindre 96.6% d'accuracy en répondant systématiquement "pas de panne" à tout, sans avoir rien compris aux données. C'est exactement ce qui va se passer avec le premier modèle, on le verra plus bas.

### Types de pannes

Parmi les machines en panne, on distingue 5 types de défaillance :

| Sigle | Type de panne | Description |
|-------|--------------|-------------|
| TWF | Tool Wear Failure | Usure de l'outil |
| HDF | Heat Dissipation Failure | Dissipation thermique insuffisante |
| PWF | Power Failure | Panne liée à la puissance |
| OSF | Overstrain Failure | Surcharge mécanique |
| RNF | Random Failure | Panne aléatoire |

![Distribution des types de pannes](images/distribution_types_pannes.png)

![Distribution filtrée sur les machines en panne](images/distribution_types_pannes_filtered.png)

HDF et OSF sont les plus fréquents. TWF est déjà assez rare. Et RNF ne compte que 19 occurrences sur 10 000 lignes. Avec si peu d'exemples, aucun modèle ne pourra apprendre quelque chose de fiable pour cette classe. J'ai donc fait le choix de retirer RNF des sorties du modèle : on ne cherche pas à la prédire.

### Choix des entrées et sorties

J'ai pas mal réfléchi à ce qu'il fallait donner en entrée au modèle. Les 5 mesures capteurs brutes (Air temperature, Process temperature, Rotational speed, Torque, Tool wear) sont un bon point de départ, mais en regardant les corrélations dans les données, j'ai ajouté deux features calculées :

**8 entrées au total :**
- Les 5 mesures capteurs directes
- Le type de machine (encodé numériquement : L=0, M=1, H=2)
- `Power = Torque × Rotational speed` — corrélé aux pannes OSF et PWF, ce qui est logique physiquement (une puissance anormale signale une surcharge ou un problème électrique)
- `delta_T = Process temperature - Air temperature` — un bon indicateur pour HDF puisque c'est directement lié à la dissipation thermique

**5 sorties (softmax) :** Functional, TWF, HDF, PWF, OSF.

Le choix du **softmax** plutôt que des sigmoïdes indépendantes (approche multi-label) vient d'une observation dans les données : les pannes simultanées sont très rares, moins de 5% des cas de panne. Avec un softmax, le modèle est forcé de choisir une seule classe, ce qui simplifie l'apprentissage et donne des probabilités plus tranchées en sortie. En contrepartie, on perd la capacité de détecter les cas rares où deux pannes surviennent en même temps, mais c'est un compromis qui m'a semblé raisonnable vu les données.

---

## 5. Premier modèle : sans rééquilibrage

### Architecture

Pour le premier essai, j'ai utilisé une architecture assez classique de MLP (Multi-Layer Perceptron) :

| Couche | Neurones | Activation | Dropout |
|--------|----------|------------|---------|
| Dense 1 | 64 | ReLU | 0.15 |
| Dense 2 | 32 | ReLU | 0.15 |
| Dense 3 | 16 | ReLU | - |
| Sortie | 5 | Softmax | - |

L'architecture est compacte, et c'est voulu. D'une part, le modèle doit tourner sur un microcontrôleur avec des ressources limitées. D'autre part, on ne classifie que 8 features, il n'y a pas besoin d'un réseau à 100 000 paramètres pour ça. J'ai quand même ajouté 3 couches cachées au lieu de 2 (certains projets se contentent de Input → 64 → Sortie) parce que ça permettait de mieux séparer HDF de PWF, qui ont des signatures assez proches dans les données.

Le dropout à 0.15 est volontairement léger. Avec un réseau aussi petit, un dropout trop agressif (genre 0.4 ou 0.5) empêchait le modèle de converger correctement. C'est quelque chose que j'ai constaté en testant : à 0.3, l'accuracy de validation oscillait pas mal et les courbes de loss étaient instables. À 0.15, le modèle apprenait de façon plus régulière.

### Résultats

![Courbes d'entraînement sans rééquilibrage](images/training_curves_no_balance.png)

![Matrice de confusion sans rééquilibrage](images/confusion_matrix_no_balance.png)

L'accuracy atteint 99%. Sur le papier c'est excellent. En réalité c'est trompeur. Le modèle classe correctement presque toutes les machines fonctionnelles, et il arrive à détecter une partie des pannes HDF (recall 0.81), PWF (0.75) et OSF (0.95). Mais pour TWF, c'est le néant : recall de 0.00. Les 9 instances de TWF dans le test set sont toutes classées comme fonctionnelles.

Le problème, c'est que TWF ne représente qu'une trentaine d'exemples dans le training set. Le modèle n'a littéralement pas assez de matière pour apprendre à quoi ressemble cette panne, et comme il minimise la loss globale, il "préfère" classer ces cas ambigus en Functional (la classe dominante) plutôt que de risquer des erreurs.

En contexte industriel, un modèle qui rate 100% d'un type de panne est inutilisable. L'accuracy globale ne veut rien dire si on ne regarde pas le recall par classe.

---

## 6. Deuxième modèle : avec SMOTE

### Pourquoi SMOTE et pas autre chose

Pour corriger le problème de déséquilibre, j'ai utilisé SMOTE (Synthetic Minority Over-sampling Technique). L'idée de SMOTE, c'est de générer des points synthétiques pour les classes minoritaires en interpolant entre des exemples existants et leurs plus proches voisins. C'est mieux que de simplement dupliquer les exemples (oversampling naïf), parce que ça crée de la variété dans les données synthétiques au lieu de juste répéter les mêmes points.

J'aurais pu utiliser des class weights à la place (donner plus de poids aux erreurs sur les classes rares pendant l'entraînement), mais en pratique SMOTE donnait de meilleurs résultats sur ce dataset, surtout pour TWF.

### Choix importants dans l'implémentation

Quelques détails qui ont leur importance et qui m'ont demandé pas mal de tâtonnements :

- **Split avant SMOTE, pas l'inverse.** C'est un piège classique : si on applique SMOTE sur tout le dataset puis qu'on split, les points synthétiques du test set sont des interpolations de points du training set. On a du data leakage et les métriques de test sont faussées. En faisant le split d'abord, le test set garde la distribution réelle (~96.6% Functional), ce qui donne des métriques honnêtes.

- **k_neighbors=3 au lieu de 5 (défaut).** TWF ne compte qu'une trentaine d'exemples d'entraînement. Avec k=5, SMOTE va chercher des voisins trop éloignés dans l'espace des features, et les points synthétiques générés sont de mauvaise qualité (ils tombent dans des zones qui ne correspondent pas vraiment à une panne TWF). Avec k=3, les voisins sont plus proches et les interpolations ont plus de sens.

- **Filtrage des cas multi-label.** 23 lignes du dataset avaient plusieurs types de panne en même temps (par exemple TWF et OSF simultanément). Je les ai retirées avant d'appliquer SMOTE, parce que l'interpolation entre un voisin TWF pur et un voisin TWF+OSF produirait des points incohérents. Ce n'est que 23 lignes sur 8000, donc la perte est négligeable.

- **StandardScaler avant SMOTE.** SMOTE calcule les distances euclidiennes entre points pour trouver les voisins. Sans normalisation, la vitesse de rotation (en milliers de rpm) dominerait complètement le calcul de distance par rapport au couple (en Nm) ou à l'usure (en minutes). Le StandardScaler met toutes les features sur la même échelle.

### Résultats

![Courbes d'entraînement avec SMOTE](images/training_curves_balanced.png)

![Matrice de confusion avec SMOTE](images/confusion_matrix_balanced.png)

Le rééquilibrage a corrigé le défaut principal. Les recalls par classe :

| Classe | Recall sans SMOTE | Recall avec SMOTE |
|--------|-------------------|-------------------|
| Functional | ~1.00 | 0.97 |
| TWF | 0.00 | 0.22 |
| HDF | 0.81 | 0.86 |
| PWF | 0.75 | 0.88 |
| OSF | 0.95 | 0.94 |

TWF passe de 0.00 à 0.22, ce qui reste faible mais au moins le modèle a commencé à apprendre une signature pour cette panne. HDF et PWF s'améliorent nettement. OSF reste stable.

Le compromis, c'est que 66 machines fonctionnelles sont maintenant classées à tort comme défaillantes (contre 8 avant), ce qui fait baisser l'accuracy globale à environ 96%. En contexte industriel, c'est un compromis acceptable : une inspection inutile coûte beaucoup moins cher qu'une panne non détectée. C'est mieux de prédire une fausse panne que de rater une vraie.

---

## 7. Export et conversion du modèle

Le modèle est exporté au format TFLite (`.tflite`) plutôt qu'en `.h5`. Ce n'était pas mon premier choix : initialement j'exportais en .h5, qui est le format natif de Keras. Mais j'ai découvert que Keras 3 ajoute un attribut `quantization_config` dans chaque couche Dense lors de la sauvegarde en .h5, et X-CUBE-AI (v10) ne sait pas le lire. L'import dans CubeMX plante avec une erreur de désérialisation pas très explicite. J'ai perdu un bon moment là-dessus avant de comprendre que le problème venait du format et pas du modèle lui-même (j'en reparle dans la section [Problèmes rencontrés](#10-problèmes-rencontrés-et-bugs)).

Le format TFLite contourne ce problème car il a son propre format de sérialisation, indépendant de Keras.

Fichiers exportés :
- `modele/modele_maintenance.tflite` : le modèle converti (15.6 KB)
- `modele/x_test.npy` : données de test (1996 échantillons, 8 features normalisées)
- `modele/y_test.npy` : labels one-hot (1996 échantillons, 5 classes)

---

## 8. Déploiement sur STM32L4R9

### 8.1 Configuration X-CUBE-AI

Le modèle TFLite a été importé dans STM32CubeMX via le middleware X-CUBE-AI. La configuration :

- **Nom du réseau :** `ai4i` (ce nom est utilisé partout dans le code C généré : `ai_ai4i_create_and_init`, `ai_ai4i_run`, etc.)
- **Compression :** none — le modèle fait 12 Ko de poids, il n'y a aucun intérêt à compresser
- **Options :** `allocate-inputs` et `allocate-outputs` activées — les buffers d'entrée/sortie sont alloués dans le buffer d'activations au lieu d'être déclarés séparément, ce qui économise un peu de RAM

![Configuration X-CUBE-AI dans CubeMX](images/config-cubeai.png)

### 8.2 Analyse du modèle

Avant de générer le code, X-CUBE-AI analyse le modèle pour vérifier qu'il tiendra dans les contraintes mémoire de la cible. Résultats complets dans `stm32/rapport-analyse.txt`.

| Métrique | Valeur |
|----------|--------|
| Paramètres | 3 269 (12.77 KiB) |
| MACC (opérations multiply-accumulate) | 3 456 |
| Poids (FLASH, lecture seule) | 13 076 B (12.77 KiB) |
| Activations (RAM, lecture-écriture) | 384 B |
| Entrée | float32(1×8), 32 B |
| Sortie | float32(1×5), 20 B |
| **FLASH totale (modèle + runtime)** | **23 700 B (23.14 KiB)** |
| **RAM totale** | **2 868 B (2.80 KiB)** |

#### Graphe du modèle

![Graphe du modèle dans X-CUBE-AI](images/graph-ai4i-model.png)

Ce graphe est généré par X-CUBE-AI et représente le pipeline d'inférence tel qu'il sera exécuté sur la STM32. L'entrée (`serving_default_keras_tensor_1470_output_array`) est un vecteur de 8 float32 — nos 8 features normalisées.

Ce qu'on voit, c'est que chaque couche Dense qu'on a définie dans Keras est décomposée en deux opérations distinctes par le runtime : un bloc **Dense** (la multiplication matricielle + biais) suivi d'un bloc **Non Linearity** (la fonction d'activation). Dans Keras on écrit `Dense(64, activation='relu')` et ça a l'air d'être une seule chose, mais côté exécution C c'est bien deux étapes séparées. Les trois premières non-linéarités sont des ReLU, la dernière (`nl_4`) est le softmax qui produit les probabilités de sortie.

Pour chaque couche Dense, le graphe affiche les dimensions des poids : la matrice (sortie × entrée) et le vecteur de biais. La première couche a une matrice 64×8 = 512 poids + 64 biais. La deuxième (32×64 = 2 048 poids + 32 biais) est de loin la plus lourde du réseau. Les couches suivantes sont de plus en plus petites, ce qui reflète la stratégie de compression progressive vers les 5 classes de sortie : 8 → 64 → 32 → 16 → 5. On élargit d'abord la représentation pour capturer des combinaisons de features, puis on la réduit progressivement jusqu'à l'espace de classification.

La sortie finale (`nl_4_output_array`) est un vecteur de 5 valeurs : les probabilités pour Functional, TWF, HDF, PWF, OSF.

#### Memory layout

![Layout mémoire du modèle](images/memory-layout-ai4i-model.png)

Ce diagramme est probablement le plus intéressant des deux, mais il est aussi moins intuitif à lire. Il montre comment la RAM est utilisée à chaque étape de l'inférence.

L'axe horizontal représente les opérations exécutées séquentiellement, de gauche à droite : `gemm_0`, `nl_0_nl`, `gemm_1`, `nl_1_nl`, `gemm_2`, `nl_2_nl`, `gemm_3`, `nl_4`. Les `gemm` correspondent aux couches Dense (multiplication matricielle) et les `nl` aux fonctions d'activation. L'axe vertical représente les adresses mémoire en octets, relatives au début du pool d'activations.

Les zones cyan représentent les **activations**, c'est-à-dire les buffers temporaires qui contiennent les tenseurs intermédiaires (les vecteurs de sortie de chaque couche). On voit que les premières étapes (`gemm_0`, `gemm_1`) ont les colonnes les plus hautes : c'est logique, on y manipule les vecteurs les plus larges (64 float32 = 256 octets). À partir de `gemm_2` la hauteur diminue parce que les vecteurs passent à 32 puis 16 valeurs. À `nl_4` (la dernière activation, le softmax), l'empreinte est minimale puisqu'on ne travaille plus que sur 5 valeurs.

La bande `weights_array` en haut du diagramme représente les poids du réseau. En réalité ils sont stockés en Flash (lecture seule, 13 Ko) et pas en RAM, mais le diagramme montre leur emplacement logique dans l'espace d'adressage du runtime. On voit qu'ils sont accédés par chaque opération `gemm` mais pas par les `nl` — ce qui est normal puisque les fonctions d'activation n'ont pas de paramètres appris, elles appliquent juste une fonction mathématique sur les valeurs en place.

Le point le plus important de ce diagramme, c'est le mécanisme d'**overlay mémoire** (visible via le `heap_overlay_pool`). X-CUBE-AI ne va pas allouer un buffer séparé pour chaque tenseur intermédiaire — ça demanderait 64+32+16+5 = 117 floats = 468 octets rien que pour les activations. À la place, il réutilise les mêmes zones de RAM pour des tenseurs qui n'ont pas besoin de coexister en même temps. Quand `gemm_1` s'exécute, le buffer de sortie de `gemm_0` a déjà été consommé par `nl_0_nl` et peut être écrasé. C'est grâce à ce mécanisme que le budget total d'activations n'est que de **384 octets** alors que la somme brute de tous les tenseurs intermédiaires serait bien supérieure. Concrètement, c'est une allocation/désallocation dynamique au sein d'un buffer statique de taille fixe, ce qui est typique de l'embarqué où on veut éviter le `malloc` à tout prix.

#### Observations générales

La STM32L4R9 dispose de 2 Mo de Flash et 640 Ko de RAM. Mon modèle occupe 23 Ko en Flash et 2.8 Ko en RAM, soit environ 1% des ressources disponibles. C'est rassurant, mais ça montre aussi qu'on aurait largement pu se permettre un réseau plus gros si les performances l'avaient justifié.

Le nombre de MACC (3 456) est très faible. Pour donner un ordre de grandeur, un petit réseau de classification d'images c'est plusieurs millions de MACC. Ici l'inférence prend environ 0.002 ms sur le host en validation desktop, et c'est quasi-instantané sur la carte. Normal : on ne fait que 4 multiplications matricielles sur 8 features.

En regardant la répartition par couche (confirmée visuellement par le memory layout), la couche Dense 64→32 concentre à elle seule 61.1% des opérations et 63.6% de la mémoire poids. C'est attendu : c'est la couche avec le plus de connexions (64 × 32 = 2 048 poids). Si on devait optimiser la taille ou la vitesse du modèle, c'est cette couche qu'il faudrait cibler en premier.

J'ai laissé la compression à "none" car le modèle fait déjà 12 Ko de poids. La quantification int8 diviserait la taille par ~4, mais vu la marge mémoire, ça n'apporterait rien de concret et pourrait dégrader la précision des sorties softmax.

### 8.3 Validation sur desktop

Avant de flasher le modèle sur la carte, X-CUBE-AI permet de le valider "on desktop" : le code C généré est compilé et exécuté directement sur le PC avec les données de test. Ça permet de vérifier que la conversion TFLite → C n'a pas cassé quelque chose. Résultats dans `stm32/rapport-validation-desktop.txt`.

| Modèle | Accuracy | RMSE |
|--------|----------|------|
| Modèle TFLite original | 96.04% | 0.1138 |
| Modèle C généré (HOST) | 96.04% | 0.1138 |
| **Cross-accuracy (ref vs C)** | **100.00%** | 0.000000043 |

La cross-accuracy de 100% est le point important ici. Ça veut dire que le modèle C produit exactement les mêmes sorties que le modèle TFLite, à la précision flottante près (RMSE de 4.3×10⁻⁸, c'est du bruit numérique). Ce résultat est utile parce que si le modèle se comporte bizarrement une fois sur la carte, on sait que le problème ne vient pas de la conversion mais forcément de la communication UART ou du preprocessing des données.

La matrice de confusion de la validation desktop confirme la même répartition que Python :
- C0 (Functional) : 1868 corrects, 66 faux positifs de panne répartis sur les autres classes
- C1 (TWF) : seulement 2 détectés sur 9 — c'est la faiblesse connue du modèle
- C2 (HDF), C3 (PWF), C4 (OSF) : bien détectés globalement

### 8.4 Code embarqué : ce que j'ai écrit et pourquoi

X-CUBE-AI génère un squelette dans `app_x-cube-ai.c` avec des fonctions vides à remplir. Toute la logique que j'ai ajoutée se trouve dans les zones `USER CODE`. La carte n'a pas de capteurs industriels branchés, donc on utilise l'UART (port série via le câble ST-Link) pour envoyer les données depuis le PC et récupérer les prédictions.

Le protocole d'échange entre le PC et la carte est illustré ci-dessous :

![Protocole de communication UART](images/protocole_uart.png)

En résumé : le PC envoie `0xAB`, la carte répond `0xCD` (synchronisation), puis on boucle — le PC envoie 32 octets (8 float32), la carte fait l'inférence et renvoie 5 octets (les probabilités en uint8). Le code ci-dessous détaille chaque étape.

#### Constantes et configuration

```c
extern UART_HandleTypeDef huart2;
#define INPUT_SIZE 8       // 8 float32 inputs
#define INPUT_BYTES (INPUT_SIZE * 4)  // 8 × 4 = 32 octets
#define OUTPUT_SIZE 5      // 5 classes en sortie
#define UART_TIMEOUT 5000
#define SYNC_BYTE 0xAB
#define ACK_BYTE 0xCD
```

`huart2` est déclaré en `extern` parce qu'il est défini dans `main.c` par le code généré par CubeMX. C'est l'UART2 qui correspond au Virtual COM Port du ST-Link — c'est par là que transitent toutes les données.

Le timeout de 5000 ms est volontairement large. Au début j'avais mis 1000 ms, mais il arrivait que le script Python soit un peu lent à envoyer les données (surtout au premier échantillon après la synchro), et la carte partait en timeout. 5 secondes c'est confortable sans être bloquant.

#### Synchronisation UART

```c
void uart_sync(void)
{
    uint8_t rx = 0;
    uint8_t ack = ACK_BYTE;
    while (rx != SYNC_BYTE)
    {
        HAL_UART_Receive(&huart2, &rx, 1, HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, &ack, 1, UART_TIMEOUT);
}
```

Sans cette étape, la carte commencerait à lire des octets dès le démarrage sans savoir si le script Python tourne, et tout serait désynchronisé. J'ai choisi `HAL_MAX_DELAY` (attente infinie) plutôt qu'un timeout parce que tant que le PC n'a pas lancé le script, la carte n'a rien d'autre à faire. C'est basique, mais ça marche.

#### Réception des données d'entrée

```c
int acquire_and_process_data(ai_i8* data[])
{
    HAL_StatusTypeDef status = HAL_UART_Receive(
        &huart2,
        (uint8_t *)data[0],
        INPUT_BYTES,
        UART_TIMEOUT
    );
    if (status != HAL_OK)
        return -1;
    return 0;
}
```

Un point important ici : on écrit directement dans `data[0]`, qui est le pointeur vers le buffer d'entrée du réseau de neurones. On n'utilise pas de buffer intermédiaire. C'est possible parce que les données arrivent déjà au bon format (8 float32 normalisés, envoyés en little-endian par Python, ce qui est aussi l'endianness de l'ARM Cortex-M4).

Ce `data[0]` pointe soit vers un buffer statique, soit vers une zone dans le buffer d'activations (si `allocate-inputs` est activé). Dans notre cas c'est le buffer d'activations, ce qui veut dire que ce pointeur n'est valide qu'après l'appel à `ai_boostrap()`. Si on essaie d'écrire dedans avant, on écrit à l'adresse NULL. C'est un piège que j'ai découvert de manière un peu douloureuse (le firmware crashait silencieusement au premier échantillon reçu).

#### Envoi des résultats

```c
int post_process(ai_i8* data[])
{
    ai_float *output = (ai_float *)data[0];
    uint8_t results[OUTPUT_SIZE];

    for (int i = 0; i < OUTPUT_SIZE; i++)
    {
        results[i] = (uint8_t)(output[i] * 255.0f);
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(
        &huart2,
        results,
        OUTPUT_SIZE,
        UART_TIMEOUT
    );
    if (status != HAL_OK)
        return -1;
    return 0;
}
```

Les sorties du réseau sont 5 float32 (les probabilités softmax). Je les convertis en uint8 en multipliant par 255 avant de les envoyer. On passe de 20 octets à 5 octets par inférence. Pour notre petit projet la différence n'est pas énorme, mais c'est un pattern qu'on retrouve souvent en embarqué quand on veut limiter le volume de données sur le bus série.

La résolution de la conversion est de 1/255 ≈ 0.4%. En pratique, ça veut dire que deux classes séparées par moins de 0.4% de probabilité pourraient être inversées par l'arrondi. Mais pour de la classification, on prend l'argmax (la classe avec la plus grande probabilité), et les probabilités du softmax sont généralement assez tranchées. Sur les 1996 tests, cette perte de précision n'a affecté aucune prédiction.

#### Boucle principale

```c
void MX_X_CUBE_AI_Process(void)
{
    int res = -1;
    uart_sync();

    if (ai4i) {
        do {
            res = acquire_and_process_data(data_ins);
            if (res == 0)
                res = ai_run();
            if (res == 0)
                res = post_process(data_outs);
        } while (res == 0);
    }
}
```

La boucle est volontairement linéaire : réception → inférence → envoi, en série. Pas de DMA, pas d'interruptions. C'est suffisant ici parce que le temps d'inférence est négligeable (quelques microsecondes) et que le bottleneck est de toute façon le débit UART à 115200 baud. Si une des étapes échoue (timeout UART, erreur réseau), `res` passe à -1 et la boucle s'arrête. Dans un vrai système industriel il faudrait gérer les erreurs de façon plus fine (retry, log, watchdog), mais pour du test c'est suffisant.

Le `if (ai4i)` vérifie que le réseau a été correctement initialisé par `ai_boostrap()`. Si `ai_ai4i_create_and_init` avait échoué (modèle corrompu, pas assez de RAM), `ai4i` serait `AI_HANDLE_NULL` et on n'entrerait pas dans la boucle.

### 8.5 Côté PC : le script de communication

Le script `scripts/communication.py` fait le miroir de ce qui se passe côté carte :

1. Il ouvre le port série (`COM3`, 115200 baud)
2. Il attend 2 secondes pour que la connexion s'établisse (sans ce délai, les premiers octets sont parfois perdus)
3. Il envoie `0xAB` et attend `0xCD`
4. Il boucle sur les 1996 échantillons de test : envoi de 8 float32, réception de 5 uint8, comparaison de l'argmax avec le label attendu

Le script affiche l'accuracy tous les 200 échantillons pour suivre la progression. À la fin, il donne le résultat global.

---

## Auteur

Matteo Quintaneiro | Mines Saint-Étienne, 2026
