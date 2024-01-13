
#define GLOBAL
#include "globals.h"
#include "forwards.h"



void writeLog(char * que)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char * str;
    str=(char*)malloc(500);
    if(str)
    {
        bzero(str,500);
        sprintf(str,"[%d-%02d-%02d %02d:%02d:%02d] %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,que);
        int son=strlen(str);
        int fueron=fprintf(myFile,str);
        if(fueron<son)
        {
            // presume the file is full so we now recycle file
            fclose(myFile);
            remove("/spiffs/log.txt");

            myFile= fopen("/spiffs/log.txt", "a");
            if (myFile == NULL) 
            {
    #ifdef DEBB            
                printf("%sFailed to open file for append erase\n",SYSTEM);
    #endif            
            }
            sprintf(str,"[%d-%02d-%02d %02d:%02d:%02d] %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,"File was recycled");
            fprintf(myFile,str);
        }
        fflush(myFile);
        free(str);
    }
}

void logFileInit()
{
    esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 2,
    .format_if_mount_failed = true
    };

     esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf( "Failed to mount or format filesystem\n");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            printf( "Failed to find SPIFFS partition\n");
        } else {
            printf( "Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
#ifdef DEBB        
        printf("%sFailed to get SPIFFS partition information (%s). Formatting...\n",SYSTEM, esp_err_to_name(ret));
#endif        
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
#ifdef DEBB        
        // printf("%sPartition size: total: %d, used: %d\n", total, used);
#endif        
    }
    
    myFile= fopen("/spiffs/log.txt", "a");
    if (myFile == NULL) {
#ifdef DEBB        
        printf("%sFailed to open file for append\n",SYSTEM);
#endif        
        return;
    }
}

