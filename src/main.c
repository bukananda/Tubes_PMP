#include <stdio.h>
#include <stdlib.h>
#include "../include/pengelolaanData.h"
#include "../include/jadwal.h"
#include "../include/info_jadwal.h"

char *namaFile = "data_dokter.csv";

int main() {
    Jadwal jadwal_dokter[30];
    NodeDataJadwal *headDataJadwal = NULL; NodeDataJadwal *tailDataJadwal = NULL;
    int pilihan;

    AllComponentJadwal(jadwal_dokter, namaFile, 1);
    jumlahShiftDokter(&headDataJadwal, &tailDataJadwal, jadwal_dokter, namaFile);

    do {
        printf("\n==== MENU ====\n");
        printf("1. Tambah Data Dokter\n");
        printf("2. Tampilkan Semua Data\n");
        printf("3. Tampilkan Data Berdasarkan Filter\n");
        printf("4. Hapus Data Dokter\n");
        printf("5. Lakukan Penjadwalan Secara Acak\n");
        printf("6. Tampilkan Jadwal\n");
        printf("7. Tampilkan Jumlah Pelanggaran Preferensi Shift\n");
        printf("8. Tampilkan Jumlah Shift\n");
        printf("0. Keluar\n");
        printf("Pilihan Anda: ");
        scanf("%d", &pilihan);
        getchar(); 

        switch (pilihan) {
            case 1: tambahDataDokter(namaFile); break;
            case 2: tampilkanSemua(namaFile); break;
            case 3: tampilkanFilter(namaFile); break;
            case 4: hapusDataDokter(namaFile); break;
            case 5:
                AllComponentJadwal(jadwal_dokter, namaFile, 0);
                jumlahShiftDokter(&headDataJadwal, &tailDataJadwal, jadwal_dokter, namaFile);
                break;
            case 6: pemilihanJadwal(jadwal_dokter, "jadwal.csv"); break;
            case 7: jumlahPelanggaran(jadwal_dokter); break;
            case 8: pilihanTampilan(&headDataJadwal); break;
            case 0: printf("Keluar dari program.\n"); break;
            default: printf("Pilihan tidak valid.\n"); break;
        }
    } while (pilihan != 0); return 0;
}