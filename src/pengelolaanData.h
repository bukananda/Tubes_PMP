#ifndef PENGELOLAANDATA_H
#define PENGELOLAANDATA_H

#include "data_types.h"
#include "cJSON.h"
#include "ctype.h"

void init_file_manager();
void deinit_file_manager();

void readCSV(NodeData **head, NodeData **tail, FILE *fptr);

int tambahDataDokter(const char *file, Data *newDokter);
cJSON* tampilkanSemua(const char *doctorFilePath, const char *scheduleFilePath);
void tampilkanFilter(const char *file);
int hapusDataDokter(const char *file, const char *name_to_delete);

// NEW: Function to update doctor data by original name
int updateDataDokter(const char *file, const char *original_name, const Data *updatedDokter);

const char* get_string_value_safe(cJSON *item, const char* default_val);
int get_int_value_safe(cJSON *item, int default_val);

int get_nth_day(const char* start_date_str, const char* target_date_str);

cJSON* get_schedule_data(const char *filepath, int mode, int nth_day, int nth_week);

Jadwal* read_schedule_csv_to_jadwal_array(const char *filepath, int docs_per_shifts[3]);
void free_jadwal_array(Jadwal* jadwal_array, int size, int docs_per_shifts[3]);

#endif