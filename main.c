#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include "sqlite3.h"
#include "database.h"
#include "my_utils.h"

void get_input(char* input) 
{
    fgets(input, 50, stdin);
    // Remove the '\n' character
    input[strlen(input) - 1] = '\0';
}

int ask_id(const char *msg)
{
    int id;
    bool incorrect_input = true;
    char input[5];

    while (incorrect_input) 
    {
        printf("%s", msg);
        fgets(input, 5, stdin);
        input[strlen(input) - 1] = '\0';

        if (try_convert_to_id(input, &id)) {incorrect_input = false;}
    }

    return id;
}

void waiting() 
{
    printf("Press enter to continue... ");
    char buffer[50];
    fgets(buffer, 50, stdin);
}

int main() 
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("AccountDB.db", &db);
    if (rc) 
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("- To exit the program, type: \033[1;31mquit\033[0m\n");
    printf("- To return to the start menu, type: \033[0;32mstart\033[0m\n");
    printf("\n");
    
    bool running_program = true;
    while (running_program) 
    {
        /* Shows users first 10 websites */
        printf("Your websites: \n");
        int row_count = show_websites(0, db, zErrMsg, &rc);
        waiting();

        printf("To add a website type: \033[0;35madd\033[0m\n");
        printf("Search website by name, or enter website's id:\n");
        
        bool searching_website = true;
        while (searching_website) 
        {
            printf("[add/website_name/website_id]: ");
            char input[50];
            get_input(input);
            printf("\n");

            int website_id = -1;
            bool website_is_opened = false;
            if (!strcmp(input, "quit")) 
            {
                searching_website = false;
                running_program = false;
                break;
            } else if (!strcmp(input, "start")) 
            {
                searching_website = false;
                break;
            } else if (try_convert_to_id(input, &website_id)) 
            {
                /* Open website(view accounts from the website) by its id */
                int website_exists = open_website_by_id(website_id, db, zErrMsg, &rc);
                if (website_exists == 0) 
                {
                    waiting();
                    continue;
                } 
                else 
                {
                    website_is_opened = true;
                }
            } else if (!strcmp(input, "add")) 
            {
                /* Add a website */
                printf("Enter the name of the website, which you want to add\n");
                printf("[website_name]: ");
                char website_name[50];
                get_input(website_name);  

                add_website(website_name, db, zErrMsg, &rc);
                if (rc == SQLITE_OK) 
                {
                    printf("Website '%s' has been succesfully added to your list!\n\n", 
                            website_name);
                    // Open the new website 
                    website_id = search_website_by_name(website_name, db, zErrMsg, &rc);
                    website_is_opened = true;
                } else 
                {
                    printf("Can't add this website, maybe it is already on your list.\n\n");
                }
                continue;
            } else 
            {
                /* Search the website by it's name */

                // If exactly 1 website was found by the searched name, 
                //   then open the website,
                //   otherwise continue asking website_name/website_id.
                website_id = search_website_by_name(input, db, zErrMsg, &rc);
                if (website_id >= 0) 
                {
                    website_is_opened = true;
                } else 
                {
                    waiting();
                    continue;
                }
            }


            waiting();
            while (website_is_opened) 
            {
                // User has opened a website accounts.
                /* Add an account */
                // OR
                /* Remove the website or account */
                printf("To add an account type: \033[0;35madd\033[0m\n");
                printf("To remove accounts(or website) type: \033[0;35mremove\033[0m\n");
                printf("[add/remove]: ");
                char choice[50];
                get_input(choice);
                
                if (!strcmp(choice, "quit")) 
                {
                    website_is_opened = false;
                    searching_website = false;
                    running_program = false;
                    break;
                } else if (!strcmp(choice, "start")) 
                {
                    website_is_opened = false;
                    searching_website = false;
                    break;
                } else if (!strcmp(choice, "add")) 
                {
                    char username[50];
                    char email[50];
                    char password[50];
                    printf("  [username]: ");
                    get_input(username);
                    printf("  [email]: ");
                    get_input(email);
                    printf("  [password]: ");
                    get_input(password);

                    add_account(website_id, username, email, password, db, zErrMsg, &rc);
                    if (rc == SQLITE_OK) 
                    {
                        printf("Account has been succesfully added to your list!\n\n");
                    } else 
                    {
                        printf("Can't add this account, maybe it is already on your list.\n\n");
                    }
                    waiting();
                    open_website_by_id(website_id, db, zErrMsg, &rc);            
                    continue;
                } else if (!strcmp(choice, "remove")) 
                {
                    int account_id = -1;
                    char input_id[50];
                    printf("[account_id]: ");
                    get_input(input_id);

                    if (!strcmp(choice, "quit")) 
                    {
                        website_is_opened = false;
                        searching_website = false;
                        running_program = false;
                        break;
                    } else if (!strcmp(choice, "start")) 
                    {
                        website_is_opened = false;
                        searching_website = false;
                        break;
                    } else if (try_convert_to_id(input_id, &account_id)) 
                    {
                        remove_account(account_id, db, zErrMsg, &rc);
                        if (rc == SQLITE_OK) 
                        {
                            printf("Account has been succesfully removed!\n\n");
                        } else 
                        {
                            printf("Can't remove account with such id.\n\n");
                        }
                        waiting();
                        open_website_by_id(website_id, db, zErrMsg, &rc);                          
                    } else 
                    {
                        continue;
                    }
                    continue;
                } else 
                {
                    continue;// TODO
                }
            }
        }
        
    }

    sqlite3_close(db);
    return 0;
}