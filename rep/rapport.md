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

Pour lancer le programme compilé vous devez utiliser la commande suivante :

```bash
// TODO
```

## Les paramètres

\pagebreak

## Tests (lancement)

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

## Messages d'erreur

## Élaboration des tests unitaires

Pour lancer les tests pensez à regarder la section ['Test'](#tests).

Dans la continuité du projet d'analyse syntaxique, nous avons implémenté notre série de test avec Python.

Nous avons conservé les tests de notre précédent projet désormais présents sous les répertoires ``syn-good`` et ``syn-err``. Nous les avons étendues avec des tests pour l'émission d'avertissement (``warn``), d'erreurs sémantiques (``sem-err``), et enfin pour avec des codes compilables avec une sortie prédictible (``good``).

### C'est quoi ces commentaires dans les codes ``sem-err/`` et ``warn/`` !?

Pendant le développement de la partie de vérification sémantique du projet, nous nous sommes demandé de comment s'assurer que notre compilateur émette toujours les mêmes erreurs et warnings.

La solution la plus évidente était d'afficher en sortie (``stdout``) un code d'erreur correspondant. Cependant nous avons écarté ce choix pour éviter de complexifier la structure du projet. Nous nous sommes donc satisfait de compter les occurrences d'erreurs et avertissements. Par conséquent, nous écrivons dans les codes de tests, le nombre d'erreur/avertissement attendu à l'aide commentaires comme ceci :

```c
// nb_errors=1
// nb_warnings=0
```

Ces derniers sont interprétés par le script Python, et comparés à la sortie du compilateur.

\pagebreak

### Valgrind mais pas pour les fuites

Malheuresement, le projet possède des fuites de mémoire qui n'ont pas su être corrigées.

Cependant nous l'avons tout de même implémenté parmi les tests, mais uniquement pour vérifier l'absence d'accès à des zones mémoires non-initialisées. Cela devrait limiter les problèmes "It's works on my machine.``...

### Emprunt d'un compilateur concurrent...

Développer un compilateur c'est bien, mais développer un compilateur qui génère du code *correct* c'est encore mieux.

Pour vérifier que nos programmes générés possèdent bien le comportement désiré, nous avons décidé d'implémenter dans nos tests une phase de compilation des codes ``good/`` avec TPCC et GCC.
Les binaires produits sont exécutés et leurs sortie standard/codes d'erreurs comparées.
Cela permet ainsi de nous assurer en partie de validité du code produit.

## Difficultés rencontrées

# Conclusion

## Améliorations possibles

## Points positifs

## Points négatifs

