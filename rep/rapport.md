\pagebreak

# Manuel utilisateur

## Compilation

Pour lancer le projet, vous devez décompresser l'archive puis vous déplacer vers la racine du projet pour enfin le compiler avec la commande `make`.

```bash
tar xvf ProjetASL3_LABORDE_SEBAN.tar.gz
cd ProjetASL3_LABORDE_SEBAN
make
```

## Nettoyage

Pour nettoyer le projet une fois l'utilisation terminée vous pouvez utiliser les commandes :

-  `make distclean`: pour supprimmer les fichiers objets.

-  `make clean`: pour supprimmer les fichiers objets et l'exécutable.

## Lancer le programme

Pour lancer le programme compilé vous devez utiliser la commande suivante suivante :

```bash
./bin/tpcas
```

## Les paramètres

Prototype des paramètres de la commande.

```bash
./bin/tpcas [-t] [-h] file
```

- `file` (facultatif) : Chemin vers le fichier source à charger avec l'analyseur syntaxique, par défaut, lecture de l'entrée standard.

- `-t / --tree` (facultatif) :

   - Si le code source est valide, affiche l'arbre abstrait.

      - Attention : si le code possède une erreur syntaxique, l'arbre ne s'affiche pas !

- `-h / --help` (facultatif) : Affiche un bref menu d'aide.

\pagebreak

# Tests

## Lancement

Vous pouvez lancer les tests par deux manières différentes :

  - Via le terminal et la commande suivante :

    ```bash
    python3 test/test.py
    ```

    Le script peut également produire une sortie sans couleurs, idéal pour créer un récapitulatif sans les caractères d'échappement ANSI :

    ```bash
    python3 test/test.py --no-color
    ```

  - Via VSCode :

    - Depuis la palette de commandes (Ctrl+Maj+P), sélectionner *Python: configure tests*

    - Choisir ``unittest``, puis sélectionner le répertoire ``test``, et enfin le *pattern* ``test*.py``

    - Cliquez sur l'onglet `Test` sur le volet gauche de l'éditeur.

    - Lancez les tests avec le bouton *play* (sous forme de flèche).

# Avancement du projet

## Langage basique / Extensions

Nous avons réalisé l'analyseur "basique", mais également ajouté le support des tableaux.
Comme le sujet le requiert, ceux-cis peuvent être déclarés par une constante entière, les expressions constantes sont interdites.

## Messages d'erreur

Les messages d'erreur affichent le couple ligne et colonne de leurs localisation.

## Arbres abstraits

Les arbres abstraits ont été implémentés et peuvent être affichés avec l'option [`-t/--tree`](#les-paramètres)

## Divergence sur l'élaboration des tests unitaires

Pour lancer les tests pensez à regarder la section ['Test'](#tests).

Nous avons choisi, après en avoir demandé l'autorisation, d'utiliser un script Python, et non Bash, pour exécuter les tests unitaires.

En effet, l'usage de Python nous permet l'utilisation de la bibliothèque native ``unittest``.
Elle permet de simplifier la création de tests unitaires, en les séparant par type de tests, et permet de produire automatiquement un rapport des comportements inattendus.

De plus, le module ``unittest`` est intégré dans de nombreux environnements de programmation, en particulier VSCode qui possède un menu spécifique affichant le déroulement en détail des tests exécutés.

Il serait également enviseagable d'automatiser l'exécution du script, par un système d'intégration coninue pouvant être fourni par les ``workflow``s de Github et Gitlab.

\pagebreak

## Difficultés rencontrées

### Création des arbres avec valeurs

Lorsque nous avons voulu produire les "arbres à valeurs", nous avons, dans un premier temps, pensés à créer une unique fonction pour ajouter les attributs.
Cependant, nous avons rencontré des problèmes pour le transformer en union d'attributs. Nous avons donc créé une fonction par champ de l'union.

### Dépendances dans le Makefile :

La compilation d'un projet utilisant flex et bison est assez particulière, notamment car Bison crée deux fichiers en sortie : l'analyseur syntaxique, et un fichier en-tête contenant la définition des tokens et de l'union. Or ce dernier, est nécessaire à flex. Il faut donc pouvoir signifier à make, que la commande bison produit deux cibles. Cependant, nous n'avions pas encore rencontré un tel cas dans nos précédents projets de C.

Après quelques recherches, et examination de la documentation, il s'avère que Make permet la définition de "cibles groupées" (*grouped targets*) à l'aide du séparateur ``&:``, nous pouvons donc utiliser cette règle :

```make
obj/tpc.tab.h obj/tpc.tab.c &: src/tpc.y $(MODULES)
```

# Conclusion

## Améliorations possibles

- Nous pourrions envisager l'ajout d'un mode permettant de continuer la construction de l'arbre, malgré la détection d'une erreur syntaxique.

## Points positifs

- Projet innovant :
  - Nous n'avions pas eu de projets similaires auparavant. Il a été agréable de découvrir `flex` et `bison`.

- Arbres abstraits :

  - Création et organisation des nœuds de l'arbre (nœuds listes, nœuds particuliers (``EmptyInstr``))

- Creation des tests unitaires :

  - Création de tests règles par règles.

  - Chercher à "casser" le programme.

## Points négatifs

Nous n'avons pas vraiment de points négatifs, nous avons apprécié le projet dans son ensemble.

_____

Quentin Laborde & Nicolas Seban
