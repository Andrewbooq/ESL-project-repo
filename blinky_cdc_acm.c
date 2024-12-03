#include "blinky_cdc_acm.h"

#include "app_usbd_cdc_acm.h"
#include "blinky_log.h"

static cdc_acm_t g_cdc_acm = 
{
    .cmd_index = 0,
    .on_command = NULL,
    .need_send = false,
    .sent = 0,
    .want_to_send = 0,
};

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm2,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);


static void blinky_append_rx_char(char ch)
{
    // end zero string
    if(g_cdc_acm.cmd_index < (BLINKY_COMMAND_SIZE - 1))
    {
        g_cdc_acm.command_buffer[g_cdc_acm.cmd_index] = ch;
        g_cdc_acm.cmd_index++;
    }
}

static void blinky_handle_command()
{
//    if (strlen(g_cdc_acm.command_buffer) == 0)
//    {
//        // Don't react on enter empty string
//        return;
//    }

    if(g_cdc_acm.on_command)
        g_cdc_acm.on_command(g_cdc_acm.command_buffer);

    memset(g_cdc_acm.command_buffer, 0, sizeof(g_cdc_acm.command_buffer));
    g_cdc_acm.cmd_index = 0;
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&usb_cdc_acm2, g_cdc_acm.rx_buffer, BLINKY_READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        NRF_LOG_INFO("tx done");
        if(g_cdc_acm.need_send)
        {
            NRF_LOG_INFO("need_send=%u", g_cdc_acm.need_send);
            NRF_LOG_INFO("want_to_sent=%u", g_cdc_acm.want_to_send);
            NRF_LOG_INFO("sent=%u", g_cdc_acm.sent);

            if(g_cdc_acm.want_to_send == g_cdc_acm.sent)
            {
                g_cdc_acm.need_send = false;
                g_cdc_acm.sent = 0;
                g_cdc_acm.want_to_send = 0;
                memset(g_cdc_acm.tx_buffer, 0, sizeof(g_cdc_acm.tx_buffer));
            }

            uint32_t size = 0;
            if((g_cdc_acm.want_to_send - g_cdc_acm.sent) > NRFX_USBD_EPSIZE)
            {
                size = NRFX_USBD_EPSIZE;
            }
            else
            {
                size = g_cdc_acm.want_to_send - g_cdc_acm.sent;
            }

            app_usbd_cdc_acm_write(&usb_cdc_acm2, g_cdc_acm.tx_buffer + g_cdc_acm.sent, size);
            g_cdc_acm.sent += size;
        }

        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm2);
            NRF_LOG_INFO("rx size: %d", size);

            /* It's the simple version of an echo. Note that writing doesn't
             * block execution, and if we have a lot of characters to read and
             * write, some characters can be missed.
             */
            if (g_cdc_acm.rx_buffer[0] == '\r' || g_cdc_acm.rx_buffer[0] == '\n')
            {
                blinky_handle_command();
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm2, "\r\n", 2);
            }
            else
            {
                blinky_append_rx_char(g_cdc_acm.rx_buffer[0]);
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm2, g_cdc_acm.rx_buffer, BLINKY_READ_SIZE);
            }

            /* Fetch data until internal buffer is empty */
            ret = app_usbd_cdc_acm_read(&usb_cdc_acm2, g_cdc_acm.rx_buffer, BLINKY_READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}

void blinky_cdc_acm_init(command_cb_t on_command)
{
    memset(g_cdc_acm.rx_buffer, 0, sizeof(g_cdc_acm.rx_buffer));
    memset(g_cdc_acm.tx_buffer, 0, sizeof(g_cdc_acm.tx_buffer));
    memset(g_cdc_acm.command_buffer, 0, sizeof(g_cdc_acm.command_buffer));

    g_cdc_acm.on_command = on_command;
    app_usbd_class_inst_t const * class_cdc_acm2 = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm2);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm2);
    UNUSED_VARIABLE(ret);
    ASSERT(ret == NRF_SUCCESS);
}

void blinky_cdc_acm_loop(void)
{
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
}

bool blinky_cdc_acm_send_str(const char* str)
{
    if (g_cdc_acm.need_send)
    {
        /* The current buffer is in progress*/
        return false;
    }
    
    CRITICAL_REGION_ENTER();
    strncpy(g_cdc_acm.tx_buffer, str, BLINKY_SEND_SIZE);
    g_cdc_acm.need_send = true;
    g_cdc_acm.sent = 0;
    g_cdc_acm.want_to_send = strlen(str);
    CRITICAL_REGION_EXIT();
    return true;
}
