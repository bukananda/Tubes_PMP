//System Libraries
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

//Webview Libraries
#define WEBVIEW_IMPLEMENTATION
#define WEBVIEW_STATIC
#include "webview_amalgamation.h"

//Mongoose Library
#include "../dependencies/mongoose.h"

//cJSON Library
#include "../dependencies/cJSON.h"

//Backend files & functions
#include "data_types.h"
#include "pengelolaanData.h"
#include "jadwal.h"
#include "info_jadwal.h"

//Import/include parsed to bytes page files
#include "../ui/main-page.h"
#include "../ui/main-script.h"
#include "../ui/schedule-page.h"
#include "../ui/schedule-script.h"
#include "../ui/favicon.h"
#include "../ui/user-icon.h"

//Defining important values
#define STR(x) #x
#define XSTR(x) STR(x)
#define PORT 19248
#define TEMP_DIR_BASE_NAME "RS_Semoga_Sehat_Selalu_Management_System"
#define LOCALHOST_URL "http://127.0.0.1:" XSTR(PORT)

//Buffer & variable for server
char g_temp_dir[MAX_PATH];
static volatile int server_ready = 0;
static volatile int running = 1;

//Data pathS
const char *doctorFilePath = "./data/data_dokter.csv";
const char *scheduleFilePath = "./data/jadwal.csv";
const char *scheduleDateLogPath = "./data/jadwal_tanggal_mulai.log";
const char *schedulePersonnelLogPath = "./data/jadwal_jumlah_personnel.log";

//Write file to temp helper function
void write_file(const char *filename, const unsigned char *data, unsigned int len) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "C: Error: Could not open file for writing: %s (Error: %lu)\n", filename, GetLastError());
        perror("fopen (write_file)");
        return;
    }
    size_t bytes_written = fwrite(data, 1, len, fp);
    if (bytes_written != len) {
        fprintf(stderr, "C: Warning: Mismatched bytes written for %s. Expected %u, wrote %zu.\n", filename, len, bytes_written);
    }
    fclose(fp);
}

//Handle mongoose server connection and directory
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        struct mg_http_serve_opts opts = {.root_dir = (const char *)c->fn_data};
        mg_http_serve_dir(c, hm, &opts);
    }
}

//Run mongoose server
DWORD WINAPI run_server(LPVOID lpParam) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, LOCALHOST_URL, fn, lpParam);
    server_ready = 1;
    while (running) {
        mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);
    printf("C: Mongoose server thread exited.\n");
    return 0;
}

/* --------------------------------- CALLBACK FUNCTIONS --------------------------------- */
void append_doctor_cb(const char *seq, const char *req, void *arg);
void load_doctors_cb(const char *seq, const char *req, void *arg);
void update_doctor_cb(const char *seq, const char *req, void *arg);
void generate_schedule_cb(const char *seq, const char *req, void *arg);
void load_saved_start_date(char *buffer, size_t buff_size);
void get_personnel_per_shift_cb(const char *seq, const char *req, void *arg);
void get_prev_start_date_cb(const char *seq, const char *req, void *arg);
void load_daily_schedule_cb(const char *seq, const char *req, void *arg);
void load_weekly_schedule_cb(const char *seq, const char *req, void *arg);
/* --------------------------------- CALLBACK FUNCTIONS --------------------------------- */

