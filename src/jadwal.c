#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jadwal.h"
#include "pengelolaanData.h"

// Fungsi untuk memasukkan data ke paling depan dari linked list
static void insertHead(NodeData **head, Data data){ // **head merupakan pointer dari headnya, data merupakan data dari dokter yang bersangkutan
    NodeData *nodebaru = (NodeData *)malloc(sizeof(NodeData)); // Inisiasi data baru
    nodebaru->data_dokter = data; // Memasukkan data ke nodebaru

    if (*head == NULL){ // Jika head masih berada di null, maka
        *head = nodebaru; // head akan dipoint ke nodebaru
        nodebaru->next = NULL; // Node selanjutnya dari nodebaru dipoint ke null
    } else if (*head != NULL) { // Jika head bukan berupa null
        nodebaru->next = *head; // next dari nodebaru dipoint ke head
        *head = nodebaru; // head dipindah alihkan ke nodebaru
    }
}

// Fungsi untuk memasukkan data ke paling belakang dari linked list
static void insertTail(NodeData **head, NodeData **tail, Data data){ // **head merupakan pointer dari headnya, **tail merupakan pointer dari tailnya, data merupakan data dari dokter yang bersangkutan
    NodeData *nodebaru = (NodeData *)malloc(sizeof(NodeData)); // Inisiasi data baru
    nodebaru->data_dokter = data; // Memasukkan data ke nodebaru

    if (*tail == NULL){ // Jika tail pada linked list tersebut adalah null, maka
        *head = nodebaru; // head akan diinisiasi ke nodebaru
        *tail = nodebaru; // tail akan diinisiasi ke nodebaru
        nodebaru->next = NULL; // node selanjutnya dari nodebaru dipoint ke null
    } else if (*tail != NULL) { // Jika tail bukan null, maka
        (*tail)->next = nodebaru; // Node selanjutnya dari tail akan dipoint ke nodebaru
        *tail = nodebaru; // tail dipindah alihkan ke nodebaru
        nodebaru->next = NULL; // selanjutnya dari nodebaru dipoint ke null
    }
}

// Fungsi untuk menghapus node berdasarkan linked list yang mana, Node sebelumnya dan Node sekarang
static void removeNode(NodeData **head, NodeData **tail, NodeData *prev, NodeData *curr){ // **head merupakan pointer dari headnya, **tail merupakan pointer dari tailnya, *prev merupakan node sebelumnya, *curr adalah node saat ini
    if (prev == NULL) { // Jika node sebelumnya adalah null
        *head = curr->next; // head dipoint ke node selanjutnya dari curr
        if (*tail == curr) { // jika tail merupakan pointer saat ini
            *tail = NULL; // tail dipoint ke null
        }
    } else { // Jika node sebelumnya adalah bukan null
        prev->next = curr->next; // menyambugnkan node previous ke node selanjutnya
        if (*tail == curr) { // Jika node saat ini merupakan node saat ini
            *tail = prev; // tail dipindah alihkan ke node sebelumnya
        }
    }
    free(curr); // membebaskan memori dari node saat ini yang artinya menghapus node saat ini
}

// Fungsi untuk mengetahui panjang dari linked list tersebut
int sizeLL(NodeData **head){ // **head merupakan pointer dari headnya
    int n = 0; // Awalan n berupa nol
    NodeData *curr = *head; // node saat ini di inisialisasi ke head dari linked list tersebut
    while(curr!=NULL){ // Jika node saat ini berupa null maka pengulangan dihentikan
        n++; // Penambahan nilai n sebanyak satu
        curr = curr->next; // Node saat ini dimajukan ke nextnya
    }
    return n; // Jika pengulangan dihentikan lakukan return nilai n nya
}

