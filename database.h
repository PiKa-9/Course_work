#ifndef _DATABASE_H
#define _DATABASE_H

#include "sqlite3.h"

/* 'callback' functions are called after every returned row from the database table */

int count_rows_callback(void *rows_count, int column_count, char **data, char **columns);
// Returns how many rows are returned after executing sql statement in 'sql_msg'
int count_rows(char *sql_msg, sqlite3 *db, char *zErrMsg, int *rc);


// Prints the website's id and name
//   data[0] - website's id;
//   data[1] - website's name;
int show_website_callback(void *rows_count, int column_count, char **data, char **columns);
// Shows first 30 user's websites containing the searched name
//   returns a number of showed websites
int search_website_by_name(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);


// Prints the website name, then each account's username, email and password
//   data[0] - website's name;
//   data[1] - account's username;
//   data[2] - account's email;
//   data[3] - account's password;
//   data[4] - account's id;
int show_account_callback(void *rows_count, int column_count, char **data, char **columns);
// Show all accounts from a specific website 
int open_website_by_id(int website_id, sqlite3 *db, char *zErrMsg, int *rc);


// Prints the website's id and name
//   data[0] - website's id;
//   data[1] - website's name;
int show_website_callback(void *rows_count, int column_count, char **data, char **columns);
// Shows 10 websites, starting from website with id 'current_id'
int show_websites(int current_id, sqlite3 *db, char *zErrMsg, int *rc);


// Writes the website's id to 'id' variable
int get_website_id_callback(void *id, int column_count, char **data, char **columns);
// Searches websites by website_name
//   if exactly one website was found, then opens the website
// Returns: 
//          -1, if no websites were found
//          -2, if >=2 websites were found
//          website's_id, if exactly one website was found
int search_website_by_name(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);


void add_website(char *website_name, sqlite3 *db, char *zErrMsg, int *rc);

void add_account(int website_id, char *username, char *email, char *password, sqlite3 *db, char *zErrMsg, int *rc);

void remove_account(int account_id, sqlite3 *db, char *zErrMsg, int *rc);

void remove_website(int website_id, sqlite3 *db, char *zErrMsg, int *rc);

#endif // DATABASE_H