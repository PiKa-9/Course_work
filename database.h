#ifndef _DATABASE_H
#define _DATABASE_H

#include "sqlite3.h"

bool convert_to_id(const char *string, int *result);

/* 'callback' functions are called after every returned row from the database table */

// User searches the website by its name -> show first 10 found websites to the user
int show_website_callback(void *rows_count, int column_count, char **data, char **columns);
void show_list_of_websites(char *searched_website_name, sqlite3 *db, char *zErrMsg, int *rc);

// We need to get the website_id, so it's easier to access it in the database
int get_website_id_callback(void *id, int column_count, char **data, char **columns);
int get_website_id(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);

// When we open website, firstly we print the website's name, before showing accounts from this website
int print_current_website_name_callback(void *NotUsed, int column_count, char **data, char **columns);
void print_current_website_name(const int website_id, sqlite3 *db, char *zErrMsg, int *rc);

// User enters id -> Check if there is a website with such id
int check_if_website_exists_callback(void *website_exists, int column_count, char **data, char **columns);
int check_if_website_exists(int website_id, sqlite3 *db, char *zErrMsg, int *rc);

// User searched websites by name -> we need to count how many websites we've got, 
//   so if exactly 1 website was found, then open that website straightaway (show accounts).
int get_website_count_callback(void *count, int column_count, char **data, char **columns);
int get_website_count(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);

// Show the account info for each account from currently opened website
int show_account_callback(void *account_count, int column_count, char **data, char **columns);
void show_list_of_accounts(int website_id, sqlite3 *db, char *zErrMsg, int *rc);

int add_website(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);

void add_account(int website_id, char *username, char *email, char *password, sqlite3 *db, char *zErrMsg, int *rc);

// User entered id of the account, which he wants to remove -> we need to check if account with such id exists
int check_if_account_exists_callback(void *account_exists, int column_count, char **data, char **columns);
int check_if_account_exists(const int account_id, const int website_id, sqlite3 *db, char *zErrMsg, int *rc);
void remove_account(const int account_id, const int website_id, sqlite3 *db, char *zErrMsg, int *rc);

void remove_website(int website_id, sqlite3 *db, char *zErrMsg, int *rc);

// If some website/account was removed, then the ids can be not from 1 to n,
//   so we update ids to make them from 1 to n, 
//   since when we print it's better to see numbers from 1 to n (not some random ids).
void update_website_ids(sqlite3 *db, char *zErrMsg, int *rc);
void update_account_ids(sqlite3 *db, char *zErrMsg, int *rc);

#endif // DATABASE_H