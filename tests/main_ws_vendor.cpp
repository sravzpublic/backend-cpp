/**
 * Sravz LLC
 * TODO: Handle multiple topics
 **/
#include <main.hpp>

int main(int argc, char **argv) {
    ws_vendor("ws.eodhistoricaldata.com", "443",
              "{\"action\": \"subscribe\", \"symbols\": \"ETH-USD, BTC-USD\"}",
              "/ws/crypto?api_token=demo", 3);
}