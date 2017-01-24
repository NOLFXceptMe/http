/* Naveen Kumar Molleti	(06CS3009)
   Sweta Yamini		(06CS3008)

	Networks Lab assignment 4
*/

/* A UDP-only DNS server */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>

#include<netdb.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT	9000
#define BUFSZ	1024
#define STRSZ	80
#define IPSZ	16

extern int errno;

int main(int argc, char* argv[])
{
	int i;
	int sockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	struct hostent *he;
	struct in_addr **h_addr_list, *addr_list;

	int data_bytes;
	char buffer[BUFSZ], inbuf[BUFSZ];
	unsigned int n_addr;
	unsigned int counter = 0;

	/* Parse command line */
	char* serverName = argv[1];
	size_t numIp = argc-2;

	if((sockfd = socket(PF_INET, SOCK_DGRAM, 0))<0){
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if((bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
		printf("Failed to bind local address\n");
		exit(0);
	}

	while(1){
		clilen = sizeof(cli_addr);
		
		if((data_bytes = recvfrom(sockfd, buffer, BUFSZ-1, 0,  (struct sockaddr *)&cli_addr, &clilen))<0){
			printf("Data receive failed\n");
			exit(0);
		}

		buffer[data_bytes] = '\0';
		
		/* Begin processing */
		printf("Retrieving DNS info for %s\n", buffer);
		memset(inbuf, '\0', BUFSZ);
		strcpy(inbuf, buffer);
		memset(buffer, '\0', BUFSZ);

		if(strcmp(inbuf, serverName)){
			if((he = gethostbyname(inbuf)) == NULL){
				printf("Failed to retrieve information\n");
				sprintf(buffer, "%d", 0);
				if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr))<0){
					printf("Data send failed\n");
					exit(0);
				}  
				continue;
			}

			printf("Sending DNS info for %s\n", he->h_name);

			/* Look at the size of the returned list, create space, copy data, and send */
			//printf("Size returned is %d\n", sizeof(he->h_addr_list));
			n_addr =  sizeof(he->h_addr_list)/4;
			h_addr_list = (struct in_addr**)he->h_addr_list;
			addr_list = (struct in_addr*)malloc(n_addr * sizeof(struct in_addr));
			for(i=0; h_addr_list[i]!=0; ++i){
				addr_list[i] = *h_addr_list[i];
			}

			/* Processing done */
		}else{
			n_addr = numIp;
			counter++;
			addr_list = (struct in_addr*)malloc(n_addr * sizeof(struct in_addr));
			for(i=0; i<n_addr; ++i){
				addr_list[i].s_addr = inet_addr(argv[(i+counter)%n_addr+2]);
			}
		}

		/* Sending no. of IP addresses */
		sprintf(buffer, "%u", n_addr);
		if(sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr))<0){
			printf("Data send failed\n");
			exit(0);
		}  

		/* Sending list of IP addresses */
		if(sendto(sockfd, (char *)addr_list, sizeof(addr_list), 0,  (struct sockaddr *)&cli_addr, sizeof(cli_addr))<0){
			printf("Data send failed\n");
			exit(0);
		}

		printf("DNS info sent\n");
	}

	close(sockfd);

	return 0;
}
