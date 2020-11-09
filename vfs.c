#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIALTABLE 37
#define ENDTABLE 36
#define INITIALDATAS 1038
#define ENDDATAS 1037

#define LS 0
#define CAT 1
#define WRITE 2
#define IMPORT 3
#define EXPORT 4
#define EXIT 5

FILE *createFile(char *name);
FILE *openFile(char *name);
void writeFile(FILE *fp, int position, int size, char *content);
void readFile(FILE *fp, int position, int size, char *receptor);

void createInitialFile(char *name);
void insertEMagic();
void verifyEMagic();
void insertHeader();
void readHeader();
void insertInitialHeader();
void openInitialFile(char *name);
void arg(int argc, char *argv[]);
void addId();
void readId(int index);
char *getNameId(int index);
int getPositionInTable(int index);
void updateHeader(int sizeFile);
int numerOfIds();
void swapPositions(int i, int j);
void quickTable();
void qt(int left, int right);
int binaryTable(char *name);
void timeString(char *str);
void addData(char *str, int size);
void readData(char *str, int position, int size);
void ls();
void write();
void cat();
void zeroString(char *str, int size);
int copyExternalFiles(FILE *fpOrigin, FILE *fpDestiny, int pDestiny);
void import();
void export();
void exit1();
void error();

void inputLine(char *sinalizer, char *text);
int comand();

struct {
    int initialTable;
    long endTable;
    long initialDatas;
    long endDatas;
} header;
struct {
    char name[30];
    char data[15];
    long start;
    long size;
} id;
FILE *fpInitial;

int main(int argc, char *argv[]) {
    
    arg(argc, argv);

    for(;;) {
        switch(comand()) {
            case LS: ls(); break;
            case CAT: cat(); break;
            case WRITE: write(); break;
            case IMPORT: import(); break;
            case EXPORT: export(); break;
            case EXIT: exit1(); break;
            default: error(); break;
        }       
    }   

    fclose(fpInitial);

    return 0;
}

// File
FILE *createFile(char *name) {
    FILE *fp;

    if((fp = fopen(name, "wb+")) == NULL) {
            printf("Error creating file.\n");
            exit(1);
    } 

    return fp;
}

FILE *openFile(char *name) {
    FILE *fp;

    if((fp = fopen(name, "rb+")) == NULL) {
            printf("Error opening file.\n");
            exit(1);
    } 

    return fp;
}

void writeFile(FILE *fp, int position, int size, char *content) {
    fseek(fp, position, SEEK_SET);
    fwrite(content, size, 1, fp);
}

void readFile(FILE *fp, int position, int size, char *receptor) {
    fseek(fp, position, SEEK_SET);
    fread(receptor, size, 1, fp);
}

// Control
void exit1() {
    printf("Good by :)\n");
    exit(0);
}

void createInitialFile(char *name) {
    fpInitial = createFile(name);
    insertEMagic();
    insertInitialHeader();
}

void insertEMagic() {
    writeFile(fpInitial, 0, 4, "VFSF");
}

void verifyEMagic() {
    char emagic[5];
    readFile(fpInitial, 0, 4, emagic);

     if(strcmp(emagic, "VFSF")) {
        printf("Error, it`s not a VFS file.\n");
        exit(1);
    }
}

void insertHeader() {    
    writeFile(fpInitial, 5, sizeof(header), &header);
}

void insertInitialHeader() {
    header.initialTable = INITIALTABLE;
    header.endTable = ENDTABLE;
    header.initialDatas = INITIALDATAS;
    header.endDatas = ENDDATAS;

    insertHeader();
}

void readHeader() {
    readFile(fpInitial, 5, sizeof(header), &header);
}

void openInitialFile(char *name) {
    fpInitial = openFile(name);
    verifyEMagic();
    readHeader();
}

void arg(int argc, char *argv[]) {
    if(argc > 2)
        createInitialFile(argv[2]);
    else
        openInitialFile(argv[1]);
}

void updateHeader(int sizeFile) {
    header.endTable += sizeof(id);
    header.endDatas += sizeFile;

    insertHeader();
}

void addId() {
    writeFile(fpInitial, header.endTable+1, sizeof(id), &id);
}

void readId(int index) {
    readFile(fpInitial, getPositionInTable(index), sizeof(id), &id);
}

char *getNameId(int index) {
    readId(index);
    return id.name;
}

int getPositionInTable(int index) {
    return header.initialTable+sizeof(id)*index;
}

int numberOfIds() {
    int i = header.endTable;
    int j = header.initialTable;
    int v = (header.endTable-header.initialTable)/sizeof(id);
    return v;
}

