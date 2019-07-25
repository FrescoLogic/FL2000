// fl2000_hdmi.c
//
// (c)Copyright 2009-2013, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

#define HDCP_ENABLE     0               /* not yet done */

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//

bool
fl2000_hdmi_bit_set(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint8_t offset_bit)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = fl2000_hdmi_read_byte_simple(dev_ctx, offset, &is_good);
        if (!is_good)
                goto exit;

        byte_data |= (1 << offset_bit);

        fl2000_hdmi_write_byte_simple(dev_ctx, offset, byte_data, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_bit_clear(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint8_t offset_bit)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = fl2000_hdmi_read_byte_simple(dev_ctx, offset, &is_good);
        if (!is_good)
                goto exit;

        byte_data &= ~(1 << offset_bit);

        fl2000_hdmi_write_byte_simple(dev_ctx, offset, byte_data, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

uint8_t
fl2000_hdmi_read_byte_simple(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        bool* is_good)
{
        int status;
        uint8_t byte_data;

        byte_data = 0;
        status = fl2000_hdmi_read_byte(dev_ctx, offset, &byte_data);
        if (status < 0)
        {
                *is_good = false;
                goto exit;
        }
        else
        {
                *is_good = true;
        }

exit:
        return byte_data;
}

void
fl2000_hdmi_write_byte_simple(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint8_t byte_data,
        bool* is_good)
{
        int status;

        status = fl2000_hdmi_write_byte(dev_ctx, offset, &byte_data);
        if (status < 0) {
                *is_good = false;
                return;
        }

        *is_good = true;
}

void
fl2000_write_byte_simple_with_mask(
        struct dev_ctx * dev_ctx,
        uint8_t reg,
        uint8_t mask,
        uint8_t value,
        bool* is_good)
{
        uint8_t byte_data;

        if (mask != 0xFF) {
                byte_data = fl2000_hdmi_read_byte_simple(dev_ctx, reg, is_good);
                if (*is_good == false)
                        goto exit;

                byte_data &= (~mask);
                byte_data |= (value & mask);
        }
        else {
                byte_data = value;
        }

        fl2000_hdmi_write_byte_simple(dev_ctx, reg, byte_data, is_good);
        if (*is_good == false)
                goto exit;

exit:
        return;
}

int
fl2000_hdmi_read_byte(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint8_t * return_data)
{
        int read_status;
        uint8_t aligned_offset;
        uint8_t remainder;
        uint32_t return_dword;
        uint8_t byte_data;

        remainder = offset % 4;
        aligned_offset = offset - remainder;

        read_status = fl2000_i2c_read(
                dev_ctx, I2C_ADDRESS_HDMI, aligned_offset, &return_dword);
        if (read_status < 0) {
                dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
                        "fl2000_i2c_read failed.");
                goto exit;
        }

        byte_data = (uint8_t) (return_dword >> (remainder * 8));
        *return_data = byte_data;
exit:
        return read_status;
}

int
fl2000_hdmi_write_byte(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint8_t * write_byte)
{
        int status;
        uint8_t aligned_offset;
        uint8_t remainder;
        uint32_t return_dword;
        uint32_t mask_data;

        remainder = offset % 4;
        aligned_offset = offset - remainder;

        status = fl2000_i2c_read(
                dev_ctx, I2C_ADDRESS_HDMI, aligned_offset, &return_dword);
        if (status < 0)
                goto exit;

        mask_data = 0xFF;
        mask_data <<= (remainder * 8);
        mask_data = ~mask_data;
        return_dword &= mask_data;

        mask_data = 0;
        mask_data = ( uint32_t )*write_byte;
        mask_data <<= (remainder *8);
        return_dword |= mask_data;

        status = fl2000_i2c_write(
                dev_ctx, I2C_ADDRESS_HDMI, aligned_offset, &return_dword);
        if (status < 0)
                goto exit;

exit:
        return status;
}

int
fl2000_hdmi_read_dword(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint32_t* return_data)
{
        return fl2000_i2c_read(dev_ctx, I2C_ADDRESS_HDMI, offset, return_data);
}

int
fl2000_hdmi_write_dword(
        struct dev_ctx * dev_ctx,
        uint8_t offset,
        uint32_t* write_dword)
{
        return fl2000_i2c_write(dev_ctx, I2C_ADDRESS_HDMI, offset, write_dword);
}

bool
fl2000_hdmi_abort_ddc(struct dev_ctx * dev_ctx)
{
        uint8_t swReset, cpDesire, ddcMaster;
        uint8_t ddcStatus, timeout, index;
        bool is_good;

        // Save the SW reset,DDC master,and CP Desire setting.
        //
        swReset = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_SW_RST, &is_good);
        if (!is_good)
                goto exit;

        cpDesire = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_HDCP_DESIRE, &is_good);
        if (!is_good)
                goto exit;

        cpDesire &= (~HDMI_ITE_B_TX_CPDESIRE);

        fl2000_hdmi_write_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_HDCP_DESIRE, cpDesire, &is_good);
        if (!is_good)
                goto exit;

        swReset |= HDMI_ITE_B_TX_HDCP_RST_HDMITX;

        fl2000_hdmi_write_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_SW_RST, swReset, &is_good);
        if (!is_good)
                goto exit;

        ddcMaster = HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST;
        fl2000_hdmi_write_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL, ddcMaster, &is_good);
        if (!is_good)
                goto exit;

        // Do abort DDC Twice.
        //
        for(index = 0 ; index < 2; index++) {
                fl2000_hdmi_write_byte_simple(
                        dev_ctx, HDMI_ITE_REG_TX_DDC_CMD,
                        HDMI_ITE_CMD_DDC_ABORT, &is_good);
                if (!is_good)
                        goto exit;

                for(timeout = 0 ; timeout < 200; timeout++) {
                        ddcStatus = fl2000_hdmi_read_byte_simple(
                                dev_ctx, HDMI_ITE_REG_TX_DDC_STATUS, &is_good);
                        if (!is_good)
                                goto exit;


                        if (ddcStatus & HDMI_ITE_B_TX_DDC_DONE) {
                                // Success.
                                //
                                break;
                        }

                        if (ddcStatus &
                            (HDMI_ITE_B_TX_DDC_NOACK |
                             HDMI_ITE_B_TX_DDC_WAITBUS |
                             HDMI_ITE_B_TX_DDC_ARBILOSE))
                                break;

                        DELAY_MS(50);
                }
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_clear_ddc_fifo(struct dev_ctx * dev_ctx)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = HDMI_ITE_B_TX_MASTERHOST | HDMI_ITE_B_TX_MASTERDDC;
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                byte_data, &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_FIFO_CLR, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

int
fl2000_hdmi_read_edid_table(
        struct dev_ctx * dev_ctx,
        uint8_t EdidSegmentIndex,
        uint8_t EdidTableOffset,
        uint8_t ReadCount,
        uint8_t * ReturnedBuffer)
{
        int status;
        uint8_t byte_data;
        uint32_t index;
        uint32_t dword_data;
        bool is_good;

        status = 0;

        // Per ITE's feedback, we need to do thw following sequence twice to
        // prevent FIFO lockup.

        // Start sequence.
        //

        // 0x10
        //
        is_good = fl2000_hdmi_bit_set(dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERHOST);
        if (!is_good) {
                status = -1;
                goto exit;
        }

        // 0x15
        //
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_DDC_ABORT, &is_good);
        if (!is_good) {
                status = -1;
                goto exit;
        }

        // 0x10
        //
        is_good = fl2000_hdmi_bit_set(dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERHOST);
        if (!is_good) {
                status = -1;
                goto exit;
        }

        // 0x15
        //
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_DDC_ABORT, &is_good);
        if (!is_good) {
                status = -1;
                goto exit;
        }

        // End sequence.
        //

        // 0x15
        //
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_FIFO_CLR, &is_good);
        if (!is_good) {
                status = -1;
                goto exit;
        }

        dword_data = 0;
        dword_data |= 0xA001;
        dword_data |= (EdidTableOffset << 16);
        dword_data |= (ReadCount << 24);

        // 0x10
        //
        status = fl2000_hdmi_write_dword(dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL, &dword_data);
        if (status < 0) {
                status = -1;
                goto exit;
        }

        dword_data &= 0xFFFF0000;
        dword_data |= 0x00000300;
        dword_data |= EdidSegmentIndex;

        // 0x14
        //
        status = fl2000_hdmi_write_dword(
                dev_ctx, HDMI_ITE_REG_TX_DDC_EDIDSEG, &dword_data);
        if (status < 0)
                goto exit;

        DELAY_MS(10);

        // Fill EDID Table
        //
        ReadCount -= 3;
        for (index = 0; index < ReadCount; index++) {
                // 0x17
                //
                byte_data = fl2000_hdmi_read_byte_simple(
                        dev_ctx, HDMI_ITE_REG_TX_DDC_READFIFO, &is_good);
                if (!is_good)
                        goto exit;

                ReturnedBuffer[index] = byte_data;
        }

exit:
        return status;
}

