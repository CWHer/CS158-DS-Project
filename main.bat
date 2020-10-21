g++ -o2 code.cpp -o code.exe
valgrind --tool=memcheck --leak-check=full --leak-resolution=med --track-origins=yes --vgdb=no ./code.exe