#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include "sqlite3.h"
#include "database.h"

// Define the possible states of the program
typedef enum {
    STATE_START_MENU,
    STATE_SHOWING_LIST_OF_WEBSITES,
    STATE_ADDING_WEBSITE,
    STATE_SEARCHING_WEBSITE_BY_ID,
    STATE_SEARCHING_WEBSITE_BY_NAME,
    STATE_WAITING_1,
    STATE_SHOWING_LIST_OF_ACCOUNTS,
    STATE_ADDING_ACCOUNT,
    STATE_REMOVING,
    STATE_REMOVING_WEBSITE,
    STATE_REMOVING_ACCOUNT,
    STATE_WAITING_2,
    STATE_QUIT_PROGRAM
} ProgramState;

void get_input(char* input) 
{
    fgets(input, 50, stdin);
    // Remove the '\n' character
    input[strlen(input) - 1] = '\0';
}

int waiting(const char *CURRENT_indentation) 
{
    printf("%s", CURRENT_indentation);
    printf("Press enter to continue... ");
    char input[50];
    get_input(input);
    if (strcmp(input, "quit") == 0) 
    {
        return 0;
    }
    return 1;
}

int main() 
{
    /* Opening the database */
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
    /*==================================================================*/

    bool clear_console = true;
    char input[50];
    char CURRENT_searched_website_name[50];
    CURRENT_searched_website_name[0] = '\0';
    int CURRENT_website_id;
    CURRENT_website_id = -1;
    char CURRENT_indentation[8]; 
    CURRENT_indentation[0] = '\0';

    ProgramState state = STATE_START_MENU;

    while (state != STATE_QUIT_PROGRAM) 
    {
        switch (state) 
        {
            case STATE_START_MENU:
                system("cls");
                printf("- To exit the program, you can always type: \033[1;31mquit\033[0m\n");
                printf("- To return to the start menu, type: \033[0;32mstart\033[0m\n");
                clear_console = false;
                state = STATE_SHOWING_LIST_OF_WEBSITES;
                break;
            case STATE_SHOWING_LIST_OF_WEBSITES:
                if (clear_console == true) {system("cls");} else {clear_console = true;}
                CURRENT_indentation[0] = '\0';

                printf("\n");
                if (CURRENT_searched_website_name[0] == '\0') 
                {
                    printf("Your websites:\n");
                } else 
                {
                    printf("Websites, searched by '%s':\n", CURRENT_searched_website_name);
                }
                show_list_of_websites(CURRENT_searched_website_name, db, zErrMsg, &rc);

                printf("To add a website type: \033[0;35madd\033[0m\n");
                printf("Search website by name, or enter website's id:\n");
                printf("[\033[0;35madd\033[0m/\033[0;32mwebsite_name\033[0m/\033[0;32mwebsite_id\033[0m]: ");

                get_input(input); 
                if (strcmp(input, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "start") == 0) 
                {
                    CURRENT_searched_website_name[0] = '\0';
                    state = STATE_START_MENU;
                } else if (strcmp(input, "add") == 0) 
                {
                    state = STATE_ADDING_WEBSITE;
                } else if (convert_to_id(input, &CURRENT_website_id) == 1) 
                {
                    // We already have the id value in the 'CURRENT_website_id' variable
                    state = STATE_SEARCHING_WEBSITE_BY_ID;
                } else 
                {
                    // Save the entered website name
                    strcpy(CURRENT_searched_website_name, input);
                    state = STATE_SEARCHING_WEBSITE_BY_NAME;
                }
                break;
            
            case STATE_ADDING_WEBSITE:
                strcpy(CURRENT_indentation, "  ");

                printf("\n");
                printf("  [\033[0;34mreturn\033[0m/\033[0;32mwebsite_name_to_add\033[0m]: ");

                get_input(input);
                if (strcmp(input, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_WEBSITES;
                } else if (add_website(input, db, zErrMsg, &rc) == 1) 
                {
                    printf("%s", CURRENT_indentation);
                    printf("Website '%s' has been successfully added to your list!\n\n", input);

                    CURRENT_website_id = get_website_id(input, db, zErrMsg, &rc);
                    state = STATE_WAITING_2;
                } else 
                {
                    printf("%s", CURRENT_indentation);
                    printf("Can't add '%s' website to your list. Maybe it's already on your list.\n\n", input);
                    state = STATE_WAITING_1;
                }
                break;
            
            case STATE_SEARCHING_WEBSITE_BY_ID:
                CURRENT_indentation[0] = '\0';
                
                if (check_if_website_exists(CURRENT_website_id, db, zErrMsg, &rc) == 1) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                } else 
                {
                    printf("No websites with id %d found.\n\n", CURRENT_website_id);
                    CURRENT_website_id = -1;
                    state = STATE_WAITING_1;
                }
                break;

            case STATE_SEARCHING_WEBSITE_BY_NAME:
                CURRENT_indentation[0] = '\0';

                int website_count = get_website_count(input, db, zErrMsg, &rc);

                if (website_count == 1) 
                {
                    CURRENT_website_id = get_website_id(input, db, zErrMsg, &rc);
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                } else 
                {
                    state = STATE_SHOWING_LIST_OF_WEBSITES;
                }
                break;
            
            case STATE_WAITING_1:
                if (waiting(CURRENT_indentation) == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else 
                {
                    state = STATE_SHOWING_LIST_OF_WEBSITES;
                }
                printf("\n");
                break;
            
            case STATE_SHOWING_LIST_OF_ACCOUNTS:
                system("cls");
                strcpy(CURRENT_indentation, "   ");

                printf("Website '\033[0;32m");
                print_current_website_name(CURRENT_website_id, db, zErrMsg, &rc);
                printf("\033[0m':\n");

                show_list_of_accounts(CURRENT_website_id, db, zErrMsg, &rc);

                printf("\n");

                printf("   To add an account type: \033[0;35madd\033[0m\n");
                printf("   To remove an account or current website type: \033[1;31mremove\033[0m\n");
                printf("   [\033[0;34mreturn\033[0m/\033[0;35madd\033[0m/\033[1;31mremove\033[0m]: ");

                get_input(input);
                if (strcmp(input, "quit") == 0) {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "start") == 0) 
                {
                    CURRENT_website_id = -1;
                    CURRENT_searched_website_name[0] = '\0';
                    state = STATE_START_MENU;
                } else if (strcmp(input, "return") == 0) 
                {
                    CURRENT_website_id = -1;
                    state = STATE_SHOWING_LIST_OF_WEBSITES;
                } else if (strcmp(input, "add") == 0) 
                {
                    state = STATE_ADDING_ACCOUNT;
                } else if (strcmp(input, "remove") == 0) 
                {
                    state = STATE_REMOVING;
                } else 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                }
                break;
            
            case STATE_ADDING_ACCOUNT:
                strcpy(CURRENT_indentation, "    ");

                printf("\n");
                printf("   To stop adding an account type: \033[0;34mreturn\033[0m\n");
                
                char username[50];
                printf("    [username]: ");
                get_input(username);
                if (strcmp(username, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                    break;
                } else if (strcmp(username, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                    break;
                }
                
                char email[50];
                printf("    [email]: ");
                get_input(email);
                if (strcmp(email, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                    break;
                } else if (strcmp(email, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                    break;
                }

                char password[50];
                printf("    [password]: ");
                get_input(password);
                if (strcmp(password, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                    break;
                } else if (strcmp(password, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                    break;
                }

                add_account(CURRENT_website_id, username, email, password, db, zErrMsg, &rc);
                state = STATE_WAITING_2;
                break;
            
            case STATE_REMOVING:
                strcpy(CURRENT_indentation, "    ");
                
                printf("\n");
                printf("   To stop removing an account type: \033[0;34mreturn\033[0m\n");
                printf("     Remove a single account(1), or a whole website(2)?\n");
                printf("     [\033[0;34mreturn\033[0m/1/2]: ");

                get_input(input);
                if (strcmp(input, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                } else if (strcmp(input, "1") == 0) 
                {
                    state = STATE_REMOVING_ACCOUNT;
                } else if (strcmp(input, "2") == 0) 
                {
                    state = STATE_REMOVING_WEBSITE;
                } else 
                {
                    printf("\n");
                    state = STATE_REMOVING;
                }
                break;
            
            case STATE_REMOVING_ACCOUNT:
                strcpy(CURRENT_indentation, "     ");

                printf("     Type the id of an account which you want to remove:\n");
                printf("     [\033[0;34mreturn\033[0m/\033[0;36mid\033[0m]: ");

                int account_id = -1;
                get_input(input);
                if (strcmp(input, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "return") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                } else if (convert_to_id(input, &account_id) == 1) 
                {
                    remove_account(account_id, CURRENT_website_id, db, zErrMsg, &rc);
                    state = STATE_WAITING_2;
                } else 
                {
                    state = STATE_REMOVING_ACCOUNT;
                }
                break;
            
            case STATE_REMOVING_WEBSITE:
                strcpy(CURRENT_indentation, "     ");

                printf("     Are you sure you wanna remove the website?\n");
                printf("     [\033[0;36myes\033[0m/\033[0;36mno\033[0m]: ");

                get_input(input);
                if (strcmp(input, "quit") == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else if (strcmp(input, "yes") == 0) 
                {
                    remove_website(CURRENT_website_id, db, zErrMsg, &rc);
                    CURRENT_website_id = -1;
                    state = STATE_WAITING_1;
                } else if (strcmp(input, "no") == 0) 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                } else 
                {
                    state = STATE_REMOVING_WEBSITE;
                }
                break;
            
            case STATE_WAITING_2:
                if (waiting(CURRENT_indentation) == 0) 
                {
                    state = STATE_QUIT_PROGRAM;
                } else 
                {
                    state = STATE_SHOWING_LIST_OF_ACCOUNTS;
                }
                printf("\n");
                break;

            default:
                printf("%s", CURRENT_indentation);
                printf("\033[0;31mUnknown state encountered. Exiting program.\033[0m\n");
                state = STATE_QUIT_PROGRAM;
                break;
        }
    }

    update_website_ids(db, zErrMsg, &rc);
    update_account_ids(db, zErrMsg, &rc);
    sqlite3_close(db);

    return 0;
}