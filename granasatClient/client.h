/*
 * client.h
 * granaSAT.Client
 *
 *  Created on: Apr 11, 2014
 *      Author: mario
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include "packet.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Gain value depends of FS and GN value (view at function enableLSM303() at sensor.c)
#define A_GAIN 0.004    	//[G/LSB] FS=10
#define M_XY_GAIN 1100   	//[LSB/Gauss] GN=001
#define M_Z_GAIN 980	//[LSB/Gauss] GN=001
#define T_GAIN 8	//[LSB/ºC]
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Constants
static int SOCKFD;
//static char SERVER_IP [] = "192.168.0.200"; // Raspberry IP.
static char SERVER_IP [] = "127.0.0.1"; // Self IP.
extern const int IMAGE_SIZE;
extern struct packet DATA;
//extern void add_magnetometer_measure (int);

struct packet DATA;

// Functions.

/**
 * Sends an integer to the server.
 */
void sendData(int x);

/**
 * Reads the image from the server.
 * @param image_data Image stream.
 * @return Total bytes received.
 */
int getImage(unsigned char* image_data);

/**
 * Connects to the server.
 */
void connect_server ();

/**
 * Reads data from the server.
 */
gboolean read_server (struct packet* data);


#endif /* CLIENT_H_ */
