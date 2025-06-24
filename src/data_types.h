#ifndef DATA_TYPES_H
#define DATA_TYPES_H

typedef struct Data {
    char nama[100];
    char tanggal_lahir[12];
    char no_telp[15];
    char alamat[50];
    char spesialisasi[50];
    int maks_shift;
    char pref_shift[10];
} Data;

typedef struct NodeData{
    Data data_dokter; // Isi datanya
    struct NodeData *next; // Pointer untuk menuju Node selanjutnya
} NodeData;

/* Struct Jadwal ini berisi hari bekerja, daftar yang mengikuti shift pagi, daftar yang mengikuti shift siang, dan daftar yang mengikuti shift malam */ 
typedef struct Jadwal
{
    int hari;
    Data *shift_pagi; // Berbentuk pointer karena jumlah shift nya berupa array secara dinamis
    Data *shift_siang;
    Data *shift_malam;
} Jadwal;

typedef struct NodeDataJadwal{
    Data data_dokter;
    int jumlah_shift;
    struct NodeDataJadwal *next;
} NodeDataJadwal;

#endif