bool
fl2000_hdmi_read_edid_table_init(struct dev_ctx * dev_ctx)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good);
        if (!is_good)
                goto exit;

        if (byte_data & HDMI_ITE_B_TX_INT_DDC_BUS_HANG) {
                // Abort CDC
                //
                is_good = fl2000_hdmi_abort_ddc(dev_ctx);
                if (!is_good)
                        goto exit;
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_set_display_mode(struct dev_ctx * dev_ctx)
{
        bool is_good;

        is_good = fl2000_hdmi_av_mute(dev_ctx, 1);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_config_avi_info_frame(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_enable_video_output(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_config_audio_info_frame(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_enable_audio_output(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_av_mute(dev_ctx, 0);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_config_avi_info_frame(struct dev_ctx * dev_ctx)
{
        AVI_INFO_FRAME avi_info;
        bool is_good;
        uint8_t vic;
        uint8_t pixelrep;
        bool isAspectRatio16x9;
        bool isColorimetryITU709;

        memset(&avi_info, 0, sizeof(AVI_INFO_FRAME));

        if (dev_ctx->vr_params.width == 640 &&
            dev_ctx->vr_params.height == 480) {
                // 640x480p60Hz.
                //
                vic = 1;
                pixelrep = 0;
                isAspectRatio16x9 = false;
                isColorimetryITU709 = false;
        }
        else if (dev_ctx->vr_params.width == 1280 &&
                 dev_ctx->vr_params.height == 720) {
                // 720p60.
                //
                vic = 4;
                pixelrep = 0;
                isAspectRatio16x9 = true;
                isColorimetryITU709 = true;
        }
        else if (dev_ctx->vr_params.width == 1920 &&
                 dev_ctx->vr_params.height == 1080) {
                // 1080p60.
                //
                vic = 16;
                pixelrep = 0;
                isAspectRatio16x9 = true;
                isColorimetryITU709 = true;
        }
        else {
                vic = 0;
                pixelrep = 0;
                isAspectRatio16x9 = false;
                isColorimetryITU709 = false;
        }

        avi_info.AVI_HB[0] = HDMI_ITE_AVI_INFOFRAME_TYPE | 0x80;
        avi_info.AVI_HB[1] = HDMI_ITE_AVI_INFOFRAME_VER;
        avi_info.AVI_HB[2] = HDMI_ITE_AVI_INFOFRAME_LEN;

        avi_info.AVI_DB[ 0] = ( 0 << 5 ) | ( 1 << 4 );
        avi_info.AVI_DB[ 1] = 8;
        avi_info.AVI_DB[ 1] |= (!isAspectRatio16x9) ? (1 << 4) : (2 << 4);  // 4:3 or 16:9
        avi_info.AVI_DB[ 1] |= (!isColorimetryITU709) ? (1 << 6) : (2 << 6);  // 4:3 or 16:9
        avi_info.AVI_DB[ 2] = 0;
        avi_info.AVI_DB[ 3] = vic;
        avi_info.AVI_DB[ 4] = pixelrep & 3;
        avi_info.AVI_DB[ 5] = 0;
        avi_info.AVI_DB[ 6] = 0;
        avi_info.AVI_DB[ 7] = 0;
        avi_info.AVI_DB[ 8] = 0;
        avi_info.AVI_DB[ 9] = 0;
        avi_info.AVI_DB[10] = 0;
        avi_info.AVI_DB[11] = 0;
        avi_info.AVI_DB[12] = 0;

        is_good = fl2000_hdmi_enable_avi_info_frame(dev_ctx, true, &avi_info);

        return is_good;
}

bool
fl2000_hdmi_enable_avi_info_frame(
        struct dev_ctx * dev_ctx,
        bool enable,
        AVI_INFO_FRAME* avi_info_frame)
{
        bool is_good;

        is_good = true;
        if(!enable) {
                fl2000_hdmi_write_byte_simple(
                        dev_ctx, HDMI_ITE_REG_TX_AVI_INFOFRM_CTRL, 0, &is_good);
                if (!is_good)
                        goto exit;
        }
        else {
                is_good = fl2000_hdmi_set_avi_info_frame(dev_ctx, avi_info_frame);
                if (!is_good)
                        goto exit;
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_set_avi_info_frame(
        struct dev_ctx * dev_ctx,
        AVI_INFO_FRAME* avi_info_frame)
{
        uint8_t index;
        uint32_t sumOfAviDb;
        uint32_t sumOfVersionHeader;
        bool is_good;
        uint32_t dword_data;
        uint8_t byte_data;

        is_good = true;

        if(!avi_info_frame) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_switch_bank(dev_ctx, 1);

        dword_data = 0;

        // HDMI Register offset 0x58 : HDMI_ITE_REG_TX_AVIINFO_DB1
        //
        byte_data = avi_info_frame->AVI_DB[0];
        dword_data |= (byte_data);

        // HDMI Register offset 0x59 : HDMI_ITE_REG_TX_AVIINFO_DB2
        //
        byte_data = avi_info_frame->AVI_DB[1];
        dword_data |= (byte_data << 8);

        // HDMI Register offset 0x5A : HDMI_ITE_REG_TX_AVIINFO_DB3
        //
        byte_data = avi_info_frame->AVI_DB[2];
        dword_data |= (byte_data << 16);

        // HDMI Register offset 0x5B : HDMI_ITE_REG_TX_AVIINFO_DB4
        //
        byte_data = avi_info_frame->AVI_DB[3];
        dword_data |= (byte_data << 24);

        fl2000_hdmi_write_dword(dev_ctx, 0x58, &dword_data);

        dword_data = 0;

        // HDMI Register offset 0x5C : HDMI_ITE_REG_TX_AVIINFO_DB5
        //
        byte_data = avi_info_frame->AVI_DB[4];
        dword_data |= byte_data;

        // HDMI Register offset 0x5D : HDMI_ITE_REG_TX_AVIINFO_SUM
        //
        sumOfAviDb = 0;
        for(index = 0; index < HDMI_ITE_AVI_INFOFRAME_LEN ; index++) {
                sumOfAviDb += avi_info_frame->AVI_DB[index];
        }

        sumOfVersionHeader = HDMI_ITE_AVI_INFOFRAME_VER +
                HDMI_ITE_AVI_INFOFRAME_TYPE + HDMI_ITE_AVI_INFOFRAME_LEN;

        avi_info_frame->CheckSum = (uint8_t) (0x100 - sumOfAviDb -
                sumOfVersionHeader);

        byte_data = avi_info_frame->CheckSum;
        dword_data |= (byte_data << 8);

        // HDMI Register offset 0x5E : HDMI_ITE_REG_TX_AVIINFO_DB6
        //
        byte_data = avi_info_frame->AVI_DB[5];
        dword_data |= (byte_data << 16);

        // HDMI Register offset 0x5F : HDMI_ITE_REG_TX_AVIINFO_DB7
        //
        byte_data = avi_info_frame->AVI_DB[6];
        dword_data |= (byte_data << 24);

        fl2000_hdmi_write_dword(dev_ctx, 0x5C, &dword_data);

        dword_data = 0;

        // HDMI Register offset 0x60 : HDMI_ITE_REG_TX_AVIINFO_DB8
        //
        byte_data = avi_info_frame->AVI_DB[7];
        dword_data |= ( byte_data );

        // HDMI Register offset 0x61 : HDMI_ITE_REG_TX_AVIINFO_DB9
        //
        byte_data = avi_info_frame->AVI_DB[8];
        dword_data |= ( byte_data << 8 );

        // HDMI Register offset 0x62 : HDMI_ITE_REG_TX_AVIINFO_DB10
        //
        byte_data = avi_info_frame->AVI_DB[9];
        dword_data |= (byte_data << 16);

        // HDMI Register offset 0x63 : HDMI_ITE_REG_TX_AVIINFO_DB11
        //
        byte_data = avi_info_frame->AVI_DB[10];
        dword_data |= (byte_data << 24);

        fl2000_hdmi_write_dword(dev_ctx, 0x60, &dword_data);

        dword_data = 0;

        // HDMI Register offset 0x64 : HDMI_ITE_REG_TX_AVIINFO_DB12
        //
        byte_data = avi_info_frame->AVI_DB[11];
        dword_data |= byte_data;

        // HDMI Register offset 0x65 : HDMI_ITE_REG_TX_AVIINFO_DB13
        //
        byte_data = avi_info_frame->AVI_DB[12];
        dword_data |= (byte_data << 8);

        fl2000_hdmi_write_dword(dev_ctx, 0x64, &dword_data);

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        fl2000_hdmi_write_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_AVI_INFOFRM_CTRL,
                B_TX_ENABLE_PKT | B_TX_REPEAT_PKT, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_disable_audio_output(struct dev_ctx * dev_ctx)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = (HDMI_ITE_B_HDMI_AUD_RST | HDMI_ITE_B_TX_AREF_RST);
        fl2000_write_byte_simple_with_mask(dev_ctx, HDMI_ITE_REG_TX_SW_RST,
                byte_data, byte_data, &is_good);
        if (!is_good)
                goto exit;

        fl2000_write_byte_simple_with_mask(dev_ctx, 0xF, 0x10, 0x10, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_av_mute(
        struct dev_ctx * dev_ctx,
        uint8_t IsMute)
{
        uint8_t byte_data;
        bool is_good;

        is_good = fl2000_hdmi_switch_bank(dev_ctx, 0);
        if (!is_good)
                goto exit;

        fl2000_write_byte_simple_with_mask(dev_ctx, HDMI_ITE_REG_TX_GCP,
                HDMI_ITE_B_TX_SETAVMUTE, IsMute, &is_good);
        if (!is_good)
                goto exit;

        byte_data = HDMI_ITE_B_TX_ENABLE_PKT | HDMI_ITE_B_TX_REPEAT_PKT;
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_PKT_GENERAL_CTRL,
                byte_data, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_enable_video_output(struct dev_ctx * dev_ctx)
{
        uint64_t pixelClock;
        uint8_t level;
        uint8_t byte_data;
        bool is_good;
        uint8_t outputMode;

        pixelClock = dev_ctx->vr_params.h_total_time *
                dev_ctx->vr_params.v_total_time *
                dev_ctx->vr_params.freq;

        if( pixelClock > 80000000L)
                level = HDMI_ITE_PCLK_HIGH;
        else if (pixelClock > 20000000L)
                level = HDMI_ITE_PCLK_MEDIUM;
        else
                level = HDMI_ITE_PCLK_LOW;

        byte_data = (HDMI_ITE_B_HDMI_VID_RST | HDMI_ITE_B_TX_HDCP_RST_HDMITX);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST,
                byte_data, &is_good);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_set_input_mode(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_set_csc_scale( dev_ctx );
        if (!is_good)
                goto exit;

        if (dev_ctx->hdmi_running_in_dvi_mode)
                outputMode = HDMI_ITE_B_TX_MODE_DVI;
        else
                outputMode = HDMI_ITE_B_TX_MODE_HDMI;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_HDMI_MODE,
                outputMode, &is_good);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_setup_afe(dev_ctx, level);
        if (!is_good)
                goto exit;

        byte_data = (HDMI_ITE_B_TX_HDCP_RST_HDMITX);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST,
                byte_data, &is_good);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_fire_afe(dev_ctx);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_fire_afe(struct dev_ctx * dev_ctx)
{
        bool is_good;

        is_good = fl2000_hdmi_switch_bank( dev_ctx, 0 );
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AFE_DRV_CTRL, 0,
                &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_setup_afe(struct dev_ctx * dev_ctx, uint8_t Level)
{
        bool is_good;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AFE_DRV_CTRL,
                HDMI_ITE_B_TX_AFE_DRV_RST, &is_good);
        if (!is_good)
                goto exit;

        switch (Level) {
        case HDMI_ITE_PCLK_HIGH:
                fl2000_write_byte_simple_with_mask(dev_ctx, 0x62, 0x90, 0x80, &is_good);
                if (!is_good)
                        goto exit;

                fl2000_write_byte_simple_with_mask( dev_ctx, 0x64, 0x89, 0x80, &is_good);
                if (!is_good)
                        goto exit;

                fl2000_write_byte_simple_with_mask( dev_ctx, 0x68, 0x10, 0x80, &is_good);
                if (!is_good)
                        goto exit;

                break;

        default:
                fl2000_write_byte_simple_with_mask( dev_ctx, 0x62, 0x90, 0x10, &is_good);
                if (!is_good)
                        goto exit;

                fl2000_write_byte_simple_with_mask( dev_ctx, 0x64, 0x89, 0x09, &is_good);
                if (!is_good)
                        goto exit;

                fl2000_write_byte_simple_with_mask( dev_ctx, 0x68, 0x10, 0x10, &is_good);
                if (!is_good)
                        goto exit;

                break;
        }

        fl2000_write_byte_simple_with_mask(dev_ctx, HDMI_ITE_REG_TX_SW_RST,
                HDMI_ITE_B_TX_REF_RST_HDMITX | HDMI_ITE_B_HDMI_VID_RST, 0,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AFE_DRV_CTRL, 0,
                &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_switch_bank(struct dev_ctx * dev_ctx, uint8_t bank_num)
{
        bool is_good;

        bank_num &= 1;

        fl2000_hdmi_write_byte_simple(dev_ctx, 0xF, bank_num, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_set_input_mode(struct dev_ctx * dev_ctx)
{
        uint8_t byte_data;
        bool is_good;

        byte_data = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_INPUT_MODE, &is_good);
        if (!is_good)
                goto exit;

        byte_data &= ~(HDMI_ITE_M_TX_INCOLMOD | HDMI_ITE_B_TX_2X656CLK |
                HDMI_ITE_B_TX_SYNCEMB | HDMI_ITE_B_TX_INDDR |
                HDMI_ITE_B_TX_PCLKDIV2);
        byte_data |= 0x01;
        byte_data |= HDMI_ITE_B_TX_IN_RGB;

        fl2000_hdmi_write_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_INPUT_MODE, byte_data, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_set_csc_scale(struct dev_ctx * dev_ctx)
{
        uint8_t byte_data;
        bool is_good;

        fl2000_write_byte_simple_with_mask(dev_ctx, 0xF, 0x10, 0x10, &is_good);
        if (!is_good)
                goto exit;

        byte_data = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_CSC_CTRL, &is_good);
        if (!is_good)
                goto exit;

        byte_data &= ~( HDMI_ITE_M_TX_CSC_SEL | HDMI_ITE_B_TX_DNFREE_GO |
                HDMI_ITE_B_TX_EN_DITHER | HDMI_ITE_B_TX_EN_UDFILTER);
        byte_data |= HDMI_ITE_B_HDMI_CSC_BYPASS;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_CSC_CTRL,
                byte_data, &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_find_chip(struct dev_ctx * dev_ctx)
{
        bool found_hdmi = false;
        int status;
        uint32_t data;

        status = fl2000_hdmi_read_dword(dev_ctx, 0, &data);
        if (status > 0) {
                const uint32_t venderID = (data & 0xFFFF);
                const uint32_t deviceID = (data >> 16 & 0xFFF);

                if (venderID == HDMI_ITE_VENDER_ID &&
                    deviceID == HDMI_ITE_DEVICE_ID)
                        found_hdmi = true;
        }

        return found_hdmi;
}

bool
fl2000_hdmi_power_up(struct dev_ctx * dev_ctx)
{
        bool is_good = true;
        int index;
        const HDMI_REGISTER_SET_ENTRY table[] =
        {
                {0x0F, 0x78, 0x38},   // PwrOn GRCLK
                {0x05, 0x01, 0x00},   // PwrOn PCLK

                // PLL PwrOn
                //
                {0x61, 0x20, 0x00},   // PwrOn DRV
                {0x62, 0x44, 0x00},   // PwrOn XPLL
                {0x64, 0x40, 0x00},   // PwrOn IPLL

                // PLL Reset OFF
                //
                {0x61, 0x10, 0x00},   // DRV_RST
                {0x62, 0x08, 0x08},   // XP_RESETB
                {0x64, 0x04, 0x04},   // IP_RESETB

                {0x6A, 0xFF, 0x70},   // 0x30 0x70
                {0x66, 0xFF, 0x1F},   // 0x00 0x1F
                {0x63, 0xFF, 0x38},   // 0x18 0x38

                {0x0F, 0x78, 0x08},   // PwrOn IACLK
        };

        for(index = 0; index < 12; index++) {
                if (table[index].InvAndMask == 0 &&
                    table[index].OrMask == 0) {
                        DELAY_MS(table[index].Offset);
                }
                else if(table[index].InvAndMask == 0xFF) {
                        fl2000_hdmi_write_byte_simple(
                                dev_ctx, table[index].Offset,
                                table[index].OrMask, &is_good);
                        if (!is_good)
                                goto exit;
                }
                else {
                        fl2000_write_byte_simple_with_mask(
                                dev_ctx, table[index].Offset,
                                table[index].InvAndMask,
                                table[index].OrMask,
                                &is_good);
                        if (!is_good)
                                goto exit;
                }
        }

        dev_ctx->hdmi_powered_up = true;

exit:
        return (is_good);
}

bool
fl2000_hdmi_power_down(struct dev_ctx * dev_ctx)
{
        bool is_good = true;
        int index;
        HDMI_REGISTER_SET_ENTRY table[] =
        {
                // Enable GRCLK
                //
                {0x0F, 0x40, 0x00},

                // PLL Reset
                //
                {0x61, 0x10, 0x10},   // DRV_RST
                {0x62, 0x08, 0x00},   // XP_RESETB
                {0x64, 0x04, 0x00},   // IP_RESETB
                {0x01, 0x00, 0x00},   // idle(100);

                // PLL PwrDn
                //
                {0x61, 0x20, 0x20},   // PwrDn DRV
                {0x62, 0x44, 0x44},   // PwrDn XPLL
                {0x64, 0x40, 0x40},   // PwrDn IPLL

                // HDMITX PwrDn
                //
                {0x05, 0x01, 0x01},   // PwrDn PCLK
                {0x0F, 0x78, 0x78},   // PwrDn GRCLK
        };

        for( index = 0 ; index < 10; index++ ) {
                if(table[index].InvAndMask == 0 &&
                   table[ index ].OrMask == 0) {
                        // For saving HDMI render time, w/o this delay still works.
                        // So remove it for now.
                        //
                        // DELAY_MS( table[ index ].offset );
                        //
                }
                else if (table[index].InvAndMask == 0xFF) {
                        fl2000_hdmi_write_byte_simple(
                                dev_ctx,
                                table[index].Offset,
                                table[index].OrMask,
                                &is_good);
                        if (!is_good)
                                goto exit;
                }
                else {
                        fl2000_write_byte_simple_with_mask(
                                dev_ctx,
                                table[index].Offset,
                                table[index].InvAndMask,
                                table[index].OrMask,
                                &is_good);
                        if (!is_good)
                                goto exit;
                }
        }

        dev_ctx->hdmi_powered_up = false;

exit:
        return is_good;
}

bool
fl2000_hdmi_reset(struct dev_ctx * dev_ctx)
{
        bool is_good;

        is_good = fl2000_hdmi_bit_set(dev_ctx, HDMI_ITE_REG_TX_SW_RST, 5);
        if (!is_good)
                goto exit;

        DELAY_MS(300);

exit:
        return is_good;
}

bool
fl2000_hdmi_setup_ncts(struct dev_ctx * dev_ctx, uint8_t Fs)
{
        uint32_t n;
        uint8_t LoopCnt,CTSStableCnt;
        uint32_t LastCTS;
        bool is_good;
        uint8_t tempByte;
        uint32_t SumCTS;
        bool HBR_mode;

        LoopCnt = 255;
        CTSStableCnt = 0;
        LastCTS = 0;
        SumCTS = 0;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx, HDMI_ITE_REG_TX_AUD_HDAUDIO, &is_good);
        if (HDMI_ITE_B_TX_HBR & tempByte)
                HBR_mode = true;
        else
                HBR_mode = false;

        switch(Fs) {
        case HDMI_ITE_AUDFS_32KHZ:
                n = 4096;
                break;

        case HDMI_ITE_AUDFS_44p1KHZ:
                n = 6272;
                break;

        case HDMI_ITE_AUDFS_48KHZ:
                n = 6144;
                break;

        case HDMI_ITE_AUDFS_88p2KHZ:
                n = 12544;
                break;

        case HDMI_ITE_AUDFS_96KHZ:
                n = 12288;
                break;

        case HDMI_ITE_AUDFS_176p4KHZ:
                n = 25088;
                break;

        case HDMI_ITE_AUDFS_192KHZ:
                n = 24576;
                break;

        case HDMI_ITE_AUDFS_768KHZ:
                n = 24576;
                break;

        default:
                n = 6144;
                break;
        }

        // Per ITE suggestion, we got to use this N value to balance BoE monitor
        // jitter tolerance problem.
        n = 4096;

        fl2000_hdmi_switch_bank(dev_ctx, 1);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_N0,
                (uint8_t)((n)&0xFF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_N1,
                (uint8_t)((n>>8)&0xFF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_N2,
                (uint8_t)((n>>16)&0xF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_CTS0,
                (uint8_t)((LastCTS)&0xFF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_CTS1,
                (uint8_t)((LastCTS>>8)&0xFF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_PACKET_AUDIO_CTS2,
                (uint8_t)((LastCTS>>16)&0xF), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        fl2000_hdmi_write_byte_simple(dev_ctx, 0xF8, 0xC3, &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, 0xF8, 0xA5, &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(dev_ctx,
                HDMI_ITE_REG_TX_PKT_SINGLE_CTRL, &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_PKT_SINGLE_CTRL,
                tempByte & ( ~HDMI_ITE_B_TX_SW_CTS ), &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(dev_ctx, 0xF8, 0xFF, &is_good);
        if (!is_good)
                goto exit;

        if (false == HBR_mode) {
                //LPCM
                fl2000_hdmi_switch_bank( dev_ctx, 1 );
                Fs = HDMI_ITE_AUDFS_768KHZ;

                fl2000_hdmi_write_byte_simple(dev_ctx,
                        HDMI_ITE_REG_TX_AUDCHST_CA_FS, 0x00 | Fs, &is_good);

                Fs = ~Fs ; // OFS is the one's complement of FS

                tempByte = (0x0f & fl2000_hdmi_read_byte_simple(
                        dev_ctx, HDMI_ITE_REG_TX_AUDCHST_OFS_WL, &is_good));
                fl2000_hdmi_write_byte_simple(
                        dev_ctx, HDMI_ITE_REG_TX_AUDCHST_OFS_WL,
                        (Fs << 4) | tempByte, &is_good);

                fl2000_hdmi_switch_bank(dev_ctx, 0);
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_setup_ch_stat(struct dev_ctx * dev_ctx, uint8_t ChStatData[])
{
        uint8_t uc;
        bool is_good;
        uint8_t data_byte;
        uint32_t dword_data;
        uint8_t byte_data;

        fl2000_hdmi_switch_bank(dev_ctx, 1);

        uc = (ChStatData[0] << 1)& 0x7C;
        dword_data = 0;

        // HDMI Register offset 0x91 : HDMI_ITE_REG_TX_AUDCHST_MODE
        //
        byte_data = uc;
        dword_data |= (byte_data << 8);

        // HDMI Register offset 0x92 : HDMI_ITE_REG_TX_AUDCHST_CAT
        //
        byte_data = ChStatData[1];
        dword_data |= (byte_data << 16);

        // HDMI Register offset 0x93 : HDMI_ITE_REG_TX_AUDCHST_SRCNUM
        //
        byte_data = ChStatData[2] & 0xF;
        dword_data |= (byte_data << 24);

        fl2000_hdmi_write_dword(dev_ctx, 0x90, &dword_data);

        // HDMI Register offset 0x94 : HDMI_ITE_REG_TX_AUD0CHST_CHTNUM
        //
        data_byte = (ChStatData[2] >> 4) & 0xF;
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AUD0CHST_CHTNUM,
                (ChStatData[2]>>4)&0xF, &is_good);
        if (!is_good)
                goto exit;

        dword_data = 0;

        // HDMI Register offset 0x98 : HDMI_ITE_REG_TX_AUDCHST_CA_FS
        //
        byte_data = ChStatData[3];
        dword_data |= (byte_data);

        // HDMI Register offset 0x99 : HDMI_ITE_REG_TX_AUDCHST_OFS_WL
        //
        byte_data = ChStatData[3];
        dword_data |= (byte_data << 8);

        fl2000_hdmi_write_dword(dev_ctx, 0x98, &dword_data);

        fl2000_hdmi_switch_bank(dev_ctx, 0);

exit:
        return is_good;
}

bool
fl2000_hdmi_setup_pcm_audio(
        struct dev_ctx * dev_ctx,
        uint8_t AudioSrcNum,
        uint8_t AudSWL)
{
        uint8_t AudioEnable, AudioFormat ;
        bool is_good;
        uint32_t index;
        uint32_t dword_data;
        uint8_t byte_data;

        AudioEnable = 0;
        AudioFormat = 0;
        is_good = true;

        switch(AudSWL) {
        case 16:
            AudioEnable |= HDMI_ITE_M_TX_AUD_16BIT;
            break ;
        case 18:
            AudioEnable |= HDMI_ITE_M_TX_AUD_18BIT;
            break ;
        case 20:
            AudioEnable |= HDMI_ITE_M_TX_AUD_20BIT;
            break ;
        case 24:
        default:
            AudioEnable |= HDMI_ITE_M_TX_AUD_24BIT;
            break ;
        }

        if (dev_ctx->hdmi_audio_use_spdif) {
                AudioFormat &= ~HDMI_ITE_B_TX_AUDFMT_FULLPKT;
                AudioEnable |= HDMI_ITE_B_TX_AUD_SPDIF | HDMI_ITE_B_TX_AUD_EN_I2S0;
        }
        else {
                switch(AudioSrcNum) {
                case 4:
                        AudioEnable |= HDMI_ITE_B_TX_AUD_EN_I2S3 |
                                HDMI_ITE_B_TX_AUD_EN_I2S2 |
                                HDMI_ITE_B_TX_AUD_EN_I2S1 |
                                HDMI_ITE_B_TX_AUD_EN_I2S0;
                        break;
                case 3:
                        AudioEnable |= HDMI_ITE_B_TX_AUD_EN_I2S2 |
                                HDMI_ITE_B_TX_AUD_EN_I2S1 |
                                HDMI_ITE_B_TX_AUD_EN_I2S0;
                        break;
                case 2:
                        AudioEnable |= HDMI_ITE_B_TX_AUD_EN_I2S1 |
                                HDMI_ITE_B_TX_AUD_EN_I2S0;
                                break ;
                case 1:
                default:
                        AudioEnable |= HDMI_ITE_B_TX_AUD_EN_I2S0;
                        break ;
                }
                AudioFormat |= HDMI_ITE_B_TX_AUDFMT_FULLPKT;
                AudioFormat |= HDMI_ITE_B_TX_AUDFMT_32BIT_I2S;
        }

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        dword_data = 0;
        // HDMI Register offset 0xE0 : HDMI_ITE_REG_TX_AUDIO_CTRL0
        //
        byte_data = AudioEnable;
        dword_data |= byte_data;

        // HDMI Register offset 0xE1 : HDMI_ITE_REG_TX_AUDIO_CTRL1
        //
        byte_data = AudioFormat;
        dword_data |= (byte_data << 8);

        // HDMI Register offset 0xE2 : HDMI_ITE_REG_TX_AUDIO_FIFOMAP
        //
        byte_data = 0xE4;
        dword_data |= (byte_data << 16);

        // HDMI Register offset 0xE3 : HDMI_ITE_REG_TX_AUDIO_CTRL3
        //
        if (dev_ctx->hdmi_audio_use_spdif)
                byte_data = HDMI_ITE_B_TX_CHSTSEL;
        else
                byte_data = 0;

        dword_data |= (byte_data << 24);

        fl2000_hdmi_write_dword(dev_ctx, 0xE0, &dword_data);

        dword_data = 0;
        // HDMI Register offset 0xE4 : HDMI_ITE_REG_TX_AUD_SRCVALID_FLAT
        // HDMI Register offset 0xE5 : HDMI_ITE_REG_TX_AUD_HDAUDIO
        //
        fl2000_hdmi_write_dword(dev_ctx, 0xE4, &dword_data);

        if (dev_ctx->hdmi_audio_use_spdif) {
                fl2000_write_byte_simple_with_mask(
                        dev_ctx, 0x5C, 1 << 6, 1 << 6, &is_good);

                for(index=0 ;index < 100 ;index++){
                        uint8_t data_byte;

                        data_byte = fl2000_hdmi_read_byte_simple(
                                dev_ctx, HDMI_ITE_REG_TX_CLK_STATUS2, &is_good);
                        if (!is_good)
                                goto exit;

                        if(data_byte & HDMI_ITE_B_TX_OSF_LOCK)
                                break ; // stable clock.
                }
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_config_audio_info_frame(struct dev_ctx * dev_ctx)
{
        AUDIO_INFO_FRAME audioInfoFrame;
        bool is_good;

        memset(&audioInfoFrame, 0, sizeof(AUDIO_INFO_FRAME));

        audioInfoFrame.AUD_HB[0] = HDMI_ITE_AUDIO_INFOFRAME_TYPE;
        audioInfoFrame.AUD_HB[1] = 1;
        audioInfoFrame.AUD_HB[2] = HDMI_ITE_AUDIO_INFOFRAME_LEN;

        audioInfoFrame.AUD_DB[0] = 1 ;

        is_good = fl2000_hdmi_enable_audio_info_frame(dev_ctx, true, &audioInfoFrame);

        return is_good;
}

bool
fl2000_hdmi_enable_audio_info_frame(
        struct dev_ctx * dev_ctx,
        uint8_t enable,
        PAUDIO_INFO_FRAME AudioInfoFrame)
{
        bool is_good;

        if(!enable) {
                fl2000_hdmi_write_byte_simple(dev_ctx,
                        HDMI_ITE_REG_TX_AVI_INFOFRM_CTRL, 0, &is_good);
                goto exit;
        }

        is_good = fl2000_hdmi_set_audio_info_frame(dev_ctx, AudioInfoFrame);

exit:
        return is_good;
}

bool
fl2000_hdmi_set_audio_info_frame(
        struct dev_ctx * dev_ctx,
        PAUDIO_INFO_FRAME AudioInfoFrame)
{
        uint8_t checksum;
        uint8_t tempByte;
        bool is_good;

        fl2000_hdmi_switch_bank(dev_ctx, 1);

        checksum = 0x100 - ( HDMI_ITE_AUDIO_INFOFRAME_VER +
                HDMI_ITE_AUDIO_INFOFRAME_TYPE + HDMI_ITE_AUDIO_INFOFRAME_LEN);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_CC,
                AudioInfoFrame->AUD_DB[0],
                &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_CC,
                &is_good);
        if (!is_good)
                goto exit;

        checksum -= tempByte;
        checksum &= 0xFF ;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_SF,
                AudioInfoFrame->AUD_DB[1],
                &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_SF,
                &is_good);
        if (!is_good)
                goto exit;

        checksum -= tempByte;
        checksum &= 0xFF ;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_CA,
                AudioInfoFrame->AUD_DB[3],
                &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_CA,
                &is_good);
        if (!is_good)
                goto exit;

        checksum -= tempByte;
        checksum &= 0xFF ;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_DM_LSV,
                AudioInfoFrame->AUD_DB[4],
                &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_DM_LSV,
                &is_good);
        if (!is_good)
                goto exit;
        checksum -= tempByte;
        checksum &= 0xFF ;

        AudioInfoFrame->CheckSum = checksum;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_PKT_AUDINFO_SUM,
                AudioInfoFrame->CheckSum,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_AUD_INFOFRM_CTRL,
                B_TX_ENABLE_PKT | B_TX_REPEAT_PKT,
                &is_good);
        if (!is_good)
                goto exit;

exit:
        return is_good;
}

bool
fl2000_hdmi_setup_audio_output(
        struct dev_ctx * dev_ctx,
        uint32_t SampleFreq,
        uint8_t ChNum)
{
        uint8_t ChStatData[5];
        bool is_good;
        uint8_t tempByte;
        uint8_t Fs;
        uint8_t *pChStatData;

        pChStatData = NULL;

        fl2000_write_byte_simple_with_mask(
                dev_ctx,
                HDMI_ITE_REG_TX_SW_RST,
                (HDMI_ITE_B_HDMI_AUD_RST | HDMI_ITE_B_TX_AREF_RST),
                (HDMI_ITE_B_HDMI_AUD_RST | HDMI_ITE_B_TX_AREF_RST),
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_CLK_CTRL0,
                HDMI_ITE_B_TX_AUTO_OVER_SAMPLING_CLOCK | HDMI_ITE_B_TX_EXT_256FS | 0x01,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_write_byte_simple_with_mask(dev_ctx, 0x0F, 0x10, 0x00, &is_good);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_AUDIO_CTRL0,
                &is_good);

        if (!is_good)
                goto exit;

        if (dev_ctx->hdmi_audio_use_spdif) {
                fl2000_hdmi_write_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_AUDIO_CTRL0,
                        tempByte & HDMI_ITE_B_TX_AUD_SPDIF,
                        &is_good);
                if (!is_good )
                        goto exit;
        }
        else {
                fl2000_hdmi_write_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_AUDIO_CTRL0,
                        tempByte & ~HDMI_ITE_B_TX_AUD_SPDIF,
                        &is_good);
                if (!is_good)
                        goto exit;
        }

        // one bit audio have no channel status.
        //
        switch( SampleFreq ) {
        case  44100L:
                Fs = HDMI_ITE_AUDFS_44p1KHZ;
                break;
        case  88200L:
                Fs = HDMI_ITE_AUDFS_88p2KHZ;
                break ;
        case 176400L:
                Fs = HDMI_ITE_AUDFS_176p4KHZ;
                break ;
        case  32000L:
                Fs = HDMI_ITE_AUDFS_32KHZ;
                break;
        case 48000L:
                Fs = HDMI_ITE_AUDFS_48KHZ;
                break;
        case 96000L:
                Fs = HDMI_ITE_AUDFS_96KHZ;
                break;
        case 192000L:
                Fs = HDMI_ITE_AUDFS_192KHZ;
                break ;
        case 768000L:
                Fs = HDMI_ITE_AUDFS_768KHZ;
                break ;
        default:
                SampleFreq = 48000L ;
                Fs = HDMI_ITE_AUDFS_48KHZ ;
                break;
        }

        is_good = fl2000_hdmi_setup_ncts(dev_ctx, Fs);
        if (!is_good)
                goto exit;

        ChStatData[0] = 0;
        ChStatData[1] = 0;
        ChStatData[2] = ChNum;

        if (ChStatData[2] < 1)
                ChStatData[2] = 1 ;
        else if( ChStatData[2] > 4)
                ChStatData[2] = 4 ;

        ChStatData[3] = Fs;
        ChStatData[4] = (((~Fs) <<4 ) & 0xF0) | HDMI_ITE_CHTSTS_SWCODE;

        pChStatData = ChStatData;

        fl2000_write_byte_simple_with_mask(
                dev_ctx,
                HDMI_ITE_REG_TX_SW_RST,
                (HDMI_ITE_B_HDMI_AUD_RST | HDMI_ITE_B_TX_AREF_RST),
                HDMI_ITE_B_TX_AREF_RST,
                &is_good);
        if (!is_good)
                goto exit;

        pChStatData[0] &= ~(1 << 1);

        is_good = fl2000_hdmi_setup_ch_stat(dev_ctx, pChStatData);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_setup_pcm_audio(
                dev_ctx ,
                1,
                HDMI_ITE_AUDIO_NUMBER_OF_BITS);
        if (!is_good)
                goto exit;

        tempByte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_MASK1,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_MASK1,
                tempByte & ~HDMI_ITE_B_TX_AUDIO_OVFLW_MASK,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_write_byte_simple_with_mask(
                dev_ctx,
                HDMI_ITE_REG_TX_SW_RST,
                (HDMI_ITE_B_HDMI_AUD_RST | HDMI_ITE_B_TX_AREF_RST),
                0,
                &is_good);
        if (!is_good)
                goto exit;

exit:

        return is_good;
}

bool
fl2000_hdmi_enable_audio_output(struct dev_ctx * dev_ctx)
{
        return fl2000_hdmi_setup_audio_output(
                dev_ctx,
                HDMI_ITE_AUDIO_SAMPLE_FREQUENCY_HZ,
                HDMI_ITE_AUDIO_NUMBER_OF_CHANNEL);
}

#if 0
/*
 * this is for debugging. no need to port.
 */
void
fl2000_hdmi_dump_bank(struct dev_ctx * dev_ctx)
{
        uint8_t * bankPtr0;
        uint8_t * bankPtr1;
        uint8_t tempByte;
        bool is_good;
        uint8_t index;
        size_t remainder;

        bankPtr0 = ExAllocatePoolWithTag( NonPagedPool, 0x400, FL2K_POOL_TAG );
        if (NULL == bankPtr0)
                goto exit;

        bankPtr1 = bankPtr0 + 0x200;

        memset(bankPtr0, 0, 0x200);
        memset(bankPtr1, 0, 0x200);

        remainder = ( size_t )bankPtr0 % 0x100;
        if (remainder > 0)
                bankPtr0 += (0x100 - remainder);

        remainder = ( size_t )bankPtr1 % 0x100;
        if (remainder > 0)
                bankPtr1 += (0x100 - remainder);

        // Bank 0
        //
        fl2000_hdmi_write_byte_simple( dev_ctx, 0xF, 0, &is_good );
        for (index = 0; index < 0xFF; index++) {
                tempByte = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        (uint8_t) index,
                        &is_good);
                *bankPtr0 = tempByte;
                bankPtr0++;
        }
        bankPtr0 -= 0xFF;

        // Bank 1
        //
        fl2000_hdmi_write_byte_simple(dev_ctx, 0xF, 1, &is_good);
        for (index = 0; index < 0xFF; index++) {
                tempByte = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        (uint8_t) index,
                        &is_good);
                *bankPtr1 = tempByte;
                bankPtr1++;
        }
        bankPtr1 -= 0xFF;

exit:
        return;
}
#endif

bool
fl2000_hdmi_check_stable(struct dev_ctx * dev_ctx)
{
        bool is_good;
        bool isStable;
        uint8_t byte_data;
        uint32_t loopCount;

        isStable = false;
        loopCount = 0;

        while (!isStable) {
                byte_data = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_SYS_STATUS,
                        &is_good);
                if (is_good) {
                        if (byte_data & HDMI_ITE_B_TX_VIDEO_STABLE) {
                                isStable = true;
                                break;
                        }
                }
                else {
                        break;
                }

                loopCount++;
                if (loopCount > 10)
                        break;

                DELAY_MS(100);
        }

        return (isStable);
}

bool
fl2000_hdmi_read_block(struct dev_ctx * dev_ctx, uint8_t block_id)
{
        int status;
        bool is_good;
        uint8_t tempBuffer[HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE];
        uint8_t round;
        uint8_t offset;
        uint8_t * target;
        uint8_t segment_id;
        uint8_t segment_offset;
        uint8_t offset_patched;
        uint8_t * target_block;

        is_good = true;
        target_block = dev_ctx->monitor_edid[block_id];

        is_good = fl2000_hdmi_read_edid_table_init(dev_ctx);
        if (!is_good)
                goto exit;

        is_good = fl2000_hdmi_clear_ddc_fifo(dev_ctx);
        if (!is_good )
                goto exit;

        is_good = fl2000_hdmi_switch_bank(dev_ctx, 0);
        if (!is_good)
                goto exit;

        segment_id = block_id / 2;
        segment_offset = (block_id % 2) * 128;

        for (round = 0; round < 4; round++) {
                offset = HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE * round;
                target = target_block + offset;

                // BUG: Patch for the first 3 bytes.
                //
                if (round == 0) {
                        if (0 == block_id) {
                                target_block[0] = 0;
                                target_block[1] = 0xFF;
                                target_block[2] = 0xFF;
                        }
                        else if (1 == block_id || 3 == block_id) {
                                offset_patched = segment_offset + offset - 3;

                                status = fl2000_hdmi_read_edid_table(
                                        dev_ctx,
                                        segment_id,
                                        offset_patched,
                                        HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE,
                                        tempBuffer);
                                if (status < 0) {
                                        is_good = false;
                                        goto exit;
                                }
                                memcpy(target, tempBuffer, 3);
                        }
                        else {
                                //
                        }
                }
                else {
                        memset(tempBuffer, 0, HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE);
                        offset_patched = segment_offset + offset - 3;

                        status = fl2000_hdmi_read_edid_table(
                                dev_ctx,
                                segment_id,
                                offset_patched,
                                6,
                                tempBuffer);
                        if (status < 0) {
                                is_good = false;
                                goto exit;
                        }

                        memcpy(target, tempBuffer, 3);
                }

                // Normal part.
                //
                memset(tempBuffer, 0, HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE);

                status = fl2000_hdmi_read_edid_table(
                        dev_ctx,
                        segment_id,
                        segment_offset + offset,
                        HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE,
                        tempBuffer);
                if (status < 0) {
                        is_good = false;
                        goto exit;
                }

                memcpy(target + 3, tempBuffer, HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE - 3);
        }

exit:
        return is_good;
}

void
fl2000_hdmi_compliance_tweak(struct dev_ctx * dev_ctx)
{
        if (dev_ctx->vr_params.width == 640 &&
            dev_ctx->vr_params.height == 480 &&
            dev_ctx->vr_params.freq == 60) {
                dev_ctx->vr_params.h_back_porch = 48;
                dev_ctx->vr_params.v_back_porch = 33;
                dev_ctx->vr_params.h_sync_reg_2 = 0x600091;
                dev_ctx->vr_params.v_sync_reg_2 = 0x2420024;
        } else if (dev_ctx->vr_params.width == 1280 &&
                   dev_ctx->vr_params.height == 720 &&
                   dev_ctx->vr_params.freq == 60) {
                dev_ctx->vr_params.v_back_porch = 20;
                dev_ctx->vr_params.v_sync_reg_2 = 0x1A5001A;
        } else {
                // No adjustment.
                //
        }
}

void
fl2000_hdmi_generate_ddc_sclk(struct dev_ctx * dev_ctx)
{
        bool is_good;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_GEN_SCLCLK,
                &is_good);
}

void
fl2000_hdmi_init(struct dev_ctx * dev_ctx, bool resolution_change)
{
        bool is_good;

        if (!dev_ctx->hdmi_chip_found)
                return;

        if (!dev_ctx->hdmi_powered_up) {
                is_good = fl2000_hdmi_power_up(dev_ctx);
                if (!is_good ) {
                        dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
                                "fl2000_hdmi_power_up failed?");
                        return;
                }
        }

        if (resolution_change) {
                // Force HDMI monitor connected.
                //
                //HDMI_ForceMonitorConnected( dev_ctx );

                fl2000_reg_bit_set(dev_ctx, REG_OFFSET_803C, 25);

                is_good = fl2000_hdmi_set_display_mode(dev_ctx);

                fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 25);

                if (!is_good) {
                        dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
                                "fl2000_hdmi_set_display_mode failed?");
                }
        }
}

#if (HDCP_ENABLE)
void
fl2000_hdmi_hdcp_reset(struct dev_ctx * dev_ctx)
{
        uint8_t data_byte;
        bool is_good;

        data_byte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_SW_RST,
                &is_good);
        data_byte = data_byte | HDMI_ITE_B_TX_HDCP_RST_HDMITX;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_SW_RST,
                data_byte,
                &is_good);

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_HDCP_DESIRE,
                0,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_LISTCTRL,
                0,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERHOST,
                &is_good);

        fl2000_hdmi_clear_ddc_fifo(dev_ctx);
        fl2000_hdmi_abort_ddc(dev_ctx);
}

bool
fl2000_hdmi_hdcp_get_bcaps(
        struct dev_ctx * dev_ctx,
        uint8_t * bcaps,
        uint16_t * bstatus)
{
        bool is_good;
        uint8_t timeOut;
        uint8_t data_byte;

        is_good = true;

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,H
                DMI_ITE_REG_TX_DDC_HEADER,
                HDMI_ITE_DDC_HDCP_ADDRESS,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQOFF,
                0x40,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQCOUNT,
                3,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_DDC_SEQ_BURSTREAD,
                &is_good);
        if (!is_good)
                goto exit;

        for(timeOut = 200; timeOut > 0; timeOut--) {
                DELAY_MS( 1 );
                data_byte = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_DDC_STATUS,
                        &is_good);
                if (!is_good)
                        goto exit;

                if(data_byte & HDMI_ITE_B_TX_DDC_DONE)
                        break;

                if (data_byte & HDMI_ITE_B_TX_DDC_ERROR) {
                        is_good = false;
                        goto exit;
                }
        }

        if (timeOut == 0) {
                is_good = false;
                goto exit;
        }

        data_byte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_BSTAT1,
                &is_good);
        if (!is_good)
                goto exit;

        *bstatus = (uint16_t)data_byte;
        *bstatus <<= 8;
        data_byte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_BSTAT,
                &is_good);

        *bstatus |= ((uint16_t) data_byte & 0xFF);
        *bcaps = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_BCAP,
                &is_good);
exit:
        return is_good;
}

bool
fl2000_hdmi_hdcp_get_bksv(struct dev_ctx * dev_ctx, uint8_t * bksv)
{
        bool is_good;
        uint8_t data_byte;
        uint8_t timeOut;

        is_good = true;

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        data_byte = HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                data_byte,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_HEADER,
                HDMI_ITE_DDC_HDCP_ADDRESS,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQOFF,
                0x00,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQCOUNT,
                5,
                &is_good);
        if (!is_good)
                goto exit;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_DDC_SEQ_BURSTREAD,
                &is_good);
        if (!is_good)
                goto exit;

        for (timeOut = 200; timeOut > 0; timeOut--) {
                DELAY_MS(1);

                data_byte = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_DDC_STATUS,
                        &is_good);
                if (!is_good)
                        goto exit;

                if (data_byte & HDMI_ITE_B_TX_DDC_DONE)
                        break;

                if (data_byte & HDMI_ITE_B_TX_DDC_ERROR) {
                        is_good = false;
                        goto exit;
                }
        }

        if (timeOut == 0) {
                is_good = false;
                goto exit;
        }

        // Seems read same address five times.
        //
        bksv[0] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_BKSV, &is_good);
        bksv[1] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_BKSV, &is_good);
        bksv[2] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_BKSV, &is_good);
        bksv[3] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_BKSV, &is_good);
        bksv[4] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_BKSV, &is_good);

