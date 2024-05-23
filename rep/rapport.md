\pagebreak

# Manuel utilisateur

## Compilation

Pour lancer le projet, vous devez décompresser l'archive puis vous déplacer vers la racine du projet avant de le compiler avec la commande `make` :

```bash
tar xvf ProjetCompilationL3_LABORDE_SEBAN.tar.gz
cd ProjetCompilationL3_LABORDE_SEBAN
make
```

## Nettoyage

Pour nettoyer le projet une fois l'utilisation terminée vous pouvez utiliser les commandes :

-  `make distclean`: pour supprimer les fichiers objets.

-  `make clean`: pour supprimer les fichiers objets et l'exécutable.

## Utilisation

### Arguments

Notre compilateur peut accepter plusieurs paramètres en argument :

  - ``-t ``, ``--tree`` : Affiche l'arbre syntaxique généré (hérité du projet d'Analyse Syntaxique)

  - ``-a ``, ``--only-tree`` : Stoppe l'exécution après l'affichage de l'arbre syntaxique

  - ``-s ``, ``--symtabs`` : Affiche les tables des symboles de chacune des fonctions

  - ``-w ``, ``--only-semantic`` : Vérifie seulement la validité du code, et génère avertissements/erreurs le cas échéant.

  - ``-h ``, ``--help`` : Affiche un menu d'aide

Le fichier source devra être fourni soit par l'entrée standard, soit par argument positionnel.

\pagebreak

### Exemples d'utilisation

- Génération classique produisant le fichier ``monFichierSource.asm`` :

  ```shell
  ./bin/tpcc monFichierSource.tpc
  ```

- Génère le code sous ``_anonymous.asm`` et affiche au passage l'arbre puis la table des symboles :

  ```shell
  ./bin/tpcc -ts < monFichierSource.tpc
  ```

- Exécuter sans générer de code (*dry-run*) en affichant seulement avertissements et erreurs, en lisant le fichier depuis l'entrée standard :

  ```shell
  cat monFichierSource.tpc | ./bin/tpcc -w
  ```

**Remarque :** Nous ne chargeons aucun fichier à l'exécution, par conséquent, il n'est pas nécessaire de lancer le compilateur depuis la racine du projet.
En effet, lors de la compilation nous créons à l'aide de `Perl` un fichier intermédiare ``builtins.asm.inc`` formaté comme une suite de chaînes de caractères C, que nous incluons directement :

```c
void CodeWriter_load_builtins(FILE* nasm) {
  fprintf(
    nasm,
    #include "../obj/builtins.asm.inc"
  );
}
```

## Tests

Vous pouvez lancer les tests par deux manières différentes :

  - Via le terminal et la commande suivante :

    ```bash
    python3 test/test.py
    ```

    Le script peut également produire une sortie sans couleurs, idéal pour créer un récapitulatif sans les caractères d'échappement ANSI :

    ```bash
    python3 test/test.py --no-color
    ```

  - Via le Makefile :
  
    ```bash
    make test
    ```

  - Via VSCode :

    - Depuis la palette de commandes (Ctrl+Maj+P), sélectionner *Python: configure tests*

    - Choisir ``unittest``, puis sélectionner le répertoire ``test``, et enfin le *pattern* ``test*.py``

    - Cliquez sur l'onglet `Test` sur le volet gauche de l'éditeur.

    - Lancez les tests avec le bouton *play* (sous forme de flèche).

\pagebreak

# Avancement du projet

## Messages d'erreur

Afin d'améliorer les erreurs et avertissements affichés, nous avons implémenté un affichage des lignes concernées. Nous avons essayé autant que possible d'afficher l'emplacement de l'erreur sur la ligne via un curseur. cddza

```shell
warn/core/implicit_int_to_char_8.tpc:10:22: warning: 'int' to parameter of type 'char'
   10 |     res = f('a', 32770, 'a', 32770, 'a', 32770, 'a', 32770); // 4 warnings
      |                     ^
```

