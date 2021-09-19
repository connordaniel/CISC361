#include "mp3.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define BUFFSIZE 128
mp3 *head = NULL;
mp3 *tail = NULL;

int main(int argc, char *argv) {
    atexit(freeMem);
    menu(&head, &tail);
    exit(0);
}

//menu: starts the main menu and asks the user what they'd like to do,
//breaks when user selects exit.
//parameters: head and tail pointer
//returns: none

void menu(mp3 **head, mp3**tail) {
    char buff [BUFFSIZE] = {'\0'};
    int choose = 0;
    int scanner = 0;
    while (choose != 5) {
        printf("\n What would you like to do?\n1: Add MP3\n2: delete MP3(s)\n3: Print front to back\n4: Print back to front\n5: Exit program\n");
        if (fgets(buff, BUFFSIZE, stdin) != NULL){
            scanner = sscanf(buff, "%d", &choose);
            if (scanner == 0 || scanner == EOF) 
                choose = 0;
        }
        switch(choose) {
            case 1:
                add(head, tail);
                break;
            case 2:
                delete(head, tail);
                break;
            case 3:
                printForwards(*head);
                break;
            case 4:
                printBackwards(*tail);
                break;
            case 5:
                break;
        }
    }
}
//uses makeNode to generate a new node,
//then moves node to end of DLL
//params: head and tail pointer
//returns: none
void add(mp3 **head, mp3 **tail) {
    mp3 *new = makeNode();
    if (*head == NULL) {
        *head = new;
        *tail = new;
        printf("Node added");
    } else {
        (*tail)->next = new;
        new->prev = *tail;
        *tail = new;
    }
}
//creates new mp3 and prompts user for information
//params: none
//returns: newly created mp3 
mp3 *makeNode(){
    mp3 *ret = calloc(1, sizeof(mp3));
    char buff [BUFFSIZE] = {'\0'};
    int len;
    int year = 0;
    int runtime = 0;

    printf("\nEnter the song title.\n");
    if (fgets(buff, BUFFSIZE, stdin) != NULL) {
        len = strlen(buff);
        ret->title = calloc(len, sizeof(char));
        strncpy(ret->title, buff, len - 1);
    }

    printf("\nEnter the artist's name.\n");
    if (fgets(buff, BUFFSIZE, stdin) != NULL) {
        len = strlen(buff);
        ret->artist = calloc(len, sizeof(char));
        strncpy(ret->artist, buff, len - 1);
    }

    printf("\nEnter the year the song was published.\n");
    if (fgets(buff, BUFFSIZE, stdin) != NULL) {
        int scan = sscanf(buff, "%d", &year);
        ret->year = year;
    }

    printf("\nEnter the runtime in minutes.\n");
    if (fgets(buff, BUFFSIZE, stdin) != NULL) {
        int scan = sscanf(buff, "%d", &runtime);
        ret->runtime = runtime;
    }

    return ret;
}



//deletes node and frees memory if the artist matches the user-given name
//params: head and tail pointer
//returns: none
void delete(mp3 **head, mp3 **tail) {
    char buff [BUFFSIZE] = {"\0"};
    char *name;
    printf("\nWhich artist would you like to delete?\n");
    if (fgets(buff, BUFFSIZE, stdin) != NULL) {
        int len = strlen(buff);
        name = calloc(len, sizeof(char));
        strncpy(name, buff, len - 1);
    }
    mp3 *curr = *head;
    mp3 *next = NULL;

    while (curr != NULL) {
        if (strcmp(name, curr->artist) == 0) {
            next = curr->next;
            //delete node
            deleteNode(head, tail ,curr);
            printf("Deleted song\n");
            curr = next;
        } else {
            curr = curr->next;
        }
    }
    free(name);
}

//ensures that 
void deleteNode(mp3 **head, mp3 **tail, mp3 *curr) {
    if (*tail == curr)
        *tail = curr->prev;
    if (*head == curr) 
        *head = curr->next;
    if (curr->next != NULL)
        curr->next->prev = curr->prev;
    if (curr->prev != NULL) 
        curr->prev->next = curr->next;
    free(curr->title);
    free(curr->artist);
    free(curr);
}

//generic method for printing given node
void printNode(mp3 *node) {
    printf("\nTitle: %s | Artist: %s | Year: %d | Runtime: %d seconds\n", node->title, node->artist, node->year, node->runtime);
}

//starts at the given head and prints forward, uses printNode
void printForwards(mp3 *head) {
    while(head != NULL) {
        printNode(head);
        head = head->next;
    }
}
//starts at the given tail and moves backwards
void printBackwards(mp3 *tail) {
    while (tail != NULL) {
        printNode(tail);
        tail = tail->prev;
    }
}

//called when the user requests to exit, frees all memory in the linked list.
//no params, no returns
void freeMem() {
    printf("\n Cleaning up list ... \n");
    deleteAll(&head, &tail);
    printf("List Deleted.\n");
}

//iterates through list and deletes all nodes
//params: head and tail pointers
void deleteAll(mp3 **head, mp3 **tail) {
    mp3 *curr = *head;
    mp3 *next = NULL;
    while (curr != NULL) {
        next = curr->next;
        deleteNode(head, tail, curr);
        curr = next;
    }
    head = NULL;
    tail = NULL;
}