#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }
    
    return 0;
}