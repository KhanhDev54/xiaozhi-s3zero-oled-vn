#include "dns_server.h"
#include <esp_log.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#define TAG "DnsServer"

DnsServer::DnsServer() {
}

DnsServer::~DnsServer() {
    Stop();
}

void DnsServer::Start(esp_ip4_addr_t gateway) {
    // If already running, stop first
    if (running_) {
        Stop();
    }

    ESP_LOGI(TAG, "Starting DNS server");
    gateway_ = gateway;

    fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd_ < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_);

    if (bind(fd_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "failed to bind port %d", port_);
        close(fd_);
        fd_ = -1;
        return;
    }

    running_ = true;
    xTaskCreate([](void* arg) {
        DnsServer* dns_server = static_cast<DnsServer*>(arg);
        dns_server->Run();
        vTaskDelete(NULL);
    }, "DnsServerTask", 4096, this, 5, &task_handle_);
}

void DnsServer::Stop() {
    if (!running_) {
        return;
    }

    ESP_LOGI(TAG, "Stopping DNS server");
    running_ = false;

    // Close socket to unblock recvfrom
    if (fd_ >= 0) {
        shutdown(fd_, SHUT_RDWR);
        close(fd_);
        fd_ = -1;
    }

    // Wait for task to finish
    if (task_handle_ != nullptr) {
        // Give the task some time to exit gracefully
        vTaskDelay(pdMS_TO_TICKS(100));
        task_handle_ = nullptr;
    }
}

void DnsServer::Run() {
    char buffer[512];
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int len = recvfrom(fd_, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            if (!running_) {
                // Socket was closed during Stop(), exit gracefully
                break;
            }
            ESP_LOGE(TAG, "recvfrom failed, errno=%d", errno);
            continue;
        }

        if (!running_) {
            break;
        }

        // KenDev: build a well-formed reply. The old code appended an A record after
        // whatever the query carried (EDNS OPT record, AAAA/HTTPS question types), which
        // phones parse as a malformed answer and retry, delaying the captive-portal page.
        // Now: keep only the header + first question, answer A queries with the AP
        // address, and answer every other type with an empty NOERROR reply.
        if (len < 12) {
            continue;
        }
        int pos = 12;
        while (pos < len && buffer[pos] != 0) {
            pos += (uint8_t)buffer[pos] + 1;  // skip one label
        }
        if (pos + 5 > len) {
            continue;  // truncated / not a normal question
        }
        uint16_t qtype = ((uint8_t)buffer[pos + 1] << 8) | (uint8_t)buffer[pos + 2];
        len = pos + 5;  // end of the first question

        buffer[2] = (char)(0x84 | (buffer[2] & 0x01));  // QR=1, AA=1, keep RD
        buffer[3] = (char)0x80;                          // RA=1, RCODE=0
        buffer[4] = 0; buffer[5] = 1;                    // QDCOUNT = 1
        buffer[6] = 0; buffer[7] = 0;                    // ANCOUNT
        buffer[8] = 0; buffer[9] = 0;                    // NSCOUNT
        buffer[10] = 0; buffer[11] = 0;                  // ARCOUNT

        if (qtype == 1 && len + 16 <= (int)sizeof(buffer)) {  // A record
            buffer[7] = 1;
            memcpy(&buffer[len], "\xc0\x0c", 2);  // Name pointer
            len += 2;
            memcpy(&buffer[len], "\x00\x01\x00\x01\x00\x00\x00\x1c\x00\x04", 10);  // Type, class, TTL, data length
            len += 10;
            memcpy(&buffer[len], &gateway_.addr, 4);  // 192.168.4.1
            len += 4;
        }
        {
            // Log the queried name (diagnostics for slow captive-portal detection)
            char name[96];
            int n = 0;
            for (int i = 12; i < pos && n < (int)sizeof(name) - 2;) {
                int l = (uint8_t)buffer[i++];
                if (n > 0) name[n++] = '.';
                for (int k = 0; k < l && i < pos && n < (int)sizeof(name) - 1; k++) name[n++] = buffer[i++];
            }
            name[n] = 0;
            ESP_LOGI(TAG, "DNS %s %s", qtype == 1 ? "A" : (qtype == 28 ? "AAAA" : (qtype == 65 ? "HTTPS" : "other")), name);
        }

        sendto(fd_, buffer, len, 0, (struct sockaddr *)&client_addr, client_addr_len);
    }

    task_handle_ = nullptr;
    ESP_LOGI(TAG, "DNS server task exiting");
}
