#include "dac_service.h"

#include <zephyr/device.h>
#include <zephyr/drivers/dac.h>
#include <string.h>

#define ESP32_DAC_NODE DT_NODELABEL(dac)

#define DAC_OUT1_CHANNEL_ID 0  /* GPIO25 */
#define DAC_OUT2_CHANNEL_ID 1  /* GPIO26 */

#define DAC_RESOLUTION_BITS 8
#define DAC_REFERENCE_MV    3300

static const struct device *dac_dev = DEVICE_DT_GET(ESP32_DAC_NODE);

static int clamp_mv(int mv)
{
    if (mv < 0) {
        return 0;
    }

    if (mv > DAC_REFERENCE_MV) {
        return DAC_REFERENCE_MV;
    }

    return mv;
}

static int mv_to_raw_8bit(int mv)
{
    mv = clamp_mv(mv);

    return (mv * 255) / DAC_REFERENCE_MV;
}

static int get_dac_channel_id(const char *channel_name)
{
    if (strcmp(channel_name, "DAC_OUT1") == 0) {
        return DAC_OUT1_CHANNEL_ID;
    }

    if (strcmp(channel_name, "DAC_OUT2") == 0) {
        return DAC_OUT2_CHANNEL_ID;
    }

    return -1;
}

int dac_service_init(void)
{
    if (!device_is_ready(dac_dev)) {
        return -1;
    }

    struct dac_channel_cfg dac_out1_cfg = {
        .channel_id = DAC_OUT1_CHANNEL_ID,
        .resolution = DAC_RESOLUTION_BITS,
    };

    struct dac_channel_cfg dac_out2_cfg = {
        .channel_id = DAC_OUT2_CHANNEL_ID,
        .resolution = DAC_RESOLUTION_BITS,
    };

    if (dac_channel_setup(dac_dev, &dac_out1_cfg) < 0) {
        return -1;
    }

    if (dac_channel_setup(dac_dev, &dac_out2_cfg) < 0) {
        return -1;
    }

    dac_write_value(dac_dev, DAC_OUT1_CHANNEL_ID, 0);
    dac_write_value(dac_dev, DAC_OUT2_CHANNEL_ID, 0);

    return 0;
}

bool dac_service_write_mv(const char *channel_name, int mv)
{
    int channel_id = get_dac_channel_id(channel_name);

    if (channel_id < 0) {
        return false;
    }

    int raw = mv_to_raw_8bit(mv);

    if (dac_write_value(dac_dev, channel_id, raw) < 0) {
        return false;
    }

    return true;
}