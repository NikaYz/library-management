#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include <fcntl.h>
#include <sys/stat.h>

void lock_file(int fd, short type) {
    struct flock lock;
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    while (fcntl(fd, F_SETLKW, &lock) == -1) {
        if (errno != EINTR) {
            perror("fcntl");
            return;
        }
    }
}

void unlock_file(int fd) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    while (fcntl(fd, F_SETLKW, &lock) == -1) {
        if (errno != EINTR) {
            perror("fcntl");
            return;
        }
    }
}


void addBooks(int fd_books, int new_fd, int fd_admin) {
    struct book b;
    read(new_fd, &b, sizeof(b));

    b.del = 0;
    int id = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
    b.id = id;
    b.borrow = 0;
    int ans = 0;
    struct book temp;

    lock_file(fd_books, F_WRLCK);

    for (int i = 0; i < id; i++) {
        read(fd_books, &temp, sizeof(struct book));
        if (strcmp(temp.name, b.name) == 0) {
            ans = 2;
            write(new_fd, &ans, sizeof(ans));
            unlock_file(fd_books);
            return;
        }
    }
    if (write(fd_books, &b, sizeof(b)) == -1) {
        write(new_fd, &ans, sizeof(ans));
    } else {
        ans = 1;
        write(new_fd, &ans, sizeof(ans));
    }

    unlock_file(fd_books);
}


void deleteBook(int fd_books, int new_fd, int fd_admin) {
    int noOfBooks = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
    write(new_fd, &noOfBooks, sizeof(noOfBooks));
    struct book books[noOfBooks + 1];
    lseek(fd_books, 0, SEEK_SET);
    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &books[i], sizeof(struct book));
    }
    write(new_fd, &books, sizeof(books));
    int id;
    read(new_fd, &id, sizeof(id));
    lseek(fd_books, 0, SEEK_SET);
    struct book b;
    int ans = 0;

    lock_file(fd_books, F_WRLCK);

    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &b, sizeof(struct book));
        if (b.id == id && b.del != -1) {
            b.del = -1;
            lseek(fd_books, -(sizeof(struct book)), SEEK_CUR);
            write(fd_books, &b, sizeof(b));
            ans = 1;
            write(new_fd, &ans, sizeof(ans));
            unlock_file(fd_books);
            return;
        }
    }
    write(new_fd, &ans, sizeof(ans));
    unlock_file(fd_books);
}

void modifyBook(int fd_books, int new_fd, int fd_admin) {
    int noOfBooks = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
    write(new_fd, &noOfBooks, sizeof(noOfBooks));
    struct book books[noOfBooks + 1];
    lseek(fd_books, 0, SEEK_SET);
    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &books[i], sizeof(struct book));
    }
    write(new_fd, &books, sizeof(books));
    struct book b;
    read(new_fd, &b, sizeof(b));
    lseek(fd_books, 0, SEEK_SET);
    struct book temp;
    int ans = 0;

    lock_file(fd_books, F_WRLCK);

    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &temp, sizeof(struct book));
        if (b.id == temp.id) {
            lseek(fd_books, -(sizeof(struct book)), SEEK_CUR);
            write(fd_books, &b, sizeof(b));
            ans = 1;
            write(new_fd, &ans, sizeof(ans));
            unlock_file(fd_books);
            return;
        }
    }
    write(new_fd, &ans, sizeof(ans));
    unlock_file(fd_books);
}

void searchBook(int fd_books, int new_fd, int fd_admin) {
    int noOfBooks = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
    write(new_fd, &noOfBooks, sizeof(noOfBooks));
    struct book books[noOfBooks + 1];
    lseek(fd_books, 0, SEEK_SET);

    lock_file(fd_books, F_RDLCK);

    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &books[i], sizeof(struct book));
    }
    write(new_fd, &books, sizeof(books));

    unlock_file(fd_books);
}



void newUser(int fd_user, int new_fd, int fd_admin, struct login l) {
    struct user u;
    int users = lseek(fd_user, 0, SEEK_END) / sizeof(u);
    lseek(fd_user, 0, SEEK_SET);
    int ans = 0;

    lock_file(fd_user, F_WRLCK);

    for (int i = 0; i < users; i++) {
        read(fd_user, &u, sizeof(u));
        if (strcmp(u.username, l.username) == 0) {
            ans = 2;
            write(new_fd, &ans, sizeof(ans));
            unlock_file(fd_user);
            return;
        }
    }
    u.books_borrow = 0;
    u.books_return = 0;
    strcpy(u.username, l.username);
    strcpy(u.password, l.password);
    if (write(fd_user, &u, sizeof(u)) == -1) {
        write(new_fd, &ans, sizeof(ans));
    } else {
        ans = 1;
        write(new_fd, &ans, sizeof(ans));
    }
    unlock_file(fd_user);
}


