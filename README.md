# CODING PARTY

## Le principe :

On veut représenter une relation client, vendeur, caissier .
Dans notre programme, il y a autant de rayons que de vendeurs.  
Lorsqu'un client entre, il se dirige vers le vendeur qui a la plus petite liste d'attente. Lorsque le client est géré par un vendeur, si celui-ci n'est pas dans le bon rayon, il est redirigé par le vendeur vers le bon rayon.

## Comment compiler ?

On utilisera `make` pour compiler et `make clean` pour supprimer les fichiers générés lors de la compilation.

## Comment les communications sont gérées ?

On utilise uniquement des **SMP** et des **sémaphores** pour gérer les communications entre les clients, vendeurs et caissiers.

si vous souhaitez interprété ce README dans un navigateur, vous pouvez faire la
commande : pandoc README.md -o README.html
et lancer README.html dans un navigateur