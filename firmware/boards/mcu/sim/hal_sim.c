/**
 * @file hal_sim.c
 * @brief PC simulation HAL: UDP transport + virtual sensors
 */
#ifdef ESC_PLATFORM_SIM

#include "hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#endif

static uint32_t s_start_ms;
static float s_duty[3] = {0.5f, 0.5f, 0.5f};
static uint16_t s_pwm_in_us = 1000;
static int s_sock = -1;
static int s_can_sock = -1;
static struct sockaddr_in s_uart_peer;
static int s_uart_peer_set;
static struct sockaddr_in s_can_peer;
static int s_can_peer_set;
static char s_nvm_path[260];

static uint32_t now_ms(void)
{
#ifdef _WIN32
    return (uint32_t)GetTickCount() - s_start_ms;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u) - s_start_ms;
#endif
}

void hal_init(void)
{
    s_start_ms = now_ms();
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    s_sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    {
        int yes = 1;
        setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(7777);
    if (bind(s_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "[sim] bind :7777 failed (port in use?)\n");
        fflush(stderr);
    }
    memset(&s_uart_peer, 0, sizeof(s_uart_peer));
    s_uart_peer_set = 0;
    memset(&s_can_peer, 0, sizeof(s_can_peer));
    s_can_peer_set = 0;
    s_can_sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in can_addr;
    memset(&can_addr, 0, sizeof(can_addr));
    can_addr.sin_family = AF_INET;
    can_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    can_addr.sin_port = htons(7779);
    if (bind(s_can_sock, (struct sockaddr *)&can_addr, sizeof(can_addr)) != 0) {
        fprintf(stderr, "[sim] bind :7779 failed\n");
        fflush(stderr);
    }
    snprintf(s_nvm_path, sizeof(s_nvm_path), "esc32_nvm.bin");
#ifdef _WIN32
    {
        u_long nb = 1;
        ioctlsocket(s_sock, FIONBIO, &nb);
        ioctlsocket(s_can_sock, FIONBIO, &nb);
    }
#endif
    printf("[sim] UART UDP :7777  CAN UDP :7779\n");
    fflush(stdout);
}

uint32_t hal_time_us(void)
{
    return now_ms() * 1000u;
}

uint32_t hal_millis(void)
{
    return now_ms();
}

void hal_delay_ms(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000u);
#endif
}

void hal_pwm_set(float duty_a, float duty_b, float duty_c)
{
    s_duty[0] = duty_a;
    s_duty[1] = duty_b;
    s_duty[2] = duty_c;
}

/* Filled in by motor.c when ESC_PLATFORM_SIM is defined; see the rotor
 * model block at the end of motor_fast_loop(). They give hal_adc_read()
 * believable bus voltage / current readings that respond to throttle. */
extern float g_sim_ibus_a;
extern float g_sim_vbus_v;

void hal_adc_read(float *vbus, float *ibus, float *ia, float *ib, float *ic,
                      float *temp_mos)
{
    *vbus = g_sim_vbus_v;
    *ibus = g_sim_ibus_a;
    *ia = *ibus * 0.33f;
    *ib = *ibus * 0.33f;
    *ic = -*ia - *ib;
    *temp_mos = 35.0f + (*ibus) * 0.2f;
}

uint16_t hal_pwm_input_us(void)
{
    return s_pwm_in_us;
}

void hal_pwm_input_simulate(uint16_t us)
{
    s_pwm_in_us = us;
}

void hal_can_init(uint32_t baudrate)
{
    (void)baudrate;
}

int hal_can_send(uint32_t can_id, const uint8_t *data, uint8_t len)
{
    if (s_can_sock < 0 || !s_can_peer_set) {
        return 0;
    }
    uint8_t pkt[13];
    pkt[0] = (uint8_t)(can_id & 0xFF);
    pkt[1] = (uint8_t)((can_id >> 8) & 0xFF);
    pkt[2] = (uint8_t)((can_id >> 16) & 0xFF);
    pkt[3] = (uint8_t)((can_id >> 24) & 0xFF);
    pkt[4] = len;
    if (len > 8) {
        len = 8;
    }
    memcpy(&pkt[5], data, len);
    return (int)sendto(s_can_sock, (const char *)pkt, (size_t)(5 + len), 0,
                       (struct sockaddr *)&s_can_peer, sizeof(s_can_peer));
}

int hal_can_receive(uint32_t *can_id, uint8_t *data, uint8_t *len, uint8_t cap)
{
    if (s_can_sock < 0) {
        return 0;
    }
    uint8_t pkt[13];
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
    int n = (int)recvfrom(s_can_sock, (char *)pkt, (int)sizeof(pkt), 0,
                          (struct sockaddr *)&from, &flen);
    if (n < 5) {
        return 0;
    }
    s_can_peer = from;
    s_can_peer_set = 1;
    *can_id = (uint32_t)pkt[0] | ((uint32_t)pkt[1] << 8) |
              ((uint32_t)pkt[2] << 16) | ((uint32_t)pkt[3] << 24);
    *len = pkt[4];
    if (*len > cap) {
        *len = cap;
    }
    memcpy(data, &pkt[5], *len);
    return 1;
}

int hal_nvm_read(uint32_t offset, void *buf, size_t len)
{
    FILE *f = fopen(s_nvm_path, "rb");
    if (!f) {
        memset(buf, 0xFF, len);
        return -1;
    }
    fseek(f, (long)offset, SEEK_SET);
    size_t n = fread(buf, 1, len, f);
    fclose(f);
    return (n == len) ? 0 : -1;
}

int hal_nvm_write(uint32_t offset, const void *buf, size_t len)
{
    FILE *f = fopen(s_nvm_path, "r+b");
    if (!f) {
        f = fopen(s_nvm_path, "w+b");
    }
    if (!f) {
        return -1;
    }
    fseek(f, (long)offset, SEEK_SET);
    size_t n = fwrite(buf, 1, len, f);
    fclose(f);
    return (n == len) ? 0 : -1;
}

bool hal_gpio_boot_pin_active(void)
{
    return false;
}

void hal_system_reset(void)
{
    printf("[sim] reset\n");
    exit(0);
}

int hal_uart_write(const uint8_t *data, size_t len)
{
    return hal_udp_send(data, len);
}

int hal_uart_read(uint8_t *data, size_t cap)
{
    return hal_udp_poll(data, cap);
}

int hal_udp_send(const uint8_t *data, size_t len)
{
    if (s_sock < 0 || !s_uart_peer_set) {
        return 0;
    }
    return (int)sendto(s_sock, (const char *)data, (int)len, 0,
                       (struct sockaddr *)&s_uart_peer, sizeof(s_uart_peer));
}

int hal_udp_poll(uint8_t *buf, size_t cap)
{
    if (s_sock < 0) {
        return 0;
    }
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
    int n = (int)recvfrom(s_sock, (char *)buf, (int)cap, 0,
                          (struct sockaddr *)&from, &flen);
    if (n > 0) {
        s_uart_peer = from;
        s_uart_peer_set = 1;
#ifdef ESC_DEBUG_UDP
        fprintf(stderr, "[sim] uart rx %d:", n);
        for (int i = 0; i < n && i < 8; i++) {
            fprintf(stderr, " %02X", buf[i]);
        }
        fprintf(stderr, "\n");
        fflush(stderr);
#endif
    }
    return n > 0 ? n : 0;
}

#endif /* ESC_PLATFORM_SIM */
