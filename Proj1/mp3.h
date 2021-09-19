typedef struct mp3 {
    char *artist;
    char *title;
    int year;
    int runtime;
    struct mp3 *next;
    struct mp3 *prev;
} mp3;

void menu(mp3 **head, mp3 **tail);
mp3 *makeNode();
void printForwards(mp3 *head);
void printBackwards(mp3 *tail);
void add(mp3 **head, mp3 **tail);
void deleteNode(mp3 **head, mp3 **tail, mp3 *curr);
void delete(mp3 **head, mp3 **tail);
void deleteAll(mp3 **head, mp3 **tail);
void freeMem();