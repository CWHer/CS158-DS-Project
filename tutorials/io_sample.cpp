// C Files I/O
#include <iostream>
using namespace std;

int main() {
    FILE *file;
    const char *filename = "abc";
    
    file = fopen(filename, "rb+");
    // if there is no such file
    if (!file) {
        file = fopen(filename, "wb+");
    }

    bool write = false;
    int offset = 7;
    if (write) {
        int a = 123123;
        fwrite(&a, sizeof(int), 1, file);

        fseek(file, offset, SEEK_SET);
        string b = "hello";
        fwrite(&b, sizeof(string), 1, file);
    } else {
        int a;
        fread(&a, sizeof(int), 1, file);
        cout << a << endl;

        fseek(file, offset, SEEK_SET);
        string b;
        fread(&b, sizeof(string), 1, file);
        cout << b << endl;
    }  
    fclose(file);

    // if we want to clear the file
    bool clear = true;
    if (clear) {
       file = fopen(filename, "wb+");
        fclose(file); 
    }
    
    return 0;
}