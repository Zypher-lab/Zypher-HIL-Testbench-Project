#include "wifi_transport.h"

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/socket.h>
#include <string.h>

#define WIFI_SSID      "The-Avengers"
#define WIFI_PASSWORD  "readyforSSH12345"
#define TCP_PORT       5000

static struct net_mgmt_event_callback wifi_cb;
static struct k_sem                   ip_ready_sem;
static int server_fd = -1;
static int client_fd = -1;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t mgmt_event,
                               struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_DHCP_BOUND) {
        printk("IP address assigned\n");
        k_sem_give(&ip_ready_sem);
    }
}

int wifi_transport_init(void)
{
    k_sem_init(&ip_ready_sem, 0, 1);

    /* wait for WiFi stack to initialize */
    k_sleep(K_SECONDS(2));

    /* register for IP address event */
    net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
                                 NET_EVENT_IPV4_DHCP_BOUND);
    net_mgmt_add_event_callback(&wifi_cb);

    /* get STA interface */
    struct net_if *iface = net_if_get_wifi_sta();
    if (!iface) {
        iface = net_if_get_default();
    }

    /* connect */
    struct wifi_connect_req_params params = {
        .ssid        = (const uint8_t *)WIFI_SSID,
        .ssid_length = strlen(WIFI_SSID),
        .psk         = (const uint8_t *)WIFI_PASSWORD,
        .psk_length  = strlen(WIFI_PASSWORD),
        .channel     = WIFI_CHANNEL_ANY,
        .security    = WIFI_SECURITY_TYPE_PSK,
        .band        = WIFI_FREQ_BAND_2_4_GHZ,
        .mfp         = WIFI_MFP_OPTIONAL,
    };

    printk("Connecting to WiFi: %s\n", WIFI_SSID);
    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    if (ret) {
        printk("Connect request failed: %d\n", ret);
        return ret;
    }

    /* wait for IP address — up to 30 seconds */
    printk("Waiting for IP address...\n");
    ret = k_sem_take(&ip_ready_sem, K_SECONDS(30));
    if (ret) {
        printk("Timeout waiting for IP\n");
        return -ETIMEDOUT;
    }

    /* print IP */
    struct net_if_ipv4 *ipv4 = iface->config.ip.ipv4;
    if (ipv4 && ipv4->unicast[0].ipv4.is_used) {
        char ip_str[NET_IPV4_ADDR_LEN];
        net_addr_ntop(AF_INET,
                      &ipv4->unicast[0].ipv4.address.in_addr,
                      ip_str, sizeof(ip_str));
        printk("ZTB|transport=WIFI|ip=%s|port=%d\r\n", ip_str, TCP_PORT);
    }

    /* start TCP server */
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(TCP_PORT),
    };
    addr.sin_addr.s_addr = INADDR_ANY;

    server_fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) return server_fd;

    int opt = 1;
    zsock_setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    zsock_bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    zsock_listen(server_fd, 1);

    printk("TCP server listening on port %d\n", TCP_PORT);
    return 0;
}

void wifi_transport_send_line(const char *line)
{
    if (client_fd < 0) return;
    zsock_send(client_fd, line, strlen(line), 0);
}

void wifi_transport_read_line(char *buffer, size_t buffer_size)
{
    size_t index = 0;

    while (1) {
        if (client_fd < 0) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            client_fd = zsock_accept(server_fd,
                                     (struct sockaddr *)&client_addr,
                                     &addr_len);
            if (client_fd < 0) {
                k_msleep(10);
                continue;
            }
            printk("Client connected\n");
        }

        char c;
        int ret = zsock_recv(client_fd, &c, 1, 0);

        if (ret <= 0) {
            zsock_close(client_fd);
            client_fd = -1;
            index = 0;
            printk("Client disconnected\n");
            continue;
        }

        if (c == '\r' || c == '\n') {
            if (index == 0) continue;
            buffer[index] = '\0';
            return;
        }

        if (index < buffer_size - 1) {
            buffer[index++] = c;
        }
    }
}
