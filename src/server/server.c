#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define handle_error(vl, msg) \
if(vl == -1) { perror(msg); return EXIT_FAILURE; }
#define handle_error_socket(vl, msg, server_sock_fd) \
if(vl == -1) { perror(msg); if (close(server_sock_fd) == -1) perror("close socket"); return EXIT_FAILURE; }

#define LISTENING_ADDRESS "0.0.0.0" // The listening ip address (Example: 127.0.0.1 for lo / 0.0.0.0 for all)

#define ANSWER_MORE 1
#define ANSWER_STOP 0

void send_answer(int sock_fd, int answer, double* origin);
void pts_cg(const double *tab_pt, int n, double *cg_pt);
void pt_print(double abs, double ord);
void pts_print(const double *tab_pt, int n);

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <port> <p>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]); // Can be converted to strtol to better implem (using errno etc) but is ok bc return 0 on error and 0 is not an accepted value.
    if(port < 1 || port > 65535) { fprintf(stderr, "Port should be an int between 1 and 65535.\n"); return EXIT_FAILURE; }

    int p = atoi(argv[2]);
    if (p < 1) { fprintf(stderr, "The number p should be > 1"); return EXIT_FAILURE; }

    struct sockaddr_in sa;
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;
    bzero(sa.sin_zero, 8);

    int err = inet_aton(LISTENING_ADDRESS, &sa.sin_addr); // convert string version of the listening address to a binary form (in network byte order)
    if(err == 0) { fprintf(stderr, "An error occurred while parsing the IP '%s'.\n", LISTENING_ADDRESS); return EXIT_FAILURE; }

    int server_sock_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    handle_error(server_sock_fd, "socket");

    struct sockaddr_in peer_addr;

    int enable = 1;
    err = setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    handle_error_socket(err, "setsockopt", server_sock_fd);

    err = bind(server_sock_fd, (struct sockaddr *) &sa, sizeof(struct sockaddr));
    handle_error_socket(err, "bind", server_sock_fd);

    err = listen(server_sock_fd, SOMAXCONN); // Here, `SOMAXCONN` as backlog to use `/proc/sys/net/core/somaxconn` (The system defined backlog value) By default on my system: 4096
    handle_error_socket(err, "listen", server_sock_fd);

    socklen_t peer_addr_size = sizeof(peer_addr);
    int client_socket_fd = accept(server_sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    handle_error_socket(client_socket_fd, "accept", server_sock_fd);

    int count_received = 0;
    double* origin = malloc(sizeof(double) * p * 2);
    if (origin == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    while (count_received < p) {
        if (count_received != 0) send_answer(client_socket_fd, ANSWER_MORE, origin);
        size_t received_size = recv(client_socket_fd, (origin + count_received*2), sizeof(double)*2, 0);
        if (received_size == -1) {
            perror("recv"); free(origin);
            close(client_socket_fd);
            return EXIT_FAILURE;
        }
        if (origin[count_received*2] == -1.0 && origin[count_received*2+1] == -1.0) {
            // there is an error;
            fprintf(stderr, "An error occurred, the client has not generated enough points.");
            close(client_socket_fd);
            free(origin);
            return EXIT_FAILURE;
        }
        count_received++;
    }
    send_answer(client_socket_fd, ANSWER_STOP, origin);
    pts_print(origin, p);

    double gravity[2];
    pts_cg(origin, p, gravity);
    free(origin);

    size_t sending_gravity_size = send(client_socket_fd, &gravity, sizeof(double)*2, 0);
    if (sending_gravity_size == -1) {
        perror("send");
        close(client_socket_fd);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

void send_answer(int sock_fd, int answer, double* origin) {
    size_t sent = send(sock_fd, &answer, sizeof(int), 0);
    if (sent == -1) { perror("send"); free(origin); exit(EXIT_FAILURE); }
}

/**
 * Affiche les coordonnées d'un point (abscisse et ordonnée).
 *
 * @param abs abscisse
 * @param ord ordonnée
 */
void pt_print(double abs, double ord){
    printf("(%.2f,%.2f) ", abs, ord);
}

/**
 * Affiche les coordonnées des points d'un tableau (abscisse et ordonnée).
 *
 * @param tab_pt tableau des coordonnées des points
 * @param n nombre de points
 */
void pts_print(const double *tab_pt, int n){
    for (int i=0; i<2*n; i+=2){
        pt_print(tab_pt[i], tab_pt[i+1]);
        printf("\n");
    }
    printf("\n");
}

void pts_cg(const double *tab_pt, int n, double *cg_pt){
    cg_pt[0]=0; cg_pt[1]=0;
    for (int i=0; i<2*n; i+=2) {
        cg_pt[0] += tab_pt[i]; cg_pt[1] += tab_pt[i+1];
    }
    cg_pt[0] /= n; cg_pt[1] /= n;
}