int main(){
    //Initialize the RNG for scheduling
    srand(time(NULL));

    //File Manager initialization
    init_file_manager();
    //Delete Critical Section on exit
    atexit(deinit_file_manager);

    //Set temp path for extracted assets
    char initial_temp_path[MAX_PATH];
    //Check if getting temp path is successful, return 1/exit if failed
    if(GetTempPathA(MAX_PATH, initial_temp_path) == 0){ return 1; }

    //Set the temp path or dir according to the directory name
    snprintf(g_temp_dir, sizeof(g_temp_dir), "%s%s", initial_temp_path, TEMP_DIR_BASE_NAME);

    //Check if directory can be made
    if(!CreateDirectoryA(g_temp_dir, NULL)){
        if(GetLastError() != ERROR_ALREADY_EXISTS){
            //Return 1 and exit if failed generating the temp dir
            return 1;
        }
    }

    // Extract UI assets to temp dir
    char filepath[MAX_PATH];

    snprintf(filepath, sizeof(filepath), "%s\\main.html", g_temp_dir);
    write_file(filepath, main_html, main_html_len);

    snprintf(filepath, sizeof(filepath), "%s\\main-script.js", g_temp_dir);
    write_file(filepath, main_script_js, main_script_js_len);

    snprintf(filepath, sizeof(filepath), "%s\\schedule-page.html", g_temp_dir);
    write_file(filepath, schedule_page_html, schedule_page_html_len);

    snprintf(filepath, sizeof(filepath), "%s\\schedule-script.js", g_temp_dir);
    write_file(filepath, schedule_script_js, schedule_script_js_len);

    snprintf(filepath, sizeof(filepath), "%s\\favicon.png", g_temp_dir);
    write_file(filepath, favicon_png, favicon_png_len);

    snprintf(filepath, sizeof(filepath), "%s\\user.png", g_temp_dir);
    write_file(filepath, user_png, user_png_len);

    //Start Mongoose server in background thread, and pass g_temp_dir as the root_dir
    CreateThread(NULL, 0, run_server, g_temp_dir, 0, NULL);
    //Wait until the server is ready
    while (!server_ready) Sleep(100);

    //Start WebView to display UI
    struct webview *w = (struct webview *)webview_create(0, NULL); //Set to 0, disabling debug tool
    //Check if window is created
    if (!w) {
        running = 0; //Signal Mongoose thread to stop
        return 1; //Exit on error
    }

    //Get window object and set it to maximized mode
    HWND hwnd = (HWND)webview_get_window(w);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);

    //Set window title
    webview_set_title(w, "RS Semoga Sehat Selalu - Management System");
    //Set size on minimum and on default
    webview_set_size(w, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), WEBVIEW_HINT_NONE);
    webview_set_size(w, GetSystemMetrics(SM_CXSCREEN)*0.6, GetSystemMetrics(SM_CYSCREEN)*0.6, WEBVIEW_HINT_MIN);


    //Make sure window.webview object exists before binding
    const char *js_init_script = "window.webview = window.webview || {};";
    webview_init(w, js_init_script);

    //Create Bindings for C functions callable from JS (JS triggered funcs)
    webview_bind(w, "append_doctor", append_doctor_cb, w);
    webview_bind(w, "load_doctors", load_doctors_cb, w);
    webview_bind(w, "get_personnel_per_shift", get_personnel_per_shift_cb, w);
    webview_bind(w, "update_doctor", update_doctor_cb, w);
    webview_bind(w, "generate_schedule", generate_schedule_cb, w);
    webview_bind(w, "get_prev_start_date", get_prev_start_date_cb, w);
    webview_bind(w, "load_daily_schedule", load_daily_schedule_cb, w);
    webview_bind(w, "load_weekly_schedule", load_weekly_schedule_cb, w);

    //Navigate WebView to the local server URL and show the UI
    char address_buff[256];
    snprintf(address_buff, sizeof(address_buff), "%s/main.html", LOCALHOST_URL);
    webview_navigate(w, address_buff);

    webview_run(w); //Running/showing the window until app is closed

    running = 0; //Signal Mongoose thread to stop polling when app is closing

    webview_destroy(w); //Close the webview context if app is closed
    
    return 0;
}

