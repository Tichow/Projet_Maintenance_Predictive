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

## Auteur

Matteo Quintaneiro | Mines Saint-Étienne, 2026
