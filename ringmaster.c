/* What we are doing is basically separating the data by reading it word by word to process later
 * First we separated it into sequences so that each sequence will have at most one "if" and each sequence will begin with an action
 * Then we seperated those sequences into action sequences and condition sequences
 * Action sequence consists of possibly multiple actions that will be processed together depending on its condition sequence
 * Questions are handled separately only by reading data word by word
 * The data will be stored in a People array which stores all the Person instances
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structs.h"

#define INITIAL_ARRAY_SIZE 10
bool hasDuplicates(char **strArray, int size);
struct Person *findPerson (struct Person ***people, char *name, int *people_count, int *array_size);




bool checkConditionSequence(struct Condition_Sequence *sequence, struct Person ***people, int *people_count, int *people_array_size);
bool primitiveCondition(struct Person *person, char *mode, char* object, int count);

void processActionSequence(struct Action_Sequence sequence, struct Person ***people, int *people_count, int *people_array_size);
void processAction(struct Action action, struct Person ***people, int *people_count, int *people_array_size);
void primitiveAction(struct Person *person, char *mode, int num, char *object);



void createPerson(struct Person ***people, char *name, int *people_count, int *array_size);
void addItem(struct Person *person, char *item_name, int amount);
void who_at(struct Person **people, char *location, int people_count);
int getItemNumber(struct Person *person, char *item_name);
int getItemIndex(struct Person *person, char *item_name);

int getNum(char *word);
struct Action *initializeAction();
struct Condition *initializeCondition();
void actionAddSubject(struct Action *action, char *subject);
void conditionAddSubject(struct Condition *condition, char *subject);
void actionAddObject(struct Action *action, char *object, int amount);
void conditionAddObject(struct Condition *condition, char *object, int amount);
struct Action_Sequence *initializeActionSequence();
struct Condition_Sequence *initializeConditionSequence();
void sequenceAddAction(struct Action_Sequence *sequence, struct Action action);
void sequenceAddCondition(struct Condition_Sequence *sequence, struct Condition condition);
void addActionSequence(struct Action_Sequence ***array, struct Action_Sequence *sequence, int *array_size, int *num_elements);
void addConditionSequence(struct Condition_Sequence ***array, struct Condition_Sequence *sequence, int *array_size, int *num_elements);
bool checkFormat(char *word);

void freePerson(struct Person *person);
void freeAction(struct Action *action);
void freeCondition(struct Condition *condition);
void freeActionSequence(struct Action_Sequence *sequence);
void freeConditionSequence(struct Condition_Sequence *sequence);


int main(){
    char input[1025];
    int people_count = 0; // The total number of Person instances that we stored
    int people_array_size = INITIAL_ARRAY_SIZE; // Size of the array Person instances are stored in
    // Allocate the array that we will store our location and items data
    struct Person **people = calloc(people_array_size, sizeof (struct Person*));
    while(1){
        // Take input
        printf("%s",">> ");
        fflush(stdout);
        fgets(input,1025,stdin);

        // "/n" as an input is invalid
        if (strcmp(strtok(strdup(input), " "), "\n") == 0){
            printf("INVALID\n");
            fflush(stdout);
            continue;
        }
        // Trimming the new line at the end
        for (int i = 0; input[i] != '\0'; i++) {
            if (input[i] == '\n') {
                input[i] = '\0';
                break;  // Exit the loop once newline character is found
            }
        }
        // exit the whole process
        if (strcmp(strtok(strdup(input), " "), "exit") == 0 && strtok(NULL, " ") == NULL){
            break;
        }
        // Question statements
        if (strchr(input, '?') != NULL){ // If it has a "?" it is a question
            bool invalid = false;
            // we used word variable to represent current word we are processing
            char *word = strtok(input, " ");

            // If the question is who at the first word should be who
            if (strcmp(word, "who") == 0){
                word = strtok(NULL, " "); // take the next word
                if (strcmp(word, "at") != 0){ // next word should be at
                    invalid = true;
                    printf("%s\n", "INVALID");
                    fflush(stdout);
                    continue;
                }
                // After at there should be a location
                word = strtok(NULL, " ");

                if (!checkFormat(word)){ // check whether location is valid or not
                    invalid = true;
                    printf("%s\n", "INVALID");
                    fflush(stdout);
                    continue;
                }
                // If it is valid process it
                who_at(people, word, people_count);
            }
            else{ // The questions beside who at
                // there may be multiple subjects
                int subject_count = 0; // The number of subjects
                int subj_array_size = INITIAL_ARRAY_SIZE; //
                char **subjects = calloc(subj_array_size, sizeof(char*));
                // If the first word is not who then it should be a subject
                if (!checkFormat(word)){ // whether subject is valid or not
                    invalid = true;
                    printf("%s\n", "INVALID");
                    fflush(stdout);
                    free(subjects); // free the allocated memory
                    continue;
                }

                subjects[0] = strdup(word);// If subject is valid add it to subjects array
                subject_count++;
                word = strtok(NULL, " "); // Take the second word

                if (strcmp(word, "and") == 0){ // If there are more than one subjects the question must be in "subjects total item ?" format
                    while(true){ // Take all the subjects in the while loop
                        word = strtok(NULL, " "); // next subject
                        if (!checkFormat(word)){
                            invalid = true;
                            break;
                        }
                        subjects[subject_count] = strdup(word);
                        subject_count++;
                        // if there are not enough space in the array reallocate it
                        if (subject_count == subj_array_size){
                            subj_array_size *= 2;
                            subjects = realloc(subjects, subj_array_size * sizeof(char*));
                        }
                        // More than one subject is only seen total object ? question
                        word = strtok(NULL, " ");
                        if (strcmp(word, "total") == 0){ // If the word is total
                            word = strtok(NULL, " "); // next word should be the object
                            break; // Get out of the while loop for finding subjects
                        }
                    }
                    if(!checkFormat(word)){ // If the object is not valid
                        invalid = true;
                    }
                    if (invalid){
                        printf("%s\n", "INVALID");
                        fflush(stdout);
                        // free the allocated memory
                        for (int i = 0; i < subject_count; ++i) {
                            free(subjects[i]);
                        }
                        free(subjects);
                        continue;
                    }

                    int total = 0; // the number represents total
                    for (int i = 0; i < subject_count; ++i) { // For each subject
                        // find the subject and add its item number to total
                        total += getItemNumber(findPerson(&people, subjects[i], &people_count, &people_array_size), word);
                    }
                    word = strtok(NULL, " "); // "?"
                    if(strcmp(word, "?") != 0){ // no multiple items
                        printf("%s\n", "INVALID");
                        fflush(stdout);
                        // free the allocated memory
                        for (int i = 0; i < subject_count; ++i) {
                            free(subjects[i]);
                        }
                        free(subjects);
                        continue;
                    }
                    word = strtok(NULL, " ");
                    if(word != NULL){ // no word must come after "?"
                        printf("%s\n", "INVALID");
                        fflush(stdout);
                        // free the allocated memory
                        for (int i = 0; i < subject_count; ++i) {
                            free(subjects[i]);
                        }
                        free(subjects);
                        continue;
                    }
                    printf("%d\n", total); // print out the answer
                    fflush(stdout);
                }
                else if (strcmp(word, "where") == 0){ // If the question is in "subject where ?" format
                    // first find the person
                    struct Person *subject = findPerson(&people, subjects[0], &people_count, &people_array_size);
                    word = strtok(NULL, " "); // "?"
                    if(strcmp(word, "?") != 0){
                        printf("%s\n", "INVALID");
                        fflush(stdout);
                        // free the allocated memory
                        for (int i = 0; i < subject_count; ++i) {
                            free(subjects[i]);
                        }
                        free(subjects);
                        continue;
                    }
                    word = strtok(NULL, " ");
                    if(word != NULL){ // no word must come after "?"
                        printf("%s\n", "INVALID");
                        fflush(stdout);
                        // free the allocated memory
                        for (int i = 0; i < subject_count; ++i) {
                            free(subjects[i]);
                        }
                        free(subjects);
                        continue;
                    }
                    printf("%s\n", subject->location); // then print its location
                    fflush(stdout);
                }
                    // Multiple subjects already handled so the question must be in "subject total ((optional) item) ?" format
                else if (strcmp(word, "total") == 0){
                    struct Person *subject = findPerson(&people, subjects[0], &people_count, &people_array_size);
                    word = strtok(NULL, " "); // "?"
                    if(strcmp(word, "?") == 0){ // If there is no next word print out all the inventory of the subject
                        word = strtok(NULL, " ");
                        if(word != NULL){ // no word must come after "?"
                            printf("%s\n", "INVALID");
                            fflush(stdout);
                            // free the allocated memory
                            for (int i = 0; i < subject_count; ++i) {
                                free(subjects[i]);
                            }
                            free(subjects);
                            continue;
                        }
                        int grand_total = 0; // total number of objects in the inventory
                        for (int i = 0; i < subject->item_count; ++i) { // For each item
                            int amount = subject->amounts[i]; // amount of a specific item
                            if (amount == 0){ // If the amount is 0  continue
                                continue;
                            }
                            if (grand_total > 0){ // if grand total is nonzero then print "and" before the item
                                printf(" and %d ", amount);
                                printf("%s", subject->items[i]);
                                fflush(stdout);
                                grand_total += amount;
                            }
                            else{ // if the grant total is 0 then it is the first item
                                printf("%d ", amount);
                                printf("%s", subject->items[i]);
                                fflush(stdout);
                                grand_total += amount;
                            }
                        }
                        if (grand_total == 0){ // if grand total is 0 then no item in inventory
                            printf("%s", "NOTHING");
                            fflush(stdout);
                        }
                        printf("%s\n", "");
                        fflush(stdout);
                    }
                    else{   // word is item (subject total item)
                        if (!checkFormat(word)){ // check whether object is valid or not
                            invalid = true;
                            printf("%s\n", "INVALID");
                            fflush(stdout);
                            continue;
                        }
                        char *object = strdup(word); // take the object
                        word = strtok(NULL, " ");
                        if (strcmp(word, "?") != 0){ // "?"
                            invalid = true;
                            printf("%s\n", "INVALID");
                            fflush(stdout);
                            continue;
                        }
                        word = strtok(NULL, " ");
                        if (word != NULL){ // There should be nothing after "?"
                            invalid = true;
                            printf("%s\n", "INVALID");
                            fflush(stdout);
                            continue;
                        }
                        printf("%d\n", getItemNumber(subject, object));
                        fflush(stdout);


                    }

                }
                    // If the word is none of them then it is invalid
                else{
                    invalid = true;
                    printf("%s\n", "INVALID");
                    fflush(stdout);
                    // free the allocated memory
                    free(subjects[0]);
                    free(subjects);
                    continue;
                }

                // free the allocated memory
                for (int i = 0; i < subject_count; ++i) {
                    free(subjects[i]);
                }
                free(subjects);
            }

        }

            // Action statements
        else{
            bool invalid = false;
            int action_array_size = INITIAL_ARRAY_SIZE; // size of the array that stores action sequences
            int condition_array_size = INITIAL_ARRAY_SIZE; // size of the array that stores condition sequences
            int action_sequence_count = 0; // Total number of action sequences
            int condition_sequence_count = 0; // Total number of condition sequences

            struct Action_Sequence **action_sequence_list; // action_sequence_list is a pointer to an array of action sequences
            struct Condition_Sequence **condition_sequence_list; // condition_sequence_list is a pointer to an array of condition sequences
            action_sequence_list = calloc(action_array_size, sizeof(struct Action_Sequence*));
            condition_sequence_list = calloc(condition_array_size, sizeof(struct Condition_Sequence*));

            char *word;
            word = strtok(input, " "); // the first word of the sentence which is a subject
            struct Action *action = initializeAction(); // current instances to store data
            struct Condition *condition = initializeCondition();

            struct Action_Sequence *action_sequence = initializeActionSequence();
            struct Condition_Sequence *condition_sequence = initializeConditionSequence();
            // Each time loop begins with a subject of an action
            while (!invalid && word != NULL ){
                if(!checkFormat(word)){ // If subject is invalid
                    invalid = true;
                    break;
                }
                actionAddSubject(action,word); // If subject is valid add it to the action
                word = strtok(NULL," "); // take the next word after the subject
                if(word == NULL){ // if there is no next word then sentence is invalid
                    invalid = true;
                    break;
                }
                if (strcmp(word,"and") == 0){ // If next word is "and" then there is another subject so continue
                    word = strtok(NULL," ");
                    if (word == NULL){
                        invalid = true;
                        break;
                    }
                    continue;
                }


                action_keywords: // at this point we have stored all the subjects of an action and will continue from keywords


                if (strcmp(word, "go") == 0){ // If the keyword is "go" we will iterate "subject go to location" operation
                    action->mode = "go to";
                    word = strtok(NULL," ");
                    if (word == NULL){
                        invalid = true;
                        break;
                    }
                    if (strcmp(word,"to") != 0){ // Read to
                        invalid = true;
                        break;
                    }
                    word = strtok(NULL," "); // This will be the location
                    if (word == NULL){
                        invalid = true;
                        break;
                    }
                    if(!checkFormat(word)){
                        invalid = true;
                        break;
                    }
                    actionAddObject(action,word,1); // Add the location
                    word = strtok(NULL," "); // Read the next word which should be either "and" or "if"
                    if (word == NULL){ // Terminate
                        sequenceAddAction(action_sequence,*action); // add action to sequence
                        action = initializeAction();
                        // add current action sequence to action_sequence_list
                        addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                        action_sequence = initializeActionSequence();
                        break;
                    }
                    else if (strcmp(word,"and") == 0){ // Next Action
                        sequenceAddAction(action_sequence,*action); // add action to sequence
                        action = initializeAction(); // create new action
                        word = strtok(NULL," "); // read the subject of the new action
                        if (word == NULL){ // If there is no subject
                            invalid = true;
                            break;
                        }
                        continue; // each loop will begin when word = subject of an action so continue to loop
                    }
                    else if (strcmp(word,"if") == 0){ /// Condition
                        sequenceAddAction(action_sequence,*action); // add action to sequence
                        action = initializeAction(); // create a new action
                        // Add action sequence to action_sequence_list
                        addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                        action_sequence = initializeActionSequence();
                        /// Continue with conditional statement
                        word = strtok(NULL, " ");
                        if (word == NULL){ // If there is no word after "if" it is invalid
                            invalid = true;
                        }
                        //Each while loop will begin with a subject of a conditional statement
                        while (word != NULL){ // Current word is the subject of a conditional statement
                            if(!checkFormat(word)){
                                invalid = true;
                                break;
                            }
                            conditionAddSubject(condition, word);// If subject is valid add it to condition
                            word = strtok(NULL," "); // Take the next word which is keyword
                            if(word == NULL){
                                invalid = true;
                                break;
                            }
                            // If keyword is "and" then there are more subjects to process so continue from the beginning of the while loop
                            if (strcmp("and",word) == 0){
                                word = strtok(NULL," ");
                                continue;
                            }

                                // if the keyword is "at" then we will question its location
                            else if (strcmp("at",word) == 0){
                                // Format will be "subject(s) at location"
                                condition->mode = "at";
                                word = strtok(NULL," "); // read the next word which is location
                                if(word == NULL){
                                    invalid = true;
                                    break;
                                }
                                if(!checkFormat(word)){
                                    invalid = true;
                                    break;
                                }
                                conditionAddObject(condition,word,1); // add location to condition
                                word = strtok(NULL," ");
                                if (word == NULL){ // Terminate
                                    sequenceAddCondition(condition_sequence, *condition); // add condition to sequence
                                    condition = initializeCondition();
                                    addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                    condition_sequence = initializeConditionSequence();
                                    break;
                                }
                                else if (strcmp(word,"and") == 0){ //
                                    sequenceAddCondition(condition_sequence,*condition); // add condition to sequence
                                    condition = initializeCondition();
                                    word = strtok(NULL," ");
                                    if (word == NULL){
                                        invalid = true;
                                    }
                                    continue;
                                }
                            }

                                // If the next word is "has" then there are 3 possibilities:
                                // has, has more than, has less than
                            else if (strcmp(word,"has") == 0){
                                condition->mode = "has";
                                word = strtok(NULL, " "); // read the word after "has"
                                if(word == NULL){
                                    invalid = true;
                                    break;
                                }
                                if (strcmp(word, "more") == 0){ // if the word is "more"
                                    condition->mode = "has more";
                                    word = strtok(NULL," ");
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if (strcmp(word,"than") != 0){ // after "more" there should be "than"
                                        invalid = true;
                                        break;
                                    }
                                    word = strtok(NULL," "); // read the next word which is the amount of object
                                }
                                else if (strcmp(word, "less") == 0){ // if the word is "less"
                                    condition->mode = "has less";
                                    word = strtok(NULL," ");
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if (strcmp(word,"than") != 0){
                                        invalid = true;
                                        break;
                                    }
                                    word = strtok(NULL," ");// read the next word which is the amount of object
                                }

                                // Our current word is supposed to be the amount of first object
                                if (word == NULL){
                                    invalid = true;
                                }
                                int amount = getNum(word); // getNum is a method that returns -1 if the number is invalid
                                if (amount != -1){ // If the number is valid
                                    while(amount != -1){
                                        word = strtok(NULL," "); // read the next word which is the item
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if(!checkFormat(word)){
                                            invalid = true;
                                            break;
                                        }
                                        conditionAddObject(condition,word,amount); // add item as an object
                                        word = strtok(NULL," "); // read the next word
                                        if (word == NULL){ // terminate
                                            sequenceAddCondition(condition_sequence, *condition);
                                            condition = initializeCondition();
                                            addConditionSequence(&condition_sequence_list,condition_sequence, &condition_array_size, &condition_sequence_count);
                                            condition_sequence = initializeConditionSequence();
                                            break;
                                        }
                                            // After the "and" there may be next item or next sentence
                                            // If the word is a number than "amount != -1" so code will continue to store objects
                                            // If the word is not a number then loop won't continue to take objects
                                        else if(strcmp(word,"and") == 0){
                                            word = strtok(NULL," ");
                                            if (word == NULL){
                                                invalid = true;
                                            }
                                            amount = getNum(word);
                                            continue;
                                        }
                                    }
                                    if (word != NULL){ // after taking the last object add condition to the sequence
                                        sequenceAddCondition(condition_sequence, *condition);
                                        condition = initializeCondition();
                                    }
                                }
                                else{ // If it is invalid
                                    invalid = true;
                                }
                            }

                                // Since after an "and" we may have an action sentence we have to iterate it
                                // In that case our current condition subjects are actually action subjects and our current word is an action keyword
                                // That is why we have to go back action keywords
                            else if (strcmp(word, "go") == 0 || strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0){

                                // However we have to be careful about "action sequence if action sequence" case
                                if (condition_sequence->condition_count == 0){
                                    invalid = true;
                                    break;
                                }
                                // our condition subjects were actually the subjects of the next action
                                for (int i = 0; i < condition->num_of_subjects; ++i) {
                                    action->subjects[i] = strdup(condition->subjects[i]);
                                }
                                action->num_of_subjects = condition->num_of_subjects;
                                action->subj_array_size = condition->subj_array_size;
                                condition = initializeCondition(); // reset the condition
                                // add condition to the condition sequence
                                addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                condition_sequence = initializeConditionSequence();
                                goto action_keywords;
                            }
                            else{ // the keyword is invalid
                                invalid = true;
                                break;
                            }
                        }

                    }
                    else{
                        invalid = true;
                        break;
                    }


                }
                else if (strcmp(word, "sell") == 0){
                    action->mode = "sell"; // set the mode
                    word = strtok(NULL," "); // the word is the amount
                    int amount = getNum(word);
                    if (amount == -1){ // The amount is invalid
                        invalid = true;
                        break;
                    }
                    while (getNum(word) != -1){ // while there is a valid amount
                        word = strtok(NULL," "); // this is item
                        if (word == NULL){
                            invalid = true;
                            break;
                        }
                        if(!checkFormat(word)){ // check whether item is valid or not
                            invalid = true;
                            break;
                        }
                        actionAddObject(action,word,amount); // if item is valid add it as an object
                        // after adding item we should either terminate, add another item or continue with the next action, add a trader or process the condition
                        word = strtok(NULL," ");
                        if (word == NULL){ // Terminate
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction();
                            addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                            action_sequence = initializeActionSequence();
                            break;
                        }
                        else if(strcmp(word,"and") == 0){ // If the word is and
                            word = strtok(NULL," "); // take the next word after "and"
                            if (word == NULL){
                                invalid = true;
                                break;
                            }
                            amount = getNum(word);
                            if (amount == -1){ // if the next word is not a number you have a new action
                                sequenceAddAction(action_sequence,*action); // add action to sequence
                                action = initializeAction(); // reset action
                                break; // don't process amounts anymore
                            }
                            continue;
                        }
                        else if (strcmp(word,"if") == 0){ /// Condition
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction(); // create a new action
                            // Add action sequence to action_sequence_list
                            addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                            action_sequence = initializeActionSequence();
                            /// Continue with conditional statement
                            word = strtok(NULL, " ");
                            if (word == NULL){ // If there is no word after "if" it is invalid
                                invalid = true;
                            }
                            //Each while loop will begin with a subject of a conditional statement
                            while (word != NULL){ // Current word is the subject of a conditional statement
                                if(!checkFormat(word)){
                                    invalid = true;
                                    break;
                                }
                                conditionAddSubject(condition, word);// If subject is valid add it to condition
                                word = strtok(NULL," "); // Take the next word which is keyword
                                if(word == NULL){
                                    invalid = true;
                                    break;
                                }
                                // If keyword is "and" then there are more subjects to process so continue from the beginning of the while loop
                                if (strcmp("and",word) == 0){
                                    word = strtok(NULL," ");
                                    continue;
                                }

                                    // if the keyword is "at" then we will question its location
                                else if (strcmp("at",word) == 0){
                                    // Format will be "subject(s) at location"
                                    condition->mode = "at";
                                    word = strtok(NULL," "); // read the next word which is location
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if(!checkFormat(word)){
                                        invalid = true;
                                        break;
                                    }
                                    conditionAddObject(condition,word,1); // add location to condition
                                    word = strtok(NULL," ");
                                    if (word == NULL){ // Terminate
                                        sequenceAddCondition(condition_sequence, *condition); // add condition to sequence
                                        condition = initializeCondition();
                                        addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                        condition_sequence = initializeConditionSequence();
                                        break;
                                    }
                                    else if (strcmp(word,"and") == 0){ //
                                        sequenceAddCondition(condition_sequence,*condition); // add condition to sequence
                                        condition = initializeCondition();
                                        word = strtok(NULL," ");
                                        if (word == NULL){
                                            invalid = true;
                                        }
                                        continue;
                                    }
                                }

                                    // If the next word is "has" then there are 3 possibilities:
                                    // has, has more than, has less than
                                else if (strcmp(word,"has") == 0){
                                    condition->mode = "has";
                                    word = strtok(NULL, " "); // read the word after "has"
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if (strcmp(word, "more") == 0){ // if the word is "more"
                                        condition->mode = "has more";
                                        word = strtok(NULL," ");
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word,"than") != 0){ // after "more" there should be "than"
                                            invalid = true;
                                            break;
                                        }
                                        word = strtok(NULL," "); // read the next word which is the amount of object
                                    }
                                    else if (strcmp(word, "less") == 0){ // if the word is "less"
                                        condition->mode = "has less";
                                        word = strtok(NULL," ");
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word,"than") != 0){
                                            invalid = true;
                                            break;
                                        }
                                        word = strtok(NULL," ");// read the next word which is the amount of object
                                    }

                                    // Our current word is supposed to be the amount of first object
                                    if (word == NULL){
                                        invalid = true;
                                    }
                                    int amount = getNum(word); // getNum is a method that returns -1 if the number is invalid
                                    if (amount != -1){ // If the number is valid
                                        while(amount != -1){
                                            word = strtok(NULL," "); // read the next word which is the item
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if(!checkFormat(word)){
                                                invalid = true;
                                                break;
                                            }
                                            conditionAddObject(condition,word,amount); // add item as an object
                                            word = strtok(NULL," "); // read the next word
                                            if (word == NULL){ // terminate
                                                sequenceAddCondition(condition_sequence, *condition);
                                                condition = initializeCondition();
                                                addConditionSequence(&condition_sequence_list,condition_sequence, &condition_array_size, &condition_sequence_count);
                                                condition_sequence = initializeConditionSequence();
                                                break;
                                            }
                                                // After the "and" there may be next item or next sentence
                                                // If the word is a number than "amount != -1" so code will continue to store objects
                                                // If the word is not a number then loop won't continue to take objects
                                            else if(strcmp(word,"and") == 0){
                                                word = strtok(NULL," ");
                                                if (word == NULL){
                                                    invalid = true;
                                                }
                                                amount = getNum(word);
                                                continue;
                                            }
                                        }
                                        if (word != NULL){ // after taking the last object add condition to the sequence
                                            sequenceAddCondition(condition_sequence, *condition);
                                            condition = initializeCondition();
                                        }
                                    }
                                    else{ // If it is invalid
                                        invalid = true;
                                    }
                                }

                                    // Since after an "and" we may have an action sentence we have to iterate it
                                    // In that case our current condition subjects are actually action subjects and our current word is an action keyword
                                    // That is why we have to go back action keywords
                                else if (strcmp(word, "go") == 0 || strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0){

                                    // However we have to be careful about "action sequence if action sequence" case
                                    if (condition_sequence->condition_count == 0){
                                        invalid = true;
                                        break;
                                    }
                                    // our condition subjects were actually the subjects of the next action
                                    for (int i = 0; i < condition->num_of_subjects; ++i) {
                                        action->subjects[i] = strdup(condition->subjects[i]);
                                    }
                                    action->num_of_subjects = condition->num_of_subjects;
                                    action->subj_array_size = condition->subj_array_size;
                                    condition = initializeCondition(); // reset the condition
                                    // add condition to the condition sequence
                                    addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                    condition_sequence = initializeConditionSequence();
                                    goto action_keywords;
                                }
                                else{ // the keyword is invalid
                                    invalid = true;
                                    break;
                                }
                            }

                        }
                        else if (strcmp(word,"to") == 0){
                            action->mode = "sell to";
                            word = strtok(NULL," "); // current word is trader
                            if (word == NULL){
                                invalid = true;
                                break;
                            }
                            if(!checkFormat(word)){ // check if trader is valid
                                invalid = true;
                                break;
                            }
                            for (int i = 0; i < action->num_of_subjects; ++i) {
                                if (strcmp(word, action->subjects[i]) == 0){ // trader can not be a subject
                                    invalid = true;
                                    break;
                                }
                            }
                            action->trader = word;
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction();
                            // sell to operation is over next word is either "if" or "and"
                            word = strtok(NULL," ");
                            if (word == NULL){ // Terminate
                                addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                                action_sequence = initializeActionSequence();
                                break;
                            }
                            else if(strcmp(word,"and") == 0){ // new action statement
                                break; // begin the new action statement
                            }
                            else if (strcmp(word,"if") == 0){ /// Condition
                                // Add action sequence to action_sequence_list
                                addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                                action_sequence = initializeActionSequence();
                                /// Continue with conditional statement
                                word = strtok(NULL, " ");
                                if (word == NULL){ // If there is no word after "if" it is invalid
                                    invalid = true;
                                }
                                //Each while loop will begin with a subject of a conditional statement
                                while (word != NULL){ // Current word is the subject of a conditional statement
                                    if(!checkFormat(word)){
                                        invalid = true;
                                        break;
                                    }
                                    conditionAddSubject(condition, word);// If subject is valid add it to condition
                                    word = strtok(NULL," "); // Take the next word which is keyword
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    // If keyword is "and" then there are more subjects to process so continue from the beginning of the while loop
                                    if (strcmp("and",word) == 0){
                                        word = strtok(NULL," ");
                                        continue;
                                    }

                                        // if the keyword is "at" then we will question its location
                                    else if (strcmp("at",word) == 0){
                                        // Format will be "subject(s) at location"
                                        condition->mode = "at";
                                        word = strtok(NULL," "); // read the next word which is location
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if(!checkFormat(word)){
                                            invalid = true;
                                            break;
                                        }
                                        conditionAddObject(condition,word,1); // add location to condition
                                        word = strtok(NULL," ");
                                        if (word == NULL){ // Terminate
                                            sequenceAddCondition(condition_sequence, *condition); // add condition to sequence
                                            condition = initializeCondition();
                                            addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                            condition_sequence = initializeConditionSequence();
                                            break;
                                        }
                                        else if (strcmp(word,"and") == 0){ //
                                            sequenceAddCondition(condition_sequence,*condition); // add condition to sequence
                                            condition = initializeCondition();
                                            word = strtok(NULL," ");
                                            if (word == NULL){
                                                invalid = true;
                                            }
                                            continue;
                                        }
                                    }

                                        // If the next word is "has" then there are 3 possibilities:
                                        // has, has more than, has less than
                                    else if (strcmp(word,"has") == 0){
                                        condition->mode = "has";
                                        word = strtok(NULL, " "); // read the word after "has"
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word, "more") == 0){ // if the word is "more"
                                            condition->mode = "has more";
                                            word = strtok(NULL," ");
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if (strcmp(word,"than") != 0){ // after "more" there should be "than"
                                                invalid = true;
                                                break;
                                            }
                                            word = strtok(NULL," "); // read the next word which is the amount of object
                                        }
                                        else if (strcmp(word, "less") == 0){ // if the word is "less"
                                            condition->mode = "has less";
                                            word = strtok(NULL," ");
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if (strcmp(word,"than") != 0){
                                                invalid = true;
                                                break;
                                            }
                                            word = strtok(NULL," ");// read the next word which is the amount of object
                                        }

                                        // Our current word is supposed to be the amount of first object
                                        if (word == NULL){
                                            invalid = true;
                                        }
                                        int amount = getNum(word); // getNum is a method that returns -1 if the number is invalid
                                        if (amount != -1){ // If the number is valid
                                            while(amount != -1){
                                                word = strtok(NULL," "); // read the next word which is the item
                                                if(word == NULL){
                                                    invalid = true;
                                                    break;
                                                }
                                                if(!checkFormat(word)){
                                                    invalid = true;
                                                    break;
                                                }
                                                conditionAddObject(condition,word,amount); // add item as an object
                                                word = strtok(NULL," "); // read the next word
                                                if (word == NULL){ // terminate
                                                    sequenceAddCondition(condition_sequence, *condition);
                                                    condition = initializeCondition();
                                                    addConditionSequence(&condition_sequence_list,condition_sequence, &condition_array_size, &condition_sequence_count);
                                                    condition_sequence = initializeConditionSequence();
                                                    break;
                                                }
                                                    // After the "and" there may be next item or next sentence
                                                    // If the word is a number than "amount != -1" so code will continue to store objects
                                                    // If the word is not a number then loop won't continue to take objects
                                                else if(strcmp(word,"and") == 0){
                                                    word = strtok(NULL," ");
                                                    if (word == NULL){
                                                        invalid = true;
                                                    }
                                                    amount = getNum(word);
                                                    continue;
                                                }
                                            }
                                            if (word != NULL){ // after taking the last object add condition to the sequence
                                                sequenceAddCondition(condition_sequence, *condition);
                                                condition = initializeCondition();
                                            }
                                        }
                                        else{ // If it is invalid
                                            invalid = true;
                                        }
                                    }

                                        // Since after an "and" we may have an action sentence we have to iterate it
                                        // In that case our current condition subjects are actually action subjects and our current word is an action keyword
                                        // That is why we have to go back action keywords
                                    else if (strcmp(word, "go") == 0 || strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0){

                                        // However we have to be careful about "action sequence if action sequence" case
                                        if (condition_sequence->condition_count == 0){
                                            invalid = true;
                                            break;
                                        }
                                        // our condition subjects were actually the subjects of the next action
                                        for (int i = 0; i < condition->num_of_subjects; ++i) {
                                            action->subjects[i] = strdup(condition->subjects[i]);
                                        }
                                        action->num_of_subjects = condition->num_of_subjects;
                                        action->subj_array_size = condition->subj_array_size;
                                        condition = initializeCondition(); // reset the condition
                                        // add condition to the condition sequence
                                        addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                        condition_sequence = initializeConditionSequence();
                                        goto action_keywords;
                                    }
                                    else{ // the keyword is invalid
                                        invalid = true;
                                        break;
                                    }
                                }

                            }
                            else{
                                invalid = true;
                                break;
                            }
                        }
                        else{
                            invalid = true;
                            break;
                        }
                    }
                    // Sell operation is over continue with the next action
                }
                else if (strcmp(word, "buy") == 0){
                    action->mode = "buy";
                    word = strtok(NULL," "); // current word is the amount
                    int amount = getNum(word);
                    if (amount == -1){ // check whether it is valid
                        invalid = true;
                        break;
                    }
                    while (getNum(word) != -1){ // each loop begins with amount
                        word = strtok(NULL," "); // next word is the item name
                        if (word == NULL){
                            invalid = true;
                            break;
                        }
                        if(!checkFormat(word)){ // check if the item is valid
                            invalid = true;
                            break;
                        }
                        actionAddObject(action,word,amount); // add the new object
                        word = strtok(NULL," ");
                        if (word == NULL){// Terminate
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction();
                            // add action sequence to action_sequence_list
                            addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                            action_sequence = initializeActionSequence();
                            break;
                        }
                        // If the next word is "and" we should either get another item or new action statement
                        if(strcmp(word,"and") == 0){
                            word = strtok(NULL," "); // If it is a number we get another item otherwise an action statement
                            if (word == NULL){
                                invalid = true;
                                break;
                            }
                            amount = getNum(word);
                            if (amount == -1){ // new action statement
                                sequenceAddAction(action_sequence,*action); // add action to sequence
                                action = initializeAction();
                                break; // break the loop that takes objects and return the loop that takes subjects
                            }
                            continue; // if it is a number continue taking objects
                        }

                        else if (strcmp(word,"if") == 0){ /// Condition
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction(); // create a new action
                            // Add action sequence to action_sequence_list
                            addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                            action_sequence = initializeActionSequence();
                            /// Continue with conditional statement
                            word = strtok(NULL, " ");
                            if (word == NULL){ // If there is no word after "if" it is invalid
                                invalid = true;
                            }
                            //Each while loop will begin with a subject of a conditional statement
                            while (word != NULL){ // Current word is the subject of a conditional statement
                                if(!checkFormat(word)){
                                    invalid = true;
                                    break;
                                }
                                conditionAddSubject(condition, word);// If subject is valid add it to condition
                                word = strtok(NULL," "); // Take the next word which is keyword
                                if(word == NULL){
                                    invalid = true;
                                    break;
                                }
                                // If keyword is "and" then there are more subjects to process so continue from the beginning of the while loop
                                if (strcmp("and",word) == 0){
                                    word = strtok(NULL," ");
                                    continue;
                                }

                                    // if the keyword is "at" then we will question its location
                                else if (strcmp("at",word) == 0){
                                    // Format will be "subject(s) at location"
                                    condition->mode = "at";
                                    word = strtok(NULL," "); // read the next word which is location
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if(!checkFormat(word)){
                                        invalid = true;
                                        break;
                                    }
                                    conditionAddObject(condition,word,1); // add location to condition
                                    word = strtok(NULL," ");
                                    if (word == NULL){ // Terminate
                                        sequenceAddCondition(condition_sequence, *condition); // add condition to sequence
                                        condition = initializeCondition();
                                        addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                        condition_sequence = initializeConditionSequence();
                                        break;
                                    }
                                    else if (strcmp(word,"and") == 0){ //
                                        sequenceAddCondition(condition_sequence,*condition); // add condition to sequence
                                        condition = initializeCondition();
                                        word = strtok(NULL," ");
                                        if (word == NULL){
                                            invalid = true;
                                        }
                                        continue;
                                    }
                                }

                                    // If the next word is "has" then there are 3 possibilities:
                                    // has, has more than, has less than
                                else if (strcmp(word,"has") == 0){
                                    condition->mode = "has";
                                    word = strtok(NULL, " "); // read the word after "has"
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    if (strcmp(word, "more") == 0){ // if the word is "more"
                                        condition->mode = "has more";
                                        word = strtok(NULL," ");
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word,"than") != 0){ // after "more" there should be "than"
                                            invalid = true;
                                            break;
                                        }
                                        word = strtok(NULL," "); // read the next word which is the amount of object
                                    }
                                    else if (strcmp(word, "less") == 0){ // if the word is "less"
                                        condition->mode = "has less";
                                        word = strtok(NULL," ");
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word,"than") != 0){
                                            invalid = true;
                                            break;
                                        }
                                        word = strtok(NULL," ");// read the next word which is the amount of object
                                    }

                                    // Our current word is supposed to be the amount of first object
                                    if (word == NULL){
                                        invalid = true;
                                    }
                                    int amount = getNum(word); // getNum is a method that returns -1 if the number is invalid
                                    if (amount != -1){ // If the number is valid
                                        while(amount != -1){
                                            word = strtok(NULL," "); // read the next word which is the item
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if(!checkFormat(word)){
                                                invalid = true;
                                                break;
                                            }
                                            conditionAddObject(condition,word,amount); // add item as an object
                                            word = strtok(NULL," "); // read the next word
                                            if (word == NULL){ // terminate
                                                sequenceAddCondition(condition_sequence, *condition);
                                                condition = initializeCondition();
                                                addConditionSequence(&condition_sequence_list,condition_sequence, &condition_array_size, &condition_sequence_count);
                                                condition_sequence = initializeConditionSequence();
                                                break;
                                            }
                                                // After the "and" there may be next item or next sentence
                                                // If the word is a number than "amount != -1" so code will continue to store objects
                                                // If the word is not a number then loop won't continue to take objects
                                            else if(strcmp(word,"and") == 0){
                                                word = strtok(NULL," ");
                                                if (word == NULL){
                                                    invalid = true;
                                                }
                                                amount = getNum(word);
                                                continue;
                                            }
                                        }
                                        if (word != NULL){ // after taking the last object add condition to the sequence
                                            sequenceAddCondition(condition_sequence, *condition);
                                            condition = initializeCondition();
                                        }
                                    }
                                    else{ // If it is invalid
                                        invalid = true;
                                    }
                                }

                                    // Since after an "and" we may have an action sentence we have to iterate it
                                    // In that case our current condition subjects are actually action subjects and our current word is an action keyword
                                    // That is why we have to go back action keywords
                                else if (strcmp(word, "go") == 0 || strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0){

                                    // However we have to be careful about "action sequence if action sequence" case
                                    if (condition_sequence->condition_count == 0){
                                        invalid = true;
                                        break;
                                    }
                                    // our condition subjects were actually the subjects of the next action
                                    for (int i = 0; i < condition->num_of_subjects; ++i) {
                                        action->subjects[i] = strdup(condition->subjects[i]);
                                    }
                                    action->num_of_subjects = condition->num_of_subjects;
                                    action->subj_array_size = condition->subj_array_size;
                                    condition = initializeCondition(); // reset the condition
                                    // add condition to the condition sequence
                                    addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                    condition_sequence = initializeConditionSequence();
                                    goto action_keywords;
                                }
                                else{ // the keyword is invalid
                                    invalid = true;
                                    break;
                                }
                            }

                        }
                        else if (strcmp(word,"from") == 0){
                            action->mode = "buy from";
                            word = strtok(NULL," "); // next word should be trader
                            if (word == NULL){
                                invalid = true;
                                break;
                            }
                            if(!checkFormat(word)){ // check whether trader is valid or not
                                invalid = true;
                                break;
                            }
                            for (int i = 0; i < action->num_of_subjects; ++i) { // if trader is one of subjects then invalid
                                if (strcmp(word, action->subjects[i]) == 0){
                                    invalid = true;
                                    break;
                                }
                            }
                            action->trader = word;
                            sequenceAddAction(action_sequence,*action); // add action to sequence
                            action = initializeAction();
                            word = strtok(NULL," "); // the next word is either "and" or "if"
                            if (word == NULL){ // Terminate
                                addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                                action_sequence = initializeActionSequence();
                                break;
                            }
                            else if(strcmp(word,"and") == 0){ // new action statement
                                break; // begin the new action statement
                            }
                            else if (strcmp(word,"if") == 0){ /// Condition
                                // Add action sequence to action_sequence_list
                                addActionSequence(&action_sequence_list,action_sequence,&action_array_size,&action_sequence_count);
                                action_sequence = initializeActionSequence();
                                /// Continue with conditional statement
                                word = strtok(NULL, " ");
                                if (word == NULL){ // If there is no word after "if" it is invalid
                                    invalid = true;
                                }
                                //Each while loop will begin with a subject of a conditional statement
                                while (word != NULL){ // Current word is the subject of a conditional statement
                                    if(!checkFormat(word)){
                                        invalid = true;
                                        break;
                                    }
                                    conditionAddSubject(condition, word);// If subject is valid add it to condition
                                    word = strtok(NULL," "); // Take the next word which is keyword
                                    if(word == NULL){
                                        invalid = true;
                                        break;
                                    }
                                    // If keyword is "and" then there are more subjects to process so continue from the beginning of the while loop
                                    if (strcmp("and",word) == 0){
                                        word = strtok(NULL," ");
                                        continue;
                                    }

                                        // if the keyword is "at" then we will question its location
                                    else if (strcmp("at",word) == 0){
                                        // Format will be "subject(s) at location"
                                        condition->mode = "at";
                                        word = strtok(NULL," "); // read the next word which is location
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if(!checkFormat(word)){
                                            invalid = true;
                                            break;
                                        }
                                        conditionAddObject(condition,word,1); // add location to condition
                                        word = strtok(NULL," ");
                                        if (word == NULL){ // Terminate
                                            sequenceAddCondition(condition_sequence, *condition); // add condition to sequence
                                            condition = initializeCondition();
                                            addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                            condition_sequence = initializeConditionSequence();
                                            break;
                                        }
                                        else if (strcmp(word,"and") == 0){ //
                                            sequenceAddCondition(condition_sequence,*condition); // add condition to sequence
                                            condition = initializeCondition();
                                            word = strtok(NULL," ");
                                            if (word == NULL){
                                                invalid = true;
                                            }
                                            continue;
                                        }
                                    }

                                        // If the next word is "has" then there are 3 possibilities:
                                        // has, has more than, has less than
                                    else if (strcmp(word,"has") == 0){
                                        condition->mode = "has";
                                        word = strtok(NULL, " "); // read the word after "has"
                                        if(word == NULL){
                                            invalid = true;
                                            break;
                                        }
                                        if (strcmp(word, "more") == 0){ // if the word is "more"
                                            condition->mode = "has more";
                                            word = strtok(NULL," ");
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if (strcmp(word,"than") != 0){ // after "more" there should be "than"
                                                invalid = true;
                                                break;
                                            }
                                            word = strtok(NULL," "); // read the next word which is the amount of object
                                        }
                                        else if (strcmp(word, "less") == 0){ // if the word is "less"
                                            condition->mode = "has less";
                                            word = strtok(NULL," ");
                                            if(word == NULL){
                                                invalid = true;
                                                break;
                                            }
                                            if (strcmp(word,"than") != 0){
                                                invalid = true;
                                                break;
                                            }
                                            word = strtok(NULL," ");// read the next word which is the amount of object
                                        }

                                        // Our current word is supposed to be the amount of first object
                                        if (word == NULL){
                                            invalid = true;
                                        }
                                        int amount = getNum(word); // getNum is a method that returns -1 if the number is invalid
                                        if (amount != -1){ // If the number is valid
                                            while(amount != -1){
                                                word = strtok(NULL," "); // read the next word which is the item
                                                if(word == NULL){
                                                    invalid = true;
                                                    break;
                                                }
                                                if(!checkFormat(word)){
                                                    invalid = true;
                                                    break;
                                                }
                                                conditionAddObject(condition,word,amount); // add item as an object
                                                word = strtok(NULL," "); // read the next word
                                                if (word == NULL){ // terminate
                                                    sequenceAddCondition(condition_sequence, *condition);
                                                    condition = initializeCondition();
                                                    addConditionSequence(&condition_sequence_list,condition_sequence, &condition_array_size, &condition_sequence_count);
                                                    condition_sequence = initializeConditionSequence();
                                                    break;
                                                }
                                                    // After the "and" there may be next item or next sentence
                                                    // If the word is a number than "amount != -1" so code will continue to store objects
                                                    // If the word is not a number then loop won't continue to take objects
                                                else if(strcmp(word,"and") == 0){
                                                    word = strtok(NULL," ");
                                                    if (word == NULL){
                                                        invalid = true;
                                                    }
                                                    amount = getNum(word);
                                                    continue;
                                                }
                                            }
                                            if (word != NULL){ // after taking the last object add condition to the sequence
                                                sequenceAddCondition(condition_sequence, *condition);
                                                condition = initializeCondition();
                                            }
                                        }
                                        else{ // If it is invalid
                                            invalid = true;
                                        }
                                    }

                                        // Since after an "and" we may have an action sentence we have to iterate it
                                        // In that case our current condition subjects are actually action subjects and our current word is an action keyword
                                        // That is why we have to go back action keywords
                                    else if (strcmp(word, "go") == 0 || strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0){

                                        // However we have to be careful about "action sequence if action sequence" case
                                        if (condition_sequence->condition_count == 0){
                                            invalid = true;
                                            break;
                                        }
                                        // our condition subjects were actually the subjects of the next action
                                        for (int i = 0; i < condition->num_of_subjects; ++i) {
                                            action->subjects[i] = strdup(condition->subjects[i]);
                                        }
                                        action->num_of_subjects = condition->num_of_subjects;
                                        action->subj_array_size = condition->subj_array_size;
                                        condition = initializeCondition(); // reset the condition
                                        // add condition to the condition sequence
                                        addConditionSequence(&condition_sequence_list,condition_sequence,&condition_array_size,&condition_sequence_count);
                                        condition_sequence = initializeConditionSequence();
                                        goto action_keywords;
                                    }
                                    else{ // the keyword is invalid
                                        invalid = true;
                                        break;
                                    }
                                }

                            }
                            else{
                                invalid = true;
                                break;
                            }
                        }
                        else{
                            invalid = true;
                            break;
                        }
                    }
                }
                else{
                    invalid = true;
                }
            }


            // There should not be any duplicate items or subjects for each action and condition
            for (int i = 0; i < condition_sequence_count; ++i) { // For each condition sequence
                struct Condition_Sequence *seq = condition_sequence_list[i];
                for (int j = 0; j < seq->condition_count; ++j) { // For each condition
                    struct Condition *cond = seq->conditions[j];
                    if (hasDuplicates(cond->subjects,cond->num_of_subjects)){
                        invalid = true;
                    }
                    if (hasDuplicates(cond->objects,cond->num_of_objects)){
                        invalid = true;
                    }
                }
            }
            for (int i = 0; i < action_sequence_count; ++i) { // For each action sequence
                struct Action_Sequence *seq = action_sequence_list[i];
                for (int j = 0; j < seq->action_count; ++j) { // For each action
                    struct Action *act = seq->actions[j];
                    if (hasDuplicates(act->subjects,act->num_of_subjects)){
                        invalid = true;
                    }
                    if (hasDuplicates(act->objects,act->num_of_objects)){
                        invalid = true;
                    }
                }

            }


            if (invalid){
                printf("%s\n", "INVALID");
                fflush(stdout);
            }
            else{
                for (int i = 0; i < condition_sequence_count; ++i) { //For each condition sequence
                    // if condition sequence is true process the action sequence
                    if (checkConditionSequence(condition_sequence_list[i],&people,&people_count,&people_array_size)){
                        processActionSequence(*action_sequence_list[i],&people,&people_count,&people_array_size);
                    }
                }
                // if the last sequence is action sequence process it
                if (action_sequence_count > condition_sequence_count){
                    processActionSequence(*action_sequence_list[action_sequence_count-1],&people,&people_count,&people_array_size);
                }
                printf("%s\n", "OK");
                fflush(stdout);
            }



            // free the allocated memory
            freeAction(action);
            freeCondition(condition);

            for (int i = 0; i < action_sequence_count; ++i) {
                freeActionSequence(action_sequence_list[i]);
            }
            free(action_sequence_list);

            for (int i = 0; i < condition_sequence_count; ++i) {
                freeConditionSequence(condition_sequence_list[i]);
            }
            free(condition_sequence_list);
        }


    }

    // free the allocated memory
    for (int i = 0; i < people_count; ++i) {
        freePerson(people[i]);
    }
    free(people);
}

// return true if there is a duplicate in a string array
bool hasDuplicates(char **strArray, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (strcmp(strArray[i], strArray[j]) == 0) {
                return true; // Found a duplicate
            }
        }
    }
    return false; // No duplicates found
}

//will return natural number equivalent of a string (invalid case returns -1)
int getNum(char *str) {
    if (str == NULL || *str == '\0') // Check if the string is empty or NULL
        return -1;

    char numStr[100]; // Assuming maximum length of the numeric string
    int i = 0;

    // Copy digits from str to numStr
    while (*str != '\0') {
        if (isdigit(*str)) {
            numStr[i] = *str;
            i++;
        } else {
            return -1; // If any character is not a digit, return -1
        }
        str++;
    }
    numStr[i] = '\0'; // Null-terminate the numStr

    // Convert the numStr to an integer
    int num = atoi(numStr);

    if (num >= 0)
        return num;
    else
        return -1;
}

// checks whether a word is valid or not as a subject, object or location
bool checkFormat(char *word){
    // Since null case handled separately we returned true here
    if (word == NULL){
        return true;
    }
        // Subjects and objects can not be one of keywords
    else if (strcmp(word, "sell") == 0 || strcmp(word, "buy") == 0 || strcmp(word, "go") == 0 || strcmp(word, "to") == 0 || strcmp(word, "from") == 0 || strcmp(word, "and") == 0 || strcmp(word, "at") == 0 || strcmp(word, "has") == 0 || strcmp(word, "if") == 0 || strcmp(word, "less") == 0 || strcmp(word, "more") == 0 || strcmp(word, "than") == 0 || strcmp(word, "exit") == 0 || strcmp(word, "where") == 0 || strcmp(word, "total") == 0 || strcmp(word, "who") == 0 || strcmp(word, "NOBODY") == 0 || strcmp(word, "NOTHING") == 0 || strcmp(word, "NOWHERE") == 0){
        return false;
    }
    // They should consist of uppercase and lowercase letters
    for (int i = 0; i < strlen(word); ++i) {
        char chr = *(word + i);
        if (!(chr > 64 && chr < 91) && chr != 95 && !(chr > 96 && chr < 123)){
            return false;
        }
    }
    return true;
}


// Scans a people array to find a person and if the person does not exist creates its data
struct Person *findPerson(struct Person ***people, char *name, int *people_count, int *array_size) {
    for (int i = 0; i < *people_count; ++i) {
        if (strcmp((*people)[i]->name, name) == 0) {
            return (*people)[i];
        }
    }
    // Person not found, create a new one
    createPerson(people, name, people_count, array_size);
    return (*people)[*people_count - 1];
}

// Only called from "findPerson" function, creates a person
void createPerson(struct Person ***people, char *name, int *people_count, int *array_size){
    *people_count += 1; // Update the number of people
    if (*people_count == *array_size){ // If array is almost full reallocate it
        *array_size *= 2;
        *people = realloc(*people, sizeof(struct Person*) * (*array_size));
    }
    // First create the new person
    struct Person *person = calloc(1, sizeof(struct Person));
    person->name = strdup(name); // strdup itself allocates the memory so no need to do it
    person->location = strdup("NOWHERE");
    person->item_count = 0;
    person->item_array_size = INITIAL_ARRAY_SIZE;
    person->items = calloc(INITIAL_ARRAY_SIZE, sizeof(char*));
    person->amounts = calloc(INITIAL_ARRAY_SIZE, sizeof(int));
    // Then add it to the end of the "people" array
    (*people)[*people_count - 1] = person;
}

// Adds a new item to a Person
void addItem(struct Person *person, char *item_name, int amount){
    person->item_count++; // Update the number of items
    if (person->item_count == person->item_array_size){ // If array is almost full reallocate it
        person->item_array_size *= 2;
        person->items = realloc(person->items, sizeof(char*) * (person->item_array_size));
        person->amounts = realloc(person->amounts, sizeof(int) * (person->item_array_size));
    }
    // Add the item
    person->items[person->item_count - 1] = malloc(strlen(item_name) + 1); // Allocate the memory for the item name
    strcpy(person->items[person->item_count - 1], item_name); // Add the name to allocated address
    person->amounts[person->item_count - 1] = amount; // amount is integer no need to allocate
}

// prints out all the people in a specific location
void who_at(struct Person **people, char *location, int people_count){
    bool found = false;
    for (int i = 0; i < people_count; i++){
        if (strcmp(people[i]->location, location) == 0){
            if(!found){
                printf("%s", people[i]->name);
                fflush(stdout);
                found = true;
            }
            else{
                printf("%s", " and ");
                printf("%s", people[i]->name);
                fflush(stdout);
            }
        }
    }
    if (!found){ // if no one is found
        printf("%s", "NOBODY");
        fflush(stdout);
    }
    printf("%s", "\n");
    fflush(stdout);
}

// returns how many item the person has
int getItemNumber(struct Person *person, char *item_name){
    int index = getItemIndex(person,item_name);
    if( index == -1 ){
        return 0;
    }
    return person->amounts[index];
}

// returns the index of an item in the item array of a person
int getItemIndex(struct Person *person, char *item_name){
    for (int i = 0; i < person->item_count; ++i) {
        if(strcmp(person->items[i], item_name) == 0){
            return i;
        }
    }
    return -1;
}



// Processes a condition sequence and returns its value
// It calls a primitive condition function which controls a condition for only one subject and one subject
bool checkConditionSequence(struct Condition_Sequence *sequence, struct Person ***people, int *people_count, int *people_array_size) {
    for (int i = 0; i < sequence->condition_count; ++i) {
        // For each condition
        struct Condition *condition = sequence->conditions[i];
        for (int j = 0; j < condition->num_of_subjects; ++j) {
            // Every subject has to satisfy the condition for every object
            struct Person *subject = findPerson(people, condition->subjects[j], people_count, people_array_size);
            for (int k = 0; k < condition->num_of_objects; ++k) {
                bool result = primitiveCondition(subject, condition->mode, condition->objects[k], condition->amounts[k]);
                if (!result) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool primitiveCondition(struct Person *person, char *mode, char* object, int count){
    if (strcmp(mode, "at") == 0){
        if (strcmp(person -> location, object) == 0){
            return true;
        }
        return false;
    }

    if (strcmp(mode, "has") == 0){
        if (getItemNumber(person, object) == count){
            return true;
        }
        return false;
    }

    if (strcmp(mode, "has more") == 0){
        if (getItemNumber(person, object) > count){
            return true;
        }
        return false;
    }

    if (strcmp(mode, "has less") == 0){
        if (getItemNumber(person, object) < count){
            return true;
        }
        return false;
    }
    return false;
}

// Processes each action in an action sequence
void processActionSequence(struct Action_Sequence sequence, struct Person ***people, int *people_count, int *people_array_size){
    for (int i = 0; i < sequence.action_count; ++i) {
        processAction(*sequence.actions[i],people,people_count,people_array_size);
    }
}
// Primitive action can not process sell to and buy from methods.
// Instead of processing there we decided to use primitive condition to check prerequisites and if it is true process with primitive actions
// For example a buy 4 bread from b is equivalent with: a buy 4 bread and b sell 4 bread unless b has less than 4 bread
void processAction(struct Action action, struct Person ***people, int *people_count, int *people_array_size) {
    if (strcmp(action.mode, "go to") == 0) {
        for (int i = 0; i < action.num_of_subjects; ++i) {
            // Find the person
            struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
            // Process it
            primitiveAction(person, action.mode, 1, action.objects[0]);
        }
    }
    if (strcmp(action.mode, "buy") == 0) {
        for (int i = 0; i < action.num_of_subjects; ++i) {
            struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
            for (int j = 0; j < action.num_of_objects; ++j) {
                primitiveAction(person, action.mode, action.amounts[j], action.objects[j]);
            }
        }
    }

    if (strcmp(action.mode, "buy from") == 0) {
        struct Person *trader = findPerson(people, action.trader, people_count, people_array_size);
        for (int j = 0; j < action.num_of_objects; ++j) {
            // For each object
            int total = action.num_of_subjects * action.amounts[j];
            // Check whether trader has enough of them or not
            if (primitiveCondition(trader, "has less", action.objects[j], total)) {
                // If he does not have enough item return
                return;
            }
        }
        //If he has enough item
        for (int j = 0; j < action.num_of_objects; ++j) {
            int total = action.num_of_subjects * action.amounts[j];
            // Trader sells his items
            primitiveAction(trader, "sell", total, action.objects[j]);
            for (int i = 0; i < action.num_of_subjects; ++i) {
                // Subjects buy
                struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
                primitiveAction(person, "buy", action.amounts[j], action.objects[j]);
            }
        }
    }

    if (strcmp(action.mode, "sell") == 0) {
        for (int i = 0; i < action.num_of_subjects; ++i) { // First check whether each subject has enough item or not
            struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
            for (int j = 0; j < action.num_of_objects; ++j) {
                if (primitiveCondition(person, "has less", action.objects[j], action.amounts[j])) {
                    // If someone does not have enough return
                    return;
                }
            }
        }
        for (int i = 0; i < action.num_of_subjects; ++i) { // If they have enough items then make them sell
            struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
            for (int j = 0; j < action.num_of_objects; ++j) {
                primitiveAction(person, action.mode, action.amounts[j], action.objects[j]);
            }
        }
    }
    if (strcmp(action.mode, "sell to") == 0) {
        struct Person *trader = findPerson(people, action.trader, people_count, people_array_size);
        for (int i = 0; i < action.num_of_subjects; ++i) { // Similar to sell check
            struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
            for (int j = 0; j < action.num_of_objects; ++j) {
                if (primitiveCondition(person, "has less", action.objects[j], action.amounts[j])) {
                    return;
                }
            }
        }
        for (int j = 0; j < action.num_of_objects; ++j) { // Similar to sell the only difference trader buys those items
            int total = action.num_of_subjects * action.amounts[j];
            primitiveAction(trader, "buy", total, action.objects[j]);
            for (int i = 0; i < action.num_of_subjects; ++i) {
                struct Person *person = findPerson(people, action.subjects[i], people_count, people_array_size);
                primitiveAction(person, "sell", action.amounts[j], action.objects[j]);
            }
        }
    }
}
// Handles sell, go to and buy
void primitiveAction(struct Person *person, char *mode, int num, char *object){
    if(strcmp(mode,"go to") == 0){
        person->location = strdup(object);
    }
    else if(strcmp(mode,"buy") == 0){
        int index = getItemIndex(person,object);
        if (index == -1){
            addItem(person,object,num);
        }
        else{
            person->amounts[index] += num;
        }
    }
    else if(strcmp(mode,"sell") == 0){
        int index = getItemIndex(person,object);
        if (index == -1 ){return;}
        if (person->amounts[index] >= num){
            person->amounts[index] -= num;
            return;
        }
    }
}


// Constructor of an action
struct Action *initializeAction(){
    struct Action *action = calloc(1,sizeof (struct Action));
    action->num_of_objects = 0;
    action->num_of_subjects = 0;
    action->subj_array_size = INITIAL_ARRAY_SIZE;
    action->obj_array_size = INITIAL_ARRAY_SIZE;
    action->subjects = calloc(action->subj_array_size, sizeof(char*));
    action->objects = calloc(action->obj_array_size, sizeof(char*));
    action->amounts = calloc(action->obj_array_size, sizeof(int));
    return action;
}
// Constructor of condition
struct Condition *initializeCondition(){
    struct Condition *condition = calloc(1, sizeof(struct Condition));
    condition->num_of_objects = 0;
    condition->num_of_subjects = 0;
    condition->subj_array_size = INITIAL_ARRAY_SIZE;
    condition->obj_array_size = INITIAL_ARRAY_SIZE;
    condition->subjects = calloc(condition->subj_array_size, sizeof(char*));
    condition->objects = calloc(condition->obj_array_size, sizeof(char*));
    condition->amounts = calloc(condition->obj_array_size, sizeof(int));
    return condition;
}

// Adds a subject to an action
void actionAddSubject(struct Action *action, char *subject){
    action->num_of_subjects++;
    if (action->num_of_subjects == action->subj_array_size){
        action->subj_array_size *= 2;
        action->subjects = realloc(action->subjects, action->subj_array_size * sizeof(char*));
    }
    action->subjects[action->num_of_subjects - 1] = calloc(1, sizeof(char*));
    action->subjects[action->num_of_subjects - 1] = strdup(subject);

}
// Adds an object to an action
void actionAddObject(struct Action *action, char *object, int amount){
    action->num_of_objects++;
    if (action->num_of_objects == action->obj_array_size){
        action->obj_array_size *= 2;
        action->objects = realloc(action->objects, action->obj_array_size * sizeof(char*));
        action->amounts = realloc(action->amounts, action->obj_array_size * sizeof(int));
    }
    action->objects[action->num_of_objects - 1] = calloc(1, sizeof(char*));
    action->objects[action->num_of_objects - 1] = strdup(object);
    action->amounts[action->num_of_objects -1] = amount;
}

// Adds a subject to a condition
void conditionAddSubject(struct Condition *condition, char *subject){
    condition->num_of_subjects++;
    if (condition->num_of_subjects == condition->subj_array_size){
        condition->subj_array_size *= 2;
        condition->subjects = realloc(condition->subjects, condition->subj_array_size * sizeof(char*));
    }
    condition->subjects[condition->num_of_subjects - 1] = calloc(1, sizeof(char*));
    condition->subjects[condition->num_of_subjects - 1] = strdup(subject);
}
// Adds an object to a condition
void conditionAddObject(struct Condition *condition, char *object, int amount){
    condition->num_of_objects++;
    if (condition->num_of_objects == condition->obj_array_size){
        condition->obj_array_size *= 2;
        condition->objects = realloc(condition->objects, condition->obj_array_size * sizeof(char*));
        condition->amounts = realloc(condition->amounts, condition->obj_array_size * sizeof(int));
    }
    condition->objects[condition->num_of_objects - 1] = calloc(1, sizeof(char*));
    condition->objects[condition->num_of_objects - 1] = strdup(object);
    condition->amounts[condition->num_of_objects -1] = amount;
}

// Constructor of an action sequence
struct Action_Sequence *initializeActionSequence(){
    struct Action_Sequence *sequence = calloc(1,sizeof (struct Action_Sequence));
    sequence->action_array_size = INITIAL_ARRAY_SIZE;
    sequence->action_count = 0;
    sequence->actions = calloc(sequence->action_array_size, sizeof(struct Action*));
    return sequence;
}
// Constructor of a condition sequence
struct Condition_Sequence *initializeConditionSequence(){
    struct Condition_Sequence *sequence = calloc(1,sizeof(struct Condition_Sequence));
    sequence->condition_array_size = INITIAL_ARRAY_SIZE;
    sequence->condition_count = 0;
    sequence->conditions = calloc(sequence->condition_array_size, sizeof(struct Condition*));
    return sequence;
}

// Adds an action to an action sequence
void sequenceAddAction(struct Action_Sequence *sequence, struct Action action){
    struct Action *copy  = calloc(1, sizeof(struct Action));
    sequence->action_count += 1;
    if (sequence->action_count == sequence->action_array_size){
        sequence->action_array_size *= 2;
        sequence->actions = realloc(sequence->actions, sequence->action_array_size * sizeof(struct Action*));
    }
    copy->num_of_subjects = action.num_of_subjects;
    copy->num_of_objects = action.num_of_objects;
    copy->subj_array_size= action.subj_array_size;
    copy->obj_array_size = action.obj_array_size;
    copy->subjects = calloc(copy->subj_array_size, sizeof(char*));
    copy->objects = calloc(copy->obj_array_size, sizeof(char*));
    copy->amounts = calloc(copy->obj_array_size, sizeof(int));
    copy->mode = strdup(action.mode);
    if (action.trader != NULL){
        copy->trader = strdup(action.trader);
    }
    for (int i = 0; i < copy->num_of_subjects; ++i) {
        copy->subjects[i] = strdup(action.subjects[i]);
    }
    for (int i = 0; i < copy->num_of_objects; ++i) {
        copy->objects[i] = strdup(action.objects[i]);
        copy->amounts = action.amounts;
    }

    sequence->actions[sequence->action_count - 1] = copy;
}
// Adds a condition to a condition sequence
void sequenceAddCondition(struct Condition_Sequence *sequence, struct Condition condition){
    struct Condition *copy  = calloc(1, sizeof(struct Condition));
    sequence->condition_count += 1;
    if (sequence->condition_count == sequence->condition_array_size){
        sequence->condition_array_size *= 2;
        sequence->conditions = realloc(sequence->conditions, sequence->condition_array_size * sizeof(struct Condition*));
    }
    copy->num_of_subjects = condition.num_of_subjects;
    copy->num_of_objects = condition.num_of_objects;
    copy->subj_array_size= condition.subj_array_size;
    copy->obj_array_size = condition.obj_array_size;
    copy->subjects = calloc(copy->subj_array_size, sizeof(char*));
    copy->objects = calloc(copy->obj_array_size, sizeof(char*));
    copy->amounts = calloc(copy->obj_array_size, sizeof(int));
    copy->mode = strdup(condition.mode);
    for (int i = 0; i < copy->num_of_subjects; ++i) {
        copy->subjects[i] = strdup(condition.subjects[i]);
    }
    for (int i = 0; i < copy->num_of_objects; ++i) {
        copy->objects[i] = strdup(condition.objects[i]);
        copy->amounts = condition.amounts;
    }

    sequence->conditions[sequence->condition_count - 1] = copy;
}

// Adds an action sequence to an action sequence array
void addActionSequence(struct Action_Sequence ***array, struct Action_Sequence *sequence, int *array_size, int *num_elements){
    *num_elements += 1;
    if (*num_elements == *array_size){
        *array_size *= 2;
        *array = realloc(*array,(*array_size) * sizeof(struct Action_Sequence*));
    }
    (*array)[*num_elements - 1] = calloc(1,sizeof(struct Action_Sequence));
    struct Action_Sequence *copy = calloc(1,sizeof (struct Action_Sequence));
    copy->action_count = sequence->action_count;
    copy->action_array_size = sequence->action_array_size;
    copy->actions = calloc(copy->action_array_size, sizeof(struct Action*));
    for (int i = 0; i < copy->action_count; ++i) {
        copy->actions[i] = calloc(1,sizeof (struct Action));
        *copy->actions[i] = *sequence->actions[i];
    }
    (*array)[*num_elements - 1] = copy;

}
// Adds a condition sequence to a condition sequence array
void addConditionSequence(struct Condition_Sequence ***array, struct Condition_Sequence *sequence, int *array_size, int *num_elements){
    *num_elements += 1;
    if (*num_elements == *array_size){
        *array_size *= 2;
        *array = realloc(*array,(*array_size) * sizeof(struct Condition_Sequence*));
    }
    (*array)[*num_elements - 1] = calloc(1,sizeof(struct Condition_Sequence));
    struct Condition_Sequence *copy = calloc(1,sizeof (struct Condition_Sequence));
    copy->condition_count = sequence->condition_count;
    copy->condition_array_size = sequence->condition_array_size;
    copy->conditions = calloc(copy->condition_array_size, sizeof(struct Condition*));
    for (int i = 0; i < copy->condition_count; ++i) {
        copy->conditions[i] = calloc(1,sizeof (struct Condition));
        *copy->conditions[i] = *sequence->conditions[i];
    }
    (*array)[*num_elements - 1] = copy;
}

// frees the allocated memory for a person struct pointer and its contents
void freePerson(struct Person *person) {
    if (person == NULL) {
        return;
    }

    // free each item string and the items array itself
    if (person->items != NULL) {
        for (int i = 0; i < person->item_count; i++) {
            free(person->items[i]);
        }
        free(person->items);
    }

    // free the amounts array
    free(person->amounts);

    free(person);
}

// frees the allocated memory for an action struct pointer and its contents
void freeAction(struct Action *action) {
    if (action == NULL) {
        return;
    }

    // free each subject string and the subjects array itself
    if (action->subjects != NULL) {
        for (int i = 0; i < action->num_of_subjects; i++) {
            free(action->subjects[i]);
        }
        free(action->subjects);
    }

    // free each object string and the objects array itself
    if (action->objects != NULL) {
        for (int i = 0; i < action->num_of_objects; i++) {
            free(action->objects[i]);
        }
        free(action->objects);
    }

    // free the amounts array
    free(action->amounts);

    free(action);
}

// frees the allocated memory for a condition struct pointer and its contents
void freeCondition(struct Condition *condition) {
    if (condition == NULL) {
        return;
    }

    // free each subject string and the subjects array itself
    if (condition->subjects != NULL) {
        for (int i = 0; i < condition->num_of_subjects; i++) {
            free(condition->subjects[i]);
        }
        free(condition->subjects);
    }

    // free each object string and the objects array itself
    if (condition->objects != NULL) {
        for (int i = 0; i < condition->num_of_objects; i++) {
            free(condition->objects[i]);
        }
        free(condition->objects);
    }

    // free the amounts array
    free(condition->amounts);

    free(condition);
}

// frees the allocated memory for an action sequence struct pointer and its contents
void freeActionSequence(struct Action_Sequence *sequence) {
    if (sequence == NULL) {
        return;
    }

    // free each action in the actions array and the actions array itself
    if (sequence->actions != NULL) {
        for (int i = 0; i < sequence->action_count; i++) {
            freeAction(sequence->actions[i]);
        }
        free(sequence->actions);
    }

    free(sequence);
}

// frees the allocated memory for a condition sequence struct pointer and its contents
void freeConditionSequence(struct Condition_Sequence *sequence) {
    if (sequence == NULL) {
        return;
    }

    // free each condition in the conditions array and the conditions array itself
    if (sequence->conditions != NULL) {
        for (int i = 0; i < sequence->condition_count; i++) {
            freeCondition(sequence->conditions[i]);
        }
        free(sequence->conditions);
    }

    free(sequence);
}