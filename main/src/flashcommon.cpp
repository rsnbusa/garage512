
#define GLOBAL
#include "globals.h"
#include "forwards.h"


void read_flash()
{
	esp_err_t q ;
	size_t largo;

    q = nvs_open("config", NVS_READONLY, &nvshandle);
    if(q!=ESP_OK)
    {
        printf("%sError opening NVS Read File %x\n",SYSTEM,q);
        erase_config();
        return;
    }

    largo=sizeof(theConf);
    q=nvs_get_blob(nvshandle,"sysconf",(void*)&theConf,&largo);     //read the whole configuration into ram theConf

    if (q !=ESP_OK)
    {
        printf("%sError read %x largo %d aqui %d\n",SYSTEM,q,largo,sizeof(theConf));
    }
    nvs_close(nvshandle);

}

void write_to_flash() //save our configuration
{
		esp_err_t q ;
		q = nvs_open("config", NVS_READWRITE, &nvshandle);
		if(q!=ESP_OK)
		{
			printf("%sError opening NVS File RW %x\n",SYSTEM,q);
			return;
		}
		size_t req=sizeof(theConf);
		q=nvs_set_blob(nvshandle,"sysconf",&theConf,req);   //write the whole configuration(theConf) in RAM to Flash
		if (q ==ESP_OK)
		{
			q = nvs_commit(nvshandle);
			if(q!=ESP_OK)
				printf("%sFlash commit write failed %d\n",SYSTEM,q);
		}
		else
			printf("%sFail to write flash %x\n",SYSTEM,q);
		nvs_close(nvshandle);

}

void erase_config() 		//set to factory defaults
{
	printf("%sErase config\n",SYSTEM);
    wifi_prov_mgr_reset_provisioning();         // reset provisioning
    esp_wifi_restore();                         // reset wifi 
    nvs_flash_erase();                          // erase our Configuration
    nvs_flash_init();                           // init our Flash area

	memset(&theConf,0,sizeof(theConf));         
    //set default values

    theConf.centinel=                   CENTINEL;
    theConf.rel=                        1;                  //seconds, fixed 
    theConf.guard=                      1;                  //guard is armed by defaultle
    theConf.onoff=                      1;                  //SYSTEM on by default

    strcpy(theConf.name,(char*)NODENAME);

    theConf.waithal=                    KWAITHAL;           // millis to wait before check confirm_hal positions
    theConf.ciclo=                      KCYCLE;             // pwm duty cycle

// PWM defaults
    theConf.timers[TPWMON]=             KPWMTIMER;          // microseconds for ON 38K frequency burst
    theConf.timers[TPWMOFF]=            KPWMTIMER;          // microseconds for OFF 38K frequency burst
    theConf.timers[TCOOL]=              KPWMCOOL;           // miliseconds for Cooling IF led
    theConf.timers[TBREATH]=            KPWMBREATH;         // miliseconds for start of cooling period of IF led

// TIMERS DEFAULT
    theConf.timers[TOPEN]=              KOPENINGT;          // opening timer
    theConf.timers[TRELAY]=             KESPERA;            // relay timer
    theConf.timers[TREPEAT]=            KREPEATT;           // repeat timer for open/close repeat
    theConf.timers[TREPEATW]=           KREPEATW;           // seconds after firing first Relay pusle to avoiding foever pulsing the relay
    theConf.timers[TCLOSE]=             KCLOSET;            //closet timer
    theConf.timers[TDISPLAY]=           KDISPLAYTIMER;      // display timer
    theConf.waithal=                    KWAITHAL;           // debounce HAL/Switch sensor value
    theConf.timers[TBREAK]=             KBREAKTIMER;        // debounce Break value
    theConf.timers[TIFR]=               KIFRBREAK;          // ifr swithc value on/off
    theConf.timers[TFREQ]=              KFREQUENCY;         // pulse frequency

    theConf.sampler=                    8;                  //min samples for break analysis
    write_to_flash();
}
