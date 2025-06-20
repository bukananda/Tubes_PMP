#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/pengelolaanData.h"

// Struct untuk data dokter
typedef struct Data {
    char nama[100];
    int maks_shift;
    char pref_shift[10];
} Data;

// Node untuk linked list
typedef struct NodeData {
    Data data_dokter;
    struct NodeData *next;
} NodeData;

// Menambahkan node di akhir list
static void insertTail(NodeData **head, NodeData **tail, Data data) {
    NodeData *baru = (NodeData *)malloc(sizeof(NodeData));
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
static void removeNode(NodeData **head, NodeData **tail, NodeData *prev, NodeData *curr) {
    if (prev == NULL) {
        *head = curr->next;
        if (*tail == curr) *tail = NULL;
    } else {
        prev->next = curr->next;
        if (*tail == curr) *tail = prev;
    }
    free(curr);
}

// Membaca file CSV ke linked list
static void readCSV(NodeData **head, NodeData **tail, FILE *fptr) {
    Data temp;
    char line[256], *tok;
    while (fgets(line, sizeof(line), fptr)) {
        tok = strtok(line, ";");
        if (!tok) continue;
        strcpy(temp.nama, tok);

        tok = strtok(NULL, ";");
        if (!tok) continue;
        temp.maks_shift = atoi(tok);

        tok = strtok(NULL, "\n");
        if (!tok) continue;
        strcpy(temp.pref_shift, tok);

        insertTail(head, tail, temp);
    }
}

// Menulis linked list ke file CSV
void writeCSV(NodeData **head, FILE *fptr) {
    NodeData *curr = *head;
    while (curr) {
        fprintf(fptr, "%s;%d;%s\n", curr->data_dokter.nama, curr->data_dokter.maks_shift, curr->data_dokter.pref_shift);
        curr = curr->next;
    }
}

// Validasi input shift: pagi, siang, malam
int isValidShift(const char *shift) {
    return strcasecmp(shift, "pagi") == 0 || strcasecmp(shift, "siang") == 0 || strcasecmp(shift, "malam") == 0;
}

// Cek duplikasi data saat menambahkan data
int isDuplicate(NodeData *head, Data *newData) {
    while (head) {
        if (strcasecmp(head->data_dokter.nama, newData->nama) == 0 &&
            head->data_dokter.maks_shift == newData->maks_shift &&
            strcasecmp(head->data_dokter.pref_shift, newData->pref_shift) == 0) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

// Fungsi untuk menambah data dokter
void tambahDataDokter(const char *file) {
    FILE *fptr = fopen(file, "a+");
    if (!fptr) {
        printf("Gagal membuka file.\n");
        return;
    }

    NodeData *head = NULL, *tail = NULL;
    rewind(fptr);
    readCSV(&head, &tail, fptr);

    Data baru;
    printf("Nama dokter: ");
    scanf(" %[^\n]", baru.nama);

    // Input maximum shift merupakan angka integer lebih dari 0
    // Jika input salah, program meminta input baru sampai benar
    printf("Maksimal shift per minggu: ");
    while (scanf("%d", &baru.maks_shift) != 1 || baru.maks_shift <= 0) {
        printf("Input tidak valid. Masukkan angka > 0: ");
        while (getchar() != '\n');
    }

    // Input preferensi shift divalidasi dengan fungsi isValidShift
    // Jika input salah, program meminta input baru sampai benar
    printf("Preferensi shift (pagi/siang/malam): ");
    scanf(" %s", baru.pref_shift);
    while (!isValidShift(baru.pref_shift)) {
        printf("Input tidak valid. Masukkan 'pagi', 'siang', atau 'malam': ");
        scanf(" %s", baru.pref_shift);
    }

    // Validasi data yang akan ditambahkan
    // Jika data yang ditambahkan memiliki kesamaan data yang sudah disimpan, data tidak akan ditambahkan
    if (isDuplicate(head, &baru)) {
        printf("Data dokter sudah ada. Tidak ditambahkan.\n");
        fclose(fptr);
    } else {
        fprintf(fptr, "%s;%d;%s\n", baru.nama, baru.maks_shift, baru.pref_shift);
        printf("Data berhasil ditambahkan.\n");
        fclose(fptr);
    }

    while (head) {
        NodeData *tmp = head;
        head = head->next;
        free(tmp);
    }
}

// Fungsi untuk menampilkan semua data
void tampilkanSemua(const char *file) {
    NodeData *head = NULL, *tail = NULL;
    FILE *f = fopen(file, "r");
    if (!f) {
        printf("File tidak ditemukan.\n");
        return;
    }
    readCSV(&head, &tail, f);
    fclose(f);

    if (!head) {
        printf("Data kosong.\n");
        return;
    }

    NodeData *curr = head;
    int i = 1;
    while (curr) {
        printf("%d. Nama: %s, Maks Shift: %d, Preferensi Shift: %s\n", i, curr->data_dokter.nama, curr->data_dokter.maks_shift, curr->data_dokter.pref_shift);
        curr = curr->next;
        i++;
    }

    while (head) {
        NodeData *tmp = head;
        head = head->next;
        free(tmp);
    }
}

// Fungsi untuk menampilkan data berdasarkan filter pencarian (nama/max shift/preferensi shift)
void tampilkanFilter(const char *file) {
    int opsi;
    char input[100];
    NodeData *head = NULL, *tail = NULL;

    FILE *f = fopen(file, "r");
    if (!f) {
        printf("File tidak bisa dibuka.\n");
        return;
    }
    readCSV(&head, &tail, f);
    fclose(f);

    do {
        printf("Tampilkan berdasarkan:\n");
        printf("1. Nama\n2. Preferensi shift\n3. Maks shift\nPilihan: ");
        scanf("%d", &opsi);
        getchar(); 
        if (opsi < 1 || opsi > 3) 
            printf("Pilihan tidak valid. Coba lagi.\n");
    } while (opsi < 1 || opsi > 3);

    int angkaFilter = 0;
    int ditemukan = 0;
    NodeData *curr = head;

    if (opsi == 1) {
        printf("Masukkan nama dokter yang ingin dicari: ");
        scanf(" %[^\n]", input);
    } else if (opsi == 2) {
        printf("Masukkan preferensi shift (pagi/siang/malam): ");
        scanf(" %[^\n]", input);
    } else if (opsi == 3) {
        printf("Masukkan maksimum shift yang ingin dicari: ");
        scanf(" %[^\n]", input);
        angkaFilter = atoi(input);
    }

    while (curr) {
        if ((opsi == 1 && strcasecmp(curr->data_dokter.nama, input) == 0) ||
            (opsi == 2 && strcasecmp(curr->data_dokter.pref_shift, input) == 0) ||
            (opsi == 3 && curr->data_dokter.maks_shift == angkaFilter)) {
            printf("Nama: %s, Maks Shift: %d, Preferensi Shift: %s\n", 
                   curr->data_dokter.nama, curr->data_dokter.maks_shift, curr->data_dokter.pref_shift);
            ditemukan = 1;
        }
        curr = curr->next;
    }

    if (!ditemukan) printf("Data tidak ditemukan.\n");


    while (head) {
        NodeData *tmp = head;
        head = head->next;
        free(tmp);
    }
}

// Fungsi untuk menghapus data dokter
void hapusDataDokter(const char *file) {
    char nama[100];
    printf("Masukkan nama dokter yang ingin dihapus: ");
    scanf(" %[^\n]", nama);

    NodeData *head = NULL, *tail = NULL;
    FILE *f = fopen(file, "r");
    if (!f) {
        printf("Gagal membuka file.\n");
        return;
    }
    readCSV(&head, &tail, f);
    fclose(f);

    NodeData *curr = head, *prev = NULL;
    int found = 0;

    while (curr) {
        if (strcasecmp(curr->data_dokter.nama, nama) == 0) {
            NodeData *next = curr->next;
            removeNode(&head, &tail, prev, curr);
            curr = next;
            found = 1;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }

    if (found) {
        FILE *fw = fopen(file, "w");
        writeCSV(&head, fw);
        fclose(fw);
        printf("Data berhasil dihapus.\n");
    } else {
        printf("Dokter tidak ditemukan.\n");
    }

    while (head) {
        NodeData *tmp = head;
        head = head->next;
        free(tmp);
    }
}