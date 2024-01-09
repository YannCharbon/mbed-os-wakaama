# mbed-os-wakaama

Wakaama LwM2M implementation for Mbed OS using Nanostack C socket API.

Current Wakamaa commit point is `https://github.com/eclipse/wakaama/commit/514659414c792f8c252782af63551b6d09704a7b`

Note: To dissect the packets in Wireshark correctly, find the initial packet (begining of the DTLS transaction) and make it to Decode As DTLS.