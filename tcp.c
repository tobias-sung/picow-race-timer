#include "picow-race-timer.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"


//#define TEST_TCP_SERVER_IP "192.168.1.81"
//#define TCP_PORT 4242

#define BUF_SIZE 2048
#define POLL_TIME_S 5

char* tcp_ip;
int tcp_port;

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    bool connected;
} TCP_CLIENT_T;

TCP_CLIENT_T* tcp_client;

err_t tcp_client_close(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        } else {
            printf("TCP connection closed.");
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

TCP_CLIENT_T* tcp_client_init(void) {
    TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }
    ip4addr_aton(tcp_ip, &state->remote_addr);
    return state;
}


err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tcp_client_close(arg);;
    }
    printf("connected to ip:%s\n", ip4addr_ntoa(&state->remote_addr));
    state->connected = true;

    return ERR_OK;
}

void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        printf("tcp_client_err %d\n", err);
        return tcp_client_close(arg);;
    }
}

err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    return ERR_OK;
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    
    if (!p) {
        printf("Response payload is empty.\n");
        return tcp_client_close(arg);;
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        //printf("recv %d err %d\n", p->tot_len, err);
        uint8_t *buf = p->payload;
        buf[p->tot_len]='\0';
        //printf("received data:%s\n", buf);
        tcp_recved(tpcb, p->tot_len);
    }

    pbuf_free(p);

    return ERR_OK;
}

err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    return ERR_OK; 
}

bool tcp_client_open(void *arg) {
    
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), tcp_port);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    if (!state->tcp_pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, tcp_port, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

bool tcp_start(char* server, int port){
    tcp_ip = server;
    tcp_port = port;
    tcp_client = tcp_client_init();
    if (!tcp_client){
        printf("TCP client failed to initialize. \n");
        return -1;
    }
    if (!tcp_client_open(tcp_client)){
        return tcp_client_close(tcp_client);
    }
}

void tcp_send(char message[]){
    
    cyw43_arch_lwip_begin();

    err_t err = tcp_write(tcp_client->tcp_pcb, message, strlen(message), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("Failed to write data %d\n", err);
        return;
    }
    tcp_output(tcp_client->tcp_pcb);
            
    cyw43_arch_lwip_end(); 


}