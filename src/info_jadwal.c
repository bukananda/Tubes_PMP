#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/jadwal.h"
#include "../include/info_jadwal.h"

// tulis jadwal ke csv
// format : hari, nama dokter shift pagi, nama dokter shift siang, nama dokter shift malam)
void write_jadwal(Jadwal jadwal_dokter[], char *file_name){
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
    for(int i = 0; i<30; i++){
            if (i == (day - 1)){
                return jadwal_dokter[i];
            };
        };
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
void jumlahPelanggaran(Jadwal jadwal_dokter[]){
    int hari;
    char nama[100];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;

    int jumlah_pelanggaran = 0;

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
        for (int j = 0; j < jumlah_pagi; j++){
            if(strcmp(jadwal_dokter[i].shift_pagi[j].pref_shift, "pagi") != 0){jumlah_pelanggaran++; printf("%s\n", jadwal_dokter[i].shift_pagi[j].nama);}
        }

        for (int j = 0; j < jumlah_siang; j++){
            if(strcmp(jadwal_dokter[i].shift_siang[j].pref_shift, "siang") != 0){jumlah_pelanggaran++; printf("%s\n", jadwal_dokter[i].shift_siang[j].nama);}
        }

        for (int j = 0; j < jumlah_malam; j++){
            if(strcmp(jadwal_dokter[i].shift_malam[j].pref_shift, "malam") != 0){jumlah_pelanggaran++; printf("%s\n", jadwal_dokter[i].shift_malam[j].nama);}
        }
    }

    printf("\nPelanggaran Preferensi Shift: %d\n", jumlah_pelanggaran);
}

// Mengetahui jumlah shift pada setiap dokter
int countShift(char nama_dokter[], Jadwal jadwal_dokter[]){
    int hari;
    char nama[100];

    int jumlah_pagi = 0;
    int jumlah_siang = 0;
    int jumlah_malam = 0;

    int jumlah_shift = 0;

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
        for (int j = 0; j < jumlah_pagi; j++){
            if(!strcmp(jadwal_dokter[i].shift_pagi[j].nama, nama_dokter)){jumlah_shift++;}
        }

        for (int j = 0; j < jumlah_siang; j++){
            if(!strcmp(jadwal_dokter[i].shift_siang[j].nama, nama_dokter)){jumlah_shift++;}
        }

        for (int j = 0; j < jumlah_malam; j++){
            if(!strcmp(jadwal_dokter[i].shift_malam[j].nama, nama_dokter)){jumlah_shift++;}
        }
    }

    return jumlah_shift;
}

// Menghitung jumlah shift pada semua dokter
void jumlahShiftDokter(NodeDataJadwal **headDataJadwal, NodeDataJadwal **tailDataJadwal, Jadwal jadwal_dokter[], char *nama_file){
    NodeData *head = NULL; NodeData *tail = NULL;
    FILE *fptr = fopen(nama_file, "r");
    readCSV(&head, &tail, fptr);
    while(head != NULL){
        insertTailDataJadwal(headDataJadwal, tailDataJadwal, head->data_dokter, countShift(head->data_dokter.nama, jadwal_dokter));
        head = head->next;
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

void pemilihanJadwal(Jadwal jadwal_dokter[], char *nama_file){
    int pilihan = 0; int pilihanCTK = 0;
    int hari = 0; int minggu = 0;
    printf("\n1. Dalam hari\n");
    printf("2. Dalam minggu\n");
    printf("3. Semua\n");
    printf("Pilihan Anda: ");
    scanf("%d", &pilihan);
    switch (pilihan)
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
        printf("Lakukan pencetakan? 1 untuk iya dan 0 untuk tidak\n");
        scanf("%d",&pilihanCTK);
        printf("\n");
        if(pilihanCTK == 1){
            write_jadwal(jadwal_dokter, nama_file);
            printAllJadwal(jadwal_dokter);
        } else if(pilihanCTK == 0){
            printAllJadwal(jadwal_dokter);
        } else {
            printf("Perintah yang anda masukkan salah\n");
        } break;
    default:
        printf("\nPerintah yang anda masukkan salah\n");
        break;
    }
}

void TampilkanShift1(NodeDataJadwal **head, char *nama){
    NodeDataJadwal *curr = (NodeDataJadwal *)malloc(sizeof(NodeDataJadwal));
    curr = (*head);
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

void pilihanTampilan(NodeDataJadwal **head){
    char *nama = (char *)malloc(100*sizeof(char));
    int pilihan = 0;

    printf("Menampilkan semua? 1 untuk iya dan 0 untuk tidak\n");
    scanf("%d", &pilihan);
    if(pilihan == 0){
        printf("Masukkan nama: ");
        scanf("%s", nama);
        TampilkanShift1(head, nama);
    } else if (pilihan == 1){
        TampilkanShiftAll(head);
    } else {
        printf("Pilihan tidak valid.\n\n");
    }
}