#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <time.h>
#include "info_jadwal.h"
#include "pengelolaanData.h"
#include "helpers.h"
#include "cJSON.h"

//Synchronization object for protecting data.csv file access, preventing segmentation fault/sudden crash
static CRITICAL_SECTION g_file_access_cs;

//Helper function to safely get json's string value
const char* get_string_value_safe(cJSON *item, const char* default_val){
    return (item != NULL && cJSON_IsString(item)) ? item->valuestring : default_val;
}

//Helper function to safely get json's number value as int
int get_int_value_safe(cJSON *item, int default_val){
    return (item != NULL && cJSON_IsNumber(item)) ? (int)cJSON_GetNumberValue(item) : default_val;
}
// -------------------------------------------------------------------------

// Menambahkan node di akhir list
static void insertTail(NodeData **head, NodeData **tail, Data data){
    NodeData *baru = (NodeData *)malloc(sizeof(NodeData));
    //Check if allocation is possible, otherwise return error msg
    if (baru == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for new NodeData in insertTail.\n");
        return;
    }
    baru->data_dokter = data;
    baru->next = NULL;
    if (*head == NULL) {
        *head = *tail = baru;
    } else {
        (*tail)->next = baru;
        *tail = baru;
    }
}

// Menghapus node dari list
static void removeNode(NodeData **head, NodeData **tail, NodeData *prev, NodeData *curr){
    if (prev == NULL) {
        *head = curr->next;
        if (*tail == curr) *tail = NULL;
    } else {
        prev->next = curr->next;
        if (*tail == curr) *tail = prev;
    }
    //free the temp curr node, as it's now deleted
    free(curr);
}

// Fungsi untuk membaca file CSV kemudian dimasukkan ke linked list
void readCSV(NodeData **head, NodeData **tail, FILE *fptr){
    Data temp;
    char line[512];
    char *tok;
    char *next_token; //For strtok_s

    //Clear existing list before reading, avoiding duplicates
    while(*head){
        NodeData *tmp = *head;
        *head = (*head)->next;
        free(tmp);
    }
    *tail = NULL;

    while(fgets(line, sizeof(line), fptr)){
        line[strcspn(line, "\n")] = 0; //Remove newline char

        //Parse the contents
        tok = strtok_s(line, ";", &next_token);
        if (!tok) continue;
        strncpy(temp.nama, tok, sizeof(temp.nama) - 1);
        temp.nama[sizeof(temp.nama) - 1] = '\0';

        tok = strtok_s(NULL, ";", &next_token);
        if (!tok) continue;
        strncpy(temp.tanggal_lahir, tok, sizeof(temp.tanggal_lahir) - 1);
        temp.tanggal_lahir[sizeof(temp.tanggal_lahir) - 1] = '\0';

        tok = strtok_s(NULL, ";", &next_token);
        if (!tok) continue;
        strncpy(temp.no_telp, tok, sizeof(temp.no_telp) - 1);
        temp.no_telp[sizeof(temp.no_telp) - 1] = '\0';

        tok = strtok_s(NULL, ";", &next_token);
        if (!tok) continue;
        strncpy(temp.alamat, tok, sizeof(temp.alamat) - 1);
        temp.alamat[sizeof(temp.alamat) - 1] = '\0';

        tok = strtok_s(NULL, ";", &next_token);
        if (!tok) continue;
        strncpy(temp.spesialisasi, tok, sizeof(temp.spesialisasi) - 1);
        temp.spesialisasi[sizeof(temp.spesialisasi) - 1] = '\0';

        tok = strtok_s(NULL, ";", &next_token);
        if (!tok) continue;
        temp.maks_shift = atoi(tok);

        tok = strtok_s(NULL, "\n", &next_token); //"\n" delimiter indicating last char of the line
        if (!tok) {
            temp.pref_shift[0] = '\0'; //add null-terminator
        } else {
            strncpy(temp.pref_shift, tok, sizeof(temp.pref_shift) - 1);
            temp.pref_shift[sizeof(temp.pref_shift) - 1] = '\0';
        }
        //Insert it to linked list's tail
        insertTail(head, tail, temp);
    }
}

// Menulis linked list ke file CSV (hapusDataDokter & updateDataDokter)
void writeCSV(NodeData **head, FILE *fptr){
    NodeData *curr = *head;
    while (curr) {
        fprintf(fptr, "%s;%s;%s;%s;%s;%d;%s\n",
            curr->data_dokter.nama,
            curr->data_dokter.tanggal_lahir,
            curr->data_dokter.no_telp,
            curr->data_dokter.alamat,
            curr->data_dokter.spesialisasi,
            curr->data_dokter.maks_shift,
            curr->data_dokter.pref_shift);
        curr = curr->next;
    }
}

// Helper function to write from list to csv (tambahDataDokter)
void writeCSV_from_list(NodeData *head, FILE *fptr){
    NodeData *curr = head;
    while (curr) {
        fprintf(fptr, "%s;%s;%s;%s;%s;%d;%s\n",
            curr->data_dokter.nama,
            curr->data_dokter.tanggal_lahir,
            curr->data_dokter.no_telp,
            curr->data_dokter.alamat,
            curr->data_dokter.spesialisasi,
            curr->data_dokter.maks_shift,
            curr->data_dokter.pref_shift);
        curr = curr->next;
    }
}

