#include "main.h"

#include <string.h>

#define NOTE "TASK SLAVE"

void task_slave(void* p_arg) {
    while (true) {
        msg_t* message = akos_thread_wait_for_msg(OS_CFG_DELAY_MAX);

        if (message != NULL) {
            switch (message->sig) {
            case MSG_SIGNAL_TEXT: {
                if (message->type == MSG_TYPE_DYNAMIC) {
                    uint8_t payload_size = 0u;
                    char* payload =
                        akos_message_get_dynamic_data(message, &payload_size);

                    if ((payload != NULL) &&
                        (payload_size == (uint8_t)sizeof(MESSAGE_TEXT)) &&
                        (payload[payload_size - 1u] == '\0') &&
                        (memcmp(payload, MESSAGE_TEXT, sizeof(MESSAGE_TEXT)) ==
                         0)) {
                        uint32_t ticks = akos_thread_get_tick();

                        PRINT_DBG(
                            "[%02lu-%02lu-%02lu %03lu ms] [%s]\trecv \"%s\" "
                            "from task Master\r\n\r\n",
                            (unsigned long)TIME_HOURS(ticks),
                            (unsigned long)TIME_MINUTES(ticks),
                            (unsigned long)TIME_SECONDS(ticks),
                            (unsigned long)TIME_MILLIS(ticks), NOTE, payload);
                    }
                }
            } break;

            default: {
            } break;
            }

            akos_message_free(message);
        }
    }
}