int checkUser(int fd_user, struct login l, int new_fd) {
    struct user u;
    int users = lseek(fd_user, 0, SEEK_END) / sizeof(u);
    int ans = 0;
    lseek(fd_user, 0, SEEK_SET);

    lock_file(fd_user, F_RDLCK);

    for (int i = 0; i < users; i++) {
        read(fd_user, &u, sizeof(u));
        if (strcmp(l.username, u.username) == 0) {
            if (strcmp(l.password, u.password) == 0) {
                ans = 1;
                write(new_fd, &ans, sizeof(ans));
                unlock_file(fd_user);
                return 1;
            }
            ans = 2;
        }
    }
    write(new_fd, &ans, sizeof(ans));
    unlock_file(fd_user);
    return 0;
}

char* getCurrentDateTime() {
    static char buffer[MAX_BUFFER_SIZE]; // Static buffer to hold the result
    time_t rawtime;
    struct tm *timeinfo;

    // Get current time
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Format time as string
    strftime(buffer, MAX_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo); // Format as "YYYY-MM-DD HH:MM:SS"

    return buffer;
}



void borrowBook(int fd, struct login l, int fd_books, int new_fd, int fd_user) {
    int noOfBooks = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
    write(new_fd, &noOfBooks, sizeof(noOfBooks));
    struct book books[noOfBooks + 1];

    // Read lock on the books file to allow concurrent reads
    lock_file(fd_books, F_RDLCK);
    lseek(fd_books, 0, SEEK_SET);
    //printf("No of books: %d\n", noOfBooks);
    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &books[i], sizeof(struct book));
    }
    unlock_file(fd_books); // Release the read lock

    write(new_fd, &books, sizeof(books));
    int h;
    read(new_fd, &h , sizeof(int));
    if(h == 0){
        return;
    }
    int id;
    read(new_fd, &id, sizeof(id));
    //printf("Id: %d", id);
    lseek(fd_books, 0, SEEK_SET);
    struct book b;
    int ans = 0;

    // Write lock on the books file to prevent concurrent writes
    lock_file(fd_books, F_WRLCK);
    for (int i = 0; i < noOfBooks; i++) {
        read(fd_books, &b, sizeof(struct book));
        //printf("Id: %d\n", b.id);
        if (b.id == id && b.del != -1 && ((b.qty - b.borrow) > 0)) {
            int users = lseek(fd_user, 0, SEEK_END) / sizeof(struct user);
            struct user u;
            //printf("Users: %d\n", users);
            lseek(fd_user, 0, SEEK_SET);
            for (int j = 0; j < users; j++) {
                read(fd_user, &u, sizeof(u));
                //printf("User: %s %s\n", u.username, u.password);
                //printf("User Details: %d %d\n", u.books_borrow, u.books_return);
                if (strcmp(u.username, l.username) == 0) {
                    if (u.books_borrow < MAX_BORROW) {
                        u.books_borrow = u.books_borrow + 1;
                        lseek(fd_user, -(sizeof(u)), SEEK_CUR);
                        write(fd_user, &u, sizeof(u));
                        break;
                    } else {
                        ans = 3;
                        write(new_fd, &ans, sizeof(ans));
                        unlock_file(fd_books); // Release the write lock
                        return;
                    }
                }
            }
            lseek(fd_books, -(sizeof(b)), SEEK_CUR);
            b.borrow++;
            write(fd_books, &b, sizeof(b));
            struct records r;
            r.book_borrow = 1;
            r.book_id = b.id;
            strcpy(r.time_borrow, getCurrentDateTime());
            strcpy(r.time_return, "NULL");
            strcpy(r.username, l.username);
            lseek(fd, 0, SEEK_END);
            write(fd, &r, sizeof(r));

            ans = 1;
            write(new_fd, &ans, sizeof(ans));
            unlock_file(fd_books); // Release the write lock
            return;
        }
    }
    write(new_fd, &ans, sizeof(ans));
    unlock_file(fd_books); // Release the write lock
    return;
}