exit:
        return is_good;
}

void
fl2000_hdcp_clear_auth_interrupt(struct dev_ctx * dev_ctx)
{
        uint8_t bMask;
        uint8_t data_byte;
        bool is_good;

        bMask = HDMI_ITE_B_TX_KSVLISTCHK_MASK |
                HDMI_ITE_B_TX_AUTH_DONE_MASK |
                HDMI_ITE_B_TX_AUTH_FAIL_MASK;
        data_byte = fl2000_hdmi_read_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_MASK2,
                &is_good);

        data_byte &= ~bMask;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_MASK2,
                data_byte, &is_good);

        data_byte = HDMI_ITE_B_TX_CLR_AUTH_FAIL |
                    HDMI_ITE_B_TX_CLR_AUTH_DONE |
                    HDMI_ITE_B_TX_CLR_KSVLISTCHK;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_CLR0,
                data_byte,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_INT_CLR1,
                0,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_SYS_STATUS,
                HDMI_ITE_B_TX_INTACTDONE,
                &is_good);
}

void
fl2000_hdcp_start_an_cipher(
    struct dev_ctx * dev_ctx
    )
{
        bool is_good;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_AN_GENERATE,
                HDMI_ITE_B_TX_START_CIPHER_GEN,
                &is_good);
        DELAY_MS(1);
}