// Fungsi untuk mengacak urutan data dari suatu linked list kemudian ditaruh ke dua linked list yang lain 
void randomLL(NodeData **head, NodeData **tail, NodeData **head1, NodeData **head2){
    int r = 0;
    int n = sizeLL(head);
    NodeData *prev = (NodeData *)malloc(sizeof(NodeData));
    NodeData *curr = (NodeData *)malloc(sizeof(NodeData));

    Data data_temp;
    
    while(n > 0){
        prev = NULL;
        curr = *head;
        r = rand() % n;

        for (int i = 0; i < r; i++) {
            prev = curr;
            curr = curr->next;
        }

        data_temp = curr->data_dokter;
        
        removeNode(head, tail, prev, curr);
        insertHead(head1, data_temp);
        insertHead(head2, data_temp);
        n--;
    }
}

// Mengacak dua linked list yang ada
void random2LL(NodeData **head1, NodeData **head2){
    int r = 0;
    int n = sizeLL(head1);

    NodeData *head_temp1 = NULL;
    NodeData *head_temp2 = NULL;
    NodeData *prev1 = (NodeData *)malloc(sizeof(NodeData));
    NodeData *prev2 = (NodeData *)malloc(sizeof(NodeData));
    NodeData *curr1 = (NodeData *)malloc(sizeof(NodeData));
    NodeData *curr2 = (NodeData *)malloc(sizeof(NodeData));

    Data data_temp1; Data data_temp2;

    while(n > 0){
        prev1 = NULL; prev2 = NULL;
        curr1 = *head1; curr2 = *head2;
        r = rand() % n;

        for (int i = 0; i < r; i++) {
            prev1 = curr1;
            prev2 = curr2;
            curr1 = curr1->next;
            curr2 = curr2->next;
        }

        data_temp1 = curr1->data_dokter;
        data_temp2 = curr2->data_dokter;
        
        if (prev1 == NULL && prev2 == NULL) {
            *head1 = curr1->next;
            *head2 = curr2->next;
        } else {
            prev1->next = curr1->next;
            prev2->next = curr2->next;
        }
        free(curr1); free(curr2);
        insertHead(&head_temp1, data_temp1);
        insertHead(&head_temp2, data_temp2);
        n--;
    }

    *head1 = head_temp1;
    *head2 = head_temp2;
}

// Menentukan apakah jumlah shift yang ada pada semua dokter telah terpakai semua dalam satu minggu
int isAllZero(NodeData **head1){
    NodeData *curr = *head1;
    while(curr != NULL){
        if(curr->data_dokter.maks_shift == 1){
            return 0;
        }
        curr = curr->next;
    }
    return 1;
}

// Untuk mereset jumlah shiftnya pada semua dokter jika telah melewati satu minggu
void resetNode(NodeData **head1, NodeData **head2){
    NodeData *curr1 = *head1;
    NodeData *curr2 = *head2;

    while(curr1 != NULL && curr2 != NULL){
        curr1->data_dokter.maks_shift = curr2->data_dokter.maks_shift;
        curr1 = curr1->next; curr2 = curr2->next;
    }
}