//Callback function to handle appending doctors
void append_doctor_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;

    cJSON *root = NULL;
    cJSON *userDataObject = NULL;
    
    // Parse JSON payload from JavaScript
    root = cJSON_Parse(req);
    // Check if received payload is valid
    if(root == NULL){
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Invalid JSON format\"}");
        return;
    }

    if(!cJSON_IsArray(root) || cJSON_GetArraySize(root) == 0){
        cJSON_Delete(root);
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Expected array argument\"}");
        return;
    }

    // Extract JSON object
    userDataObject = cJSON_GetArrayItem(root, 0);

    // Check for object's argument
    if(!cJSON_IsObject(userDataObject)){
        cJSON_Delete(root);
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Expected object argument\"}");
        return;
    }

    Data new_doctor;
    // Name
    const char *name_val = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "name"), "UNKNOWN_NAME");
    strncpy(new_doctor.nama, name_val, sizeof(new_doctor.nama) - 1);
    new_doctor.nama[sizeof(new_doctor.nama) - 1] = '\0';

    // Birthdate
    cJSON *birthdate_object = cJSON_GetObjectItemCaseSensitive(userDataObject, "birthdate");
    char temp_birthdate_str[15];
    if(cJSON_IsObject(birthdate_object)){
        const char *day = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_object, "day"), "00");
        const char *month = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_object, "month"), "00");
        const char *year = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_object, "year"), "0000");
        snprintf(temp_birthdate_str, sizeof(temp_birthdate_str), "%s-%s-%s", day, month, year);
    }
    else{ //No birthdate obj sent
        strncpy(temp_birthdate_str, "00-00-0000", sizeof(temp_birthdate_str) - 1);
        temp_birthdate_str[sizeof(temp_birthdate_str) - 1] = '\0';
    }
    strncpy(new_doctor.tanggal_lahir, temp_birthdate_str, sizeof(new_doctor.tanggal_lahir)-1);
    new_doctor.tanggal_lahir[sizeof(new_doctor.tanggal_lahir) - 1] = '\0';

    // Phone
    const char *phone_val = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "phone"), "N/A_PHONE");
    strncpy(new_doctor.no_telp, phone_val, sizeof(new_doctor.no_telp) - 1);
    new_doctor.no_telp[sizeof(new_doctor.no_telp) - 1] = '\0';

    // Address
    const char *address_val = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "address"), "N/A_ADDR");
    strncpy(new_doctor.alamat, address_val, sizeof(new_doctor.alamat) - 1);
    new_doctor.alamat[sizeof(new_doctor.alamat) - 1] = '\0';

    // Specialization
    const char *specialty_val = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "specialty"), "N/A_SPEC");
    strncpy(new_doctor.spesialisasi, specialty_val, sizeof(new_doctor.spesialisasi) - 1);
    new_doctor.spesialisasi[sizeof(new_doctor.spesialisasi) - 1] = '\0';

    // Max Shift
    cJSON *max_shift_item = cJSON_GetObjectItemCaseSensitive(userDataObject, "max_shift");
    if(cJSON_IsString(max_shift_item)){
        new_doctor.maks_shift = atoi(get_string_value_safe(max_shift_item, "0"));
    }
    else if(cJSON_IsNumber(max_shift_item)){
        new_doctor.maks_shift = (int)cJSON_GetNumberValue(max_shift_item);
    }
    else{
        new_doctor.maks_shift = -1; //Default to -1 if parsed object is invalid
    }

    // Shift Preference
    const char *shift_preference_val = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "shift_preference"), "N/A_PREF");
    strncpy(new_doctor.pref_shift, shift_preference_val, sizeof(new_doctor.pref_shift) - 1);
    new_doctor.pref_shift[sizeof(new_doctor.pref_shift) - 1] = '\0';

    // Call backend function (asumsi tambahDataDokter returns 1 for success, 0 for fail)
    int success = tambahDataDokter(doctorFilePath, &new_doctor);

    // Notify JS to refresh docs list
    char notifyFunc[64];
    snprintf(notifyFunc, sizeof(notifyFunc), "notifyDataUpdated();");
    webview_eval(w, notifyFunc);

    cJSON_Delete(root);

    // Signal JavaScript's `await` that operation is finished
    if(success){
        webview_return(w, seq, 0, "{\"status\":\"success\", \"message\":\"Doctor appended successfully\"}");
    } 
    else{
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Failed to append doctor (duplicate or internal error)\"}");
    }
}

//Callback function to handle loading doctors
void load_doctors_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;

    //Get doctors_array in JSON array format
    cJSON *doctors_array = tampilkanSemua(doctorFilePath, scheduleFilePath);

    //If the array if empty/error, then return empty array string to JS
    if(doctors_array == NULL){
        webview_return(w, seq, 1, "[]");
        return;
    }

    //Create response string
    char *json_response_string = cJSON_PrintUnformatted(doctors_array);
    if(json_response_string == NULL) {
        cJSON_Delete(doctors_array); //Reset/clean the array
        webview_return(w, seq, 1, "{\"error\": \"Failed to serialize doctors data\"}");
        return;
    }

    //Assume default return success
    webview_return(w, seq, 0, json_response_string);

    //Clear all temps
    free(json_response_string);
    cJSON_Delete(doctors_array);
}

