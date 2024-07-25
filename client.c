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

void displayUserMenu(){
    printf("User Menu\n");
    printf("1) Borrow books\n");
    printf("2) Return books\n");
    printf("3) Exit");
    printf("\n");
}

void displayAdminMenu(){
    printf("Admin Menu\n");
    printf("1) Add new book\n");
    printf("2) Delete a book\n");
    printf("3) Modify a book\n");
    printf("4) Search for a book\n");
    printf("5) Exit\n");
} 

int authentication(int sockfd){
    char username[50];
    char password[50];
    struct login l ;
    printf("Enter username: ");
    scanf("%s",&username);
    //write(sockfd,&username,sizeof(username));
    printf("Enter password: ");
    scanf("%s",&password);
    strcpy(l.username,username);
    strcpy(l.password,password);
    printf("%s %s\n",l.username,l.password);
    write(sockfd,&l,sizeof(l));
    int ans;
    read(sockfd,&ans,sizeof(int));
    printf("%d",ans);
    return ans;
}


int main(){
    printf("Waiting for server to connect\n");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1){
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv;
    
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5555);

    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1){
        perror("Error: ");
        return -1;
    }

    int user;
    while(1){
    printf("Entering as (select option)\n");
    printf("1) User\n");
    printf("2) Admin\n");
    printf("3) Exit\n");
    scanf("%d",&user);
    write(sockfd, &user, sizeof(user));
    if(user == 1){
        while(1){
            printf("Entered as a user.\n");
            printf("1) Already registered\n");
            printf("2) New registration\n");
            printf("3) Exit\n");
            int ch ;
            scanf("%d",&ch);
            write(sockfd,&ch,sizeof(ch));
            if(ch == 1){
                int a = authentication(sockfd);
                if(a == 1){
                    printf("Successfully Login.");
                    while(1){
                        displayUserMenu();
                        int c;
                        scanf("%d",&c);
                        write(sockfd,&c,sizeof(c));
                        if(c == 1){
                            printf("To borrow a book.\n");
                            int quantity;
                            printf("Books in library available: \n");
                            read(sockfd,&quantity,sizeof(int));
                            struct book books[quantity+1];
                            read(sockfd,&books,sizeof(books));
                            int h = 0;
                            for(int i = 0 ; i < quantity ; i++){
                                if(books[i].del != -1 && (books[i].qty - books[i].borrow > 0)){
                                    printf("Id: %d , Name: %s,Author: %s,Descrp: %s\n",books[i].id,books[i].name,books[i].author,books[i].desc);
                                    h++;
                                }
                            }
                            write(sockfd, &h ,sizeof(int));
                            if(h == 0){
                                printf("No books available in library.Every book is borrowed.Please come back later\n");
                                continue;
                            }
                            printf("Please select from above which book you want to borrow by providing id ");
                            int id;
                            scanf("%d",&id);
                            write(sockfd,&id,sizeof(id));
                            int ans;
                            read(sockfd,&ans,sizeof(ans));
                            if(ans == 1){
                                printf("Book successfully borrowed.\n");
                            }
                            else if(ans == 3){
                                printf("Maximum borrowed limit overflowed.\n");
                            }
                            else{
                                printf("An error occured.\n");
                            }
                            
                        }
                        else if(c == 2){
                            printf("To return a book.\n");
                            int quantity;
                            printf("Books which you borrowed: \n");
                            read(sockfd,&quantity,sizeof(int));
                            if(quantity == 0){
                                printf("No books borrowed\n");
                                continue;
                            }
                            //struct book books[quantity+1];
                            int books[quantity];
                            read(sockfd,&books,sizeof(books));
                            printf("Book Ids:\n");
                            for(int i = 0 ; i < quantity ; i++){
                                printf("%d \n",books[i]);
                            }
                            printf("Please select from above which book you want to return by providing id ");
                            int id;
                            scanf("%d",&id);
                            write(sockfd,&id,sizeof(id));
                            int ans;
                            read(sockfd, &ans, sizeof(ans));
                            printf("Ans: %d\n",ans);
                            if(ans == 1){
                                printf("Book successfully returned.\n");
                            }
                            else if(ans == 2){
                                printf("Wrong bookId provided.\n");
                            }
                            else{
                                printf("An error occured.\n");
                            }


                        }
                        else if(c == 3){
                            printf("Exiting as a client.\n");
                            break;
                        }
                        else{
                            printf("Invalid choice. Please enter correct choice.\n");
                        }
                    }
                }
                else if(a == 2){
                    printf("Incorrect password.\n");
                }
                else{
                    printf("Incorrect details. Please try again later.\n");
                }
            }
            else if(ch == 2){
                struct login l;
                printf("Please enter new username: ");
                scanf("%s",&l.username);
                printf("Please enter password: ");
                scanf("%s",&l.password);
                write(sockfd,&l,sizeof(l));  
                int ans;
                read(sockfd,&ans,sizeof(ans));
                if(ans == 1){
                    printf("New User successfully added.\n");
                }   
                else if(ans == 2){
                    printf("User already exists.\n");
                }   
                else{
                    printf("There was an error.\n");
                }    
            }
            else if(ch == 3){
                printf("Exiting as user!\n");
                break;
            }
            else{
                printf("Please enter a correct option.\n");
            }
        }
    }
    else if(user == 2){
        printf("Entered as an admin\n");
        if(authentication(sockfd)){
            printf("Successfully Login.\n");
            while(1){
                displayAdminMenu();
                int c;
                scanf("%d",&c);
                write(sockfd,&c,sizeof(int));
                if(c == 1){
                    printf("Please provide details for the new book:\n");
                    struct book b;
                    char name[50];
                    char author[50];
                    char desc[100];
                    int quantity;
                    printf("Book name: ");
                    scanf("%s",&name);
                    printf("Book author: ");
                    scanf("%s",&author);
                    printf("Book description: ");
                    scanf("%s",&desc);
                    printf("Book quantity: ");
                    scanf("%d",&quantity);
                    strcpy(b.author,author);
                    strcpy(b.desc,desc);
                    strcpy(b.name,name);
                    b.qty = quantity;
                    write(sockfd,&b,sizeof(struct book));
                    int ans;
                    read(sockfd,&ans,sizeof(ans));
                    if(ans == 1){
                        printf("Successfully added new book.\n");
                    }
                    else if(ans == 2){
                        printf("Book already exist. So please go to modify option.\n");
                    }
                    else{
                        printf("Unsuccessful to add a new book.\n");
                    }
                    
                }
                else if(c == 2){
                    int quantity;
                    printf("Books in library: \n");
                    read(sockfd,&quantity,sizeof(int));
                    struct book books[quantity+1];
                    read(sockfd,&books,sizeof(books));
                    for(int i = 0 ; i < quantity ; i++){
                        if(books[i].del != -1){
                            printf("Id: %d , Name: %s\n",books[i].id,books[i].name);
                        }
                    }
                    printf("Please select from above which book id to delete ");
                    int id;
                    scanf("%d",&id);
                    write(sockfd,&id,sizeof(id));
                    int ans;
                    read(sockfd,&ans,sizeof(ans));
                    if(ans){
                        printf("Successfully deleted book with id %d.\n",id);
                    }
                    else{
                        printf("Unsuccessful or book does not exist.\n");
                    }

                }
                else if(c == 3){
                    int quantity;
                    printf("Books in library: \n");
                    read(sockfd,&quantity,sizeof(int));
                    struct book books[quantity+1];
                    read(sockfd,&books,sizeof(books));
                    for(int i = 0 ; i < quantity ; i++){
                        printf("Id: %d , Name: %s, Author: %s, Descrp: %s,Quantiy: %d,Deleted: %d\n",books[i].id,books[i].name,books[i].author,books[i].desc,books[i].qty,books[i].del);
                    }
                    printf("Please select from above which book id to modify ");
                    struct book b;
                    int id;
                    scanf("%d",&id);
                    b = books[id];
                    while(1){
                        printf("What you wish to modify: \n");
                        printf("1. Book Name\n");
                        printf("2. Book Author\n");
                        printf("3. Book Description\n");
                        printf("4. Book Quantity\n");
                        printf("5. Book Deleted\n");
                        printf("6. Exit\n");
                        int m ;
                        scanf("%d",&m);
                        if(m == 1){
                            printf("Modify book name:");
                            scanf("%s",&b.name);
                        }
                        else if(m ==2){
                            printf("Modify author name:");
                            scanf("%s",&b.author);
                        }
                        else if(m ==3){
                            printf("Modify desciption:");
                            scanf("%s",&b.desc);
                        }
                        else if(m ==4){
                            printf("Modify quantity:");
                            scanf("%d",&b.qty);
                        }
                        else if(m == 5){
                            printf("Modify deleted state(if non deleted then write 0 only):");
                            scanf("%s",&b.del);
                        }
                        else if(m ==6){
                            break;
                        }
                        else{
                            printf("Select correct option.\n");
                        }
                    }
                    printf("Modified details: \n");
                    printf("Id: %d , Name: %s, Author: %s, Descrp: %s,Quantiy: %d,Deleted: %d\n",b.id,b.name,b.author,b.desc,b.qty,b.del);
                    printf("To modify, write 1 or else any number");
                    int v ;
                    scanf("%d",&v);
                    if(v == 1){
                        write(sockfd,&b,sizeof(b));
                        int ans ;
                        read(sockfd,&ans,sizeof(ans));
                        if(ans){
                            printf("Successfully modified\n");
                        }
                        else{
                            printf("Modification unsuccessful\n");
                        }
                    }
                    else{
                        printf("Exit from modification\n");
                    }
                }
                else if(c == 4){
                    int quantity;
                    printf("Books in library: \n");
                    read(sockfd,&quantity,sizeof(int));
                    struct book books[quantity+1];
                    read(sockfd,&books,sizeof(books));
                    for(int i = 0 ; i < quantity ; i++){
                        //if(books[i].del != -1){
                            printf("Id: %d , Name: %s\n",books[i].id,books[i].name);
                        //}
                    }
                    printf("Please select from above which book id to search ");
                    int id;
                    scanf("%d",&id);
                    if(id < quantity && id >= 0){
                        struct book b = books[id];
                         printf("Id: %d , Name: %s, Author: %s, Descrp: %s,Quantiy: %d ,Books available in Library: %d\n",b.id,b.name,b.author,b.desc,b.qty,b.borrow);
                         if(b.del == -1){
                            printf("Book already removed from the library.");
                         }
                    }
                    else{
                        printf("Invalid Book id. Please try again later.");
                    }
                }
                else if(c == 5){
                    printf("Exiting as a admin.\n");
                    break;
                }
                else{
                    printf("Invalid choice. Please enter correct choice\n");
                }
            }
        }
        else{
            printf("Incorrect details. Please try again later.");
        }
    }
    else if(user == 3){
        printf("Exiting from client side.\n");
        break;
    }
    else{
        printf("Wrong option selected.");
    }
    }

    printf("Connection ended successfully\n");
    return 0;
}