#Sujet de programmation impérative

La page du sujet :

<https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php>

La page sur thor :

<https://thor.enseirb-matmeca.fr/ruby/projects/pr105-coursidor>

Lien ladder :

<https://www.labri.fr/perso/renault/working/teaching/projets/ladder/ladder.html>

Makefile guide :

make build → compile le serveur uniquement

make build_tests → compile alltests sans le serveur

make test → lance les tests

make install → installe server, .so, et alltests

make clean → nettoie tout

Clang-format rules :

find . -name '*.c' -o -name '*.h' | xargs clang-format -i 
( on fait . si on veut appliquer clang-format sur les fichier sur le dépôt actuel sinon on met le chemin des autres fichiers qu'on veut tester)
