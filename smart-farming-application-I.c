#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
//#include "dev/sht11/sht11-sensor.h"
#include "dev/light-sensor.h"
#include "node-id.h"

#include "net/netstack.h"

#include <stdio.h>

static struct collect_conn tc;
/*---------------------------------------------------------------------------*/
PROCESS(smart_farming_process, "Smart Farming Process");
AUTOSTART_PROCESSES(&smart_farming_process);
/*---------------------------------------------------------------------------*/
static int shutter_status;static int info[15]={0};
static void recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops) {
static int *node_ID_recv;int i=0;int j=0;
node_ID_recv = (int *)packetbuf_dataptr();
if (*(int *)packetbuf_dataptr())
info[(originator->u8[0])-1] = 1;
if (!(*(int *)packetbuf_dataptr()))
info[(originator->u8[0])-1] = 0;
for(;i<15;i++)
j=j+info[i];


if(*node_ID_recv == 1  && shutter_status == 0 && j>0){
printf("Close shutter ... sensing adverse weather conditions at node: %d.%d\n", originator->u8[0], originator->u8[1]);++shutter_status;leds_toggle(LEDS_ALL);
}
else if(*node_ID_recv == 0 && shutter_status == 1 && j == 0){
printf("Open shutter ... sensing optimum weather conditions \n");--shutter_status;leds_toggle(LEDS_ALL);
}
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smart_farming_process, ev, data) {
	static struct etimer periodic;
	static struct etimer et;
	static int val3;
	static float s3 = 0;
	static int dec1, dec2, dec3;
	//static float frac1, frac2, frac3;

	PROCESS_BEGIN()
	;
	SENSORS_ACTIVATE(light_sensor);
	//SENSORS_ACTIVATE(sht11_sensor);

	collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

	if (linkaddr_node_addr.u8[0] == 1 && linkaddr_node_addr.u8[1] == 0) {
		printf("Node %d is the sink node\n", node_id);
		collect_set_sink(&tc, 1);
	}
	static int node_ID[] = {0,0,0};/////////////////////////////////*********
	/* Allow some time for the network to settle. */
	etimer_set(&et, 30 * CLOCK_SECOND);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
static int shutter_status=0;
	while (1) {

		/* Send a packet every 30 seconds. */
		if (etimer_expired(&periodic)) {
			etimer_set(&periodic, CLOCK_SECOND * 30);
			etimer_set(&et, (random_rand() % (CLOCK_SECOND * 30))+CLOCK_SECOND *10);
		}

		PROCESS_WAIT_EVENT()
		;

		if (etimer_expired(&et)) {
			//val1 = sht11_sensor.value(SHT11_SENSOR_TEMP);
			
				//s1 = ((0.01 * val1) - 39.60);
				dec1 = 24 +  (random_rand() % 7);
				//frac1 = s1 - dec1;
			//val2 = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
			
				//s2 = (((0.0405 * val2) - 4)
				//		+ ((-2.8 * 0.000001) * (pow(val2, 2))));
				dec2 = 100 - 66 + (random_rand() % 67);//randomize humidity
				//frac2 = s2 - dec2;
			val3 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
			if (val3 != -1) {
				s3 = (float) (val3 * 0.4071);
				dec3 = s3;
				//frac3 = s3 - dec3;
			}
			printf("\nTemperature = %dC\nHumidity = %d%\nLight = %dlux\n",
			dec1, dec2, dec3);


		if((dec3 >= 90 ||  dec1 >= 30 || dec2 >= 95) && shutter_status==0)	//testing with light threshold = 80lux******change this value in "shutter_process" also
		{	
			packetbuf_clear();
				    	node_ID[0] = 1;
					packetbuf_copyfrom(node_ID, sizeof(node_ID));
			collect_send(&tc, sizeof(tc));
			shutter_status++;
		
		}
		else if(dec3 < 90 &&  dec1 < 30 && dec2 < 95 && shutter_status ==1)
		{	
			packetbuf_clear();
				    	node_ID[0] = 0;
					packetbuf_copyfrom(node_ID, sizeof(node_ID));
			collect_send(&tc, sizeof(tc));
			shutter_status--;

		}	
		}

	}
	SENSORS_DEACTIVATE(light_sensor);
	//SENSORS_DEACTIVATE(sht11_sensor);
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