//Callback function to handle updating doctors data
void update_doctor_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;

    cJSON *root = NULL;
    cJSON *payloadObject = NULL;
    
    int operation_success = 0;
    char response_message_buffer[256];
    
    //Parse JSON
    root = cJSON_Parse(req);

    //Check if it's valid, otherwise return error
    if(root == NULL || !cJSON_IsArray(root) || cJSON_GetArraySize(root) == 0){
        snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"error\", \"message\":\"Invalid JSON format for update/delete\"}");
        webview_return(w, seq, 1, response_message_buffer);
        return;
    }

    payloadObject = cJSON_GetArrayItem(root, 0);

    //Check the payload
    if(!cJSON_IsObject(payloadObject)){
        //Reset & clear temps, send error message
        cJSON_Delete(root);
        snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"error\", \"message\":\"Expected object payload\"}");
        webview_return(w, seq, 1, response_message_buffer);
        return;
    }

    cJSON *original_name_item = cJSON_GetObjectItemCaseSensitive(payloadObject, "original_name");
    const char *original_name = get_string_value_safe(original_name_item, "");

    cJSON *is_delete_item = cJSON_GetObjectItemCaseSensitive(payloadObject, "is_delete");
    int is_delete = (is_delete_item != NULL && cJSON_IsTrue(is_delete_item));


    //Check if name is missing
    if(strlen(original_name) == 0){
        //Reset & clean temps, send error
        cJSON_Delete(root);
        snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"error\", \"message\":\"Original name is required\"}");
        webview_return(w, seq, 1, response_message_buffer);
        return;
    }

    //If is a delete operation
    if(is_delete){
        //Execute deletion and get status
        operation_success = hapusDataDokter(doctorFilePath, original_name);

        //If deleted successfully
        if(operation_success){
            snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"success\", \"message\":\"Doctor deleted successfully\"}");
        }
        else{
            snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"error\", \"message\":\"Failed to delete doctor\"}");
        }

    }
    //Edit operation
    else{
        Data updated_doctor;
        //Assign values from payload to updated_doctor
        strncpy(updated_doctor.nama, get_string_value_safe(cJSON_GetObjectItemCaseSensitive(payloadObject, "name"), ""), sizeof(updated_doctor.nama) - 1);
        updated_doctor.nama[sizeof(updated_doctor.nama) - 1] = '\0';

        cJSON *birthdate_obj = cJSON_GetObjectItemCaseSensitive(payloadObject, "birthdate");
        char temp_birthdate_str[15];
        if(cJSON_IsObject(birthdate_obj)){
            const char *day = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_obj, "day"), "00");
            const char *month = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_obj, "month"), "00");
            const char *year = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(birthdate_obj, "year"), "0000");
            snprintf(temp_birthdate_str, sizeof(temp_birthdate_str), "%s-%s-%s", day, month, year);
        }
        else{
            strncpy(temp_birthdate_str, "00-00-0000", sizeof(temp_birthdate_str) - 1);
            temp_birthdate_str[sizeof(temp_birthdate_str) - 1] = '\0';
        }
        strncpy(updated_doctor.tanggal_lahir, temp_birthdate_str, sizeof(updated_doctor.tanggal_lahir)-1);
        updated_doctor.tanggal_lahir[sizeof(updated_doctor.tanggal_lahir) - 1] = '\0';
        
        strncpy(updated_doctor.no_telp, get_string_value_safe(cJSON_GetObjectItemCaseSensitive(payloadObject, "phone"), ""), sizeof(updated_doctor.no_telp)-1);
        updated_doctor.no_telp[sizeof(updated_doctor.no_telp)-1] = '\0';
        strncpy(updated_doctor.alamat, get_string_value_safe(cJSON_GetObjectItemCaseSensitive(payloadObject, "address"), ""), sizeof(updated_doctor.alamat)-1);
        updated_doctor.alamat[sizeof(updated_doctor.alamat)-1] = '\0';
        strncpy(updated_doctor.spesialisasi, get_string_value_safe(cJSON_GetObjectItemCaseSensitive(payloadObject, "specialty"), ""), sizeof(updated_doctor.spesialisasi)-1);
        updated_doctor.spesialisasi[sizeof(updated_doctor.spesialisasi)-1] = '\0';
        
        cJSON *max_shift_item = cJSON_GetObjectItemCaseSensitive(payloadObject, "max_shift");
        if(cJSON_IsString(max_shift_item)){ updated_doctor.maks_shift = atoi(get_string_value_safe(max_shift_item, "0")); }
        else if(cJSON_IsNumber(max_shift_item)){ updated_doctor.maks_shift = (int)cJSON_GetNumberValue(max_shift_item); }
        else{ updated_doctor.maks_shift = -1; }

        strncpy(updated_doctor.pref_shift, get_string_value_safe(cJSON_GetObjectItemCaseSensitive(payloadObject, "shift_preference"), ""), sizeof(updated_doctor.pref_shift)-1);
        updated_doctor.pref_shift[sizeof(updated_doctor.pref_shift)-1] = '\0';

        //Execute updateDataDokter and get status
        operation_success = updateDataDokter(doctorFilePath, original_name, &updated_doctor);

        //If successfully updated data, return success message
        if(operation_success){
            snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"success\", \"message\":\"Doctor updated successfully\"}");
        }
        else{
            snprintf(response_message_buffer, sizeof(response_message_buffer), "{\"status\":\"error\", \"message\":\"Failed to update doctor\"}");
        }
    }

    //Clear received JSON temp
    cJSON_Delete(root);

    // Notify JS to refresh list by default
    char notifyFunc[64];
    snprintf(notifyFunc, sizeof(notifyFunc), "notifyDataUpdated();");
    webview_eval(w, notifyFunc);

    // Signal JavaScript's `await` that the operation is finished
    webview_return(w, seq, !operation_success, response_message_buffer);
}

