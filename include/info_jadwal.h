#ifndef INFO_JADWAL_H
#define INFO_JADWAL_H

// Modifikasi Nodedata tetapi berisi jumlah_shiftnya juga
typedef struct NodeDataJadwal{
    Data data_dokter;
    int jumlah_shift;
    struct NodeDataJadwal *next;
} NodeDataJadwal;

Jadwal get_jadwal_hari(Jadwal jadwal_dokter[], int day);
void get_jadwal_minggu(Jadwal jadwal_dokter[], int minggu, Jadwal *jadwal_minggu);
void get_jadwal_bulan(Jadwal jadwal_dokter[], Jadwal *jadwal_bulan);
void jumlahPelanggaran(Jadwal jadwal_dokter[]);
void jumlahShiftDokter(NodeDataJadwal **headDataJadwal, NodeDataJadwal **tailDataJadwal, Jadwal jadwal_dokter[], char *nama_file);
void pemilihanJadwal(Jadwal jadwal_dokter[], char *nama_file);
void pilihanTampilan(NodeDataJadwal **head, NodeDataJadwal **tail);

#endif