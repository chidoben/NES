/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "dev/light-sensor.h"
#include "node-id.h"

#include "net/netstack.h"

#include <stdio.h>

static struct collect_conn tc;
static process_event_t event_data_ready;////////////////
/*---------------------------------------------------------------------------*/
PROCESS(smart_farming_process, "Smart Farming Process");PROCESS(print_process, "Print process");//////////////
AUTOSTART_PROCESSES(&smart_farming_process, &print_process);
/*---------------------------------------------------------------------------*/
static void recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops) {
static int *node_ID_recv;
node_ID_recv = (int *)packetbuf_dataptr();
	printf("Sink got message from %d.%d, seqno %d, hops %d\nTemperature = %dC\nHumidity = %d%\nLight = %dlux\n",
			originator->u8[0], originator->u8[1], seqno, hops, *node_ID_recv, *(node_ID_recv+1), *(node_ID_recv+2));
	
	//TODO
	//Get the sensor data from packetbuf_dataptr() and do a comparison, if the value is greater than a
	//certain threshold then print a message that says that the canopy for covering the crops has been activated.
	//E.g if humidity.value > 100 => activate canopy
static int node_ID_trigger[] = {0,0,0};
if(*(node_ID_recv+2)>80)
{
node_ID_trigger[0] = originator->u8[0];
node_ID_trigger[1] = originator->u8[1];
node_ID_trigger[2] = *(node_ID_recv+2);
event_data_ready = process_alloc_event();//////////////////
process_post(&print_process, event_data_ready, node_ID_trigger);//////////////////
}

}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smart_farming_process, ev, data) {
	static struct etimer periodic;
	static struct etimer et;
	static int val1, val2, val3;
	static float s1 = 0, s2 = 0, s3 = 0;
	static int dec1, dec2, dec3;
	static float frac1, frac2, frac3;

	PROCESS_BEGIN()
	;
	SENSORS_ACTIVATE(light_sensor);
	SENSORS_ACTIVATE(sht11_sensor);

	collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

	if (linkaddr_node_addr.u8[0] == 1 && linkaddr_node_addr.u8[1] == 0) {
		printf("Node %d is the sink node", node_id);
		collect_set_sink(&tc, 1);
	}
	static int node_ID[] = {0,0,0};/////////////////////////////////*********
	/* Allow some time for the network to settle. */
	etimer_set(&et, 10 * CLOCK_SECOND);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));

	while (1) {

		/* Send a packet every 30 seconds. */
		if (etimer_expired(&periodic)) {
			etimer_set(&periodic, CLOCK_SECOND * 30);
			etimer_set(&et, (random_rand() % (CLOCK_SECOND * 30))+CLOCK_SECOND *10);
		}

		PROCESS_WAIT_EVENT()
		;

		if (etimer_expired(&et)) {
			val1 = sht11_sensor.value(SHT11_SENSOR_TEMP);
			if (val1 != -1) {
				s1 = ((0.01 * val1) - 39.60);
				dec1 = s1;
				frac1 = s1 - dec1;
			}

			val2 = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
			if (val2 != -1) {
				s2 = (((0.0405 * val2) - 4)
						+ ((-2.8 * 0.000001) * (pow(val2, 2))));
				dec2 = s2;
				frac2 = s2 - dec2;
			}

			val3 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
			if (val3 != -1) {
				s3 = (float) (val3 * 0.4071);
				dec3 = s3;
				frac3 = s3 - dec3;
			}

			packetbuf_clear();
			//packetbuf_set_datalen(sprintf(packetbuf_dataptr(), "\nTemperature=%d.%02u C (%d)\n Humidity=%d.%02u %% (%d)\n Light=%d.%02u lux (%d)\n",
			//		dec1,(unsigned int) (frac1 * 100), val1,
                    	//		dec2,(unsigned int) (frac2 * 100), val2,
			//		dec3,(unsigned int) (frac3 * 100), val3) + 1);
				    	node_ID[0] = dec1;
	   				node_ID[1] = dec2;
					node_ID[2] = dec3;
					packetbuf_copyfrom(node_ID, sizeof(node_ID));
			collect_send(&tc, sizeof(tc));
		}

	}
	SENSORS_DEACTIVATE(light_sensor);
	SENSORS_DEACTIVATE(sht11_sensor);
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of the second process */
 PROCESS_THREAD(print_process, ev, data)
 {
     PROCESS_BEGIN();

     while (1)
     {
         // wait until we get a data_ready event
        PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
	static *data_recv;
	data_recv = (int*)data;
         // display it
        printf("Send message to %d.%d\n because Light = %dlux\n", *data_recv, *(data_recv+1), *(data_recv+2));

    }
     PROCESS_END();
 }
 /*---------------------------------------------------------------------------*/
