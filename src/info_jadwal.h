#ifndef INFO_JADWAL_H
#define INFO_JADWAL_H

#include "data_types.h"
#include "ctype.h"

Jadwal get_jadwal_hari(Jadwal jadwal_dokter[], int day);
void get_jadwal_minggu(Jadwal jadwal_dokter[], int minggu, Jadwal *jadwal_minggu);
void get_jadwal_bulan(Jadwal jadwal_dokter[], Jadwal *jadwal_bulan);
void jumlahShiftDokter(NodeDataJadwal **headDataJadwal, NodeDataJadwal **tailDataJadwal, Jadwal jadwal_dokter[], const char *nama_file);
void pemilihanJadwal(Jadwal jadwal_dokter[], const char *nama_file, int mode, int hari, int minggu);
void pilihanTampilan(NodeDataJadwal **head, NodeDataJadwal **tail);

int jumlahPelanggaran(char nama_dokter[], char pref_shift[], Jadwal jadwal_dokter[], int docs_per_shifts[3]);
int countShift(char nama_dokter[], Jadwal jadwal_dokter[], int docs_per_shifts[]);

#endif