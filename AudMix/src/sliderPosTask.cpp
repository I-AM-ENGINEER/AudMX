#include "sliderPosTask.hpp"
#include <iostream>
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "freertos/event_groups.h"
#include "device.hpp"
#include "bt_spp.h"

extern EventGroupHandle_t buttonsPressedEventGroup;
extern EventGroupHandle_t buttonsReleasedEventGroup;

void readPotentiometersButtonsTask( void *args ){
    uint16_t positions_old[SLIDERS_COUNT] = {0};
	bool buttons_old[SLIDERS_COUNT] = {false};
    int64_t timestamp = 3'000'000;

    while (1){
        static bool need_positions_send = false;
        // Check updated value
        for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            audMix.sliders[i].updatePosition();
            float position_current_f = audMix.sliders[i].readPosition();
            uint16_t position_current = (uint16_t)std::round(position_current_f * 1023.0f);
            if(!audMix.isAudioReactive()){
                audMix.sliders[i].strip.volumeSet(position_current_f);
            }
            if(positions_old[i] != position_current){
                need_positions_send = true;
                positions_old[i] = position_current;
            }

			bool button = audMix.sliders[i].readButton();
			int button_num = (int)audMix.sliders[i].readButtonNum();
			if(buttons_old[i] != button){
				if(button){
					xEventGroupSetBits(buttonsPressedEventGroup, (1 << i));
				}else{
					xEventGroupSetBits(buttonsReleasedEventGroup, (1 << i));
				}
				buttons_old[i] = button;
			}
			if(button_num >= 0){
				EventBits_t bits;
				bits = xEventGroupClearBits(buttonsPressedEventGroup, (1 << i));
				if(bits & (1 << i)){
					std::cout << "BUTTON:pressed|" << button_num << std::endl;
				}
				bits = xEventGroupClearBits(buttonsReleasedEventGroup, (1 << i));
				if(bits & (1 << i)){
					std::cout << "BUTTON:released|" << button_num << std::endl;
				}
			}
        }

        // Send every second send slider position
        if((esp_timer_get_time() - timestamp) >= 1000000LL){
            need_positions_send = true;
        }

		// Send position no faster than 100ms period
        if(((esp_timer_get_time() - timestamp) >= 100000LL) && need_positions_send){
            timestamp = esp_timer_get_time();
            std::string pot_val_str = "";

            for (uint32_t i = 0; i < SLIDERS_COUNT; i++) {
                pot_val_str += std::to_string(static_cast<int>(positions_old[i]));
                if (i < SLIDERS_COUNT - 1) {
                    pot_val_str += "|";
                }
            }

            std::cout << pot_val_str;
            ble_send_volume(pot_val_str.c_str());
			// for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            //     std::cout << (int)positions_old[i];
            //     if(i < (SLIDERS_COUNT - 1)){
                    
            //     }
            // }

            std::cout << std::endl;
            need_positions_send = false;
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
