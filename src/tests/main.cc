#include "../kvstore/kvstore.h"

int main() {
    FILE *file = fopen("data/vlog/test", "w");
    fprintf(file, "Hello, world!\n");
    fprintf(file, "This is a test file.\n");
    fclose(file);
}