//Callback function to generate schedule from BE and send to FE/JS
void generate_schedule_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;
    
    cJSON *root = NULL;
    cJSON *userDataObject = NULL;
    
    //Parse JSON payload from JavaScript
    root = cJSON_Parse(req);

    //Check if error
    if(root == NULL){
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Invalid JSON format\"}");
        return;
    }

    //Check if it's valid
    if(!cJSON_IsArray(root) || cJSON_GetArraySize(root) == 0){
        //Reset & clear temps
        cJSON_Delete(root);
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Expected array argument\"}");
        return;
    }

    //Get the object
    userDataObject = cJSON_GetArrayItem(root, 0);

    //Check if the object is valid
    if (!cJSON_IsObject(userDataObject)) {
        //Reset & clear temp, send error to JS
        cJSON_Delete(root);
        webview_return(w, seq, 1, "{\"status\":\"error\", \"message\":\"Expected object argument\"}");
        return;
    }

    //Docs per shift temp array to store number of doctors required per shift: morning[0], noon[1], night[2]
    int doctors_per_shift[3];
    //Get the starting date to start scheduling
    const char *start_date = get_string_value_safe(cJSON_GetObjectItemCaseSensitive(userDataObject, "start_date"), "N/A_STARTDATE");
    
    //Write the starting date as a log file for later processing
    FILE *f = fopen(scheduleDateLogPath, "w");
    if(f != NULL){ fprintf(f, "%s\n", start_date); } //Minimal checking, assuming received data is valid
    fclose(f);

    //Get the number of personnels required per shift
    cJSON *morning_shift = cJSON_GetObjectItemCaseSensitive(userDataObject, "doc_number_morning");
    cJSON *noon_shift = cJSON_GetObjectItemCaseSensitive(userDataObject, "doc_number_noon");
    cJSON *night_shift = cJSON_GetObjectItemCaseSensitive(userDataObject, "doc_number_night");
    //Check if it's sent as string (antisipasi)
    if(cJSON_IsString(morning_shift) || cJSON_IsString(noon_shift) || cJSON_IsString(night_shift)){
        doctors_per_shift[0] = atoi(get_string_value_safe(morning_shift, "0"));
        doctors_per_shift[1] = atoi(get_string_value_safe(noon_shift, "0"));
        doctors_per_shift[2] = atoi(get_string_value_safe(night_shift, "0"));
    }
    //Check if it's number
    else if(cJSON_IsNumber(morning_shift) || cJSON_IsNumber(noon_shift) || cJSON_IsNumber(night_shift)){
        doctors_per_shift[0] = (int)cJSON_GetNumberValue(morning_shift);
        doctors_per_shift[1] = (int)cJSON_GetNumberValue(noon_shift);
        doctors_per_shift[2] = (int)cJSON_GetNumberValue(night_shift);
    } 
    //Default to -1 if it's invalid
    else{
        doctors_per_shift[0] = -1;
        doctors_per_shift[1] = -1;
        doctors_per_shift[2] = -1;
    }

    //Write the number of personnels per shift as a log file for later processing
    char personnel_per_shift[20];
    sprintf(personnel_per_shift, "%d;%d;%d",
            doctors_per_shift[0],
            doctors_per_shift[1],
            doctors_per_shift[2]);
    FILE *f2 = fopen(schedulePersonnelLogPath, "w");
    if(f2 != NULL){ fprintf(f2, "%s\n", personnel_per_shift); } //Assume data is already valid
    fclose(f2);

    //Start scheduling by calling necessary fnctions
    Jadwal jadwal_dokter[30];
    NodeDataJadwal *headDataJadwal = NULL; NodeDataJadwal *tailDataJadwal = NULL;
    AllComponentJadwal(jadwal_dokter, doctorFilePath, start_date, doctors_per_shift);

    jumlahShiftDokter(&headDataJadwal, &tailDataJadwal, jadwal_dokter, doctorFilePath);
    pemilihanJadwal(jadwal_dokter, scheduleFilePath, 3, 0, 0);

    //Return/send to JS that operation is successful
    webview_return(w, seq, 0, "{\"status\":\"success\", \"message\":\"Doctor appended successfully\"}");    
    
    //Clear temp
    cJSON_Delete(root);
}