void
fl2000_hdcp_stop_an_cipher(struct dev_ctx * dev_ctx)
{
        bool is_good;

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_AN_GENERATE,
                HDMI_ITE_B_TX_STOP_CIPHER_GEN,
                &is_good);
}

void
fl2000_hdcp_generate_an(struct dev_ctx * dev_ctx)
{
        uint8_t data[8];
        uint8_t i;
        bool is_good;

        fl2000_hdcp_start_an_cipher(dev_ctx);
        fl2000_hdcp_stop_an_cipher(dev_ctx);

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        for (i = 0; i < 8; i++){
                data[i] = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_AN_GEN,
                        &is_good);
        }

        for(i = 0; i < 8; i++) {
                fl2000_hdmi_write_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_AN + i,
                        data[i],
                        &is_good);
        }
}

void
fl2000_hdcp_auth_fire(
    struct dev_ctx * dev_ctx
    )
{
        uint8_t data_byte;
        bool is_good;

        data_byte = HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHDCP;
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                data_byte,
                &is_good);

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_AUTHFIRE,
                1,
                &is_good);
}

bool
fl2000_hdcp_get_ksv_list(
        struct dev_ctx * dev_ctx,
        uint8_t * ksv_list,
        uint8_t downstream)
{
        uint8_t timeOut = 100 ;
        uint8_t data_byte;
        bool is_good;

        is_good = true;

        if(downstream == 0) {
                is_good = true;
                goto exit;
        }

        if (ksv_list == NULL) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_MASTER_CTRL,
                HDMI_ITE_B_TX_MASTERHOST,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_HEADER,
                0x74,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQOFF,
                0x43,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_REQCOUNT,
                downstream * 5,
                &is_good);
        fl2000_hdmi_write_byte_simple(
                dev_ctx,
                HDMI_ITE_REG_TX_DDC_CMD,
                HDMI_ITE_CMD_DDC_SEQ_BURSTREAD,
                &is_good);

        for (timeOut = 200; timeOut > 0; timeOut--) {
                data_byte = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_DDC_STATUS,
                        &is_good);
                if (data_byte & HDMI_ITE_B_TX_DDC_DONE)
                        break ;

                if (data_byte & HDMI_ITE_B_TX_DDC_ERROR) {
                        is_good = false;
                        goto exit;
                }
                DELAY_MS(5);
        }

        if (timeOut == 0) {
                is_good = false;
                goto exit;
        }

        for (timeOut = 0; timeOut < downstream * 5; timeOut++) {
                ksv_list[timeOut] = fl2000_hdmi_read_byte_simple(
                        dev_ctx,
                        HDMI_ITE_REG_TX_DDC_READFIFO,
                        &is_good);
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_hdcp_get_vr(struct dev_ctx * dev_ctx,  uint8_t * vr)
{
        uint8_t timeOut;
        uint8_t data_byte;
        bool is_good;

        is_good = true;

        if(vr == NULL) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL, HDMI_ITE_B_TX_MASTERHOST, &is_good);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_HEADER, 0x74, &is_good);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_REQOFF, 0x20, &is_good);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_REQCOUNT, 20, &is_good);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_CMD, HDMI_ITE_CMD_DDC_SEQ_BURSTREAD, &is_good);

        for (timeOut = 200; timeOut > 0; timeOut--) {
                data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_STATUS, &is_good);
                if (data_byte & HDMI_ITE_B_TX_DDC_DONE)
                        break;

                if (data_byte & HDMI_ITE_B_TX_DDC_ERROR) {
                        is_good = false;
                        goto exit;
                }

                DELAY_MS(5);
        }

        if (timeOut == 0) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        for (timeOut = 0; timeOut < 5; timeOut++) {
                fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_SEL , timeOut, &is_good );
                vr[timeOut*4]     = (uint32_t) fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE1, &is_good);
                vr[timeOut*4 + 1] = (uint32_t) fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE2, &is_good);
                vr[timeOut*4 + 2] = (uint32_t) fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE3, &is_good);
                vr[timeOut*4 + 3] = (uint32_t) fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE4, &is_good);
        }