void returnBook(int fd, struct login l, int fd_books, int new_fd, int fd_user) {
    int transactions = lseek(fd, 0, SEEK_END) / sizeof(struct records);
    lseek(fd, 0, SEEK_SET);
    int bookIdsBorrowed[transactions];
    int noOfBooksBorrowed = 0;
    struct records r;

    // Read lock on the records file to allow concurrent reads
    lock_file(fd, F_RDLCK);
    for (int i = 0; i < transactions; i++) {
        read(fd, &r, sizeof(r));
        if ((strcmp(r.username, l.username) == 0) && r.book_borrow == 1) {
            //printf("Books %d\n", r.book_id);
            bookIdsBorrowed[noOfBooksBorrowed] = r.book_id;
            noOfBooksBorrowed++;
        }
    }
    unlock_file(fd); // Release the read lock

    write(new_fd, &noOfBooksBorrowed, sizeof(int));
    if (noOfBooksBorrowed == 0) {
        return;
    }
    write(new_fd, &bookIdsBorrowed, sizeof(int) * noOfBooksBorrowed);
    int bookId;
    read(new_fd, &bookId, sizeof(int));
    int correctBookId = 0;
    for (int i = 0; i < noOfBooksBorrowed; i++) {
        if (bookIdsBorrowed[i] == bookId) {
            correctBookId = 1;
            break;
        }
    }
    int ans = 0;
    //printf("Correct: %d\n", correctBookId);
    if (correctBookId != 1) {
        ans = 2;
        write(new_fd, &ans, sizeof(int));
    }
    //printf("BookId present: %d\n", bookId);
    lseek(fd, 0, SEEK_SET);
    // Write lock on the records file to prevent concurrent writes
    lock_file(fd, F_WRLCK);
    for (int i = 0; i < transactions; i++) {
        read(fd, &r, sizeof(r));
        if ((strcmp(r.username, l.username) == 0) && r.book_borrow == 1 && r.book_id == bookId) {
            //printf("Got book id success\n");
            r.book_borrow = 0;
            strcpy(r.time_return, getCurrentDateTime());
            lseek(fd, -(sizeof(r)), SEEK_CUR);
            write(fd, &r, sizeof(r));
            //printf("Successfully changed records\n");
            unlock_file(fd); // Release the write lock
            // Update user's borrowing count
            
            int users = lseek(fd_user, 0, SEEK_END) / sizeof(struct user);
            struct user u;
            lseek(fd_user, 0, SEEK_SET);

            // Write lock on the user file to prevent concurrent writes
            lock_file(fd_user, F_WRLCK);
            for (int j = 0; j < users; j++) {
                read(fd_user, &u, sizeof(u));
                if (strcmp(u.username, l.username) == 0) {
                    u.books_borrow = u.books_borrow - 1;
                    lseek(fd_user, -(sizeof(u)), SEEK_CUR);
                    write(fd_user, &u, sizeof(u));
                    break;
                }
            }
            unlock_file(fd_user); // Release the write lock

            // Update book's borrow count
            int books = lseek(fd_books, 0, SEEK_END) / sizeof(struct book);
            struct book b;
            lseek(fd_books, 0, SEEK_SET);

            // Write lock on the books file to prevent concurrent writes
            lock_file(fd_books, F_WRLCK);
            for (int j = 0; j < books; j++) {
                read(fd_books, &b, sizeof(b));
                if (b.id == bookId) {
                    b.borrow = b.borrow - 1;
                    lseek(fd_books, -(sizeof(b)), SEEK_CUR);
                    write(fd_books, &b, sizeof(b));
                    break;
                }
            }
            unlock_file(fd_books); // Release the write lock

            ans = 1;
            printf("Successfully returning\n");
            write(new_fd, &ans, sizeof(int));
            //unlock_file(fd); // Release the write lock
            return;
        }
    }
    //printf("Last answer: %d\n", ans);
    write(new_fd, &ans, sizeof(int));
    unlock_file(fd); // Release the write lock
    return;
}