void swapPositions(int i, int j) {
    char tempI[sizeof(id)], tempJ[sizeof(id)];

    readFile(fpInitial, getPositionInTable(i), sizeof(id), &tempI);
    readFile(fpInitial, getPositionInTable(j), sizeof(id), &tempJ);

    writeFile(fpInitial, getPositionInTable(j), sizeof(id), &tempI);
    writeFile(fpInitial, getPositionInTable(i), sizeof(id), &tempJ);
}

void quickTable() {
    qt(0, numberOfIds());
}

void qt(int left, int right) {
    int i, j;
    char x[30];

    i = left; j = right;
    strcpy(x, getNameId((i+j)/2));

    do {
        while(strcmp(getNameId(i), x) < 0) i++;
        while(strcmp(getNameId(j), x) > 0) j--;

        if(i <= j) {
            swapPositions(i, j);
            i++; j--;
        }
    } while(i <= j);

    if(i < right) qt(i, right);
    if(j > left) qt(left, j);
}

int binaryTable(char *name) {
    int left, right, mid, positionFile = 0;

    left = 0; right = numberOfIds();

    while(left <= right) {
        mid = (left+right) / 2;
        if(strcmp(name, getNameId(mid)) < 0) right = mid - 1;
        else if(strcmp(name, getNameId(mid)) > 0) left = mid + 1;
        else return mid;
    }    
}

void timeString(char *str) {
    time_t t;  
    struct tm *temp;  

    t = time(NULL);
    temp = localtime(&t);
    sprintf(str, "%dh%d %d.%d.%d", temp->tm_hour, temp->tm_min, temp->tm_mday, temp->tm_mon+1, temp->tm_year);
}

void addData(char *str, int size) {
    writeFile(fpInitial, header.endDatas+1, size, str);
    header.endDatas += size;
}

void readData(char *str, int position, int size) {
    readFile(fpInitial, position, size, str);
}


void ls() {
    char name[30];
    int i, index;

    inputLine("NAME: ", name);

    if(name[0] == '*') {
        for(i = 0; i <= numberOfIds(); i++) {
            readId(i);
            printf("%s\t\t%s\n", id.name, id.data);
        }
    }
    else {
        index = binaryTable(name);
        readId(index);
        printf("%s\t\t%s\n", id.name, id.data);
    }
}

void cat() {
    char name[30], text[1000];
    int index;
    zeroString(text, sizeof(text));

    inputLine("NAME: ", name);

    index = binaryTable(name);
    readId(index);
    readData(text, id.start, id.size);

    printf("%s\n", text);
}

void zeroString(char *str, int size) {
    int i;
    for(i = 0; i < size; i++)
        str[i] = '\0';
}

void write() {
    char name[30], text[1000];   

    inputLine("NAME: ", name);
    inputLine("TEXT: ", text);

    strcpy(id.name, name);
    timeString(id.data);
    id.start = header.endDatas+1;
    id.size = strlen(text);

    addId();
    addData(text, id.size);
    updateHeader(id.size);
    quickTable();
}

int copyExternalFiles(FILE *fpOrigin, FILE *fpDestiny, int pDestiny) {
    int size = 0;
    
    fseek(fpOrigin, 0, SEEK_SET);
    fseek(fpDestiny, pDestiny, SEEK_SET);

    while(!feof(fpOrigin)) {
            putc(getc(fpOrigin), fpDestiny);
            size++;
    } 

    return size;
}

void export() {
    char name[30], text[1000];
    int index;
    FILE *fpExternal;

    zeroString(text, sizeof(text));

    inputLine("NAME: ", name);

    fpExternal = createFile(name);

    index = binaryTable(name);
    readId(index);

    readFile(fpInitial, id.start, id.size, text);
    writeFile(fpExternal, 0, strlen(text), text);

    printf("Copy complete!\n");

    fclose(fpExternal);
}

void import() {
    char name[30];
    int size;
    FILE *fpExternal;

    inputLine("NAME: ", name);

    fpExternal = openFile(name);

    size = copyExternalFiles(fpExternal, fpInitial, header.endDatas+1);

    strcpy(id.name, name);
    timeString(id.data);
    id.start = header.endDatas+1;
    id.size = size;

    addId();
    updateHeader(id.size);
    quickTable();

    printf("Copy complet!\n");

    fclose(fpExternal);    
}

void error() {
    printf("Comand not found.\n");
}

// Input
void inputLine(char *sinalizer, char *text) {
    printf("%s", sinalizer);
    scanf(" %[^\n]", text);
}

int comand() {
    char input[10];

    inputLine("-> ", input);

    if(!strcmp(input, "ls")) return LS;
    if(!strcmp(input, "cat")) return CAT;
    if(!strcmp(input, "write")) return WRITE;
    if(!strcmp(input, "import")) return IMPORT;
    if(!strcmp(input, "export")) return EXPORT;
    if(!strcmp(input, "exit")) return EXIT;

    return -1;
}