exit:
        return is_good;
}

bool
fl2000_hdmi_hdcp_get_m0(struct dev_ctx * dev_ctx, uint8_t * m0)
{
        bool is_good;

        is_good = true;

        if (!m0) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_SHA_SEL, 5, &is_good );
        m0[0] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE1, &is_good);
        m0[1] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE2, &is_good);
        m0[2] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE3, &is_good);
        m0[3] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_RD_BYTE4, &is_good);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_SEL, 0, &is_good);
        m0[4] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AKSV_RD_BYTE5, &is_good);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_SEL,1, &is_good);
        m0[5] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AKSV_RD_BYTE5, &is_good);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_SEL,2, &is_good);
        m0[6] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AKSV_RD_BYTE5, &is_good);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SHA_SEL,3, &is_good);
        m0[7] = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AKSV_RD_BYTE5, &is_good);

exit:
        return is_good;
}

void
HDMI_HDCP_SHA_Transform(
    struct dev_ctx * dev_ctx,
    uint32_t *h
    )
{
    int t;
    uint32_t tmp;

    h[ 0 ] = 0x67452301;
    h[ 1 ] = 0xefcdab89;
    h[ 2 ] = 0x98badcfe;
    h[ 3 ] = 0x10325476;
    h[ 4 ] = 0xc3d2e1f0;

    for ( t=0; t < 20; t++ )
    {
                if( t >= 16 )
                {
                tmp = dev_ctx->Hdmi.W[ ( t - 3 ) % HDMI_HDCP_WCOUNT ] ^ dev_ctx->Hdmi.W[(t - 8)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[ ( t - 14 ) % HDMI_HDCP_WCOUNT ] ^ dev_ctx->Hdmi.W[ ( t - 16 ) % HDMI_HDCP_WCOUNT ];
                dev_ctx->Hdmi.W[ ( t ) % HDMI_HDCP_WCOUNT ] = UTILITY_ROLLING_LEFT( tmp, 1 );
                }

        tmp = UTILITY_ROLLING_LEFT( h[ 0 ], 5 ) + ( ( h[ 1 ] & h[ 2 ] ) | ( h[ 3 ] & ~h[ 1 ] ) )+ h[4] + dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT] + 0x5a827999;

        h[4]=h[3];
        h[3]=h[2];
        h[2]=UTILITY_ROLLING_LEFT(h[1],30);
        h[1]=h[0];
        h[0]=tmp;

    }

    for (t=20; t < 40; t++)
    {
        tmp=dev_ctx->Hdmi.W[(t - 3)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 8)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 14)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 16)% HDMI_HDCP_WCOUNT];
        dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT]=UTILITY_ROLLING_LEFT(tmp,1);
        tmp=UTILITY_ROLLING_LEFT(h[0],5)+ (h[1] ^ h[2] ^ h[3])+ h[4] + dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT] + 0x6ed9eba1;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=UTILITY_ROLLING_LEFT(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }

    for (t=40; t < 60; t++)
    {
        tmp=dev_ctx->Hdmi.W[(t - 3)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 8)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 14)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 16)% HDMI_HDCP_WCOUNT];
        dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT]=UTILITY_ROLLING_LEFT(tmp,1);
        tmp=UTILITY_ROLLING_LEFT(h[0],5)+ ((h[1] & h[2])| (h[1] & h[3])| (h[2] & h[3]))+ h[4] + dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT] + 0x8f1bbcdc;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=UTILITY_ROLLING_LEFT(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }

    for (t=60; t < 80; t++)
    {
        tmp=dev_ctx->Hdmi.W[(t - 3)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 8)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 14)% HDMI_HDCP_WCOUNT] ^ dev_ctx->Hdmi.W[(t - 16)% HDMI_HDCP_WCOUNT];
        dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT]=UTILITY_ROLLING_LEFT(tmp,1);
        tmp=UTILITY_ROLLING_LEFT(h[0],5)+ (h[1] ^ h[2] ^ h[3])+ h[4] + dev_ctx->Hdmi.W[(t)% HDMI_HDCP_WCOUNT] + 0xca62c1d6;
        h[4]=h[3];
        h[3]=h[2];
        h[2]=UTILITY_ROLLING_LEFT(h[1],30);
        h[1]=h[0];
        h[0]=tmp;
    }

    h[0] +=0x67452301;
    h[1] +=0xefcdab89;
    h[2] +=0x98badcfe;
    h[3] +=0x10325476;
    h[4] +=0xc3d2e1f0;
}

