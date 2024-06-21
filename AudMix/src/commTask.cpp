#include "commTask.hpp"
#include "bt_interface.h"
#include "device.hpp"
#include "main.h"
#include <iostream>

#define CMD_SET_ICON    "SET_ICON"
#define CMD_VOL_UPDATE  "VOL"
#define CMD_USB_PING    "ISWORK"
#define CMD_AUDREACT    "AUDREACT"

void communicationTask( void *args ){
    std::string str = "";

    while (1){
        int res = 0;
        std::getline(std::cin, str);

        if(str.compare(0, strlen(CMD_SET_ICON), CMD_SET_ICON) == 0){
            uint8_t icon_array[((ICON_WIDTH+7)/8)*ICON_HEIGHT];
			size_t prefixLen = strlen(CMD_SET_ICON);
			if (str.size() <= prefixLen + 1) {
                res = -2;
				goto exit;
            }
            std::string afterPrefix = str.substr(prefixLen+1);
            int display_num = std::atoi(afterPrefix.c_str());
            if((display_num < 0) || (display_num >= SLIDERS_COUNT)){
                res = 1;
                goto exit;
            }
            std::cout << "Send " << sizeof(icon_array) << " bytes" << std::endl;
            
            std::cin.read((char*)icon_array, sizeof(icon_array));
            audMix.sliders[display_num].setIcon(icon_array, ICON_WIDTH, ICON_HEIGHT);
            audMix.sliders[display_num].displayIcon(true);
        }else if(str.compare(0, strlen(CMD_VOL_UPDATE), CMD_VOL_UPDATE) == 0){
            float volumeValues[5];
            const char *tokens = str.c_str() + 4; // Skip "VOL:"
            char *token;
            int index = 0;
            token = strtok((char *)tokens, "|");
            while (token != NULL && index < 5) {
                volumeValues[index++] = atof(token);
                token = strtok(NULL, "|");
            }
            
            if(audMix.isAudioReactive()){
                for(uint32_t i = 0; i < 5; i++){
                    audMix.sliders[i].strip.volumeSet(volumeValues[i]);
                }
            }
        }else if(str.compare(0, strlen(CMD_USB_PING), CMD_USB_PING) == 0){
            sleepPing();
        }else if(str.compare(0, strlen(CMD_AUDREACT), CMD_AUDREACT) == 0){
            const char *tokens = str.c_str() + 8; // Skip "AUDREACT"
            if(*tokens == ':'){
                tokens++;
                if(*tokens == 'T'){
                    audMix.isAudioReactive(true);
                }else{
                    audMix.isAudioReactive(false);
                }
            }else if(*tokens == '?'){
                if(audMix.isAudioReactive()){
                    std::cout << "AUDREACT:T" << std::endl;
                }else{
                    std::cout << "AUDREACT:F" << std::endl;
                }
            }
            
            
        }else{
            res = -1;
        }
exit:
        if(res == 0){
            std::cout << "OK" << std::endl;
        }else{
            std::cerr << "ERROR: " << res << std::endl;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
