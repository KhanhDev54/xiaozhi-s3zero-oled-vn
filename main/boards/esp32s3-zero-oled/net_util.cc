#include "net_util.h"

#include <cctype>
#include <cstdio>

#include "board.h"

namespace zero {

bool HttpGetText(const std::string& url, std::string& body, std::string& error) {
    auto network = Board::GetInstance().GetNetwork();
    if (network == nullptr) {
        error = "no network";
        return false;
    }
    auto http = network->CreateHttp(0);
    http->SetHeader("User-Agent", "xiaozhi-s3zero-oled-vn");
    if (auto opened = http->Open("GET", url); !opened) {
        error = "open failed: " + opened.error().ToString();
        return false;
    }
    auto status = http->GetStatusCode();
    if (!status) {
        error = "status failed: " + status.error().ToString();
        http->Close();
        return false;
    }
    if (*status != 200) {
        error = "HTTP " + std::to_string(*status);
        http->Close();
        return false;
    }
    body = http->ReadAll();
    http->Close();
    if (body.empty()) {
        error = "empty body";
        return false;
    }
    return true;
}

std::string UrlEncode(const std::string& value) {
    std::string out;
    char buf[12];
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            snprintf(buf, sizeof(buf), "%%%02X", c);
            out += buf;
        }
    }
    return out;
}

}  // namespace zero
