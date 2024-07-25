#include <stdio.h>
#include <time.h>

#define MAX_BUFFER_SIZE 80
#define MAX_BORROW 5

struct book{
    int id;
    char name[50];
    int qty; 
    char author[50];
    char desc[100];
    int borrow;
    int del;
};
struct login {
    char username[50];
    char password[50];
};
struct user {
    char username[50];
    char password[50];
    int books_borrow;
    int books_return;
};
struct records {
    char username[50];
    int book_id;
    int book_borrow;
    char time_borrow[MAX_BUFFER_SIZE];
    char time_return[MAX_BUFFER_SIZE];
};
