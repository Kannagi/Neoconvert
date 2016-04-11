
compilation
gcc main.c -lSDL -lSDL_image -o neoconvert 


default if 052-c1.bin and 052-c2.bin exist ,example :
neogeoconvert tile1.png +0
neogeoconvert tile2.png +32
ect
or :
neogeoconvert tile1.png +0 -o romc1.c1
neogeoconvert tile2.png +32 -o romc1.c1

You can put sprites of 31 colors

option:
-simplemap
-nomap
-palette

