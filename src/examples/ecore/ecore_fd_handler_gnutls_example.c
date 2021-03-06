//Compile with:
// gcc -o ecore_fd_handler_gnutls_example ecore_fd_handler_gnutls_example.c `pkg-config --cflags --libs ecore gnutls`

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <gnutls/gnutls.h>
#include <Ecore.h>

/* Ecore_Fd_Handler example
 * 2010 Mike Blumenkrantz
 * compile with gcc $(pkgconfig --cflags --libs gnutls ecore)
 */

#define print(...) \
do { \
  fprintf(stderr, "line %i: ", __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n");\
} while(0)

static int done = 0;

static void
tls_log_func(int level, const char *str)
{
   fprintf(stderr, "|<%d>| %s", level, str);
}

static const char *
SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_description_t status)
{
   switch (status)
     {
      case GNUTLS_HANDSHAKE_HELLO_REQUEST:
        return "Hello request";

      case GNUTLS_HANDSHAKE_CLIENT_HELLO:
        return "Client hello";

      case GNUTLS_HANDSHAKE_SERVER_HELLO:
        return "Server hello";

      case GNUTLS_HANDSHAKE_CERTIFICATE_PKT:
        return "Certificate packet";

      case GNUTLS_HANDSHAKE_SERVER_KEY_EXCHANGE:
        return "Server key exchange";

      case GNUTLS_HANDSHAKE_CERTIFICATE_REQUEST:
        return "Certificate request";

      case GNUTLS_HANDSHAKE_SERVER_HELLO_DONE:
        return "Server hello done";

      case GNUTLS_HANDSHAKE_CERTIFICATE_VERIFY:
        return "Certificate verify";

      case GNUTLS_HANDSHAKE_CLIENT_KEY_EXCHANGE:
        return "Client key exchange";

      case GNUTLS_HANDSHAKE_FINISHED:
        return "Finished";

      case GNUTLS_HANDSHAKE_SUPPLEMENTAL:
        return "Supplemental";
      default:
        return "Uncaught state";
     }
   return NULL;
}

/* Connects to the peer and returns a socket
 * descriptor.
 */
static int
tcp_connect(void)
{
   const char *PORT = "443";
   const char *SERVER = "69.58.181.89"; //verisign.com
   int err, sd;
   int flag = 1, curstate = 0;
   struct sockaddr_in sa;

   /* sets some fd options such as nonblock */
   sd = socket(AF_INET, SOCK_STREAM, 0);
   if (sd < 0) abort();
   if (fcntl(sd, F_SETFL, O_NONBLOCK) < 0) perror("fcntl");
   eina_file_close_on_exec(sd, EINA_TRUE);
   if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate, sizeof(curstate)) < 0) perror("setsockopt");

   if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) perror("setsockopt");

   memset(&sa, '\0', sizeof (sa));
   sa.sin_family = AF_INET;
   sa.sin_port = eina_htons(atoi(PORT));
   if (inet_pton(AF_INET, SERVER, &sa.sin_addr)) perror("inet_pton");

   /* connects to server
    */
   err = connect(sd, (struct sockaddr *)&sa, sizeof (sa));
   if ((err < 0) && (errno != EINPROGRESS))
     {
        print("Connect error\n");
        exit(1);
     }

   return sd;
}

/* closes the given socket descriptor.
 */
static void
tcp_close(int sd)
{
#ifdef _WIN32
   shutdown(sd, SD_BOTH);     /* no more receptions */
   closesocket(sd);
#else
   shutdown(sd, SHUT_RDWR);     /* no more receptions */
   close(sd);
#endif
}

static Eina_Bool
_process_data(gnutls_session_t client, Ecore_Fd_Handler *fd_handler)
{
   static int ret, lastret;
   static unsigned int count = 0;

   if (!done)
     {
        lastret = ret;
        ret = gnutls_handshake(client);
        count++;
        if (gnutls_record_get_direction(client))
          ecore_main_fd_handler_active_set(fd_handler, ECORE_FD_WRITE);
        else
          ecore_main_fd_handler_active_set(fd_handler, ECORE_FD_READ);
        /* avoid printing messages infinity times */
        if (lastret != ret)
          {
             print("gnutls returned with: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
             if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
               print("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(client)));
             print("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(client)));
             print("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(client)));
          }

        if (gnutls_error_is_fatal(ret))
          {
             print("yarrr this be an error!");
             exit(1);
          }
     }
   if (ret == GNUTLS_E_SUCCESS)
     {
        done = 1;
        print("Handshake successful in %u handshake calls!", count);
        ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_RENEW;
}

int
main(void)
{
   /* credentials */
   gnutls_anon_client_credentials_t c_anoncred;
   gnutls_certificate_credentials_t c_certcred;

   gnutls_session_t client;
   int sd;

   /* General init. */
   gnutls_global_init();
   ecore_init();
   gnutls_global_set_log_function(tls_log_func);
   gnutls_global_set_log_level(6);

   /* Init client */
   gnutls_anon_allocate_client_credentials(&c_anoncred);
   gnutls_certificate_allocate_credentials(&c_certcred);
   gnutls_init(&client, GNUTLS_CLIENT);
   /* set very specific priorities */
   gnutls_priority_set_direct(client, "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:+VERS-TLS1.2:+VERS-TLS1.1:+VERS-TLS1.0:+VERS-SSL3.0", NULL);
   gnutls_credentials_set(client, GNUTLS_CRD_ANON, c_anoncred);
   gnutls_credentials_set(client, GNUTLS_CRD_CERTIFICATE, c_certcred);
   gnutls_server_name_set(client, GNUTLS_NAME_DNS, "www.verisign.com", strlen("www.verisign.com"));

   /* connect to the peer
    */
   sd = tcp_connect();

   /* associate gnutls with socket */
   gnutls_transport_set_ptr(client, (gnutls_transport_ptr_t)(uintptr_t)sd);
   /* add a callback for data being available for send/receive on socket */
   if (!ecore_main_fd_handler_add(sd, ECORE_FD_READ | ECORE_FD_WRITE, (Ecore_Fd_Cb)_process_data, client, NULL, NULL))
     {
        print("could not create fd handler!");
        exit(1);
     }
   /* begin main loop */
   ecore_main_loop_begin();

   gnutls_bye(client, GNUTLS_SHUT_RDWR);

   gnutls_deinit(client);

   tcp_close(sd);

   return 0;
}