void
HDMI_HDCP_SHA_Simple(
    struct dev_ctx * dev_ctx,
    void *p,
    WORD len,
    uint8_t * output
    )
{
    // SHA_State s;
    WORD i,t;
    uint32_t c;
    uint8_t * pBuff=p;

    for(i=0;i < len;i++)
    {
        t=i/4;
        if(i%4==0)
        {
            dev_ctx->Hdmi.W[t]=0;
        }
        c=pBuff[i];
        c <<=(3-(i%4))*8;
        dev_ctx->Hdmi.W[t] |=c;
    }

    t=i/4;
    if(i%4==0)
    {
        dev_ctx->Hdmi.W[t]=0;
    }
    //c=0x80 << ((3-i%4)*24);
    c=0x80;
    c <<=((3-i%4)*8);
    dev_ctx->Hdmi.W[t]|=c;t++;
    for(; t < 15;t++)
    {
        dev_ctx->Hdmi.W[t]=0;
    }
    dev_ctx->Hdmi.W[15]=len*8;

    HDMI_HDCP_SHA_Transform( dev_ctx, dev_ctx->Hdmi.VH );

    for(i=0;i < 5;i++)
    {
        output[i*4+3]=( uint8_t )((dev_ctx->Hdmi.VH[i]>>24)&0xFF);
        output[i*4+2]=( uint8_t )((dev_ctx->Hdmi.VH[i]>>16)&0xFF);
        output[i*4+1]=( uint8_t )((dev_ctx->Hdmi.VH[i]>>8)&0xFF);
        output[i*4+0]=( uint8_t )(dev_ctx->Hdmi.VH[i]&0xFF);
    }
}

