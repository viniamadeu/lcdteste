#include "lvgl.h"
#include "ui_home.h"
#include <unistd.h>

int main(void)
{
    lv_init();

    /* aqui entra a init do display do simulator */

    ui_home_create();

    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}
