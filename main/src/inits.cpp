#define GLOBAL
#include "globals.h"
#include "forwards.h"


void init_conf()
{
    // all Task handle set to NULL for security
    powerHandle=blinker=closedHandle=openingHandle=openedHandle=closingHandle=unknownHandle=fadeHandle=farrowHandle=obstructHandle=NULL;
    laserState=OFF;
    blockrequest=false;
    activo=false;

    CONTACT=0;
    NOCONTACT=1;
    gOnOff=POWER_ON;

#ifdef DISPLAY
	u8g2_esp32_hal_t   u8g2_esp32_hal;
    u8g2_esp32_hal.mosi=(gpio_num_t)-1;
    u8g2_esp32_hal.clk=(gpio_num_t)-1;
    u8g2_esp32_hal.cs=(gpio_num_t)-1;
    u8g2_esp32_hal.reset=(gpio_num_t)-1;
    u8g2_esp32_hal.dc=(gpio_num_t)-1;
	u8g2_esp32_hal.sda   =(gpio_num_t)SDA;
	u8g2_esp32_hal.scl  = (gpio_num_t)SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(
		&u8g2,
		U8G2_R0,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
	u8x8_SetI2CAddress(&u8g2.u8x8,0x78);    //FIXED 
    u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
    u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
    stx=10;
    sty=38;
#endif

   	// some GPIO
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_down_en =GPIO_PULLDOWN_ENABLE;
   	io_conf.pull_up_en =GPIO_PULLUP_DISABLE;
	io_conf.pin_bit_mask = (1ULL<<RELAY |1ULL<<SYSLED|1ULL<<DLED|1ULL<<IFRTEST|1ULL<<IFRTESTSAMPLE); //output pins
	gpio_config(&io_conf);

   	// SSR GPIO, pull up needed
	io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en =GPIO_PULLUP_ENABLE;
	io_conf.pin_bit_mask = (1ULL<<SSR|1ULL<<PWRPIN); //output pins
	gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)SSR,gOnOff);      // turn it on as default

	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en =GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
	io_conf.pin_bit_mask = (1ULL<<HALOP |1ULL<<HALCL|1ULL<<RESETP);     //input pins
	// io_conf.pin_bit_mask = (1ULL<<HALOP |1ULL<<HALCL|1ULL<<RESETP||1ULL<<IFR);     //input pins
	gpio_config(&io_conf);

    gpio_set_intr_type((gpio_num_t)HALOP, GPIO_INTR_ANYEDGE);               //interrupt pins
    gpio_set_intr_type((gpio_num_t)HALCL, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type((gpio_num_t)EXTP, GPIO_INTR_ANYEDGE);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)HALOP, halISR, (void*)(gpio_num_t)HALOP);
    gpio_isr_handler_add((gpio_num_t)HALCL, halISR, (void*)(gpio_num_t)HALCL);
    gpio_isr_handler_add((gpio_num_t)EXTP, halISR, (void*)(gpio_num_t)EXTP);

    lqueue = xQueueCreate(20, sizeof(lqueuecmd));
    blockqueue = xQueueCreate(5, sizeof(uint32_t));

// Timers names
    strcpy(timer_names[0],"Opening");
    strcpy(timer_names[1],"Close");
    strcpy(timer_names[2],"Repeat");
    strcpy(timer_names[3],"Relay");
    strcpy(timer_names[4],"RepeatW");
    strcpy(timer_names[5],"Display");
    strcpy(timer_names[6],"Break");
    strcpy(timer_names[7],"Pwmcool");
    strcpy(timer_names[8],"Pwmon");
    strcpy(timer_names[9],"Pwmoff");
    strcpy(timer_names[10],"Pwmbreath");
    strcpy(timer_names[11],"ifrbreak");
    strcpy(timer_names[12],"frequency");

// Door State names
    strcpy(doorState[0],"Closed");
    strcpy(doorState[1],"Openning");
    strcpy(doorState[2],"Opened");
    strcpy(doorState[3],"Closing");
    strcpy(doorState[4],"Off");
    strcpy(doorState[5],"Unknown");
    strcpy(doorState[6],"Prov");
    strcpy(doorState[7],"Relay");
    strcpy(doorState[8],"Guard");
    strcpy(doorState[9],"Align");