BOOLEAN
HDMI_HDCP_CheckSHA(
    struct dev_ctx * dev_ctx,
    uint8_t m0[],
    uint16_t BStatus,
    uint8_t ksv_list[],
    int downstream,
    uint8_t Vr[]
    )
{
    WORD i,n;

    for( i = 0 ; i < downstream*5 ; i++ )
    {
        dev_ctx->Hdmi.HdcpSHABuffer[i] = ksv_list[i] ;
    }

    dev_ctx->Hdmi.HdcpSHABuffer[i++] = BStatus & 0xFF;
    dev_ctx->Hdmi.HdcpSHABuffer[i++] = (BStatus>>8) & 0xFF;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        dev_ctx->Hdmi.HdcpSHABuffer[i] = m0[n] ;
    }
    n = i;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        dev_ctx->Hdmi.HdcpSHABuffer[i] = 0 ;
    }

    HDMI_HDCP_SHA_Simple( dev_ctx, dev_ctx->Hdmi.HdcpSHABuffer, n, dev_ctx->Hdmi.V );
    for(i = 0 ; i < 20 ; i++ )
    {
        if(dev_ctx->Hdmi.V[i] != Vr[i])
        {
            return false ;
        }
    }
    return true ;
}

void
HDMI_HDCP_ResumeRepeaterAuthenticate(
    struct dev_ctx * dev_ctx
    )
{
    bool is_good;

    fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_LISTCTRL, HDMI_ITE_B_TX_LISTDONE, &is_good );
    fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL, HDMI_ITE_B_TX_MASTERHDCP, &is_good );
}

void
HDMI_HDCP_CancelRepeaterAuthenticate(
    struct dev_ctx * dev_ctx
    )
{
    bool is_good;
    uint8_t data_byte;

    data_byte = HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST;
    fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL, data_byte, &is_good );
    fl2000_hdmi_abort_ddc( dev_ctx );

    data_byte = HDMI_ITE_B_TX_LISTFAIL | HDMI_ITE_B_TX_LISTDONE;
    fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_LISTCTRL, data_byte, &is_good );

    fl2000_hdcp_clear_auth_interrupt( dev_ctx );
}

