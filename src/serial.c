#include "serial.h"

#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_serial.h"

#include "boards.h"

NRF_SERIAL_UART_DEF(m_serial, 0);

static nrf_drv_uart_config_t m_config;

static void serial_evt_handler(struct nrf_serial_s const *p_serial,
                               nrf_serial_event_t event);

#define SERIAL_FIFO_TX_SIZE 256
#define SERIAL_FIFO_RX_SIZE 256

NRF_SERIAL_QUEUES_DEF(serial0_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 1
#define SERIAL_BUFF_RX_SIZE 1

NRF_SERIAL_BUFFERS_DEF(serial0_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

#define SERIAL_TIMEOUT_MS 100

// TODO: set event handler. That way we can know when there's data!
NRF_SERIAL_CONFIG_DEF(serial_config_dma, NRF_SERIAL_MODE_DMA,
                      &serial0_queues, &serial0_buffs, serial_evt_handler, NULL);

char m_tx_buf[SERIAL_FIFO_TX_SIZE];

// Tracking if there are any peripheral errors
bool m_driver_error = false;

static void serial_evt_handler(struct nrf_serial_s const *p_serial,
                               nrf_serial_event_t event)
{

    switch (event)
    {
        case NRF_SERIAL_EVENT_RX_DATA:
            break;
        case NRF_SERIAL_EVENT_DRV_ERR:
        {
            m_driver_error = true;
            break;
        }
        case NRF_SERIAL_EVENT_FIFO_ERR:
            break;
        default:
            break;
    }
}

void serial_begin(uint32_t _baud)
{
    serial_begin_pins(_baud, TX, RX);
}

void serial_begin_pins(uint32_t _baud, uint8_t tx, uint8_t rx)
{

    nrf_uart_baudrate_t baud = NRF_UART_BAUDRATE_14400;

    // TODO: define these better
    switch (_baud)
    {
        case 9600:
            baud = NRF_UART_BAUDRATE_9600;
            break;
        case 14400:
            baud = NRF_UART_BAUDRATE_14400;
            break;
        case 28800:
            baud = NRF_UART_BAUDRATE_28800;
            break;
        case 38400:
            baud = NRF_UART_BAUDRATE_38400;
            break;
        case 115200:
            baud = NRF_UART_BAUDRATE_115200;
            break;
        default:
            NRF_LOG_ERROR("BAUD %d is not supported.", _baud);
            APP_ERROR_CHECK(NRF_ERROR_INVALID_PARAM);
            return;
    }

    m_config.pselrxd = rx;
    m_config.pseltxd = tx;
    m_config.pselrts = 0;
    m_config.pselcts = 0;
    m_config.hwfc = NRF_UART_HWFC_DISABLED;
    m_config.parity = NRF_UART_PARITY_EXCLUDED;
    m_config.baudrate = baud;
    m_config.interrupt_priority = UART_DEFAULT_CONFIG_IRQ_PRIORITY;
    m_config.use_easy_dma = true;

    ret_code_t err_code = nrf_serial_init(&m_serial, &m_config, &serial_config_dma);
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(1);

    // Drain the queues
    nrf_serial_rx_drain(&m_serial);
}

// TODO: confirm this works as expected.
int serial_available()
{
    return nrf_queue_utilization_get(serial0_queues.p_rxq);
}

//TODO: adding /0 chars?
size_t serial_println(const char *data)
{
    size_t size = strlen(data);
    // Return if invalid
    if (size + 1 > SERIAL_FIFO_TX_SIZE)
        return NRF_ERROR_INVALID_PARAM;

    // Temp buffer for modifying
    memcpy(m_tx_buf, data, SERIAL_FIFO_TX_SIZE);

    // Add newline
    m_tx_buf[size] = '\n';

    // NRF_LOG_HEXDUMP_INFO(buf, size + 1);

    size_t bytes_written = 0;

    // Write the bytes
    ret_code_t err_code = nrf_serial_write(&m_serial, m_tx_buf, size + 1, &bytes_written, 0);
    APP_ERROR_CHECK(err_code);

    return bytes_written;
}

int serial_read()
{

    if (serial_available() <= 0)
    {
        return -1;
    }

    uint8_t data;
    size_t bytes_read = 0;

    ret_code_t err_code = nrf_serial_read(&m_serial,
                                          &data,
                                          1,
                                          &bytes_read,
                                          0);
    APP_ERROR_CHECK(err_code);

    return data;
}

void serial_process()
{

    if (m_driver_error)
    {
        m_driver_error = false;

        // NRF_LOG_INFO("drvr error");

        ret_code_t err_code;

        err_code = nrf_serial_uninit(&m_serial);
        APP_ERROR_CHECK(err_code);

        err_code = nrf_serial_init(&m_serial, &m_config, &serial_config_dma);
        APP_ERROR_CHECK(err_code);

        nrf_delay_ms(1);

        // Drain the queues
        nrf_serial_rx_drain(&m_serial);
    }
}