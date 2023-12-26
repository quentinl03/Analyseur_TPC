# Quentin LABORDE & Nicolas SEBAN
# Rapport Projet Analyse Syntaxique L3 2023-2024 : Analyseur TPC

- [Quentin LABORDE \& Nicolas SEBAN](#quentin-laborde--nicolas-seban)
- [Rapport Projet Analyse Syntaxique L3 2023-2024 : Analyseur TPC](#rapport-projet-analyse-syntaxique-l3-2023-2024--analyseur-tpc)
  - [Manuel utilisateur](#manuel-utilisateur)
    - [Compilation](#compilation)
    - [Nettoyage](#nettoyage)
    - [Lancer le programme](#lancer-le-programme)
    - [Les paramètres](#les-paramètres)
    - [Tests](#tests)
      - [Lancement](#lancement)
  - [Avancement du projet](#avancement-du-projet)
    - [Langage basique / avancé](#langage-basique--avancé)
    - [Messages d'erreur](#messages-derreur)
    - [Arbres abstraits](#arbres-abstraits)
    - [Test (divergence : bash-\>unitest)](#test-divergence--bash-unitest)
    - [Difficultés rencontrées](#difficultés-rencontrées)
  - [Conclusion](#conclusion)
    - [Améliorations possibles](#améliorations-possibles)
    - [Points positifs](#points-positifs)
    - [Points négatifs](#points-négatifs)


## Manuel utilisateur

### Compilation

Pour lancer le projet vous devez décompresser l'archive puis vous mettre dans la racine du projet et enfin vous pouvez le compiler avec la commande `make`.

```bash
tar xvf ProjetASL3_LABORDE_SEBAN.tar.gz
cd ProjetASL3_LABORDE_SEBAN
make
```

### Nettoyage 

Pour nettoyer le projet une fois l'utilisation terminé vous pouvez utiliser les commandes :
-  `make distclean`: pour supprimmer les fichiers objets.
-  `make clean`: pour supprimmer les fichiers objets et l'exécutable.

### Lancer le programme

Pour lancer le programme une fois compilé vous devez utiliser la commande suivante suivante :

```bash
./bin/tpcas
```

### Les paramètres

Prototype des paramètres de la commande.
```
./bin/tpcas [-t] [-h] file
```


`file` (Obligataire) :
   - Chemin vers le fichier à charger avec l'analyseur syntaxique.

`-t / --tree` (Facultatif) :
   - Si le fichier est valide affiche l'arbre abstrait. 
   - /!\ Si le ficher n'est pas value l'arbre ne s'affiche pas! /!\

`-h / --help` (Facultatif) :
   - Affiche un bref menu d'aide.


### Tests

#### Lancement 
Vous pouvez lancer les tests de 2 manières différentes :
  - Via le terminal :
    - Via la commande suivante :
    - ```bash
      python3 test/test.py
      ```
  - Via VSCode :
    - Cliquez sur l'onglet `Test` sur la partie gauche de votre éditeur.
    - Puis lancer les tests avec le bouton play (sous forme de flèche).


## Avancement du projet

### Langage basique / avancé

Nous avons réalisé l'analyseur 'basique' la version amélioré avec la gestion des tableaux.

### Messages d'erreur

Les messages d'erreur affichent la ligne et la colonne de leurs localisation.

### Arbres abstraits

Les arbres arstraits ont été fait et peuvent être activé avec l'option [`-t/--tree`](#les-paramètres)

### Test (divergence : bash->unitest)

Puis lancer les tests pensez à voir ['Test'](#tests).

les différences...

### Difficultés rencontrées

- Création des arbres avec valeurs : Lorsque nous avons voulu changer les arbres à 'noeux' en arbres à 'valeurs'. Nous avons d'abord voulu faire une seule fonction pour ajouter les attributs. Cependant nous avons eu des problèmes pour le transformer en union d'atribut. Nous avons donc fait une fonction par champs dans l'union.

- Dépendance dans le Makefile :


## Conclusion 

### Améliorations possibles

- -tree_force


### Points positifs
- Projet innovant :
  - Nous n'avons pas eu de projet similaire au paravant. Il a été agréable de découvrir `lex` et `bison`.
- Arbres abstraits :
  - Création et oragnisation des noeux d'arbres (noeux listes, noeux pas toujours créé (EmptyInstr))
- Creation de tests unitaires : 
  - Création de test règles par règles.
  - Chercher à 'casser' le programme.  

### Points négatifs

Nous n'avons pas vraiment de points négatifs nous avons aprécié le projet dans son ensemble.

_____

Quentin Laborde & Nicolas Seban