//set timers
    for (int a=0;a<TPWMON;a++)
    {
        int duration=theConf.timers[a]*(a<TCOOL?WORKTIME:1);  //all timers in seconds except last one
        if (duration<1)
        {
            duration=1000;
            printf("%sTimer[%d][%s] was 0 now set to 1\n",SYSTEM,a,timer_names[a]);
        }
#ifdef DEBB
        if (theConf.traceBoot)   
            printf("%sCreate timer[%d] = %s time %dms\n",SYSTEM,a,timer_names[a],duration);
#endif            
        timers[a] = xTimerCreate(
        timer_names[a], 
        pdMS_TO_TICKS(duration),
        pdFALSE, 
        (void*)a, 
        a<TCOOL?timermgr:coolMgr);  //timermgr for all timers except coolmgr for cooling process
    }

    doorBits = xEventGroupCreate();

// Rainmaker App Commands with Name and Routine to execute
int x=0;

	strcpy((char*)&cmds[ x].comando,CMD0);			cmds[x++].code=remote_cmd;
	strcpy((char*)&cmds[ x].comando,CMD1);			cmds[x++].code=wait_cmd;
	strcpy((char*)&cmds[ x].comando,CMD2);			cmds[x++].code=reset_cmd;
	strcpy((char*)&cmds[ x].comando,CMD4);			cmds[x++].code=closebreak_cmd;
    strcpy((char*)&cmds[ x].comando,CMD3);			cmds[x++].code=guard_cmd;
	strcpy((char*)&cmds[ x].comando,CMD5);			cmds[x++].code=keepopen_cmd;
	strcpy((char*)&cmds[ x].comando,CMD6);			cmds[x++].code=dutycycle_cmd;
	strcpy((char*)&cmds[ x].comando,CMD8);			cmds[x++].code=name_cmd;
	strcpy((char*)&cmds[ x].comando,CMD10);			cmds[x++].code=onoff_cmd;
	strcpy((char*)&cmds[ x].comando,CMD9);			cmds[x++].code=align_cmd;
	strcpy((char*)&cmds[ x].comando,CMD11);			cmds[x++].code=hstart_cmd;
	strcpy((char*)&cmds[ x].comando,CMD12);			cmds[x++].code=htime_cmd;
	strcpy((char*)&cmds[ x].comando,CMD13);			cmds[x++].code=sampler_cmd;
	// strcpy((char*)&cmds[ x].comando,CMD14);			cmds[x++].code=breath_cmd;


	dispSem= xSemaphoreCreateBinary();
	xSemaphoreGive(dispSem);

    KMINPULSE=theConf.sampler;
    if(KMINPULSE==0)
        KMINPULSE=10;

}

