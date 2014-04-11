/*
 * main.c
 * granaSAT.Client 
 *
 *  Created on: Feb 22, 2014
 *      Author: Mario Román
 */

// Client libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "packet.h"
#include "client.h"

// GTK GUI library
#include "gtkgui.h"

// Constants
const int ETHERNET_MAX = 1000;
const int IMAGE_SIZE = 1280*960;


void error(char *msg) {
	perror(msg);
	exit(1);
}


// Client/Server on Raspberry
struct packet DATA;

int getImage(unsigned char* image_data) {
	const int total_bytes = IMAGE_SIZE;
	int bytes_received = 0;
	int n;

	while(bytes_received < total_bytes){
		if ( (n = read(SOCKFD,image_data+bytes_received,total_bytes-bytes_received) ) < 0 ){
			error( ( "ERROR reading from socket") );
			break;
		}
		else
			bytes_received += n;
	}

	return bytes_received;
}

int connect_server () {
	int sockfd, portno = 51717;
	//char serverIp[] = "192.168.0.100"; // Raspberry IP
	char serverIp[] = "127.0.0.1";
	struct sockaddr_in serv_addr;
	struct hostent *server;

	printf("contacting %s on port %d\n", serverIp, portno);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ERROR opening socket");

	if (( server = gethostbyname( serverIp ) ) == NULL)
		error("ERROR, no such host\n");

	bzero( (char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if ( connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	printf("Connected\n");

	return sockfd;
}

void print_data (struct packet* data) {
	printf("Temperature\tHighByte: %d\n\t\tLowByte: %d\n", data->temp.highByte, data->temp.lowByte);
}

void read_server (struct packet* data) {
	static int n = 0;

    unsigned char image_stream[IMAGE_SIZE];
    FILE* raw_image;
    char string[80];

	// Temperature
    sendData(REQ_TEMP);
    if ((read(SOCKFD,(char*) DATA.temp.highByte,sizeof(unsigned char)) ) < 0)
        	error("ERROR reading Temp.HighByte from socket");
    if ((read(SOCKFD,(char*) DATA.temp.lowByte,sizeof(unsigned char)) ) < 0)
            error("ERROR reading Temp.LowByte from socket");

    // Magnetometer
    sendData(REQ_MAGN);
    if ((read(SOCKFD,(char*) DATA.magnetometer,sizeof(unsigned char[6])) ) < 0)
            	error("ERROR reading Magnetometer from socket");

    // Accelerometer
    sendData(REQ_ACCE);
    if ((read(SOCKFD,(char*) DATA.accelerometer,sizeof(unsigned char[6])) ) < 0)
                	error("ERROR reading Accelerometer from socket");

	// Image
	// Receive the image
    sendData(REQ_IMAG);
	int num_bytes_received = getImage(image_stream);

	// Store the image in a file called image_received_n.data
	sprintf(string, "images/image_received_%d.data", n);
	raw_image = fopen(string, "w");
	fwrite(image_stream, 1, IMAGE_SIZE, raw_image);

	// Some debugging information
	printf("Iteration %d, number of bytes received:\t%d\n", n, num_bytes_received );
	n++;
}


int main (int argc, char* argv[])
{
	// Measures.
	int magnetometer_measures []  = {10,20,40,35,13,50,20,16,35,90,17,20,12,35,98,43,20,10,35,78,10,5,10,35,43,34,35,34,23,34};
	int accelerometer_measures [] = {10,20,10,35,98,10,20,40,35,13,50,20,16,35,90,17,20,12,35,98,43,20,10,35,78,10,5,10,35,43};


    // Initialize GTK.
    GtkBuilder* builder;
	GtkWidget* main_window;
	GtkWidget* main_container;

	gtk_init (&argc, &argv);
	srand(time(NULL));

	// Initialize Builder.
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, FILEGLADE, NULL);

	// Main Window and main container.
	main_window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	main_container = GTK_WIDGET (gtk_builder_get_object (builder, "main_fixed"));
	gtk_widget_show (main_window);
	gtk_widget_show (main_container);

	// Add widgets.
	add_temperature_labels (builder);
	add_ethernet_slider (builder, main_container);
	add_plots (builder, magnetometer_measures, accelerometer_measures);
	add_image_window();

	// Building.
	gtk_builder_connect_signals (builder, NULL);
	g_object_unref (G_OBJECT (builder));
	gtk_window_set_keep_above ( (GtkWindow *) main_window, TRUE);


	// Terminate application when window is destroyed.
	g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);


	// Client
    SOCKFD = connect_server();
    g_timeout_add (REFRESH_INTERVAL, (GSourceFunc) read_server, (gpointer) &DATA);


	// GTK event loop.
	gtk_main();


	// Close server
	sendData(-2);
	close(SOCKFD);

	return 0;
}




