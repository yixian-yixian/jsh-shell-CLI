make clean
make
# valgrind --leak-check=full --show-leak-kinds=all -v ./shell
./shell
# valgrind ./shell