bool
HDMI_HDCP_AuthenticateRepeater(
    struct dev_ctx * dev_ctx
    )
{
    bool is_good;
    bool is_good;
    uint8_t data_byte;
    uint8_t uc ,ii;
    uint8_t downstream;
    uint8_t BCaps;
    uint16_t BStatus;
    uint16_t timeOut;

    is_good = fl2000_hdmi_hdcp_get_bcaps( dev_ctx, &BCaps, &BStatus );
    if ( ! is_good )
    {
        goto exit;
    }

    DELAY_MS( 2 );

    data_byte = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
    data_byte &= ( HDMI_ITE_B_TX_INT_HPD_PLUG | HDMI_ITE_B_TX_INT_RX_SENSE );
    if( ! data_byte )
    {
        goto exit;
    }

    fl2000_hdcp_auth_fire( dev_ctx );

    for( ii=0; ii < 55; ii++ )
    {
        data_byte = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
        if( ( HDMI_ITE_B_TX_INT_HPD_PLUG | HDMI_ITE_B_TX_INT_RX_SENSE ) & data_byte )
        {
            goto exit;
        }

        DELAY_MS( 10 );
    }

    for( timeOut = 10 ; timeOut > 0 ; timeOut-- )
    {
        data_byte = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
        if( ( HDMI_ITE_B_TX_INT_HPD_PLUG | HDMI_ITE_B_TX_INT_RX_SENSE ) & data_byte )
        {
            goto exit;
        }

        uc = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
        if( uc & HDMI_ITE_B_TX_INT_DDC_BUS_HANG )
        {
            goto exit;
        }

        uc = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT2, &is_good );

        if( uc & HDMI_ITE_B_TX_INT_AUTH_FAIL )
        {
            goto exit;
        }

        if( uc & HDMI_ITE_B_TX_INT_KSVLIST_CHK )
        {
            fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_CLR0, HDMI_ITE_B_TX_CLR_KSVLISTCHK, &is_good );
            fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_CLR1, 0, &is_good );
            fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_SYS_STATUS, HDMI_ITE_B_TX_INTACTDONE, &is_good );
            fl2000_hdmi_write_byte_simple( dev_ctx, HDMI_ITE_REG_TX_SYS_STATUS, 0, &is_good );

            break ;
        }

        DELAY_MS( 5 );
    }

    if( timeOut == 0 )
    {
        goto exit;
    }

    for( timeOut = 500; timeOut > 0; timeOut -- )
    {
        data_byte = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
        if(( HDMI_ITE_B_TX_INT_HPD_PLUG | HDMI_ITE_B_TX_INT_RX_SENSE )& data_byte )
        {
            goto exit;
        }

        is_good = fl2000_hdmi_hdcp_get_bcaps( dev_ctx, &BCaps,&BStatus );
        if( is_good == false )
        {
            goto exit;
        }

        if( BCaps & HDMI_ITE_B_TX_CAP_KSV_FIFO_RDY )
        {
             break ;
        }

        DELAY_MS( 5 );
    }

    if( timeOut == 0 )
    {
        goto exit;
    }

    fl2000_hdmi_clear_ddc_fifo( dev_ctx );
    fl2000_hdmi_generate_ddc_sclk( dev_ctx );

    downstream = ( BStatus & HDMI_ITE_M_TX_DOWNSTREAM_COUNT );

    if( downstream > 6 || BStatus & ( HDMI_ITE_B_TX_MAX_CASCADE_EXCEEDED | HDMI_ITE_B_TX_DOWNSTREAM_OVER ) )
    {
        goto exit;
    }

    is_good = fl2000_hdcp_get_ksv_list( dev_ctx, dev_ctx->Hdmi.KSVList, downstream );
    if( ! is_good )
    {
        goto exit;
    }

    is_good = fl2000_hdmi_hdcp_get_vr( dev_ctx, dev_ctx->Hdmi.VR );
    if( ! is_good )
    {
        goto exit;
    }

    is_good = fl2000_hdmi_hdcp_get_m0( dev_ctx, dev_ctx->Hdmi.M0 );
    if( ! is_good )
    {
        goto exit;
    }

    is_good = HDMI_HDCP_CheckSHA( dev_ctx,
                                  dev_ctx->Hdmi.M0,
                                  BStatus,
                                  dev_ctx->Hdmi.KSVList,
                                  downstream,
                                  dev_ctx->Hdmi.VR );
    if( ! is_good )
    {
        goto exit;
    }

    data_byte = fl2000_hdmi_read_byte_simple( dev_ctx, HDMI_ITE_REG_TX_INT_STAT1, &is_good );
    if( ( HDMI_ITE_B_TX_INT_HPD_PLUG | HDMI_ITE_B_TX_INT_RX_SENSE ) & data_byte )
    {
        goto exit;
    }

    HDMI_HDCP_ResumeRepeaterAuthenticate( dev_ctx );

    return true;

exit:

    HDMI_HDCP_CancelRepeaterAuthenticate( dev_ctx );

    return false;
}

bool
fl2000_hdmi_hdcp_authenticate(struct dev_ctx * dev_ctx)
{
        bool is_good;
        uint8_t data_byte;
        uint8_t bCaps;
        uint16_t bSts;
        uint16_t timeOut;
        uint8_t revoked;
        uint8_t BKSV[5];

        revoked = false;
        is_good = false;
        bCaps = 0;

        data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SYS_STATUS, &is_good);
        data_byte &= HDMI_ITE_B_TX_VIDEO_STABLE;
        if(!data_byte) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_hdcp_reset(dev_ctx);

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        for (timeOut = 0; timeOut < 80; timeOut++) {
                DELAY_MS(15);

                is_good = fl2000_hdmi_hdcp_get_bcaps(dev_ctx, &bCaps, &bSts);
                if(!is_good)
                        goto exit;

                data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_HDMI_MODE, &is_good);
                data_byte &= HDMI_ITE_B_TX_MODE_HDMI;
                if (data_byte) {
                        data_byte &= HDMI_ITE_B_TX_CAP_HDMI_MODE;
                        if( data_byte )
                                break;
                }
                else {
                        data_byte &= HDMI_ITE_B_TX_CAP_HDMI_MODE;
                        if(!data_byte)
                                break;
                }
        }

        fl2000_hdmi_hdcp_get_bksv(dev_ctx, BKSV);

        data_byte = 0;
        for (timeOut = 0 ; timeOut < 5; timeOut++) {
                data_byte += UTILITY_CountBitSettedInOneByte(BKSV[timeOut]);
        }

        if( data_byte != 20 ) {
                is_good = false;
                goto exit;
        }

        fl2000_hdmi_switch_bank(dev_ctx, 0);

        data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST, &is_good);
        data_byte &= ~( HDMI_ITE_B_TX_HDCP_RST_HDMITX);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST, data_byte, &is_good);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_HDCP_DESIRE, HDMI_ITE_B_TX_CPDESIRE, &is_good);

        fl2000_hdcp_clear_auth_interrupt(dev_ctx);

        fl2000_hdcp_generate_an(dev_ctx);

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_LISTCTRL, 0, &is_good);

        fl2000_hdmi_clear_ddc_fifo(dev_ctx);

        data_byte = bCaps & HDMI_ITE_B_TX_CAP_HDMI_REPEATER;
        if (!data_byte) {
                fl2000_hdcp_auth_fire(dev_ctx);

                // wait for status;
                //
                for (timeOut = 250; timeOut > 0; timeOut--) {
                        DELAY_MS(5);

                        data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_AUTH_STAT, &is_good);
                        if (data_byte & HDMI_ITE_B_TX_AUTH_DONE)
                                break;

                        data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_INT_STAT2, &is_good);
                        data_byte &= HDMI_ITE_B_TX_INT_AUTH_FAIL;
                        if (!data_byte) {
                                fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_INT_CLR0, HDMI_ITE_B_TX_CLR_AUTH_FAIL, &is_good);
                                fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_INT_CLR1, 0, &is_good);
                                fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SYS_STATUS, HDMI_ITE_B_TX_INTACTDONE, &is_good);
                                fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SYS_STATUS, 0, &is_good);
                                is_good = false;
                                goto exit;
                        }
                }

                if (timeOut == 0) {
                        is_good = false;
                        goto exit;
                }

        return is_good;
        }

exit:
    is_good = HDMI_HDCP_AuthenticateRepeater(dev_ctx);
    return is_good;
}

void
fl2000_hdmi_hdcp_reset_auth(struct dev_ctx * dev_ctx)
{
        uint8_t data_byte;
        bool is_good;

        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_LISTCTRL, 0, &is_good);
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_HDCP_DESIRE, 0, &is_good);

        data_byte = fl2000_hdmi_read_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST, &is_good);
        data_byte &= ~HDMI_ITE_B_TX_HDCP_RST_HDMITX;
        data_byte |= HDMI_ITE_B_TX_HDCP_RST_HDMITX;
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_SW_RST, data_byte, &is_good);

        data_byte = HDMI_ITE_B_TX_MASTERDDC | HDMI_ITE_B_TX_MASTERHOST;
        fl2000_hdmi_write_byte_simple(dev_ctx, HDMI_ITE_REG_TX_DDC_MASTER_CTRL, data_byte, &is_good);

        fl2000_hdcp_clear_auth_interrupt(dev_ctx);
        fl2000_hdmi_abort_ddc(dev_ctx);
}

void
fl2000_hdcp_enable(struct dev_ctx * dev_ctx, bool enable)
{
        bool is_good;

        if (enable) {
                is_good = fl2000_hdmi_hdcp_authenticate(dev_ctx);
                if(!is_good)
                        fl2000_hdmi_hdcp_reset_auth(dev_ctx);
        }
        else {
                fl2000_hdmi_hdcp_reset_auth(dev_ctx);
        }
}
#endif /* HDCP_ENABLE */
