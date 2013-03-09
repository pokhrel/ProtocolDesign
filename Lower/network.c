#include"network.h"

int setsocket(const char service[]){
	struct addrinfo hints;
	struct addrinfo *result,*rp;
	int s,sfd;
          
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if( (s = getaddrinfo(NULL,service, &hints, &result)) != 0){
    	perror("Error: getaddrinfo,");
    	return -1;
   	}

    for(rp = result; rp != NULL; rp = rp->ai_next) {
    	if( (sfd = socket(rp->ai_family, rp->ai_socktype,
        			rp->ai_protocol)) == -1){
			perror("Error: socket,");      		
			continue;
		}
		
		if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;	/* Success */
		perror("Error: bind,");
        close(sfd);
	}

    if (rp == NULL) {	/* No address succeeded */
    	fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(result);	/* No longer needed */

	return sfd;
}
