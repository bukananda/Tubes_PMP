#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "jadwal.h"
#include "pengelolaanData.h"
#include "helpers.h"

// tulis jadwal ke csv
// format : hari, nama dokter shift pagi, nama dokter shift siang, nama dokter shift malam)
void write_jadwal(Jadwal jadwal_dokter[], const char *file_name){
    char str[50];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;

    while(jadwal_dokter[0].shift_pagi[jumlah_pagi].nama[0] != '\0'){
        jumlah_pagi++;
    }

    while(jadwal_dokter[0].shift_siang[jumlah_siang].nama[0] != '\0'){
        jumlah_siang++;
    }

    while(jadwal_dokter[0].shift_malam[jumlah_malam].nama[0] != '\0'){
        jumlah_malam++;
    }

    FILE *f_ptr = fopen(file_name, "w"); 
    if (f_ptr == NULL) 
    { 
        printf("Could not open file"); 
    } 
    else{
        for (int i = 0; i<30; i++){
            fprintf(f_ptr,"%d;", jadwal_dokter[i].hari);
            for(int j = 0; j < jumlah_pagi; j++){
                if(j != jumlah_pagi - 1){fprintf(f_ptr,"%s,", jadwal_dokter[i].shift_pagi[j].nama);} else {fprintf(f_ptr,"%s;", jadwal_dokter[i].shift_pagi[j].nama);}
            }
            for(int j = 0; j < jumlah_siang; j++){
                if(j != jumlah_siang - 1){fprintf(f_ptr,"%s,", jadwal_dokter[i].shift_siang[j].nama);} else {fprintf(f_ptr,"%s;", jadwal_dokter[i].shift_siang[j].nama);}
            }
            for(int j = 0; j < jumlah_malam; j++){
                if(j != jumlah_malam - 1){fprintf(f_ptr,"%s,", jadwal_dokter[i].shift_malam[j].nama);} else {fprintf(f_ptr,"%s\n", jadwal_dokter[i].shift_malam[j].nama);}
            }            
        }
    }
    fclose(f_ptr);
};