// Melakukan pengisian shift pada satu hari dan tiap shiftnya entah itu shift pagi, shift siang dan shift malam yang ditentukan dari argumen "waktu"
void pengisian_shift(Jadwal *jadwal_1hari, NodeData **head1, NodeData **head2, int jumlah_shift, int waktu){
    NodeData *curr1 = *head1;
    NodeData *curr2 = *head2;
    int i = 0;
    while(curr1 != NULL && curr2 != NULL){
        if(curr1->data_dokter.maks_shift != 0){
            if(!strcmp(curr1->data_dokter.pref_shift, "pagi") && waktu == 1){
                jadwal_1hari->shift_pagi[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            } else if(!strcmp(curr1->data_dokter.pref_shift, "siang") && waktu == 2){
                jadwal_1hari->shift_siang[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            } else if(!strcmp(curr1->data_dokter.pref_shift, "malam") && waktu == 3){
                jadwal_1hari->shift_malam[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            }
        }
        if(i == jumlah_shift){
            return;
        }
        curr1 = curr1->next;
        curr2 = curr2->next;
    }

    random2LL(head1, head2);

    curr1 = *head1;
    curr2 = *head2;
    while(!isAllZero(head1)){
        if(curr1 == NULL && curr2 == NULL){
            curr1 = *head1;
            curr2 = *head2;
        }
        if(curr1->data_dokter.maks_shift != 0){
            if(waktu == 1){
                jadwal_1hari->shift_pagi[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            } else if(waktu == 2){
                jadwal_1hari->shift_siang[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            } else if(waktu == 3){
                jadwal_1hari->shift_malam[i] = curr2->data_dokter;
                curr1->data_dokter.maks_shift -= 1;
                i++;
            }
        }
        if(i == jumlah_shift){
            return;
        }
        curr1 = curr1->next;
        curr2 = curr2->next;
    }
};

// Untuk menullkan data jika index dari shift tersebut telah melebihi yang ditentukan misalkan pada shift_pagi hanya ada 3 dokter saja maka indeks ke 4 dari shift_pagi tersebut di null kan sebagai pembatas akhir dari array shift_pagi
void nulifikasiData(Jadwal jadwal_dokter[], int idx, int jumlah_pagi, int jumlah_siang, int jumlah_malam){
    strcpy(jadwal_dokter[idx-1].shift_pagi[jumlah_pagi].nama, "\0");
    strcpy(jadwal_dokter[idx-1].shift_siang[jumlah_siang].nama, "\0");
    strcpy(jadwal_dokter[idx-1].shift_malam[jumlah_malam].nama, "\0");
    jadwal_dokter[idx-1].shift_pagi[jumlah_pagi].maks_shift = 0;
    jadwal_dokter[idx-1].shift_siang[jumlah_siang].maks_shift = 0;
    jadwal_dokter[idx-1].shift_malam[jumlah_malam].maks_shift = 0;
    strcpy(jadwal_dokter[idx-1].shift_pagi[jumlah_pagi].pref_shift, "\0");
    strcpy(jadwal_dokter[idx-1].shift_siang[jumlah_siang].pref_shift, "\0");
    strcpy(jadwal_dokter[idx-1].shift_malam[jumlah_malam].pref_shift, "\0");
};

// Melakukan mekanisme penjadwalan dengan menggabungkan mekanisme pengisian pagi, siang, dan malam
void penjadwalan(Jadwal jadwal_dokter[], NodeData **head, NodeData **tail, NodeData **head1, NodeData **head2, int jumlah_pagi, int jumlah_siang, int jumlah_malam){
    for(int i = 1; i <= 30; i++){
        if(i % 7 == 1 && i != 1){
            resetNode(head1, head2);
        }
        jadwal_dokter[i-1].hari = i;
        jadwal_dokter[i-1].shift_pagi = (Data *) malloc(sizeof(Data)*(jumlah_pagi+1));
        jadwal_dokter[i-1].shift_siang = (Data *) malloc(sizeof(Data)*(jumlah_siang+1));
        jadwal_dokter[i-1].shift_malam = (Data *) malloc(sizeof(Data)*(jumlah_malam+1));
        pengisian_shift(&jadwal_dokter[i-1], head1, head2, jumlah_pagi, 1);
        pengisian_shift(&jadwal_dokter[i-1], head1, head2, jumlah_siang, 2);
        pengisian_shift(&jadwal_dokter[i-1], head1, head2, jumlah_malam, 3);
        nulifikasiData(jadwal_dokter, i, jumlah_pagi, jumlah_siang, jumlah_malam);
        random2LL(head1, head2);
    }
};

// Memasukkan semua komponen yang dibutuhkan untuk melakukan operasi penjadwalan ini
void AllComponentJadwal(Jadwal jadwal_dokter[], const char *nama_file, const char *tanggal_mulai, int jumlah_shift[]){
    NodeData *head = NULL; NodeData *tail = NULL;
    NodeData *head1 = NULL;
    NodeData *head2 = NULL;
    
    FILE *fptr;
    fptr = fopen(nama_file,"r");
    
    readCSV(&head , &tail, fptr);
    randomLL(&head, &tail, &head1, &head2);
    penjadwalan(jadwal_dokter, &head, &tail, &head1, &head2, jumlah_shift[0], jumlah_shift[1], jumlah_shift[2]);

    fclose(fptr);
};