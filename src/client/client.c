#define _DEFAULT_SOURCE

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <stdbool.h>
#include <time.h>

/* cst pour les coordonnées */
#define MIN_C 0
#define MAX_C 9
#define DIV 10.0

#define ANSWER_MORE 1
#define ANSWER_STOP 0


int gen_int_rand(int min, int max);
void pts_gen(double *tab_pt, int n);
void pt_print(double abs, double ord);
void pts_print(const double *tab_pt, int n);

int main(int argc, char **argv) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <address_str> <port> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* address_str = argv[1];
    int port = atoi(argv[2]);
    if(port < 1 || port > 65535) { fprintf(stderr, "The port should be an int between 1 and 65535.\n"); return EXIT_FAILURE; }
    int n = atoi(argv[3]);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0; hints.ai_protocol = 0;

    struct addrinfo *result, *rp;

    int s = getaddrinfo(address_str, argv[2], &hints, &result);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int socket_fd = -1;
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(socket_fd == -1) continue;
        if(connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(socket_fd);
    }
    freeaddrinfo(result);
    if(rp == NULL || socket_fd == -1) {
        fprintf(stderr, "Could not connect\n");
        return EXIT_FAILURE;
    }

    // Le client génère n points dans un repère orthonormée
    double* origin = malloc(sizeof(double) * n * 2);
    double* posArray = origin;
    if (origin == NULL) { perror("malloc"); return EXIT_FAILURE; }
    srandom(time(NULL));
    pts_gen(posArray, n);

    // Sending data

    for (int sent = 0; sent < n; sent++) {
        size_t byte_sent = send(socket_fd, posArray, sizeof(double) * 2, 0);
        if(byte_sent == -1) { perror("send"); free(origin); close(socket_fd);  return EXIT_FAILURE; }
        posArray += 2;

        int answer = -1;
        size_t answer_meta = recv(socket_fd, &answer, sizeof(int), 0);
        if (answer_meta == -1) { perror("recv"); free(origin); close(socket_fd); return EXIT_FAILURE; }
        if (answer == -1) { fprintf(stderr, "No answer received from the server"); free(origin); close(socket_fd); return EXIT_FAILURE; }
        if (answer == ANSWER_STOP) {
            printf("All points are sent.\n\n");
            break;
        }
        if (answer == ANSWER_MORE && sent == n-1) {
            printf("No more points to send.");
            double error_answer[2];
            error_answer[0] = error_answer[1] = -1.0;
            size_t error_send = send(socket_fd, &error_answer, sizeof(double) *2, 0);
            if (error_send == -1) {
                perror("send");
            }
            close(socket_fd);
            free(origin);
            return EXIT_FAILURE;
        }
    }
    free(origin);

    double gravity_point[2];
    size_t gravity_size = recv(socket_fd, gravity_point, sizeof(double) * 2, 0);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    if (gravity_size == -1) {
        perror("recv");
        return EXIT_FAILURE;
    }

    printf("Gravity Point: ");
    pt_print(gravity_point[0], gravity_point[1]);
    printf("\n");


    return EXIT_SUCCESS;
}



/**
 * Tire et renvoie un entier pseudo-aléatoire entre min et max bornes incluses.
 *
 * @param min valeur minimale de l'entier
 * @param max valeur maximale de l'entier
 */
int gen_int_rand(int min, int max) {
    return (int)((random()/(INT_MAX+1.0))*(max-min+1)+min);
}

/**
 * Génère aléatoirement les coordonnées d'un nombre de points (dans un repère orthonormé et ici entre 0 - inclus et 1 - non inclus, à la précision d'une décimale).
 *
 * @param tab_pt tableau des coordonnées des points
 * @param n nombre de points
 */
void pts_gen(double *tab_pt, int n){
    for (int i = 0; i < 2*n; i += 2){
        tab_pt[i] = gen_int_rand(MIN_C, MAX_C)/DIV;// abscisse
        tab_pt[i+1] = gen_int_rand(MIN_C, MAX_C)/DIV;// ordonnée
    }
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
    }
    printf("\n");
}