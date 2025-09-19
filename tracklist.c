#include "globals.h"
#include "pico/time.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct TRACK {
    int track_id;
    char* car_id;
    float time;
    struct TRACK* next;
} TRACK;


absolute_time_t start; //Global variable for the start time of the race

TRACK* head; //Head of track linked list
TRACK* waiting; //Track that is waiting to be registered to a car

void setWaiting (char* id){
    waiting->car_id = id;
}


TRACK* getWaiting(){
    return waiting;
}

void initializeHead(){
    head = (TRACK*) malloc(sizeof(TRACK));
    head->track_id = -1;
    head->time = 0;
    head->car_id = NULL;
    head->next = NULL;
}

void printList(){
    if(head->next != NULL){
        TRACK* pointer = head->next;
        while (pointer != NULL){
            debug_print("Track %d registered to car", pointer->track_id);
            if (pointer->car_id != NULL){
                debug_print(" %s ", pointer->car_id);
            } else {
                debug_print(" NULL ");
            }
            debug_print("with time: %f \n", pointer->time);
            pointer = pointer->next;
        }
    } else {
        debug_print("No tracks registered yet.\n");
    }
}

TRACK* getHead(){
    return head;
}

absolute_time_t getStart(){
    return start;
}

void setStart(absolute_time_t timestamp){
    start = timestamp;
}

void deleteList(TRACK* track){
    TRACK* current = track;
    TRACK* next;

    while(current != NULL){
        debug_print("Deleting track %d \n", current->track_id);
        next = current->next;
        free(current);
        current = next;
    }
}

void resetList(){
    deleteList(head->next);
    waiting = NULL;
    head->next = NULL;
    start = 0;
}

void completeTrack(int track_num, float time){
    TRACK* pointer = head->next;
    while (pointer != NULL){
        if (pointer->track_id == track_num){
            pointer->time = time;
            break;
        }
        pointer = pointer->next;
    }
}

int addTrackCar(int track_num, char* car_id){
    debug_print("Adding track %d: %s \n", track_num, car_id);

    //Check if this track exists already
    TRACK* pointer = head->next;
    int exists = 0;
    while(pointer != NULL){
        if (pointer->track_id == track_num){
            debug_print("Track exists.");
            exists = 1;
            break;
        }
        pointer = pointer->next;
    }
    
    if (!exists){
        if (start == 0){
            //If the track does not exist, add it to the linked list of tracks
            TRACK* newTrack = (TRACK*) malloc(sizeof(TRACK));
            newTrack->track_id = track_num;
            newTrack->time = 0;
            newTrack->car_id = car_id;
            newTrack->next = NULL;

            if (head->next != NULL){
                newTrack->next = head->next;
            } 
            head->next = newTrack;
            return 1;
        } else {
            debug_print("Error, track not registered before start of race. \n");
            return -1;
        }
    }
}


int parseRegistration(char* cmd){
    debug_print("Parsing %s\n", cmd);
    int track_num;


    char* id = NULL, *tmp = NULL;
    size_t size = 0, index = 0;
    int i = 0;
    
    //Loop through the URL until we hit the 'K' character
    while (cmd[i] != 'K' && i != strlen(cmd)){
        i++;
    }

    if (cmd[i] == 'K'){
        i++;

        track_num = cmd[i] - '0';

        debug_print("ID of track is: %d \n", track_num);
        
        i++;

        //Check if the 'id=' keyword exists
        if (cmd[i] == '-' && cmd[i + 1] == 'I' && cmd[i + 2] == 'D' && cmd[i+3] == '='){
            //Advance the i variable forward so that we start from after the 'ID=' keyword
            i += 4;
        } else{
            debug_print("Error. No 'id=' keyword in URL \n");
            return -1;
        }
    } else {
        debug_print("Error. No 'K' character in URL \n");
        return -1;
    }

    
    while(i != strlen(cmd)){
        if (size <= index) {
            size += 1;
            tmp = realloc(id, size);
            if (!tmp) {
                free(id);
                id = NULL;
                break;
            }
            id = tmp;
        }
        id[index++] = cmd[i]; 
        i++;
    }

    //Add a null terminator to the end of "id"
    size += 1;
    tmp = realloc(id, size);
    if (!tmp) {
        free(id);
        id = NULL;
        return -1;
    }
    id = tmp;
    id[index++] = '\0'; 


    addTrackCar(track_num, id);

    return 0;
}

int addTrack(char* cmd){

    char* id = NULL, *tmp = NULL;
    size_t size = 0, index = 0;
    int i = 0;


    //Parse through the input until we get to the '-' symbol
    while (i != strlen(cmd)){
        if ( cmd[i] == '-'){
                i++;
            break;
        }
        i++;
    }

    int track_num = cmd[i] - '0';

    //Check if this track exists already
    TRACK* pointer = head->next;
    int exists = 0;
    while(pointer != NULL){
        if (pointer->track_id == track_num){
            debug_print("Track exists.");
            //If the track exists and there is no race going on, we set this track to be the next track to be assigned a car ID
            if (start == 0){
                if (pointer->car_id == NULL){
                    waiting = pointer;
                }
            }
            exists = 1;
            break;
        }
        pointer = pointer->next;
    }
    
    if (!exists){
        if (start == 0){
            //If the track does not exist, add it to the linked list of tracks
            TRACK* newTrack = (TRACK*) malloc(sizeof(TRACK));
            newTrack->track_id = track_num;
            newTrack->time = 0;
            newTrack->car_id = NULL;
            newTrack->next = NULL;

            if (head->next != NULL){
                newTrack->next = head->next;
            } 
            head->next = newTrack;
            waiting = newTrack;
            return 1;
        } else {
            debug_print("Error, track not registered before start of race. \n");
            return -1;
        }
    }
    return 1;
}

int parseURL(char* url){
    char* id = NULL, *tmp = NULL;
    size_t size = 0, index = 0;
    int i = 0;
    //Loop through the URL until we hit the '?' character
    while (url[i] != '?' && i != strlen(url)){
        i++;
    }

    if (url[i] == '?'){
        //Check if the 'id=' keyword exists
        if (url[i+1] == 'i' && url[i + 2] == 'd' && url[i + 3] == '='){
            //Advance the i variable forward so that we start from after the 'id=' keyword
            i += 4;
        } else{
            debug_print("Error. No 'id=' keyword in URL \n");
            return -1;
        }
    } else {
        debug_print("Error. No '?' character in URL \n");
        return -1;
    }

    //Keep parsing the URL until we hit the '&' character
    //Add every character to the variable "id". We adjust the size of "id" with every character added.
    while(url[i] != '&' && i != strlen(url)){
        if (size <= index) {
            size += 1;
            tmp = realloc(id, size);
            if (!tmp) {
                free(id);
                id = NULL;
                break;
            }
            id = tmp;
        }
        id[index++] = url[i]; 
        i++;
    }

    //Add a null terminator to the end of "id"
    size += 1;
    tmp = realloc(id, size);
    if (!tmp) {
        free(id);
        id = NULL;
        return -1;
    }
    id = tmp;
    id[index++] = '\0'; 

    //Assign the car id to whichever track is currently waiting for one.
    if (waiting != NULL){
        waiting->car_id = id;
        debug_print("Car with id %s now registered to Track %d \n", waiting->car_id, waiting->track_id);
    } else {
        debug_print("Error. Scan track first before scanning car.");
    }
    return 0;
}