void init_rmaker_device()
{
    esp_rmaker_config_t *rainmaker_cfg =(esp_rmaker_config_t*)malloc(sizeof(esp_rmaker_config_t));
    if(rainmaker_cfg)
    {
        bzero(rainmaker_cfg,sizeof(esp_rmaker_config_t));
        rainmaker_cfg->enable_time_sync=false;

        esp_rmaker_node_t *node = esp_rmaker_node_init(rainmaker_cfg, "Door Device", theConf.name);
        if (!node) {
            printf("%sCould not initialise node. Aborting!!!\n",SYSTEM);
            vTaskDelay(5000/portTICK_PERIOD_MS);
            abort();
        }

        esp_rmaker_device_t *switch_device = esp_rmaker_device_create(theConf.name, "esp.device.lock", NULL);
        esp_rmaker_device_add_cb(switch_device, write_cb, NULL);        // routine to manage callbacks when receiving from the Internet
        esp_rmaker_device_add_param(switch_device, esp_rmaker_param_create("name", NULL, esp_rmaker_str(theConf.name),
                PROP_FLAG_READ | PROP_FLAG_WRITE )); //name can not be changed. Why?

        status_param = esp_rmaker_param_create(CMD7, NULL, esp_rmaker_str(CMD7), PROP_FLAG_READ );     //Door state
        esp_rmaker_device_add_param(switch_device, status_param);
        
        esp_rmaker_param_t *dummy_param = esp_rmaker_param_create(CMD0, NULL, esp_rmaker_bool(true), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param,ESP_RMAKER_UI_TRIGGER );
        esp_rmaker_device_add_param(switch_device, dummy_param);
        esp_rmaker_device_assign_primary_param(switch_device, dummy_param);     //remote Button

    
        // esp_rmaker_param_t *dummy_param4 = esp_rmaker_param_create(CMD2, NULL, esp_rmaker_bool(true), PROP_FLAG_READ | PROP_FLAG_WRITE);
        // esp_rmaker_param_add_ui_type(dummy_param4, ESP_RMAKER_UI_TRIGGER);
        // esp_rmaker_device_add_param(switch_device, dummy_param4);

        esp_rmaker_param_t *dummy_param5 = esp_rmaker_param_create(CMD4, NULL, esp_rmaker_bool(true), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param5, ESP_RMAKER_UI_TRIGGER);
        esp_rmaker_device_add_param(switch_device, dummy_param5);                   //closew break button

        esp_rmaker_param_t *dummy_param6 = esp_rmaker_param_create(CMD5, NULL, esp_rmaker_bool(true), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param6, ESP_RMAKER_UI_TRIGGER);
        esp_rmaker_device_add_param(switch_device, dummy_param6);                   //keep open button


        wait_param = esp_rmaker_param_create(CMD1, NULL, esp_rmaker_int(5), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_bounds(wait_param, esp_rmaker_int(1), esp_rmaker_int(90), esp_rmaker_int(1));
        esp_rmaker_param_add_ui_type(wait_param, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_device_add_param(switch_device, wait_param);                 // Door time to wait 

        esp_rmaker_param_t *dummy_param7 = esp_rmaker_param_create(CMD6, NULL, esp_rmaker_int(theConf.ciclo), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param7, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(dummy_param7, esp_rmaker_int(10), esp_rmaker_int(90), esp_rmaker_int(25));
        esp_rmaker_device_add_param(switch_device, dummy_param7);               // PWM duty cycle. Should not be used but....

        float algo=1.0/theConf.timers[TFREQ]*1000000;
        int maxsamp=theConf.timers[TPWMON]/(int)algo;                           // sampling rate, should be internal but ...

        guard_param = esp_rmaker_param_create(CMD3, NULL, esp_rmaker_bool(theConf.guard), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(guard_param, ESP_RMAKER_UI_TOGGLE);
        esp_rmaker_device_add_param(switch_device, guard_param);               // Guard on-off check button



        esp_rmaker_param_t *dummy_param3 = esp_rmaker_param_create(CMD9, NULL, esp_rmaker_bool(false), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param3, ESP_RMAKER_UI_TOGGLE);
        esp_rmaker_device_add_param(switch_device, dummy_param3);               //Align on-off chedck button. Should be internal but ...

        onoff_param = esp_rmaker_param_create(CMD10, NULL, esp_rmaker_bool(theConf.onoff), PROP_FLAG_READ| PROP_FLAG_WRITE );
        esp_rmaker_param_add_ui_type(onoff_param,  ESP_RMAKER_UI_TOGGLE);
        esp_rmaker_device_add_param(switch_device, onoff_param);                // System On-Off button

        hstart = esp_rmaker_param_create(CMD11, NULL, esp_rmaker_int(0), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(hstart, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(hstart, esp_rmaker_int(0), esp_rmaker_int(23), esp_rmaker_int(23));
        esp_rmaker_device_add_param(switch_device, hstart);                         // power off start hour slider

        htime = esp_rmaker_param_create(CMD12, NULL, esp_rmaker_int(0), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(htime, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(htime, esp_rmaker_int(0), esp_rmaker_int(23), esp_rmaker_int(5));
        esp_rmaker_device_add_param(switch_device, htime);                      // power on hour slider

        esp_rmaker_param_t *dummy_param8 = esp_rmaker_param_create(CMD13, NULL, esp_rmaker_int(theConf.sampler+1), PROP_FLAG_READ | PROP_FLAG_WRITE);
        esp_rmaker_param_add_ui_type(dummy_param8, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(dummy_param8, esp_rmaker_int(12), esp_rmaker_int(maxsamp), esp_rmaker_int(12));
        esp_rmaker_device_add_param(switch_device, dummy_param8);               // samp slider

        // esp_rmaker_param_t *dummy_param9 = esp_rmaker_param_create("Breath", NULL, esp_rmaker_int(theConf.timers[TBREATH]), PROP_FLAG_READ | PROP_FLAG_WRITE);
        // esp_rmaker_param_add_ui_type(dummy_param9, ESP_RMAKER_UI_SLIDER);
        // esp_rmaker_param_add_bounds(dummy_param9, esp_rmaker_int(40), esp_rmaker_int(200), esp_rmaker_int(60));
        // esp_rmaker_device_add_param(switch_device, dummy_param9);               // breath slider

        esp_rmaker_node_add_device(node, switch_device);

        free(rainmaker_cfg);
    }
}