//Callback function to get personnel per shift from the log file
void get_personnel_per_shift_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;
    //Initiate read operation from the personnel per shift log file
    FILE *f = fopen(schedulePersonnelLogPath, "r");
    if(f != NULL){
        char buffer[100] = {0};
        if(fgets(buffer, sizeof(buffer), f) != NULL){
            //Return the extracted data to JS if the log isn't empty
            webview_return(w, seq, 0, cJSON_PrintUnformatted(cJSON_CreateString(buffer)));
        } 
        else{
            //Return not found to JS if unable to read file
            webview_return(w, seq, 1, "Failed to read file");
        }
        fclose(f);
    }
    else{
        //Return not found to JS if there's error
        webview_return(w, seq, 1, "File not found");
    }
}

//Helper function to load previously saved start date from generating schedule request
void load_saved_start_date(char *buffer, size_t buff_size){
    //Initiate read operation from the log file
    FILE *f = fopen(scheduleDateLogPath, "r");
    if(f != NULL){
        char temp[128];
        //Return the extracted data from log file
        if(fgets(temp, sizeof(temp), f) != NULL){
            strncpy(buffer, temp, buff_size - 1);
            buffer[buff_size - 1] = '\0';
        }
        else{
            //Return UNSET if there's no schedule yet
            strncpy(buffer, "UNSET", buff_size - 1);
        }
        fclose(f);
    }
    else{
        //Return UNSET if there's no schedule yet/file error
        strncpy(buffer, "UNSET", buff_size - 1);
    }
}

//Callback function to get previously saved start date
void get_prev_start_date_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;
    char start_date[15];
    //Call previously made helper function
    load_saved_start_date(start_date, sizeof(start_date));
    //If the date exists (has been scheduled/initiated scheduling before), return the start date, otherwise return UNSET text
    (strlen(start_date) != 0) ? webview_return(w, seq, 0, cJSON_PrintUnformatted(cJSON_CreateString(start_date))) : webview_return(w, seq, 1, cJSON_PrintUnformatted(cJSON_CreateString("UNSET")));
}

