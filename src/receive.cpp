
#include <M5Unified.h>
#include <receive.h>
#include <QuickEspNow.h>
#include "main.h"
#include <lvgl_porting.h>

unsigned long beepEndTime = 0;
bool beepActive = false;
#define MAX_DEVICES 10                   // Maksimum jumlah pengirim

void playBeep(void) { 
  unsigned long currentTime = millis();
    
    // Jika waktu telah melebihi durasi beep (3000 ms) dan status disconnected
    if (beepActive && (currentTime - beepEndTime > 3000)) {
        // Matikan beep setelah 3 detik
        M5.Speaker.tone(0);  // Hentikan suara beep
        beepActive = false;
    }

    // Jika perangkat masih disconnected dan beep tidak aktif
    if (!beepActive) {
        M5.Speaker.tone(4000, 100); // Bunyi beep
        beepEndTime = currentTime;  // Atur waktu beep dimulai
        beepActive = true;           // Tandai bahwa beep sedang aktif
    }
}

// Menyimpan alamat MAC pengirim dan label terkait
struct SenderInfo {
    uint8_t address[6];  // Alamat MAC pengirim
    lv_obj_t *label;     // Label LVGL untuk pengirim ini
    lv_obj_t *btn;       // Tombol untuk menghapus pairing
    unsigned long lastReceived; // Waktu terakhir data diterima
    bool isDisconnected; // Status disconnect
};

std::vector<SenderInfo> senders;  // Daftar pengirim
const unsigned long TIMEOUT_MS = 3000;  // Waktu timeout untuk disconnect

// Fungsi untuk menghapus pengirim dari daftar dan dari layar
void delete_sender(lv_event_t *e) {
    SenderInfo *sender = (SenderInfo *)lv_event_get_user_data(e);
    
    // Mencari pengirim yang sesuai
    auto it = std::find_if(senders.begin(), senders.end(), [&](const SenderInfo &s) {
        return memcmp(s.address, sender->address, 6) == 0;
    });
    
    if (it!= senders.end()) {
        // Hapus objek dari tampilan LVGL hanya jika objek masih valid
         /*  if (it->label) {
            lv_obj_del(it->label); // Hapus label
            it->label = nullptr;   // Set ke null untuk menghindari akses lagi
        }
*/
     if (it->btn) {
            lv_obj_del(it->btn); // Hapus tombol
            lv_obj_del(it->label);
            it->label = nullptr;
            it->btn = nullptr;   // Set ke null untuk menghindari akses lagi
        }
        // Hapus pengirim dari daftar
        senders.erase(it);
    }
}

void create_delete_button(SenderInfo &sender, int index) {
    // Membuat tombol hapus jika belum ada
    if (!sender.btn) {
        sender.btn = lv_btn_create(lv_scr_act());
        lv_obj_align(sender.btn, LV_ALIGN_TOP_LEFT, 220, index * 60);  // Posisi tombol di samping label
        lv_obj_set_size(sender.btn, 100, 30);
        
        lv_obj_t *btn_label = lv_label_create(sender.btn);
        lv_label_set_text(btn_label, "Hapus");
        lv_obj_center(btn_label);

        // Sambungkan event tombol ke fungsi delete_sender
        lv_obj_add_event_cb(sender.btn, delete_sender, LV_EVENT_CLICKED, &sender);
        
    }
}

void update_label(SenderInfo &sender, int rssi) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), MACSTR, MAC2STR(sender.address));

    // Menghitung indeks dari sender dalam vektor senders
    int index = std::distance(senders.begin(), std::find_if(senders.begin(), senders.end(), [&](const SenderInfo &s) {
        return memcmp(s.address, sender.address, 6) == 0;
    }));

    // Update label pengirim
    lv_label_set_text_fmt(sender.label, "Alat (%d)\nRSSI: %d dBm\nFrom: %s", index + 1, rssi, macStr);
    lv_obj_align(sender.label, LV_ALIGN_TOP_LEFT, 10, index * 60);  // Posisi label berdasarkan urutan
  

}

SenderInfo* find_sender(uint8_t* address) {
    for (auto &sender : senders) {
        if (memcmp(sender.address, address, 6) == 0) {
            return &sender;
        }
    }
    return nullptr;
}

void dataReceived(uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
    SenderInfo *sender = find_sender(address);

    if (!sender) {
        if (senders.size() < MAX_DEVICES) {
            // Pengirim baru, tambahkan ke daftar
            SenderInfo newSender;
            memcpy(newSender.address, address, 6);
            newSender.label = lv_label_create(lv_scr_act());
            newSender.btn = nullptr;  // Tombol belum dibuat
            newSender.isDisconnected = false;  // Status aktif
            senders.push_back(newSender);
            sender = &senders.back();
           
          
        } else  {
            Serial.println("Jumlah pengirim melebihi batas!");
            return;
        }
    } 


    // Reset status disconnect dan waktu terakhir penerimaan
    sender->isDisconnected = false;
    sender->lastReceived = millis();
  
    // Update label dan buat tombol jika perlu
    update_label(*sender, rssi);
}

void checkTimeout() {
    unsigned long currentTime = millis();
    for (auto &sender : senders) {
        if (currentTime - sender.lastReceived > TIMEOUT_MS && !sender.isDisconnected) {
            // Pengirim ini dianggap disconnect
            int index = std::distance(senders.begin(), std::find_if(senders.begin(), senders.end(), [&](const SenderInfo &s) {
                return memcmp(s.address, sender.address, 6) == 0;
            }));
            
            lv_label_set_text_fmt(sender.label, "Alat (%d)\nStatus: DISCONNECTED", index + 1);
            
            create_delete_button(sender, index);  // Buat tombol hapus saat disconnect
            sender.isDisconnected = true;  // Tandai sebagai disconnected
        }    
            if (sender.isDisconnected){
                playBeep();
        
            } 
    }
}
           