```shell
warn/core/NonVoidFunctionWithoutReturn_1.tpc:7:0: warning: missing return statement in function
'f' returning non-void
    7 | }
      | ^
```

```shell
sem-err/symtable/multiple_redeclaration.tpc:20:27: error: redeclaration of parameter 'a'
   20 | int g(int a, int b, int a[]) { // Error: redeclaration of 'a'
      |                          ^
```

## Élaboration des tests unitaires

Pour lancer les tests, veuillez vous référer à la section ['Test'](#Tests).

Dans la continuité du projet d'analyse syntaxique, nous avons implémenté notre série de tests avec Python.

Nous avons conservé les tests de notre précédent projet, désormais situés dans les répertoires ``syn-good`` et ``syn-err``. Nous les avons étendu avec des tests pour l'émission d'avertissements (``warn``), d'erreurs sémantiques (``sem-err``), et enfin pour des codes compilables avec une sortie prédictible (``good``).

### C'est quoi ces commentaires dans les codes ``sem-err/`` et ``warn/`` !?

Pendant le développement de la partie de vérification sémantique du projet, nous nous sommes demandé comment s'assurer que notre compilateur émette toujours les mêmes erreurs et warnings.

La solution la plus évidente était d'afficher en sortie (``stdout``) un code d'erreur correspondant. Cependant nous avons écarté ce choix pour éviter de complexifier la structure du projet.
Nous nous sommes donc satisfait de compter les occurrences d'erreurs et avertissements. Par conséquent, nous écrivons dans les codes de tests, le nombre d'erreurs/avertissements attendus à l'aide commentaires comme ceci :

```c
// nb_errors=1
// nb_warnings=0
```

Ces derniers sont interprétés par le script Python, et comparés à la sortie du compilateur.

\pagebreak

### Valgrind mais pas pour les fuites

Malheureusement, le projet possède des fuites de mémoire qui n'ont pas su être corrigées.

Cependant nous l'avons tout de même implémenté parmi les tests, mais uniquement pour vérifier l'absence d'accès à des zones mémoires non-initialisées. Cela devrait limiter les problèmes du type "It's works on my machine."...

### Emprunt d'un compilateur concurrent...

Pour vérifier que nos programmes générés possèdent bien le comportement désiré, nous avons implémenté dans nos tests une phase de compilation des codes ``good/`` avec TPCC et GCC.
Les binaires produits sont exécutés et leurs sorties standard/codes d'erreurs comparées avec celles de GCC.
Cela permet ainsi de nous assurer en partie de la validité du code produit.

### Fiabiliser le projet

Afin de s'assurer que nos tests soient exécutés régulièrement, nous avons à l'aide des *Actions* de Github, mis en place un système d'intégration continue.
Ainsi à chaque *push* sur une branche, les suites de tests sont toujours exécutées. Cela nous permet de limiter les régressions entre les commits.

Vous pouvez observer son fonctionnement sur cette [page](https://github.com/quentinl03/Analyseur_TPC/actions).

\pagebreak

## Modification par rapport à AS

Durant nos tentatives pour produire des routines NASM valides, nous avons remarqué différentes erreurs dans notre arbre :

  - Un cas très spécifique avec les tableaux produisait un arbre incohérent. La définition d'une fonction prenant seulement un tableau en paramètre, inhibait la création d'un nœud ``ListTypVar``.
  
    ```c
    int f(int tab[]) {}
    ```

    Avec ce code, nous obtenions cette arbre :

    ```
    DeclFonct : 
    ├── EnTeteFonct : 
    │   ├── Type : int
    │   ├── Ident : f
    │   └── DeclFonctArray : int
    │       └── Ident : tab
    ```

    Au lieu de celui-ci :

    ```
    DeclFonct : 
    ├── EnTeteFonct : 
    │   ├── Type : int
    │   ├── Ident : f
    │   └── ListTypVar : 
    │       └── DeclFonctArray : int
    │           └── Ident : tab
    ```

    Fort heureusement, une pré-assertion a échoué précocement dans la table des symboles. En conséquence, la recherche de la source de l'erreur a été plutôt aisée.

    La règle fautive était plutôt évidente :

    ```
    ListTypVar | DeclFonctArray {$$ = $1;}
    ```

  - Afin de gérer avec plus de praticité la présence de l'opérateur unaire, nous avons ajouté un label ``AddsubU`` pour le distinguer de l'opérateur binaire :

    ```
    F :  ADDSUB F     {$$ = makeNode(AddsubU); ...}
    ```

## Une table des symboles

Nous avons implémenté les tables des symboles à l'aide de tableaux triés.

### Pourquoi pas une table de hachage ?

La table de hachage permet l'accès à une entrée en temps constant. Cela est très avantageux car nous les ajoutons une seule fois, mais nous pouvons y accéder plusieurs fois.
Cependant, une implémentation de A à Z est lourde et chronophage. Et une utilisation de la ``glibc`` réduit les intérêts de développement personnel algorithmique... 

### Notre implémentation 

Les tableaux triés sont un bon compromis en termes de complexité / temps investi.
En effet, la structure est plus légère qu'une table de hachage mais reste plus rapide qu'un parcours basique en temps linéaire.
D'autre part, nous nous sommes permis d'utiliser les fonctions standard `bsearch` et `qsort` car nous avions déjà implémenté par le passé, une fonction de tri rapide et une recherche dichotomique.

## Des instructions originales : lea, sete, movzx, cqo ?!

### Les tableaux

L'implémentation des tableaux nécessite la possibilité d'indexer des éléments, avec une opération courante de la forme

$$
\text{Elem}_\text{addr} =
\text{Array}_\text{addr} + \text{index} \times \text{sizeof}(\text{Array}_\text{type})
$$

Ce calcul pourrait être implémenté avec les instructions ``add`` et ``mul``, cependant en plus d'être fastidieux, cela alourdit le code généré.

Pendant le cours d'Architecture des ordinateurs, nous avons découvert l'instruction ``lea`` (*Load Effective Address*).

En NASM, l'instruction déréférencement (``[]``) accepte l'usage des opérateurs multiplication et addition à l'intérieur de ses crochets. Cependant, nous souhaitons uniquement calculer l'adresse d'un élément. C'est ici qu'intervient l'instruction ``lea``.

Elle permet d'utiliser l'opérateur de déréférencement, *sans déréférencer*, puis de stocker la valeur calculée dans sa première opérande.

```c
fprintf(
    nasm,
    "lea rax, [rdx + rax * %d]\n"
    "push rax\n",
    symbol->type_size);
```

### La négation booléenne

La négation booléenne peut-être implémenté avec l'équivalent d'une condition :

```c
int a = 1;
if (a != 0) {
  a = 0;
}
else {
  a = 1;
}
```

Nous pourrions donc implémenter un dur cet équivalent en NASM, cependant c'est assez lourd, et inefficace, car nous devons utiliser des jumps.

Une meilleure alternative, découverte à l'aide de [Compiler Explorer](https://godbolt.org/) est d'utiliser l'inststruction ``sete`` (*Set if Equals*).

Comme son nom l'indique, elle met à ``1`` son opérande (octet inférieur d'un registre) si le résultat de la dernière comparaison était une égalité (drapeau ``ZF`` (*Zero Flag*) positif).

```asm
pop rdi
cmp rdi, 0
sete al        ; si rdi == 0, `al` vaut 1, 0 sinon
push rax
```

\pagebreak

Cependant, un problème subsiste. Seulement le registre inférieur de ``rax`` possède la valeur ``0x01``, les octets *à gauche* peuvent conserver des données faussant la valeur du registre dans son ensemble.
Or, notre compilateur utilise pour l'ensemble de ses types une taille de 8 octets. La valeur à empiler doit, par conséquent, posséder ses 63 bits à zéro.

Pour satisfaire cette contrainte, une méthode simple est de réaliser un ``mov``, sur le même registre. Cependant, ce dernier impose à ses deux opérandes d'être de même taille.
Ainsi, il existe une instruction plus appropriée nommée ``movzx`` permettant de copier la valeur de sa seconde opérande (registre de taille inférieure), vers sa première opérande (registre de taille supérieure), en complétant les octets manquant par des zéros :

```asm
movzx rax, al  ; Adds 0s to the left of the regis
```

Ainsi, l'ensemble de la procédure devient :

```asm
pop rdi
cmp rdi, 0
sete al        ; si rdi == 0, `al` vaut 1, 0 sinon
movzx rax, al  ; Ajoute des zéros à gauche du registre
push rax
```

### La division et le modulo avec une opérande négative

Notre projet stocke ses entiers sous 8 octets, or le résultat d'une division avec ``idiv`` stocke le couple (quotient, reste) sous deux ``DWORD``s (32 bits).

Or les nombres sont stockés par complément à deux. Cela signifie que les nombres négatifs possèdent leurs bits de signe à leur gauche :

Par exemple : $-2_{\text{dec}} =\underbrace{1111\dotsc 1111\ 1111}_{63\ \text{bits}} 0_{\text{bin}}$

Ainsi pour conserver le signe de la division, et ne pas obtenir des résultats incohérents, il est nécessaire de copier le bit de signe du dividende (``rax``) vers le registre ``rdx``.
Pour cela, nous pourrions simplement copier ``rax`` vers ``rdx``, mais il existe une instruction spécialement dédiée : ``cqo``.

Nous utilisons ainsi cette routine :

```asm
pop rcx
pop rax
cqo       ; Copie le bit de signe de rax, vers tous les bits de rdx
idiv rcx
push %s
```

\pagebreak

# Conclusion

## Améliorations possibles

Nous pouvons toujours améliorer notre travail, mais il faut savoir être raisonnable, et repérer ce qui peut être implémenté "facilement" :

- Nous avons choisi par praticité, d'implémenter tous les types sous 8 octets. Cependant il serait plus raisonnable d'utiliser 1 octet pour les types `char`, et 4 octets pour les `int`.

- Nous avons fait un fichier NASM sans indentation par facilité au début, afin d'ajouter rapidement des fonctionnalités. Mais il aurait pu être raisonnable de l'implémenter afin de rendre la relecture et le débogage plus simple des fichier générés.\
La macro présente ci-dessous nous permet de gérer les indentations. Cependant nous trouvions dommage qu'il faille utiliser une macro par ligne, contre un seul `fprintf` par bloc actuellement. Même si la sortie standard est *bufferisée*, cela reste "moins agréable" visuellement dans le fichier de voir un appel de fonction/macro par ligne, alors que l'on peut utiliser la concaténation des chaînes de caractère C.

  ```c
  // Ajoute indentLevel espaces à chaque début de ligne avant d'écrire sur __output
  #define EMIT(str, ...) fprintf(__output, "%*s" str "\n", indentLevel, "", ##__VA_ARGS__)

  // Ajoute un commentaire au fichier
  #define COMM(str, ...) EMIT("; " str, ##__VA_ARGS__)

  // Pseudo fonction devant remplacer les appels à fprintf
  #define ASM_BLOCK(file, ...) do { \
      FILE* __output = file;        \
      (__VA_ARGS__);                \
      fputc('\n', __output);        \
  } while (0)
  ```



## Avis personnel sur le projet

Ce projet est très intéressant algorithmiquement, il se distingue d'autres projets car il demande de la réflexion et pas juste de la rédaction de code.

Nous sommes divergents sur notre partie préférée du projet, Quentin préférait le projet d'Analyse Syntaxique car bien plus lié à la théorie du cours qu'à la programmation : Bison se rapprochait énormément de la syntaxe des règles syntaxiques vues en cours. Quant à Nicolas, il à préféré le projet de Compilation, car plus tourné vers l'implémentation directe et l'utilisation d'instructions assembleur, étant déjà curieux sur le fonctionnement de ces derniers.