//Callback faunction to handle loading of daily schedule
void load_daily_schedule_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;

    char start_date[15];
    //Get the saved start date
    load_saved_start_date(start_date, sizeof(start_date));

    //Only load daily schedule when scheduling has been made (start date exists or the value isn't unset)
    if(strcasecmp(start_date, "UNSET") != 0){
        //Parse json
        cJSON *root = cJSON_Parse(req);

        //Check if parsing is done, otherwise return error
        if(root == NULL){
            const char* seq_start = strstr(req, "\"seq\":\"");
            char seq_buf[64] = "unknown";
            if(seq_start){
                sscanf(seq_start + strlen("\"seq\":\""), "%63[^\"]", seq_buf);
            }
            webview_return(w, seq_buf, 1, "{\"status\": \"error\", \"message\": \"Invalid JSON input to C function.\"}");
            return;
        }

        //Get the value of the target date (nth-day to be laoded)
        const char *target_date = get_string_value_safe(cJSON_GetArrayItem(root, 0), "");

        //Check if the target date is valid/exists
        if(target_date == NULL || strlen(target_date) == 0){
            //Reset & clear temps, return error message
            cJSON_Delete(root);
            webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Date parameter is missing or invalid\"}");
            return;
        }
        
        //Find the current target date as the nth-day from the start date (difference) by calling get_nth_day helper function
        int nth_day = get_nth_day(start_date, target_date);

        //Get the data of the particular day in JSON by calling get_schedule_data helper function in mode 0 (for daily schedule)
        cJSON *daily_data_json = get_schedule_data(scheduleFilePath, 0, nth_day, 0);

        //If the json exists
        if(daily_data_json){
            char *json_string = cJSON_PrintUnformatted(daily_data_json);
            if (json_string){
                //Return the parsed string if is valid/exists
                webview_return(w, seq, 0, json_string);
                //free the temp
                free(json_string);
            }
            else{
                //Return error message if unable to serialize json
                webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Internal JSON serialization error.\"}\n");
            }
            //Reset & clear temp
            cJSON_Delete(daily_data_json);
        }
        else{
            //Return error if failed to load data
            webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Failed to load daily shift data from CSV.\"}\n");
        }
        //Reset & clear temp
        cJSON_Delete(root);
    }
    else{
        //Return error if date param i smissing
        webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Date parameter is missing or invalid\"}");
        return;
    }
}

//Callback function to handle loading of weekly schedule
void load_weekly_schedule_cb(const char *seq, const char *req, void *arg){
    struct webview *w = (struct webview *)arg;

    //Parse payload
    cJSON *root = cJSON_Parse(req);

    //Check if it exists, otherwise return error message
    if(root == NULL){
        const char* seq_start = strstr(req, "\"seq\":\"");
        char seq_buf[64] = "unknown";
        if (seq_start) {
            sscanf(seq_start + strlen("\"seq\":\""), "%63[^\"]", seq_buf);
        }
        webview_return(w, seq_buf, 1, "{\"status\": \"error\", \"message\": \"Invalid JSON input to C function.\"}");
        return;
    }

    //Get the nth_week to be searched from the payload object
    int nth_week = atoi(get_string_value_safe(cJSON_GetArrayItem(root, 0), "0"));

    //Check if the target nth_week is valid from week 1 to week 5
    if(nth_week <= 0 || nth_week > 5){
        //Reset & clear temps, return error message
        cJSON_Delete(root);
        webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Nth week parameter is missing or invalid\"}");
        return;
    }

    //Get the weekly data in JSON array format by calling get_schedule_data helper function again in mode 1 (for weekly schedule)
    cJSON *weekly_data_json = get_schedule_data(scheduleFilePath, 1, 0, nth_week);

    //If the json exists
    if(weekly_data_json){
        char *json_string = cJSON_PrintUnformatted(weekly_data_json);
        if(json_string){
            //Return the json string format if it exists/valid
            webview_return(w, seq, 0, json_string);
            free(json_string); //free the temp
        }
        else{
            //Return error message if failed to serialize
            webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Internal JSON serialization error.\"}\n");
        }
        //Reset & clear the temp
        cJSON_Delete(weekly_data_json);
    }
    else{
        //Return error message if failed to load from csv
        webview_return(w, seq, 1, "{\"status\": \"error\", \"message\": \"Failed to load weekly shift data from CSV.\"}\n");
    }
}
