#ifndef JADWAL_H
#define JADWAL_H

// Struct Data ini berisi nama dokter, maksimal shift dalam satu minggu, dan preferensi shift yang diinginkan entah itu pagi, siang, atau malam.
typedef struct Data{
    char nama[100];
    int maks_shift;
    char pref_shift[10];
} Data;

// Struct NodeData ini merupakan Node untuk membuat linked list yang didalam datanya berupa data dokter
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

void AllComponentJadwal(Jadwal jadwal_dokter[], char *nama_file, int inisialisasi);
void readCSV(NodeData **head, NodeData **tail, FILE *fptr);

#endif