int isValidShift(const char *shift){
    return strcasecmp(shift, "pagi") == 0 || strcasecmp(shift, "siang") == 0 || strcasecmp(shift, "malam") == 0;
}

int isDuplicate(NodeData *head, Data *newData){
    while (head) {
        if (strcasecmp(head->data_dokter.nama, newData->nama) == 0 &&
            strcasecmp(head->data_dokter.tanggal_lahir, newData->tanggal_lahir) == 0 &&
            strcasecmp(head->data_dokter.no_telp, newData->no_telp) == 0 &&
            strcasecmp(head->data_dokter.alamat, newData->alamat) == 0 &&
            strcasecmp(head->data_dokter.spesialisasi, newData->spesialisasi) == 0 &&
            head->data_dokter.maks_shift == newData->maks_shift &&
            strcasecmp(head->data_dokter.pref_shift, newData->pref_shift) == 0) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

void freeLL(NodeData *head){
    while (head){
        NodeData *tmp = head;
        head = head->next;
        free(tmp);
    }
}

//Fungsi untuk menambah data dokter
int tambahDataDokter(const char *file, Data *newDokter) {
    EnterCriticalSection(&g_file_access_cs);

    NodeData *head = NULL, *tail = NULL;
    FILE *fptr = fopen(file, "r");
    //If file exists, start reading/extracting the data
    if(fptr){
        readCSV(&head, &tail, fptr);
        fclose(fptr);
    }
    else{
        //Print error msg on not found
        printf("C (pengelolaanData): Data file '%s' not found for reading. Starting with empty list.\n", file);
    }

    //Check if there's any duplicate of newDokter data and the data previously saved
    if(isDuplicate(head, newDokter)){
        //Print error message and free the initiated linked list, then return 0
        printf("C (pengelolaanData): Data dokter sudah ada. Tidak ditambahkan.\n");
        freeLL(head);
        LeaveCriticalSection(&g_file_access_cs);
        return 0; //Indicate failure (cause: duplicate)
    }
    else{
        //Otherwise, insert it into the tail of the linked list
        insertTail(&head, &tail, *newDokter);

        //Do atomic writing to prevent access error
        char temp_file_path[MAX_PATH];
        snprintf(temp_file_path, sizeof(temp_file_path), "%s.tmp", file);

        FILE *f_temp = fopen(temp_file_path, "w");
        //Check if creation/opening of the temp file is successful
        if(!f_temp){
            //Print error message and free linked list, then return 0
            fprintf(stderr, "C (pengelolaanData): Error: Could not open temporary file '%s' for writing (tambahDataDokter). Error: %lu\n", temp_file_path, GetLastError());
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; //Indicate failure
        }

        //If it's successful, then write it to the CSV (temp) and close the temp file
        writeCSV_from_list(head, f_temp);
        fclose(f_temp);
        
        //Check if possible to move file (replace)
        if(MoveFileExA(temp_file_path, file, MOVEFILE_REPLACE_EXISTING)){
            //On success, free the linked list, leave crit section and return 1 (success code for main.c to know)
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 1; //Indicate success
        }
        else{
            //On failure, print error message, delete the previous atomically created temp file, free linked list, and return 0 for failure
            fprintf(stderr, "C (pengelolaanData): Error: Gagal menyimpan data atomik ke '%s' (tambahDataDokter). Error: %lu\n", file, GetLastError());
            DeleteFileA(temp_file_path);
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; //Indicate failure
        }
    }
}

// Fungsi untuk menampilkan semua data (Atomic Read with Critical Section)
cJSON* tampilkanSemua(const char *doctorFilePath, const char *scheduleFilePath){
    EnterCriticalSection(&g_file_access_cs);

    NodeData *head = NULL, *tail = NULL;
    FILE *f = fopen(doctorFilePath, "r");
    if(!f){
        //Check if file is not found, return empty json array
        printf("C (pengelolaanData): File '%s' tidak ditemukan. Mengembalikan data kosong.\n", doctorFilePath);
        LeaveCriticalSection(&g_file_access_cs);
        return cJSON_CreateArray();
    }
    
    //Read CSV file and extract it to the linked list
    readCSV(&head, &tail, f);
    //Close the file
    fclose(f);

    //If reading failed
    if(!head){
        //Print error msg, and return empty json array
        printf("C (pengelolaanData): Data kosong dari file '%s'.\n", doctorFilePath);
        LeaveCriticalSection(&g_file_access_cs);
        return cJSON_CreateArray();
    }

    //Create doctors_array
    cJSON *doctors_array = cJSON_CreateArray();
    if(doctors_array == NULL){
        //Check if creation of the array is successful, otherwise print error msg and return NULL
        fprintf(stderr, "C (pengelolaanData): Error: Gagal membuat cJSON array untuk dokter.\n");
        freeLL(head);
        LeaveCriticalSection(&g_file_access_cs);
        return NULL;
    }

    NodeData *curr = head;
    int docs_per_shifts[3] = {0,0,0}; //0-Init
    //Read the schedule from CSV and extract it as array of Jadwal
    Jadwal *jadwal_dokter = read_schedule_csv_to_jadwal_array(scheduleFilePath, docs_per_shifts);
    if(jadwal_dokter == NULL){
        //If the array is NULL, print error msg
        fprintf(stderr, "C (tampilkanSemua): Warning: Failed to load schedule data from %s. Doctor shift counts and violation counts will be 0.\n", scheduleFilePath);
    }

    int i = 0;
    //Loop all nodes in the linked list
    while(curr){
        cJSON *doctor_obj = cJSON_CreateObject();
        //Check if the created doctor's object is NULL, then print error msg, and clear all temps/LLs, returning with NULL
        if(doctor_obj == NULL){
            fprintf(stderr, "C (pengelolaanData): Error: Gagal membuat cJSON object untuk dokter %s.\n", curr->data_dokter.nama);
            cJSON_Delete(doctors_array);
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return NULL;
        }

        //Create the JSON object data from the extracted data (from file)
        cJSON_AddStringToObject(doctor_obj, "name", curr->data_dokter.nama);
        cJSON_AddStringToObject(doctor_obj, "birthdate", curr->data_dokter.tanggal_lahir);
        cJSON_AddStringToObject(doctor_obj, "phone", curr->data_dokter.no_telp);
        cJSON_AddStringToObject(doctor_obj, "address", curr->data_dokter.alamat);
        cJSON_AddStringToObject(doctor_obj, "specialty", curr->data_dokter.spesialisasi);
        cJSON_AddNumberToObject(doctor_obj, "max_shift", curr->data_dokter.maks_shift);
        cJSON_AddStringToObject(doctor_obj, "shift_preference", curr->data_dokter.pref_shift);

        //Default values 0 (or not scheduled yet)
        int curr_doc_allocated_shift = 0;
        int curr_doc_pref_violation = 0;
        
        //If scheduling was done before and start date exists, making jadwal_dokter valid, then add the corresponding data to the doctors (their allocated shift and number of shift preference violation)
        if(jadwal_dokter){
            curr_doc_allocated_shift = countShift(curr->data_dokter.nama, jadwal_dokter, docs_per_shifts);
            curr_doc_pref_violation = jumlahPelanggaran(curr->data_dokter.nama, curr->data_dokter.pref_shift, jadwal_dokter, docs_per_shifts);
        }
        else{
            //Print error message if there's still no schedule 
            printf("C (tampilkanSemua): Schedule not loaded, counts for %s set to 0.\n", curr->data_dokter.nama);
        }

        cJSON_AddNumberToObject(doctor_obj, "allocated_shift", curr_doc_allocated_shift);
        cJSON_AddNumberToObject(doctor_obj, "preference_violation", curr_doc_pref_violation);
        cJSON_AddStringToObject(doctor_obj, "photo", "user.png");

        cJSON_AddItemToArray(doctors_array, doctor_obj);
        curr = curr->next;
        i++;
    }

    //Clear the linked list and arrays
    freeLL(head);
    LeaveCriticalSection(&g_file_access_cs);
    if(jadwal_dokter){ free_jadwal_array(jadwal_dokter, 30, docs_per_shifts); }
    return doctors_array;
}

//Function to update doctor's data by comparing the doctor's name
int updateDataDokter(const char *file, const char *original_name, const Data *updatedDokter) {
    EnterCriticalSection(&g_file_access_cs);

    NodeData *head = NULL, *tail = NULL;
    FILE *fptr = fopen(file, "r");
    if(fptr){
        //On successfully opening the file, start reading/extracting then close it
        readCSV(&head, &tail, fptr);
        fclose(fptr);
    }
    else{
        //If data doesn't exist or opening failed, then return 0 on failure
        fprintf(stderr, "C (pengelolaanData): Error: Data file '%s' not found for update.\n", file);
        LeaveCriticalSection(&g_file_access_cs);
        return 0; // Failure
    }

    NodeData *curr = head;
    int found = 0;
    while(curr){
        if(strcasecmp(curr->data_dokter.nama, original_name) == 0){
            strncpy(curr->data_dokter.nama, updatedDokter->nama, sizeof(curr->data_dokter.nama) - 1);
            curr->data_dokter.nama[sizeof(curr->data_dokter.nama) - 1] = '\0';

            strncpy(curr->data_dokter.tanggal_lahir, updatedDokter->tanggal_lahir, sizeof(curr->data_dokter.tanggal_lahir) - 1);
            curr->data_dokter.tanggal_lahir[sizeof(curr->data_dokter.tanggal_lahir) - 1] = '\0';

            strncpy(curr->data_dokter.no_telp, updatedDokter->no_telp, sizeof(curr->data_dokter.no_telp) - 1);
            curr->data_dokter.no_telp[sizeof(curr->data_dokter.no_telp) - 1] = '\0';
            
            strncpy(curr->data_dokter.alamat, updatedDokter->alamat, sizeof(curr->data_dokter.alamat) - 1);
            curr->data_dokter.alamat[sizeof(curr->data_dokter.alamat) - 1] = '\0';
            
            strncpy(curr->data_dokter.spesialisasi, updatedDokter->spesialisasi, sizeof(curr->data_dokter.spesialisasi) - 1);
            curr->data_dokter.spesialisasi[sizeof(curr->data_dokter.spesialisasi) - 1] = '\0';

            curr->data_dokter.maks_shift = updatedDokter->maks_shift;
            
            strncpy(curr->data_dokter.pref_shift, updatedDokter->pref_shift, sizeof(curr->data_dokter.pref_shift) - 1);
            curr->data_dokter.pref_shift[sizeof(curr->data_dokter.pref_shift) - 1] = '\0';

            found = 1;
            break;
        }
        curr = curr->next;
    }

    if(found){
        char temp_file_path[MAX_PATH];
        snprintf(temp_file_path, sizeof(temp_file_path), "%s.tmp", file);

        FILE *f_temp = fopen(temp_file_path, "w");
        //Check if atomic write to tmp file is possible, otherwise return 0 on failure
        if(!f_temp){
            fprintf(stderr, "C (pengelolaanData): Error: Could not open temporary file for update write: %s. Error: %lu\n", temp_file_path, GetLastError());
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; // Failure
        }
        printf("C (pengelolaanData): Writing updated list to temporary file.\n");
        writeCSV_from_list(head, f_temp);
        fclose(f_temp);
        printf("C (pengelolaanData): Finished writing to temporary file.\n");

        if(MoveFileExA(temp_file_path, file, MOVEFILE_REPLACE_EXISTING)){
            //Notify on success
            printf("C (pengelolaanData): Doctor '%s' updated and saved atomically.\n", original_name);
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 1; // Success
        }
        else{
            //Print error message, delete atomically created temp file, and clean the temps
            fprintf(stderr, "C (pengelolaanData): Error: Failed to atomically replace file after update. Error: %lu\n", GetLastError());
            DeleteFileA(temp_file_path);
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; // Failure
        }
    }
    else{
        //Print error message
        printf("C (pengelolaanData): Doctor '%s' not found for update.\n", original_name);
        freeLL(head);
        LeaveCriticalSection(&g_file_access_cs);
        return 0; // Failure
    }
}

//Fungsi untuk menghapus data dokter, menerima pula nama yang akan dihapus
int hapusDataDokter(const char *file, const char *name_to_delete) {
    EnterCriticalSection(&g_file_access_cs);

    NodeData *head = NULL, *tail = NULL;
    FILE *f = fopen(file, "r");
    if(!f){
        //Print error message and return 0 on failing to open file
        printf("C (pengelolaanData): File '%s' not found for deletion. Error: %lu\n", file, GetLastError());
        LeaveCriticalSection(&g_file_access_cs);
        return 0; // Failure
    }
    //If file opening succeeds, then proceed to read/extract data
    readCSV(&head, &tail, f);
    fclose(f);

    NodeData *curr = head, *prev = NULL;
    int found = 0;
    NodeData *next_node = NULL;

    //Loop through all doctors data in the linked list
    while(curr){
        //Check to find doctor's data by comparing the names, if found then remove the node from the linked list
        if(strcasecmp(curr->data_dokter.nama, name_to_delete) == 0) {
            next_node = curr->next;
            removeNode(&head, &tail, prev, curr);
            curr = next_node;
            found = 1;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }

    if(found){
        //Proceed to rewrite the file atomically
        char temp_file_path[MAX_PATH];
        snprintf(temp_file_path, sizeof(temp_file_path), "%s.tmp", file);

        FILE *fw = fopen(temp_file_path, "w");
        if(!fw){
            //Print error message on failure to open the temp file
            fprintf(stderr, "C (pengelolaanData): Error: Could not open temporary file for delete write: %s. Error: %lu\n", temp_file_path, GetLastError());
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; // Failure
        }
        //Write to the file
        writeCSV(&head, fw);
        fclose(fw);
        
        //Replace original csv with the temp file
        if(MoveFileExA(temp_file_path, file, MOVEFILE_REPLACE_EXISTING)){
            //Clear temps and return 1 on success
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 1; // Success
        }
        else{
            //Print error msg, delete the temp file, and clear temps before returning 0 on failure
            fprintf(stderr, "C (pengelolaanData): Error: Failed to atomically replace file after delete. Error: %lu\n", GetLastError());
            DeleteFileA(temp_file_path);
            freeLL(head);
            LeaveCriticalSection(&g_file_access_cs);
            return 0; // Failure
        }
    }
    else{
        //In case the doctor is not found at all, then print error msg and clear all temps, returning 0 on failure
        printf("C (pengelolaanData): Doctor '%s' not found for deletion.\n", name_to_delete);
        freeLL(head); // Still free the list even if not found
        LeaveCriticalSection(&g_file_access_cs);
        return 0; // Failure
    }
}

//Helper function to get difference from target_date to start_date (target_date is nth-date from start_date)
int get_nth_day(const char* start_date_str, const char* target_date_str){
    struct tm tm_start = {0}; //Init 0
    struct tm tm_target = {0}; //Init 0

    int d, m, y;

    //Extract from DD-MM-YYYY format
    if (sscanf(start_date_str, "%d-%d-%d", &d, &m, &y) != 3) {
        fprintf(stderr, "Error: Invalid start date format '%s'. Expected DD-MM-YYYY.\n", start_date_str);
        return -1;
    }
    tm_start.tm_mday = d;
    tm_start.tm_mon = m - 1;
    tm_start.tm_year = y - 1900;
    tm_start.tm_isdst = -1;

    time_t time_start = mktime(&tm_start);
    if (time_start == (time_t)-1) {
        fprintf(stderr, "Error: Could not convert start date '%s' to time_t.\n", start_date_str);
        return -1;
    }

    if (sscanf(target_date_str, "%d-%d-%d", &d, &m, &y) != 3) {
        fprintf(stderr, "Error: Invalid target date format '%s'. Expected DD-MM-YYYY.\n", target_date_str);
        return -1;
    }
    tm_target.tm_mday = d;
    tm_target.tm_mon = m - 1;
    tm_target.tm_year = y - 1900;
    tm_target.tm_isdst = -1;

    time_t time_target = mktime(&tm_target);
    if (time_target == (time_t)-1) {
        fprintf(stderr, "Error: Could not convert target date '%s' to time_t.\n", target_date_str);
        return -1;
    }

    double diff_seconds = difftime(time_target, time_start);

    long long diff_days_ll = (long long)(diff_seconds / (24 * 60 * 60));

    if (diff_days_ll < 0) {
        fprintf(stderr, "Warning: Target date '%s' is before start date '%s'.\n", target_date_str, start_date_str);
        return -2;
    }
    return (int)diff_days_ll + 1;
}

static cJSON* create_daily_shift_json(int nth_day, const char* pagi_csv_str, const char* siang_csv_str, const char* malam_csv_str) {
    cJSON *day_obj = cJSON_CreateObject();
    if (!day_obj) return NULL;

    cJSON_AddItemToObject(day_obj, "nth_day", cJSON_CreateNumber(nth_day));

    cJSON *shifts_obj = cJSON_CreateObject();
    if (!shifts_obj) { cJSON_Delete(day_obj); return NULL; }

    cJSON *pagi_array = cJSON_CreateArray();
    cJSON *siang_array = cJSON_CreateArray();
    cJSON *malam_array = cJSON_CreateArray();
    if (!pagi_array || !siang_array || !malam_array) {
        cJSON_Delete(pagi_array); cJSON_Delete(siang_array); cJSON_Delete(malam_array);
        cJSON_Delete(shifts_obj); cJSON_Delete(day_obj);
        return NULL;
    }

    char *name_token;
    char *context_str;
    char temp_buf[5100];

    //Morning shift
    strncpy(temp_buf, pagi_csv_str, sizeof(temp_buf) - 1);
    temp_buf[sizeof(temp_buf) - 1] = '\0';
    context_str = NULL;
    if (strlen(temp_buf) > 0) {
        name_token = strtok_s(temp_buf, ",", &context_str);
        while (name_token != NULL) {
            cJSON_AddItemToArray(pagi_array, cJSON_CreateString(name_token));
            name_token = strtok_s(NULL, ",", &context_str);
        }
    }

    //Noon shift
    strncpy(temp_buf, siang_csv_str, sizeof(temp_buf) - 1);
    temp_buf[sizeof(temp_buf) - 1] = '\0';
    context_str = NULL;
    if (strlen(temp_buf) > 0) {
        name_token = strtok_s(temp_buf, ",", &context_str);
        while (name_token != NULL) {
            cJSON_AddItemToArray(siang_array, cJSON_CreateString(name_token));
            name_token = strtok_s(NULL, ",", &context_str);
        }
    }

    //Night shift
    strncpy(temp_buf, malam_csv_str, sizeof(temp_buf) - 1);
    temp_buf[sizeof(temp_buf) - 1] = '\0';
    context_str = NULL;
    if (strlen(temp_buf) > 0) {
        name_token = strtok_s(temp_buf, ",", &context_str);
        while (name_token != NULL) {
            cJSON_AddItemToArray(malam_array, cJSON_CreateString(name_token));
            name_token = strtok_s(NULL, ",", &context_str);
        }
    }
    
    cJSON_AddItemToObject(shifts_obj, "pagi", pagi_array);
    cJSON_AddItemToObject(shifts_obj, "siang", siang_array);
    cJSON_AddItemToObject(shifts_obj, "malam", malam_array);
    
    cJSON_AddItemToObject(day_obj, "shifts", shifts_obj);
    return day_obj;
}

cJSON* get_schedule_data(const char *filepath, int mode, int nth_day_target, int nth_week_target) {
    FILE *file = fopen(filepath, "r");
    char line[15100];

    if (!file) {
        perror("C: Failed to open schedule CSV file for reading (get_schedule_data)");
        return NULL; // Return NULL on file open error
    }

    cJSON *result_json = NULL;

    if (mode == 0) { // Daily Schedule
        printf("C: get_schedule_data: Mode 0 (Daily) requested for Nth Day: %d\n", nth_day_target);
        if (nth_day_target <= 0 || nth_day_target > 30) {
            fprintf(stderr, "C: Invalid nth_day (%d) for daily schedule. Must be between 1 and 30.\n", nth_day_target);
            fclose(file);
            return NULL;
        }

        char pagi_buf[5000];
        char siang_buf[5000];
        char malam_buf[5000];
        int nth_day;
        int current_line_num = 0;

        while (fgets(line, sizeof(line), file)) {
            current_line_num++;
            if (current_line_num == nth_day_target) { // This is the line we're interested in
                int scan_count = sscanf(line, "%d;%5000[^;];%5000[^;];%5000[^\n]", 
                                        &nth_day, pagi_buf, siang_buf, malam_buf);
                if (scan_count >= 4) {
                    result_json = create_daily_shift_json(nth_day, pagi_buf, siang_buf, malam_buf);
                    printf("C: Found daily schedule for day %d.\n", nth_day);
                } else {
                    fprintf(stderr, "C: Malformed line at nth_day %d: %s\n", nth_day_target, line);
                }
                break; // Found our day, exit loop
            }
        }
        if (!result_json) {
            fprintf(stderr, "C: No daily schedule data found for nth_day %d.\n", nth_day_target);
        }

    } 
    else if (mode == 1) { // Weekly Schedule
        printf("C: get_schedule_data: Mode 1 (Weekly) requested for Nth Week: %d\n", nth_week_target);
        if (nth_week_target <= 0 || nth_week_target > 5) { // 5 weeks max for a 30-day schedule
            fprintf(stderr, "C: Invalid nth_week (%d) for weekly schedule. Must be between 1 and 5.\n", nth_week_target);
            fclose(file);
            return NULL;
        }

        result_json = cJSON_CreateObject();
        cJSON *weekly_data_array_json = cJSON_CreateArray();
        if (!result_json || !weekly_data_array_json) {
            cJSON_Delete(result_json);
            fprintf(stderr, "C: Failed to create JSON objects for weekly result.\n");
            fclose(file);
            return NULL;
        }

        // Determine start and end day offsets (0-indexed from schedule_start_date_str)
        int start_day_offset = (nth_week_target * 7) - 6;
        int end_day_offset = start_day_offset + 6; // Standard week is 7 days
        if (nth_week_target == 5) { // Special case for week 5 (days 29-30)
            end_day_offset = 30;
        }
        
        // Array to hold parsed data for the current week, indexed by tm_wday (0=Sun, 6=Sat)
        typedef struct {
            int nth_day;
            char pagi[5100];
            char siang[5100];
            char malam[5100];
            int is_populated;
        } DayRawShiftData;

        DayRawShiftData temp_weekly_raw_data[7]; // 0=Sun, ..., 6=Sat

        //Init
        for(int i = 0; i < 7; i++) {
            temp_weekly_raw_data[i].is_populated = 0;
            temp_weekly_raw_data[i].nth_day = 0;
            memset(temp_weekly_raw_data[i].pagi, 0, sizeof(temp_weekly_raw_data[i].pagi));
            memset(temp_weekly_raw_data[i].siang, 0, sizeof(temp_weekly_raw_data[i].siang));
            memset(temp_weekly_raw_data[i].malam, 0, sizeof(temp_weekly_raw_data[i].malam));
        }

        char pagi_buf[5000];
        char siang_buf[5000];
        char malam_buf[5000];
        int nth_day;
        int current_line_num = 0; // Corresponds to 1-indexed day in CSV

        while (fgets(line, sizeof(line), file)) {
            current_line_num++; // Now current_line_num is the 1-indexed day from CSV start

            // Check if this line is within our target week range
            if (current_line_num >= start_day_offset && current_line_num <= end_day_offset) {
                int scan_count = sscanf(line, "%d;%5000[^;];%5000[^;];%5000[^\n]", 
                                        &nth_day, pagi_buf, siang_buf, malam_buf);
                if (scan_count >= 4) {
                    int index = current_line_num - start_day_offset;
                    temp_weekly_raw_data[index].is_populated = 1;
                    temp_weekly_raw_data[index].nth_day = nth_day;
                    strncpy(temp_weekly_raw_data[index].pagi, pagi_buf, sizeof(temp_weekly_raw_data[index].pagi) - 1);
                    strncpy(temp_weekly_raw_data[index].siang, siang_buf, sizeof(temp_weekly_raw_data[index].siang) - 1);
                    strncpy(temp_weekly_raw_data[index].malam, malam_buf, sizeof(temp_weekly_raw_data[index].malam) - 1);
                } else {
                    fprintf(stderr, "C: Malformed line in CSV for weekly parse: %s\n", line);
                }
            } else if (current_line_num > end_day_offset) {
                break; // Beyond the target week, stop reading
            }
        }
        
        for (int i = 0; i < 7; i++) { 
            // Calculate the actual Nth Day for this specific slot in the week,
            // based on the week's starting day.
            int actual_nth_day_for_slot = start_day_offset + i;

            // Only add data up to the last day of the 30-day schedule (Day 30)
            if (actual_nth_day_for_slot <= 30) { 
                cJSON *day_obj;

                // Check if this specific day within the week was populated from CSV
                // Also check if the nth_day from temp_weekly_raw_data matches the expected day for this slot.
                // This guards against potential issues if CSV lines are out of order or nth_day doesn't match current_line_num.
                if (temp_weekly_raw_data[i].is_populated && temp_weekly_raw_data[i].nth_day == actual_nth_day_for_slot){
                    day_obj = create_daily_shift_json( 
                        temp_weekly_raw_data[i].nth_day,
                        temp_weekly_raw_data[i].pagi,
                        temp_weekly_raw_data[i].siang,
                        temp_weekly_raw_data[i].malam
                    );
                } else {
                    // If no data found for this specific date (e.g., end of week 5 where there are only 2 days)
                    // create an empty JSON object for this day.
                    day_obj = create_daily_shift_json(actual_nth_day_for_slot, "", "", ""); 
                }

                if (day_obj) {
                    cJSON_AddItemToArray(weekly_data_array_json, day_obj);
                } else {
                    fprintf(stderr, "C: Failed to create JSON for day %d in week %d.\n", actual_nth_day_for_slot, nth_week_target);
                }
            }
            // The `actual_nth_day_for_slot <= 30` condition correctly stops adding days beyond the 30-day schedule.
            // No need for explicit `break` for Week 5 here.
        }

        cJSON_AddItemToObject(result_json, "week_data", weekly_data_array_json);

    } else {
        fprintf(stderr, "C: Invalid mode specified: %d.\n", mode);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return result_json;
}

static Data* parse_and_allocate_doctors_array(const char* input_string, int* num_doctors_out, int expected_total_count) {
    *num_doctors_out = 0;
    
    if(expected_total_count <= 0){
        return NULL;
    }

    Data* doctors_array = (Data*)malloc(expected_total_count * sizeof(Data));
    if(doctors_array == NULL){
        fprintf(stderr, "Error: Failed to allocate memory for doctors_array for expected count %d.\n", expected_total_count);
        return NULL;
    }

    // Initialize all 0
    for(int k = 0; k < expected_total_count; ++k){
        memset(doctors_array[k].nama, 0, sizeof(doctors_array[k].nama));
        memset(doctors_array[k].tanggal_lahir, 0, sizeof(doctors_array[k].tanggal_lahir));
        memset(doctors_array[k].no_telp, 0, sizeof(doctors_array[k].no_telp));
        memset(doctors_array[k].alamat, 0, sizeof(doctors_array[k].alamat));
        memset(doctors_array[k].spesialisasi, 0, sizeof(doctors_array[k].spesialisasi));
        doctors_array[k].maks_shift = 0;
        memset(doctors_array[k].pref_shift, 0, sizeof(doctors_array[k].pref_shift));
    }

    if(input_string == NULL || strlen(input_string) == 0){
        return doctors_array;
    }

    char temp_str[5000];
    strncpy(temp_str, input_string, sizeof(temp_str) - 1);
    temp_str[sizeof(temp_str) - 1] = '\0';

    char *name_token;
    char *context = NULL;
    int current_doctor_idx = 0;

    name_token = strtok_s(temp_str, ",", &context);
    while(name_token != NULL && current_doctor_idx < expected_total_count){
        trim_and_normalize_whitespace(name_token);
        
        if (strlen(name_token) > 0) {
            strncpy(doctors_array[current_doctor_idx].nama, name_token, sizeof(doctors_array[current_doctor_idx].nama) - 1);
            doctors_array[current_doctor_idx].nama[sizeof(doctors_array[current_doctor_idx].nama) - 1] = '\0';
            
            current_doctor_idx++;
        }
        name_token = strtok_s(NULL, ",", &context);
    }

    *num_doctors_out = current_doctor_idx;
    
    if (*num_doctors_out != expected_total_count) {
        fprintf(stderr, "WARNING: Parsed %d doctors from CSV string '%s' for a shift, but allocated for %d. Remaining slots in array are empty.\n",
                *num_doctors_out, input_string, expected_total_count);
    }

    return doctors_array;
}

Jadwal* read_schedule_csv_to_jadwal_array(const char *filepath, int docs_per_shifts[3]){
    //Read schedule file
    FILE *file = fopen(filepath, "r");
    //If file doesn't exist or invalid then return NULL
    if(!file){
        docs_per_shifts[0] = 0; docs_per_shifts[1] = 0; docs_per_shifts[2] = 0;
        return NULL;
    }

    //Allocate memory for jadwal array
    Jadwal* jadwal_array = (Jadwal*)malloc(30 * sizeof(Jadwal));
    if(jadwal_array == NULL){
        //If failed to allocate memory, print error message and close file, and return NULL
        fprintf(stderr, "Error: Failed to allocate memory for Jadwal array.\n");
        fclose(file);
        docs_per_shifts[0] = 0; docs_per_shifts[1] = 0; docs_per_shifts[2] = 0;
        return NULL;
    }

    //Initialize all pointers to NULL for 30 days schedule
    for(int i = 0; i < 30; ++i){
        jadwal_array[i].hari = 0;
        jadwal_array[i].shift_pagi = NULL;
        jadwal_array[i].shift_siang = NULL;
        jadwal_array[i].shift_malam = NULL;
    }
    //Init the number of doctor personnels
    docs_per_shifts[0] = 0; docs_per_shifts[1] = 0; docs_per_shifts[2] = 0;

    char line[15100]; //Assuming max of 40-50 docs per shift

    //Get docs_per_shift from first line, use it assumption
    long first_data_line_pos = ftell(file);
    if (fgets(line, sizeof(line), file)) {
        int nth_day_read;
        char pagi_csv_str[5000];
        char siang_csv_str[5000];
        char malam_csv_str[5000];

        memset(pagi_csv_str, 0, sizeof(pagi_csv_str));
        memset(siang_csv_str, 0, sizeof(siang_csv_str));
        memset(malam_csv_str, 0, sizeof(malam_csv_str));
        
        int scan_count = sscanf(line, "%d;%5000[^;];%5000[^;];%5000[^\n]", 
                                &nth_day_read, pagi_csv_str, siang_csv_str, malam_csv_str);

        if(scan_count >= 4 && nth_day_read == 1){ //Check if it's line/day 1 to use as assumption
            docs_per_shifts[0] = count_tokens(pagi_csv_str);
            docs_per_shifts[1] = count_tokens(siang_csv_str);
            docs_per_shifts[2] = count_tokens(malam_csv_str);
        }
        else{
            //Print error message if data is malformed
            fprintf(stderr, "Warning: First data line in CSV is malformed or not Day 1. docs_per_shifts will remain 0. Line: %s\n", line);
        }
    }
    else{
        //If there's no valid data on the first line, output error msg
        fprintf(stderr, "C: No data lines found after header in schedule CSV.\n");
        //Free the array and close file
        free(jadwal_array);
        fclose(file);
        return NULL; // Return NULL if no data lines
    }

    //Get the rest of the data
    //Return file pointer to the start
    fseek(file, first_data_line_pos, SEEK_SET);

    //Start filling data into array from index 0 (1st day)
    int current_array_idx = 0;
    while(fgets(line, sizeof(line), file) && current_array_idx < 30){
        int nth_day_read;
        char pagi_csv_str[5000];
        char siang_csv_str[5000];
        char malam_csv_str[5000];

        memset(pagi_csv_str, 0, sizeof(pagi_csv_str));
        memset(siang_csv_str, 0, sizeof(siang_csv_str));
        memset(malam_csv_str, 0, sizeof(malam_csv_str));
        
        int scan_count = sscanf(line, "%d;%5000[^;];%5000[^;];%5000[^\n]", 
                                &nth_day_read, pagi_csv_str, siang_csv_str, malam_csv_str);

        if(scan_count >= 4 && nth_day_read >= 1 && nth_day_read <= 30){ 
            Jadwal* current_jadwal = &jadwal_array[nth_day_read - 1];
            current_jadwal->hari = nth_day_read;

            int num_pagi_actual_parsed = 0;
            int num_siang_actual_parsed = 0;
            int num_malam_actual_parsed = 0;

            //Pass the docs_per_shifts values as the expected allocation size for the array
            current_jadwal->shift_pagi = parse_and_allocate_doctors_array(pagi_csv_str, &num_pagi_actual_parsed, docs_per_shifts[0]);
            current_jadwal->shift_siang = parse_and_allocate_doctors_array(siang_csv_str, &num_siang_actual_parsed, docs_per_shifts[1]);
            current_jadwal->shift_malam = parse_and_allocate_doctors_array(malam_csv_str, &num_malam_actual_parsed, docs_per_shifts[2]);
        } 
        else{
            //Print error message if it's going out of range of 30 days
            fprintf(stderr, "Warning: Malformed line in CSV or out of range (line %d, read day %d). Skipping and leaving Jadwal slot empty: %s\n", current_array_idx + 1, nth_day_read, line);
        }
        //Increment the index
        current_array_idx++;
    }

    //Close file and return the scanned array of Jadwal from the file
    fclose(file);
    return jadwal_array;
}

//Function to clear array of Jadwal
void free_jadwal_array(Jadwal* jadwal_array, int size, int docs_per_shifts[3]){
    if(jadwal_array == NULL) return;

    //Loop for 30 days and clear
    for(int i = 0; i < size; ++i){
        if(jadwal_array[i].shift_pagi){
            free(jadwal_array[i].shift_pagi);
            jadwal_array[i].shift_pagi = NULL;
        }
        if(jadwal_array[i].shift_siang){
            free(jadwal_array[i].shift_siang);
            jadwal_array[i].shift_siang = NULL;
        }
        if(jadwal_array[i].shift_malam){
            free(jadwal_array[i].shift_malam);
            jadwal_array[i].shift_malam = NULL;
        }
    }
    //Free the array
    free(jadwal_array);
}

void init_file_manager() {
    InitializeCriticalSection(&g_file_access_cs);
}

void deinit_file_manager() {
    DeleteCriticalSection(&g_file_access_cs);
}
