# Roulette Simulator 🎰

Un simulateur de roulette européenne développé en C++17, s'exécutant entièrement dans la console.

Ce programme **n'utilise pas d'argent réel**, **pas de système de mise** et **ne distribue aucune récompense**. Il s'agit d'un outil pédagogique permettant d'observer le comportement de tirages aléatoires et les statistiques qui en découlent (loi des grands nombres).

## ✨ Fonctionnalités

- **Tirages aléatoires :** Simulation d'une roulette européenne standard (37 cases, de 0 à 36).
- **Gestion des couleurs :** Détermination automatique de la couleur (Rouge / Noir / Vert).
- **Statistiques complètes :** Fréquences observées, moyennes et comparaisons avec les probabilités théoriques.
- **Simulations de masse :** Génération instantanée de jusqu'à 1 000 000 de tirages pour analyser les écarts statistiques.
- **Analyse de séries (Streaks) :** Suivi des plus longues suites consécutives de Rouge, de Noir ou sans Zéro.
- **Sauvegarde des données :** Exportation des résultats de la session dans un fichier texte (`roulette_statistics.txt`).
- **Robustesse :** Sécurisation complète des entrées clavier contre les saisies invalides.

## 🛠️ Compilation et Exécution

Le projet est entièrement contenu dans un seul fichier : `main.cpp`.

### Via Terminal (Linux / macOS / Windows avec MinGW)
```bash
g++ -std=c++17 -O2 -o roulette main.cpp
./roulette
```

### Via le Makefile fourni
```bash
make       # Compile le projet
make run   # Compile et lance le simulateur
make clean # Nettoie les fichiers générés
```

## 🧑‍💻 Contexte du projet
Ce projet est notre **premier programme C++ relativement complexe**. Il a été réalisé dans un but pédagogique pour apprendre à organiser du code de manière propre, structurée et lisible sans utiliser de variables globales.

**Développeurs :**
- Lucas
- Florian