int main(){
    printf("Setting up server\n");

    //file containing all the records is called records.txt

    int fd = open("records.txt", O_RDWR | O_CREAT, 0777);
    int fd_books = open("books.txt", O_RDWR | O_CREAT, 0777);
    int fd_user = open("users.txt", O_RDWR | O_CREAT, 0777);
    int fd_admin = open("adminReceipt.txt", O_RDWR | O_CREAT, 0777);
    
    lseek(fd_admin, 0, SEEK_SET);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1){
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in server, client;
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("Error: ");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
        perror("Error: ");
        return -1;
    }

    if (listen(sockfd, 5) == -1){
        perror("Error: ");
        return -1;
    }

    int size = sizeof(client);
    printf("Server set up successfully\n");


    while (1){

        int new_fd = accept(sockfd, (struct sockaddr *)&client, &size);
        if (new_fd == -1){
            // perror("Error: ");
            return -1;
        }

        if (!fork()){
            printf("Connection with client successful\n");
            close(sockfd);

            int user;
            while(1){
            read(new_fd, &user, sizeof(int));
            //printf("%d",user);
            //printf("Hello\n");
            if (user == 1){
                while(1){
                
                    int m ;
                    read(new_fd,&m,sizeof(m));
                    //printf("%d\n",m);
                    if(m == 1){
                        struct login l;
                        read(new_fd,&l,sizeof(l));
                        printf("%s %s",l.username,l.password);
                        if(checkUser(fd_user,l,new_fd)){
                            int ch;
                            while (1){
                                // fd = open("records.txt", O_RDWR | O_CREAT, 0777);
                                // fd_books = open("books.txt", O_RDWR | O_CREAT, 0777);
                                // fd_user = open("users.txt", O_RDWR | O_CREAT, 0777);
                                // fd_admin = open("adminReceipt.txt", O_RDWR | O_CREAT, 0777);
                                read(new_fd, &ch, sizeof(int));
                                lseek(fd, 0, SEEK_SET);
                                lseek(fd_books,0,SEEK_SET);
                                lseek(fd_user,0,SEEK_SET);
                                if (ch == 1){
                                    
                                    borrowBook(fd,l,fd_books,new_fd,fd_user);
                                    
                                }
                                else if (ch == 2){
                                    returnBook(fd,l,fd_books,new_fd,fd_user);
                                }
                                else if(ch == 3){
                                    break;
                                }
                                else{
                                    continue;
                                }
                                // close(fd);
                                // close(fd_admin);
                                // close(fd_books);
                                // close(fd_user);

                                
                            }
                        }
                    }
                    else if(m == 2){
                        struct login l;
                        read(new_fd,&l,sizeof(l));
                        newUser(fd_user,new_fd,fd_admin,l);
                    }
                    else if(m == 3){
                        printf("Connection terminated\n");
                        break;
                    }
                    else{
                        

                    }
                }

            }
            else if (user == 2){
                int ch;
                struct login l ,temp;
                read(new_fd,&l,sizeof(l));
                printf("%s %s\n",l.username,l.password);
                lseek(fd_admin, 0, SEEK_SET);
                read(fd_admin,&temp,sizeof(temp));
                printf("%s %s\n",temp.username,temp.password);
                int ans=0;
                //printf("% %s\n",l.username,l.password);
                if((strcmp(temp.username,l.username) == 0) && (strcmp(temp.password,l.password) == 0)){
                    //printf("True\n");
                    ans=1;
                    write(new_fd,&ans,sizeof(ans));
                }
                else{
                    //printf("False\n");
                    write(new_fd,&ans,sizeof(ans));
                    break;
                }
                while (1){
                    fd_books = open("books.txt", O_RDWR | O_CREAT, 0777);
                    read(new_fd, &ch, sizeof(ch));

                    lseek(fd, 0, SEEK_SET);
                    lseek(fd_books, 0, SEEK_SET);
                    //lseek(fd_custs, 0, SEEK_SET);

                    if (ch == 1){
                        addBooks(fd_books,new_fd, fd_admin);
                    } 
                    else if (ch == 2){
                        deleteBook(fd_books, new_fd, fd_admin);
                    }
                    else if (ch == 3){
                        modifyBook(fd_books, new_fd, fd_admin);
                    }

                    else if (ch == 4){
                        searchBook(fd_books, new_fd, fd_admin);
                    }
                    else if (ch == 5){
                        //close(new_fd);
                        close(fd_admin);
                        break;
                    }
                    else{
                        continue;
                    }

                    close(fd_books);
                }

            }
            else if(user == 3){
                printf("Connection terminated\n");
                break;
            }
            else{
                continue;
            }
            
            }
        }else{
            close(new_fd);

        }
   
    }
    close(fd_books);
    close(fd_admin);
    close(fd);
    close(fd_user);
 return 0;
}