// Fungsi untuk memasukkan data ke paling belakang dari linked list
void insertTailDataJadwal(NodeDataJadwal **head, NodeDataJadwal **tail, Data data, int jumlah_shift){ // **head merupakan pointer dari headnya, **tail merupakan pointer dari tailnya, data merupakan data dari dokter yang bersangkutan
    NodeDataJadwal *nodebaru = (NodeDataJadwal *)malloc(sizeof(NodeData)); // Inisiasi data baru
    nodebaru->data_dokter = data; // Memasukkan data ke nodebaru
    nodebaru->jumlah_shift = jumlah_shift;

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

// mengembalikan data jadwal pada hari yang dipilih
Jadwal get_jadwal_hari(Jadwal jadwal_dokter[], int day){
    for(int i = 0; i < 30; i++){
        if (i == (day - 1)){
            return jadwal_dokter[i];
        }
    }
    return (Jadwal){0};
}

// mengembalikan data jadwal harian pada satu minggu yang dipilih
void get_jadwal_minggu(Jadwal jadwal_dokter[], int minggu, Jadwal *jadwal_minggu){
    int i = 0;
    for (int j = 0; j<5; j++){
        if (j == (minggu-1)){
            if (j<4){
                for (int k = 0; k<7; k++){
                    jadwal_minggu[k] = get_jadwal_hari(jadwal_dokter, i);
                    i++;
                }
            }
            else { // untuk minggu ke-5 dimana hanya ada 2 hari
                for (int k = 0; k<2; k++){ 
                    jadwal_minggu[k] = get_jadwal_hari(jadwal_dokter, i);
                    i++;
                }
            }
        }
        i += 7;
    }

}

// mengembalikan data jadwal dokter dalam satu bulan
void get_jadwal_bulan(Jadwal jadwal_dokter[], Jadwal *jadwal_bulan){
    for(int i = 0; i<30; i++){
        jadwal_bulan[i] = jadwal_dokter[i];
    };
}

// Menghitung jumlah pelanggaran
int jumlahPelanggaran(char nama_dokter[], char pref_shift[], Jadwal jadwal_dokter[], int docs_per_shifts[3]){
    int jumlah_pelanggaran = 0;

    //Check if docs per shift number is valid
    if(docs_per_shifts[0] <= 0 || docs_per_shifts[1] <= 0 || docs_per_shifts[2] <= 0){
        return 0; //return 0 on error
    }

    //Trim doctors name for excess/wrong inputs
    char trimmed_nama_dokter[100];
    strncpy(trimmed_nama_dokter, nama_dokter, sizeof(trimmed_nama_dokter) - 1);
    trimmed_nama_dokter[sizeof(trimmed_nama_dokter) - 1] = '\0';
    trim_and_normalize_whitespace(trimmed_nama_dokter);

    //Make sure shift preference is also valid and trims is
    char trimmed_pref_shift[10];
    strncpy(trimmed_pref_shift, pref_shift, sizeof(trimmed_pref_shift) - 1);
    trimmed_pref_shift[sizeof(trimmed_pref_shift) - 1] = '\0';
    trim_and_normalize_whitespace(trimmed_pref_shift);

    //Cek if it's invalid, return 0 on that error
    if(trimmed_pref_shift == NULL || (strcmp(trimmed_pref_shift, "pagi") != 0 && strcmp(trimmed_pref_shift, "siang") != 0 && strcmp(trimmed_pref_shift, "malam") != 0)){ return 0; }

    //Loop for 30 days
    for(int i = 0; i < 30; i++){
        //Check morning shift
        if(jadwal_dokter[i].shift_pagi != NULL){
            //Loop all the doctors on that shift
            for(int j = 0; j < docs_per_shifts[0]; j++){
                // Compare with trimmed doctor name and check if the doctor works on that shift
                if(strcmp(jadwal_dokter[i].shift_pagi[j].nama, trimmed_nama_dokter) == 0){
                    //Compare it with the doctor's preference, if violates then increment jumlah_pelanggaran
                    if(strcmp(trimmed_pref_shift, "pagi") != 0){
                        jumlah_pelanggaran++;
                    }
                }
            }
        }

        //Check noon shift
        if(jadwal_dokter[i].shift_siang != NULL){
            for(int j = 0; j < docs_per_shifts[1]; j++){
                if(strcmp(jadwal_dokter[i].shift_siang[j].nama, trimmed_nama_dokter) == 0){
                    if(strcmp(trimmed_pref_shift, "siang") != 0){
                        jumlah_pelanggaran++;
                    }
                }
            }
        }

        //Check night shift
        if(jadwal_dokter[i].shift_malam != NULL){
            for(int j = 0; j < docs_per_shifts[2]; j++){
                if(strcmp(jadwal_dokter[i].shift_malam[j].nama, trimmed_nama_dokter) == 0){
                    if(strcmp(trimmed_pref_shift, "malam") != 0){
                        jumlah_pelanggaran++;
                    }
                }
            }
        }
    }
    return jumlah_pelanggaran;
}

// Mengetahui jumlah shift pada setiap dokter
int countShift(char nama_dokter[], Jadwal jadwal_dokter[], int docs_per_shifts[3]){
    int jumlah_shift = 0;

    //Check if the docs_per_shifts array is valid, return 0 on error
    if(docs_per_shifts[0] <= 0 || docs_per_shifts[1] <= 0 || docs_per_shifts[2] <= 0){ return 0; }

    // Apply trimming to the input doctor's name
    char trimmed_nama_dokter[100];
    strncpy(trimmed_nama_dokter, nama_dokter, sizeof(trimmed_nama_dokter) - 1);
    trimmed_nama_dokter[sizeof(trimmed_nama_dokter) - 1] = '\0';
    trim_and_normalize_whitespace(trimmed_nama_dokter);

    //Loop for 30 days
    for(int i = 0; i < 30; i++){ 
        //Check for morning shift
        if(jadwal_dokter[i].shift_pagi != NULL){ 
            //Check for all doctors, if the targeted doctor's name shows, increment jumlah_shift
            for(int j = 0; j < docs_per_shifts[0]; j++){
                if(!strcmp(jadwal_dokter[i].shift_pagi[j].nama, trimmed_nama_dokter)){
                    jumlah_shift++;
                }
            }
        }

        //Check for noon shift
        if(jadwal_dokter[i].shift_siang != NULL){ 
            for(int j = 0; j < docs_per_shifts[1]; j++){
                if(!strcmp(jadwal_dokter[i].shift_siang[j].nama, trimmed_nama_dokter)){
                    jumlah_shift++;
                }
            }
        }

        // Check for night shift
        if(jadwal_dokter[i].shift_malam != NULL){ 
            for(int j = 0; j < docs_per_shifts[2]; j++){
                if(!strcmp(jadwal_dokter[i].shift_malam[j].nama, trimmed_nama_dokter)){
                    jumlah_shift++;
                }
            }
        }
    }
    return jumlah_shift;
}

// Menghitung jumlah shift pada semua dokter
void jumlahShiftDokter(NodeDataJadwal **headDataJadwal, NodeDataJadwal **tailDataJadwal, Jadwal jadwal_dokter[], const char *nama_file){
    if(*headDataJadwal != NULL && *tailDataJadwal != NULL){
        NodeDataJadwal *curr = *headDataJadwal;
        while(curr != NULL){
            NodeDataJadwal *next = curr->next;
            free(curr);
            curr = next;
        }
        *headDataJadwal = NULL;
        *tailDataJadwal = NULL;
    }
    NodeData *head = NULL; NodeData *tail = NULL;
    FILE *fptr = fopen(nama_file, "r");
    readCSV(&head, &tail, fptr);
    NodeData *curr = head;
    while(curr != NULL){
        // insertTailDataJadwal(headDataJadwal, tailDataJadwal, curr->data_dokter, countShift(curr->data_dokter.nama, jadwal_dokter, ));
        curr = curr->next;
    }
}

// Print nama dokter pada suatu jadwal dalam satu bulan 
void printAllJadwal(Jadwal jadwal_dokter[]){
    int hari;
    char nama[100];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;
    while(jadwal_dokter[0].shift_pagi[jumlah_pagi].nama[0] != '\0'){
        jumlah_pagi++;
    }

    while(jadwal_dokter[0].shift_siang[jumlah_siang].nama[0] != '\0'){
        jumlah_siang++;
    }

    while(jadwal_dokter[0].shift_malam[jumlah_malam].nama[0] != '\0'){
        jumlah_malam++;
    }

    for(int i = 0; i < 30; i++){
        hari = jadwal_dokter[i].hari;
        printf("Hari ke-%d\n",hari);

        printf("Shift pagi: ");
        for (int j = 0; j < jumlah_pagi; j++){
            (j != jumlah_pagi-1) ? printf("%s, ", jadwal_dokter[i].shift_pagi[j].nama) : printf("%s\n", jadwal_dokter[i].shift_pagi[j].nama);
        }

        printf("Shift siang: ");
        for (int j = 0; j < jumlah_siang; j++){
            (j != jumlah_siang-1) ? printf("%s, ", jadwal_dokter[i].shift_siang[j].nama) : printf("%s\n", jadwal_dokter[i].shift_siang[j].nama);
        }

        printf("Shift malam: ");
        for (int j = 0; j < jumlah_malam; j++){
            (j != jumlah_malam-1) ? printf("%s, ", jadwal_dokter[i].shift_malam[j].nama) : printf("%s\n\n", jadwal_dokter[i].shift_malam[j].nama);
        }
    }
}

void jadwalHari(Jadwal jadwal_dokter[], int hari){
    char nama[100];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;
    while(jadwal_dokter[0].shift_pagi[jumlah_pagi].nama[0] != '\0'){
        jumlah_pagi++;
    }

    while(jadwal_dokter[0].shift_siang[jumlah_siang].nama[0] != '\0'){
        jumlah_siang++;
    }

    while(jadwal_dokter[0].shift_malam[jumlah_malam].nama[0] != '\0'){
        jumlah_malam++;
    }

    Jadwal jadwalHari = get_jadwal_hari(jadwal_dokter, hari);
    printf("Hari ke-%d\n",hari);

    printf("Shift pagi: ");
    for (int j = 0; j < jumlah_pagi; j++){
        (j != jumlah_pagi-1) ? printf("%s, ", jadwalHari.shift_pagi[j].nama) : printf("%s\n", jadwalHari.shift_pagi[j].nama);
    }

    printf("Shift siang: ");
    for (int j = 0; j < jumlah_siang; j++){
        (j != jumlah_siang-1) ? printf("%s, ", jadwalHari.shift_siang[j].nama) : printf("%s\n", jadwalHari.shift_siang[j].nama);
    }

    printf("Shift malam: ");
    for (int j = 0; j < jumlah_malam; j++){
        (j != jumlah_malam-1) ? printf("%s, ", jadwalHari.shift_malam[j].nama) : printf("%s\n\n", jadwalHari.shift_malam[j].nama);
    }
}

void jadwalMinggu(Jadwal jadwal_dokter[], int minggu){
    int hari = 0; int hariPerminggu = 0;
    hariPerminggu = (minggu != 5) ? 7 : 2;

    char nama[100];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;
    while(jadwal_dokter[0].shift_pagi[jumlah_pagi].nama[0] != '\0'){
        jumlah_pagi++;
    }

    while(jadwal_dokter[0].shift_siang[jumlah_siang].nama[0] != '\0'){
        jumlah_siang++;
    }

    while(jadwal_dokter[0].shift_malam[jumlah_malam].nama[0] != '\0'){
        jumlah_malam++;
    }

    Jadwal *JadwalMinggu = (Jadwal *)malloc(7*sizeof(Jadwal));
    get_jadwal_minggu(jadwal_dokter, minggu, JadwalMinggu);

    for(int i = 0; i < hariPerminggu; i++){
        hari = JadwalMinggu[i].hari;
        printf("Hari ke-%d\n",hari);

        printf("Shift pagi: ");
        for (int j = 0; j < jumlah_pagi; j++){
            (j != jumlah_pagi-1) ? printf("%s, ", jadwal_dokter[i].shift_pagi[j].nama) : printf("%s\n", jadwal_dokter[i].shift_pagi[j].nama);
        }

        printf("Shift siang: ");
        for (int j = 0; j < jumlah_siang; j++){
            (j != jumlah_siang-1) ? printf("%s, ", jadwal_dokter[i].shift_siang[j].nama) : printf("%s\n", jadwal_dokter[i].shift_siang[j].nama);
        }

        printf("Shift malam: ");
        for (int j = 0; j < jumlah_malam; j++){
            (j != jumlah_malam-1) ? printf("%s, ", jadwal_dokter[i].shift_malam[j].nama) : printf("%s\n\n", jadwal_dokter[i].shift_malam[j].nama);
        }
    }
}

void pemilihanJadwal(Jadwal jadwal_dokter[], const char *nama_file, int mode, int hari, int minggu){
    switch (mode)
    {
        case 1:
            printf("Masukkan hari: ");
            scanf("%d",&hari);
            printf("\n");
            jadwalHari(jadwal_dokter, hari);
            break;
        case 2:
            printf("Masukkan minggu: ");
            scanf("%d",&minggu);
            printf("\n");
            jadwalMinggu(jadwal_dokter, minggu);
            break;
        case 3:
            printf("\n");
            write_jadwal(jadwal_dokter, nama_file);
            printAllJadwal(jadwal_dokter);
            break;
        default:
            printf("\nPerintah yang anda masukkan salah\n");
            break;
    }
}

void TampilkanShift1(NodeDataJadwal **head, NodeDataJadwal **tail, char *nama){
    NodeDataJadwal *curr = *head;
    while(curr != NULL){
        if(!strcmp(curr->data_dokter.nama, nama)){
            printf("\nNama: %s\n",curr->data_dokter.nama);
            printf("Jumlah shift: %d\n",curr->jumlah_shift);
            return;
        }
        curr = curr->next;
    }
    printf("\nNama tidak ada pada file\n");
}

void TampilkanShiftAll(NodeDataJadwal **head){
    NodeDataJadwal *curr = (NodeDataJadwal *)malloc(sizeof(NodeDataJadwal));
    curr = (*head);
    while(curr != NULL){
        printf("\nNama: %s\n",curr->data_dokter.nama);
        printf("Jumlah shift: %d\n",curr->jumlah_shift);
        curr = curr->next;
    }
}

void pilihanTampilan(NodeDataJadwal **head, NodeDataJadwal **tail){
    char *nama = (char *)malloc(100*sizeof(char));
    int pilihan = 0;

    printf("Menampilkan semua? 1 untuk iya dan 0 untuk tidak\n");
    scanf("%d", &pilihan);
    if(pilihan == 0){
        printf("Masukkan nama: ");
        scanf("%s", nama);
        TampilkanShift1(head, tail, nama);
    } else if (pilihan == 1){
        TampilkanShiftAll(head);
    } else {
        printf("Pilihan tidak valid.\n\n");
    }
}