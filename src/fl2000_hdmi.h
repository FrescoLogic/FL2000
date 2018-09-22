// fl2000_hdmi.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion file.
//

#ifndef _FL2000_HDMI_H_
#define _FL2000_HDMI_H_

#define HDMI_ITE_AUDIO_SAMPLE_FREQUENCY         ( HDMI_ITE_AUDFS_44p1KHZ )
#define HDMI_ITE_AUDIO_SAMPLE_FREQUENCY_HZ      ( 44100L )
#define HDMI_ITE_AUDIO_NUMBER_OF_CHANNEL        ( 2 )
#define HDMI_ITE_AUDIO_TYPE                     ( HDMI_ITE_T_AUDIO_LPCM )
#define HDMI_ITE_AUDIO_NUMBER_OF_BITS           ( 16 )

#define HDMI_ITE_VENDER_ID                      ( 0x4954 )
#define HDMI_ITE_DEVICE_ID                      ( 0x612 )

#define HDMI_ITE_EACH_TIME_READ_EDID_MAX_SIZE   ( 0x20 )

#define HDMI_ITE_DDC_HDCP_ADDRESS               ( 0x74 )

#define HDMI_ITE_SEGMENT_ID_EDID_TABLE          ( 0 )

#define HDMI_ITE_PCLK_LOW                       ( 0 )
#define HDMI_ITE_PCLK_MEDIUM                    ( 1 )
#define HDMI_ITE_PCLK_HIGH                      ( 2 )

#define HDMI_ITE_AUDFS_22p05KHZ                 ( 4 )
#define HDMI_ITE_AUDFS_44p1KHZ                  ( 0 )
#define HDMI_ITE_AUDFS_88p2KHZ                  ( 8 )
#define HDMI_ITE_AUDFS_176p4KHZ                 ( 12 )
#define HDMI_ITE_AUDFS_24KHZ                    ( 6 )
#define HDMI_ITE_AUDFS_48KHZ                    ( 2 )
#define HDMI_ITE_AUDFS_96KHZ                    ( 10 )
#define HDMI_ITE_AUDFS_192KHZ                   ( 14 )
#define HDMI_ITE_AUDFS_768KHZ                   ( 9 )
#define HDMI_ITE_AUDFS_32KHZ                    ( 3 )
#define HDMI_ITE_AUDFS_OTHER                    ( 1 )

#if( HDMI_ITE_AUDIO_NUMBER_OF_BITS == 16 )
#define HDMI_ITE_CHTSTS_SWCODE                  ( 0x02 )
#elif(HDMI_ITE_AUDIO_NUMBER_OF_BITS==18)
#define HDMI_ITE_CHTSTS_SWCODE                  ( 0x04 )
#elif(HDMI_ITE_AUDIO_NUMBER_OF_BITS==20)
#define HDMI_ITE_CHTSTS_SWCODE                  ( 0x03 )
#else
#define HDMI_ITE_CHTSTS_SWCODE                  ( 0x0B )
#endif

#define HDMI_ITE_AVI_INFOFRAME_LEN              ( 13 )
#define HDMI_ITE_AVI_INFOFRAME_TYPE             ( 0x82 )
#define HDMI_ITE_AVI_INFOFRAME_VER              ( 0x02 )

#define HDMI_ITE_AUDIO_INFOFRAME_LEN            ( 10 )
#define HDMI_ITE_AUDIO_INFOFRAME_TYPE           ( 0x84 )
#define HDMI_ITE_AUDIO_INFOFRAME_VER            (  0x01 )

// HDMI ITE Register Definition.
//

// 0x04
//
#define HDMI_ITE_REG_TX_SW_RST                  ( 0x04 )
#define HDMI_ITE_B_TX_ENTEST                    ( 1 << 7 )
#define HDMI_ITE_B_TX_REF_RST_HDMITX            ( 1 << 5 )
#define HDMI_ITE_B_TX_AREF_RST                  ( 1 << 4 )
#define HDMI_ITE_B_HDMI_VID_RST                 ( 1 << 3 )
#define HDMI_ITE_B_HDMI_AUD_RST                 ( 1 << 2 )
#define HDMI_ITE_B_TX_HDMI_RST                  ( 1 << 1 )
#define HDMI_ITE_B_TX_HDCP_RST_HDMITX           ( 1 << 0 )

// 0x05
//
#define HDMI_ITE_REG_TX_INT_CTRL                ( 0x05 )
#define HDMI_ITE_B_TX_INTPOL_ACTL               ( 0 )
#define HDMI_ITE_B_TX_INTPOL_ACTH               ( 1 << 7 )
#define HDMI_ITE_B_TX_INT_PUSHPULL              ( 0 )
#define HDMI_ITE_B_TX_INT_OPENDRAIN             ( 1 << 6 )

// 0x06
//
#define HDMI_ITE_REG_TX_INT_STAT1               ( 0x06 )
#define HDMI_ITE_B_TX_INT_AUD_OVERFLOW          ( 1 << 7 )
#define HDMI_ITE_B_TX_INT_ROMACQ_NOACK          ( 1 << 6 )
#define HDMI_ITE_B_TX_INT_RDDC_NOACK            ( 1 << 5 )
#define HDMI_ITE_B_TX_INT_DDCFIFO_ERR           ( 1 << 4 )
#define HDMI_ITE_B_TX_INT_ROMACQ_BUS_HANG       ( 1 << 3 )
#define HDMI_ITE_B_TX_INT_DDC_BUS_HANG          ( 1 << 2 )
#define HDMI_ITE_B_TX_INT_RX_SENSE              ( 1 << 1 )
#define HDMI_ITE_B_TX_INT_HPD_PLUG              ( 1 << 0 )

// 0x07
//
#define HDMI_ITE_REG_TX_INT_STAT2               ( 0x07 )
#define HDMI_ITE_B_TX_INT_HDCP_SYNC_DET_FAIL    ( 1 << 7 )
#define HDMI_ITE_B_TX_INT_VID_UNSTABLE          ( 1 << 6 )
#define HDMI_ITE_B_TX_INT_PKTACP                ( 1 << 5 )
#define HDMI_ITE_B_TX_INT_PKTNULL               ( 1 << 4 )
#define HDMI_ITE_B_TX_INT_PKTGENERAL            ( 1 << 3 )
#define HDMI_ITE_B_TX_INT_KSVLIST_CHK           ( 1 << 2 )
#define HDMI_ITE_B_TX_INT_AUTH_DONE             ( 1 << 1 )
#define HDMI_ITE_B_TX_INT_AUTH_FAIL             ( 1 << 0 )

// 0x08
//
#define HDMI_ITE_REG_TX_INT_STAT3               ( 0x08 )
#define HDMI_ITE_B_TX_INT_AUD_CTS               ( 1 << 6 )
#define HDMI_ITE_B_TX_INT_VSYNC                 ( 1 << 5 )
#define HDMI_ITE_B_TX_INT_VIDSTABLE             ( 1 << 4 )
#define HDMI_ITE_B_TX_INT_PKTMPG                ( 1 << 3 )
#define HDMI_ITE_B_TX_INT_PKTSPD                ( 1 << 2 )
#define HDMI_ITE_B_TX_INT_PKTAUD                ( 1 << 1 )
#define HDMI_ITE_B_TX_INT_PKTAVI                ( 1 << 0 )

// 0x09
//
#define HDMI_ITE_REG_TX_INT_MASK1               ( 0x09 )
#define HDMI_ITE_B_TX_AUDIO_OVFLW_MASK          ( 1 << 7 )
#define HDMI_ITE_B_TX_DDC_NOACK_MASK            ( 1 << 5 )
#define HDMI_ITE_B_TX_DDC_FIFO_ERR_MASK         ( 1 << 4 )
#define HDMI_ITE_B_TX_DDC_BUS_HANG_MASK         ( 1 << 2 )
#define HDMI_ITE_B_TX_RXSEN_MASK                ( 1 << 1 )
#define HDMI_ITE_B_TX_HPD_MASK                  ( 1 << 0 )

// 0x0A
//
#define HDMI_ITE_REG_TX_INT_MASK2               ( 0x0A )
#define HDMI_ITE_B_TX_PKT_AVI_MASK              ( 1 << 7 )
#define HDMI_ITE_B_TX_PKT_VID_UNSTABLE_MASK     ( 1 << 6 )
#define HDMI_ITE_B_TX_PKT_ACP_MASK              ( 1 << 5 )
#define HDMI_ITE_B_TX_PKT_NULL_MASK             ( 1 << 4 )
#define HDMI_ITE_B_TX_PKT_GEN_MASK              ( 1 << 3 )
#define HDMI_ITE_B_TX_KSVLISTCHK_MASK           ( 1 << 2 )
#define HDMI_ITE_B_TX_AUTH_DONE_MASK            ( 1 << 1 )
#define HDMI_ITE_B_TX_AUTH_FAIL_MASK            ( 1 << 0 )

// 0x0B
//
#define HDMI_ITE_REG_TX_INT_MASK3               ( 0x0B )
#define HDMI_ITE_B_TX_HDCP_SYNC_DET_FAIL_MASK   ( 1 << 6 )
#define HDMI_ITE_B_TX_AUDCTS_MASK               ( 1 << 5 )
#define HDMI_ITE_B_TX_VSYNC_MASK                ( 1 << 4 )
#define HDMI_ITE_B_TX_VIDSTABLE_MASK            ( 1 << 3 )
#define HDMI_ITE_B_TX_PKT_MPG_MASK              ( 1 << 2 )
#define HDMI_ITE_B_TX_PKT_SPD_MASK              ( 1 << 1 )
#define HDMI_ITE_B_TX_PKT_AUD_MASK              ( 1 << 0 )

// 0x0C
//
#define HDMI_ITE_REG_TX_INT_CLR0                ( 0x0C )
#define HDMI_ITE_B_TX_CLR_PKTACP                ( 1 << 7 )
#define HDMI_ITE_B_TX_CLR_PKTNULL               ( 1 << 6 )
#define HDMI_ITE_B_TX_CLR_PKTGENERAL            ( 1 << 5 )
#define HDMI_ITE_B_TX_CLR_KSVLISTCHK            ( 1 << 4 )
#define HDMI_ITE_B_TX_CLR_AUTH_DONE             ( 1 << 3 )
#define HDMI_ITE_B_TX_CLR_AUTH_FAIL             ( 1 << 2 )
#define HDMI_ITE_B_TX_CLR_RXSENSE               ( 1 << 1 )
#define HDMI_ITE_B_TX_CLR_HPD                   ( 1 << 0 )

// 0x0D
//
#define HDMI_ITE_REG_TX_INT_CLR1                ( 0x0D )
#define HDMI_ITE_B_TX_CLR_VSYNC                 ( 1 << 7 )
#define HDMI_ITE_B_TX_CLR_VIDSTABLE             ( 1 << 6 )
#define HDMI_ITE_B_TX_CLR_PKTMPG                ( 1 << 5 )
#define HDMI_ITE_B_TX_CLR_PKTSPD                ( 1 << 4 )
#define HDMI_ITE_B_TX_CLR_PKTAUD                ( 1 << 3 )
#define HDMI_ITE_B_TX_CLR_PKTAVI                ( 1 << 2 )
#define HDMI_ITE_B_TX_CLR_HDCP_SYNC_DET_FAIL    ( 1 << 1 )
#define HDMI_ITE_B_TX_CLR_VID_UNSTABLE          ( 1 << 0 )

// 0x0E
//
#define HDMI_ITE_REG_TX_SYS_STATUS              ( 0x0E )
#define HDMI_ITE_B_TX_INT_ACTIVE                ( 1 << 7 )
#define HDMI_ITE_B_TX_HPDETECT                  ( 1 << 6 )
#define HDMI_ITE_B_TX_RXSENDETECT               ( 1 << 5 )
#define HDMI_ITE_B_TX_VIDEO_STABLE              ( 1 << 4 )
#define HDMI_ITE_B_TX_CLR_AUD_CTS               ( 1 << 1 )
#define HDMI_ITE_B_TX_INTACTDONE                ( 1 << 0 )

// 0x0F
//
#define HDMI_ITE_REG_TX_BANK_CTRL               ( 0x0F )
#define HDMI_ITE_B_TX_BANK0                     ( 0 )
#define HDMI_ITE_B_TX_BANK1                     ( 1 )

// 0x10
//
#define HDMI_ITE_REG_TX_DDC_MASTER_CTRL         ( 0x10 )
#define HDMI_ITE_B_TX_MASTERROM                 ( 1 << 1 )
#define HDMI_ITE_B_TX_MASTERDDC                 ( 0 << 1 )
#define HDMI_ITE_B_TX_MASTERHOST                ( 1 << 0 )
#define HDMI_ITE_B_TX_MASTERHDCP                ( 0 << 0 )

// 0x11
//
#define HDMI_ITE_REG_TX_DDC_HEADER              ( 0x11 )

// 0x12
//
#define HDMI_ITE_REG_TX_DDC_REQOFF              ( 0x12 )

// 0x13
//
#define HDMI_ITE_REG_TX_DDC_REQCOUNT            ( 0x13 )

// 0x14
//
#define HDMI_ITE_REG_TX_DDC_EDIDSEG             ( 0x14 )

// 0x15
//
#define HDMI_ITE_REG_TX_DDC_CMD                 ( 0x15 )
#define HDMI_ITE_CMD_DDC_SEQ_BURSTREAD          ( 0 )
#define HDMI_ITE_CMD_LINK_CHKREAD               ( 2 )
#define HDMI_ITE_CMD_EDID_READ                  ( 3 )
#define HDMI_ITE_CMD_FIFO_CLR                   ( 9 )
#define HDMI_ITE_CMD_GEN_SCLCLK                 ( 0xA )
#define HDMI_ITE_CMD_DDC_ABORT                  ( 0xF )

// 0x16
//
#define HDMI_ITE_REG_TX_DDC_STATUS              ( 0x16 )
#define HDMI_ITE_B_TX_DDC_DONE                  ( 1 << 7 )
#define HDMI_ITE_B_TX_DDC_ACT                   ( 1 << 6 )
#define HDMI_ITE_B_TX_DDC_NOACK                 ( 1 << 5 )
#define HDMI_ITE_B_TX_DDC_WAITBUS               ( 1 << 4 )
#define HDMI_ITE_B_TX_DDC_ARBILOSE              ( 1 << 3 )
#define HDMI_ITE_B_TX_DDC_ERROR                 ( HDMI_ITE_B_TX_DDC_NOACK | HDMI_ITE_B_TX_DDC_WAITBUS | HDMI_ITE_B_TX_DDC_ARBILOSE )
#define HDMI_ITE_B_TX_DDC_FIFOFULL              ( 1 << 2 )
#define HDMI_ITE_B_TX_DDC_FIFOEMPTY             ( 1 << 1 )

// 0x17
//
#define HDMI_ITE_REG_TX_DDC_READFIFO            ( 0x17 )

// 0x18
//
#define HDMI_ITE_REG_TX_ROM_STARTADDR           ( 0x18 )

// 0x19
//
#define HDMI_ITE_REG_TX_HDCP_HEADER             ( 0x19 )

// 0x1A
//
#define HDMI_ITE_REG_TX_ROM_HEADER              ( 0x1A )

// 0x1B
//
#define HDMI_ITE_REG_TX_BUSHOLD_T               ( 0x1B )

// 0x1C
//
#define HDMI_ITE_REG_TX_ROM_STAT                ( 0x1C )
#define HDMI_ITE_B_TX_ROM_DONE                  ( 1 << 7 )
#define HDMI_ITE_B_TX_ROM_ACTIVE	            ( 1 << 6 )
#define HDMI_ITE_B_TX_ROM_NOACK	                ( 1 << 5 )
#define HDMI_ITE_B_TX_ROM_WAITBUS	            ( 1 << 4 )
#define HDMI_ITE_B_TX_ROM_ARBILOSE	            ( 1 << 3 )
#define HDMI_ITE_B_TX_ROM_BUSHANG	            ( 1 << 2 )

// 0x1F
//
#define HDMI_ITE_REG_TX_AN_GENERATE             ( 0x1F )
#define HDMI_ITE_B_TX_START_CIPHER_GEN          ( 1 )
#define HDMI_ITE_B_TX_STOP_CIPHER_GEN           ( 0 )

// 0x20
//
#define HDMI_ITE_REG_TX_HDCP_DESIRE             ( 0x20 )
#define HDMI_ITE_B_TX_CPDESIRE                  ( 1 << 0 )

// 0x21
//
#define HDMI_ITE_REG_TX_AUTHFIRE                ( 0x21 )

// 0x22
//
#define HDMI_ITE_REG_TX_LISTCTRL                ( 0x22 )
#define HDMI_ITE_B_TX_LISTFAIL                  ( 1 << 1 )
#define HDMI_ITE_B_TX_LISTDONE                  ( 1 << 0 )

// 0x28
//
#define HDMI_ITE_REG_TX_AN                      ( 0x28 )

// 0x30
//
#define HDMI_ITE_REG_TX_AN_GEN                  ( 0x30 )

// 0x38
//
#define HDMI_ITE_REG_TX_ARI                     ( 0x38 )

// 0x38
//
#define HDMI_ITE_REG_TX_ARI0                    ( 0x38 )

// 0x39
//
#define HDMI_ITE_REG_TX_ARI1                    ( 0x39 )

// 0x3A
//
#define HDMI_ITE_REG_TX_APJ                     ( 0x3A )

// 0x3B
//
#define HDMI_ITE_REG_TX_BKSV                    ( 0x3B )

// 0x40
//
#define HDMI_ITE_REG_TX_BRI                     ( 0x40 )
#define HDMI_ITE_REG_TX_BRI0                    ( 0x40 )

// 0x41
//
#define HDMI_ITE_REG_TX_BRI1                    ( 0x41 )

// 0x42
//
#define HDMI_ITE_REG_TX_BPJ                     ( 0x42 )

// 0x43
//
#define HDMI_ITE_REG_TX_BCAP                    ( 0x43 )
#define HDMI_ITE_B_TX_CAP_HDMI_REPEATER         ( 1 << 6 )
#define HDMI_ITE_B_TX_CAP_KSV_FIFO_RDY          ( 1 << 5 )
#define HDMI_ITE_B_TX_CAP_HDMI_FAST_MODE        ( 1 << 4 )
#define HDMI_ITE_B_CAP_HDCP_1p1                 ( 1 << 1 )
#define HDMI_ITE_B_TX_CAP_FAST_REAUTH           ( 1 << 0 )

// 0x44
//
#define HDMI_ITE_REG_TX_BSTAT                   ( 0x44 )
#define HDMI_ITE_REG_TX_BSTAT0                  ( 0x44 )

// 0x45
//
#define HDMI_ITE_REG_TX_BSTAT1                  ( 0x45 )
#define HDMI_ITE_B_TX_CAP_HDMI_MODE             ( 1 << 12 )
#define HDMI_ITE_B_TX_CAP_DVI_MODE              ( 0 << 12 )
#define HDMI_ITE_B_TX_MAX_CASCADE_EXCEEDED      ( 1 << 11 )
#define HDMI_ITE_M_TX_REPEATER_DEPTH            ( 0x7 << 8 )
#define HDMI_ITE_O_TX_REPEATER_DEPTH            ( 8 )
#define HDMI_ITE_B_TX_DOWNSTREAM_OVER           ( 1 << 7 )
#define HDMI_ITE_M_TX_DOWNSTREAM_COUNT          ( 0x7F )

// 0x46
//
#define HDMI_ITE_REG_TX_AUTH_STAT               ( 0x46 )
#define HDMI_ITE_B_TX_AUTH_DONE                 ( 1 << 7 )

// 0x50
//
#define HDMI_ITE_REG_TX_SHA_SEL                 ( 0x50 )

// 0x51
//
#define HDMI_ITE_REG_TX_SHA_RD_BYTE1            ( 0x51 )

// 0x52
//
#define HDMI_ITE_REG_TX_SHA_RD_BYTE2            ( 0x52 )

// 0x53
//
#define HDMI_ITE_REG_TX_SHA_RD_BYTE3            ( 0x53 )

// 0x54
//
#define HDMI_ITE_REG_TX_SHA_RD_BYTE4            ( 0x54 )

// 0x55
//
#define HDMI_ITE_REG_TX_AKSV_RD_BYTE5           ( 0x55 )

// 0x58
//
#define HDMI_ITE_REG_TX_CLK_CTRL0               ( 0x58 )
#define HDMI_ITE_O_TX_OSCLK_SEL                 ( 5 )
#define HDMI_ITE_M_TX_OSCLK_SEL                 ( 3 )
#define HDMI_ITE_B_TX_AUTO_OVER_SAMPLING_CLOCK  ( 1 << 4 )
#define HDMI_ITE_O_TX_EXT_MCLK_SEL              ( 2 )
#define HDMI_ITE_M_TX_EXT_MCLK_SEL              ( 3 << HDMI_ITE_O_TX_EXT_MCLK_SEL )
#define HDMI_ITE_B_TX_EXT_128FS                 ( 0 << HDMI_ITE_O_TX_EXT_MCLK_SEL )
#define HDMI_ITE_B_TX_EXT_256FS                 ( 1 << HDMI_ITE_O_TX_EXT_MCLK_SEL )
#define HDMI_ITE_B_TX_EXT_512FS                 ( 2 << HDMI_ITE_O_TX_EXT_MCLK_SEL )
#define HDMI_ITE_B_TX_EXT_1024FS                ( 3 << HDMI_ITE_O_TX_EXT_MCLK_SEL )
#define HDMI_ITE_F_AUDIO_ON                     ( 1 << 7 )
#define HDMI_ITE_F_AUDIO_HBR                    ( 1 << 6 )
#define HDMI_ITE_F_AUDIO_DSD                    ( 1 << 5 )
#define HDMI_ITE_F_AUDIO_NLPCM                  ( 1 << 4 )
#define HDMI_ITE_F_AUDIO_LAYOUT_1               ( 1 << 3 )
#define HDMI_ITE_F_AUDIO_LAYOUT_0               ( 0 << 3 )

// 0x58 ~ 0x65
//
#define HDMI_ITE_REG_TX_AVIINFO_DB1             ( 0x58 )
#define HDMI_ITE_REG_TX_AVIINFO_DB2             ( 0x59 )
#define HDMI_ITE_REG_TX_AVIINFO_DB3             ( 0x5A )
#define HDMI_ITE_REG_TX_AVIINFO_DB4             ( 0x5B )
#define HDMI_ITE_REG_TX_AVIINFO_DB5             ( 0x5C )
#define HDMI_ITE_REG_TX_AVIINFO_SUM             ( 0x5D )
#define HDMI_ITE_REG_TX_AVIINFO_DB6             ( 0x5E )
#define HDMI_ITE_REG_TX_AVIINFO_DB7             ( 0x5F )
#define HDMI_ITE_REG_TX_AVIINFO_DB8             ( 0x60 )
#define HDMI_ITE_REG_TX_AVIINFO_DB9             ( 0x61 )
#define HDMI_ITE_REG_TX_AVIINFO_DB10            ( 0x62 )
#define HDMI_ITE_REG_TX_AVIINFO_DB11            ( 0x63 )
#define HDMI_ITE_REG_TX_AVIINFO_DB12            ( 0x64 )
#define HDMI_ITE_REG_TX_AVIINFO_DB13            ( 0x65 )

// 0x59
//
#define HDMI_ITE_REG_TX_CLK_CTRL1               ( 0x59 )
#define HDMI_ITE_B_TX_EN_TXCLK_COUNT            ( 1 << 5 )
#define HDMI_ITE_B_TX_VDO_LATCH_EDGE            ( 1 << 3 )

// 0x5E
//
#define HDMI_ITE_REG_TX_CLK_STATUS1             ( 0x5E )

// 0x5F
//
#define HDMI_ITE_REG_TX_CLK_STATUS2             ( 0x5F )
#define HDMI_ITE_B_TX_IP_LOCK                   ( 1 << 7 )
#define HDMI_ITE_B_TX_XP_LOCK                   ( 1 << 6 )
#define HDMI_ITE_B_TX_OSF_LOCK                  ( 1 << 5 )

// 0x60
//
#define HDMI_ITE_REG_TX_AUD_COUNT               ( 0x60 )

// 0x61
//
#define HDMI_ITE_REG_TX_AFE_DRV_CTRL            ( 0x61 )
#define HDMI_ITE_B_TX_AFE_DRV_PWD               ( 1 << 5 )
#define HDMI_ITE_B_TX_AFE_DRV_RST               ( 1 << 4 )

// 0x68
//
#define HDMI_ITE_REG_TX_PKT_AUDINFO_CC          ( 0x68 )

// 0x69
//
#define HDMI_ITE_REG_TX_PKT_AUDINFO_SF          ( 0x69 )

// 0x6B
//
#define HDMI_ITE_REG_TX_PKT_AUDINFO_CA          ( 0x6B )

// 0x6C
//
#define HDMI_ITE_REG_TX_PKT_AUDINFO_DM_LSV      ( 0x6C )

// 0x6D
//
#define HDMI_ITE_REG_TX_PKT_AUDINFO_SUM         ( 0x6D )

// 0x70
//
#define HDMI_ITE_REG_TX_INPUT_MODE              ( 0x70 )
#define HDMI_ITE_B_TX_INDDR	                    ( 1 << 2 )
#define HDMI_ITE_B_TX_SYNCEMB	                ( 1 << 3 )
#define HDMI_ITE_B_TX_2X656CLK	                ( 1 << 4 )
#define HDMI_ITE_B_TX_PCLKDIV2                  ( 1 << 5 )
#define HDMI_ITE_M_TX_INCOLMOD	                ( 3 << 6 )
#define HDMI_ITE_B_TX_IN_RGB                    ( 0 )
#define HDMI_ITE_B_TX_IN_YUV422                 ( 1 << 6 )
#define HDMI_ITE_B_TX_IN_YUV444                 ( 2 << 6 )

// 0x71
//
#define HDMI_ITE_REG_TX_TXFIFO_RST              ( 0x71 )
#define HDMI_ITE_B_TX_ENAVMUTERST	            ( 1 )

// 0x72
//
#define HDMI_ITE_REG_TX_CSC_CTRL                ( 0x72 )
#define HDMI_ITE_B_HDMI_CSC_BYPASS              ( 0 )
#define HDMI_ITE_B_HDMI_CSC_RGB2YUV             ( 2 )
#define HDMI_ITE_B_HDMI_CSC_YUV2RGB             ( 3 )
#define HDMI_ITE_M_TX_CSC_SEL                   ( 3 )
#define HDMI_ITE_B_TX_EN_DITHER                 ( 1 << 7 )
#define HDMI_ITE_B_TX_EN_UDFILTER               ( 1 << 6 )
#define HDMI_ITE_B_TX_DNFREE_GO                 ( 1 << 5 )

// 0xC0
//
#define HDMI_ITE_REG_TX_HDMI_MODE               ( 0xC0 )
#define HDMI_ITE_B_TX_MODE_DVI                  ( 0 )
#define HDMI_ITE_B_TX_MODE_HDMI                 ( 1 )

// 0xC1
//
#define HDMI_ITE_REG_TX_AV_MUTE                 ( 0xC1 )
#define HDMI_ITE_REG_TX_GCP                     ( 0xC1 )
#define HDMI_ITE_B_TX_SETAVMUTE                 ( 1 << 0 )

// 0xC6
//
#define HDMI_ITE_REG_TX_PKT_GENERAL_CTRL        ( 0xC6 )
#define HDMI_ITE_B_TX_ENABLE_PKT                ( 1 )
#define HDMI_ITE_B_TX_REPEAT_PKT                ( 2 )

// 0xD2
//
#define HDMI_ITE_REG_TX_3D_INFO_CTRL            ( 0xD2 )

// 0xE0
//
#define HDMI_ITE_REG_TX_AUDIO_CTRL0             ( 0xE0 )
#define HDMI_ITE_M_TX_AUD_SWL                   ( 3 << 6 )
#define HDMI_ITE_M_TX_AUD_16BIT                 ( 0 << 6 )
#define HDMI_ITE_M_TX_AUD_18BIT                 ( 1 << 6 )
#define HDMI_ITE_M_TX_AUD_20BIT                 ( 2 << 6 )
#define HDMI_ITE_M_TX_AUD_24BIT                 ( 3 << 6 )
#define HDMI_ITE_B_TX_SPDIFTC                   ( 1 << 5 )
#define HDMI_ITE_B_TX_AUD_SPDIF                 ( 1 << 4 )
#define HDMI_ITE_B_TX_AUD_I2S                   ( 0 << 4 )
#define HDMI_ITE_B_TX_AUD_EN_I2S3               ( 1 << 3 )
#define HDMI_ITE_B_TX_AUD_EN_I2S2               ( 1 << 2 )
#define HDMI_ITE_B_TX_AUD_EN_I2S1               ( 1 << 1 )
#define HDMI_ITE_B_TX_AUD_EN_I2S0               ( 1 << 0 )
#define HDMI_ITE_B_TX_AUD_EN_SPDIF              ( 1 )

// 0xE1
//
#define HDMI_ITE_REG_TX_AUDIO_CTRL1                 ( 0xE1 )
#define HDMI_ITE_B_TX_AUDFMT_STD_I2S                ( 0 << 0 )
#define HDMI_ITE_B_TX_AUDFMT_32BIT_I2S              ( 1 << 0 )
#define HDMI_ITE_B_TX_AUDFMT_LEFT_JUSTIFY           ( 0 << 1 )
#define HDMI_ITE_B_TX_AUDFMT_RIGHT_JUSTIFY          ( 1 << 1 )
#define HDMI_ITE_B_TX_AUDFMT_DELAY_1T_TO_WS         ( 0 << 2 )
#define HDMI_ITE_B_TX_AUDFMT_NO_DELAY_TO_WS         ( 1 << 2 )
#define HDMI_ITE_B_TX_AUDFMT_WS0_LEFT               ( 0 << 3 )
#define HDMI_ITE_B_TX_AUDFMT_WS0_RIGHT              ( 1 << 3 )
#define HDMI_ITE_B_TX_AUDFMT_MSB_SHIFT_FIRST        ( 0 << 4 )
#define HDMI_ITE_B_TX_AUDFMT_LSB_SHIFT_FIRST        ( 1 << 4 )
#define HDMI_ITE_B_TX_AUDFMT_RISE_EDGE_SAMPLE_WS    ( 0 << 5 )
#define HDMI_ITE_B_TX_AUDFMT_FALL_EDGE_SAMPLE_WS    ( 1 << 5 )
#define HDMI_ITE_B_TX_AUDFMT_NOT_FULLPKT            ( 0 << 6 )
#define HDMI_ITE_B_TX_AUDFMT_FULLPKT                ( 1 << 6 )

// 0xE2
//
#define HDMI_ITE_REG_TX_AUDIO_FIFOMAP           ( 0xE2 )
#define HDMI_ITE_O_TX_FIFO3SEL                  ( 6 )
#define HDMI_ITE_O_TX_FIFO2SEL                  ( 4 )
#define HDMI_ITE_O_TX_FIFO1SEL                  ( 2 )
#define HDMI_ITE_O_TX_FIFO0SEL                  ( 0 )
#define HDMI_ITE_B_TX_SELSRC3                   ( 3 )
#define HDMI_ITE_B_TX_SELSRC2                   ( 2 )
#define HDMI_ITE_B_TX_SELSRC1                   ( 1 )
#define HDMI_ITE_B_TX_SELSRC0                   ( 0 )

// 0xE3
//
#define HDMI_ITE_REG_TX_AUDIO_CTRL3             ( 0xE3 )
#define HDMI_ITE_B_TX_AUD_MULCH                 ( 1 << 7 )
#define HDMI_ITE_B_TX_EN_ZERO_CTS               ( 1 << 6 )
#define HDMI_ITE_B_TX_CHSTSEL                   ( 1 << 4 )
#define HDMI_ITE_B_TX_S3RLCHG                   ( 1 << 3 )
#define HDMI_ITE_B_TX_S2RLCHG                   ( 1 << 2 )
#define HDMI_ITE_B_TX_S1RLCHG                   ( 1 << 1 )
#define HDMI_ITE_B_TX_S0RLCHG                   ( 1 << 0 )

// 0xE4
//
#define HDMI_ITE_REG_TX_AUD_SRCVALID_FLAT       ( 0xE4 )
#define HDMI_ITE_B_TX_AUD_SPXFLAT_SRC3          ( 1 << 7 )
#define HDMI_ITE_B_TX_AUD_SPXFLAT_SRC2          ( 1 << 6 )
#define HDMI_ITE_B_TX_AUD_SPXFLAT_SRC1          ( 1 << 5 )
#define HDMI_ITE_B_TX_AUD_SPXFLAT_SRC0          ( 1 << 4 )
#define HDMI_ITE_B_TX_AUD_ERR2FLAT              ( 1 << 3 )
#define HDMI_ITE_B_TX_AUD_S3VALID               ( 1 << 2 )
#define HDMI_ITE_B_TX_AUD_S2VALID               ( 1 << 1 )
#define HDMI_ITE_B_TX_AUD_S1VALID               ( 1 << 0 )

// 0xE5
//
#define HDMI_ITE_REG_TX_AUD_HDAUDIO             ( 0xE5 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS0          ( 0x30 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS1          ( 0x31 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS2          ( 0x32 )
#define HDMI_ITE_REG_PACKET_AUDIO_N0            ( 0x33 )
#define HDMI_ITE_REG_PACKET_AUDIO_N1            ( 0x34 )
#define HDMI_ITE_REG_PACKET_AUDIO_N2            ( 0x35 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS_CNT0      ( 0x35 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS_CNT1      ( 0x36 )
#define HDMI_ITE_REG_PACKET_AUDIO_CTS_CNT2      ( 0x37 )

// 0x91 ~ 0x99
//
#define HDMI_ITE_REG_TX_AUDCHST_MODE            ( 0x91 )
#define HDMI_ITE_REG_TX_AUDCHST_CAT             ( 0x92 )
#define HDMI_ITE_REG_TX_AUDCHST_SRCNUM          ( 0x93 )
#define HDMI_ITE_REG_TX_AUD0CHST_CHTNUM         ( 0x94 )
#define HDMI_ITE_REG_TX_AUD1CHST_CHTNUM         ( 0x95 )
#define HDMI_ITE_REG_TX_AUD2CHST_CHTNUM         ( 0x96 )
#define HDMI_ITE_REG_TX_AUD3CHST_CHTNUM         ( 0x97 )
#define HDMI_ITE_REG_TX_AUDCHST_CA_FS           ( 0x98 )
#define HDMI_ITE_REG_TX_AUDCHST_OFS_WL          ( 0x99 )

// 0xC5
//
#define HDMI_ITE_REG_TX_PKT_SINGLE_CTRL         ( 0xC5 )
#define HDMI_ITE_B_TX_SINGLE_PKT                ( 1 )
#define HDMI_ITE_B_TX_SW_CTS                    ( 1 << 1 )

// 0xCD
//
#define HDMI_ITE_REG_TX_AVI_INFOFRM_CTRL        ( 0xCD )
#define B_TX_ENABLE_PKT                         ( 1 )
#define B_TX_REPEAT_PKT                         ( 1 << 1 )

// 0xCE
//
#define HDMI_ITE_REG_TX_AUD_INFOFRM_CTRL        ( 0xCE )

// 0xF0
//
#define HDMI_ITE_T_AUDIO_MASK                   ( 0xF0 )
#define HDMI_ITE_T_AUDIO_OFF                    ( 0 )
#define HDMI_ITE_T_AUDIO_HBR                    ( HDMI_ITE_F_AUDIO_ON | HDMI_ITE_F_AUDIO_HBR )
#define HDMI_ITE_T_AUDIO_DSD                    ( HDMI_ITE_F_AUDIO_ON | HDMI_ITE_F_AUDIO_DSD )
#define HDMI_ITE_T_AUDIO_NLPCM                  ( HDMI_ITE_F_AUDIO_ON | HDMI_ITE_F_AUDIO_NLPCM )
#define HDMI_ITE_T_AUDIO_LPCM                   ( HDMI_ITE_F_AUDIO_ON )

#define HDMI_ITE_B_TX_HBR                       ( 1 << 3 )
#define HDMI_ITE_B_TX_DSD                       ( 1 << 1 )

typedef struct _HDMI_REGISTER_SET_ENTRY_
{
	uint8_t Offset;
	uint8_t InvAndMask;
	uint8_t OrMask;
} 	HDMI_REGISTER_SET_ENTRY;

typedef struct _AVI_INFO_FRAME_
{
	uint8_t AVI_HB[3];
	uint8_t CheckSum;
	uint8_t AVI_DB[HDMI_ITE_AVI_INFOFRAME_LEN];
} 	AVI_INFO_FRAME, *PAVI_INFO_FRAME;

typedef struct _AUDIO_INFO_FRAME_
{
	uint8_t AUD_HB[3];
	uint8_t CheckSum;
	uint8_t AUD_DB[HDMI_ITE_AUDIO_INFOFRAME_LEN];
} 	AUDIO_INFO_FRAME, *PAUDIO_INFO_FRAME;

bool
fl2000_hdmi_bit_set(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint8_t offset_bit);

bool
fl2000_hdmi_bit_clear(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint8_t offset_bit);

uint8_t
fl2000_hdmi_read_byte_simple(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	bool* is_good);

void
fl2000_hdmi_write_byte_simple(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint8_t byte_data,
	bool* is_good);

void
fl2000_write_byte_simple_with_mask(
	struct dev_ctx * dev_ctx,
	uint8_t reg,
	uint8_t mask,
	uint8_t value,
	bool* is_good);

int
fl2000_hdmi_read_byte(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint8_t * return_data);

int
fl2000_hdmi_write_byte(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint8_t * write_byte);

int
fl2000_hdmi_read_dword(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint32_t* return_data);

int
fl2000_hdmi_write_dword(
	struct dev_ctx * dev_ctx,
	uint8_t offset,
	uint32_t* write_dword);

bool
fl2000_hdmi_abort_ddc(struct dev_ctx * dev_ctx);

bool
fl2000_clear_ddc_fifo(struct dev_ctx * dev_ctx);

int
fl2000_hdmi_read_edid_table(
	struct dev_ctx * dev_ctx,
	uint8_t EdidSegmentIndex,
	uint8_t EdidTableOffset,
	uint8_t ReadCount,
	uint8_t * ReturnedBuffer);

bool
fl2000_hdmi_read_edid_table_init(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_set_display_mode(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_config_avi_info_frame(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_enable_avi_info_frame(
	struct dev_ctx * dev_ctx,
	bool enable,
	AVI_INFO_FRAME* avi_info_frame);

bool
fl2000_hdmi_set_avi_info_frame(
	struct dev_ctx * dev_ctx,
	AVI_INFO_FRAME* avi_info_frame);

bool
fl2000_hdmi_disable_audio_output(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_av_mute(struct dev_ctx * dev_ctx, uint8_t IsMute);

bool
fl2000_hdmi_enable_video_output(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_fire_afe(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_setup_afe(struct dev_ctx * dev_ctx, uint8_t Level);

bool
fl2000_hdmi_switch_bank(struct dev_ctx * dev_ctx, uint8_t bank_num);

bool
fl2000_hdmi_set_input_mode(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_set_csc_scale(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_find_chip(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_power_up(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_power_down(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_reset(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_config_audio_info_frame(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_enable_audio_info_frame(
	struct dev_ctx * dev_ctx,
	uint8_t IsEnable,
	PAUDIO_INFO_FRAME AudioInfoFrame);

bool
fl2000_hdmi_set_audio_info_frame(
	struct dev_ctx * dev_ctx,
	PAUDIO_INFO_FRAME AudioInfoFrame);

bool
fl2000_hdmi_enable_audio_output(struct dev_ctx * dev_ctx);

//void
//fl2000_hdmi_dump_bank(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_check_stable(struct dev_ctx * dev_ctx);

bool
fl2000_hdmi_read_block(struct dev_ctx * dev_ctx, uint8_t block_id);

void
fl2000_hdmi_compliance_tweak(struct dev_ctx * dev_ctx);

void
fl2000_hdmi_init(struct dev_ctx * dev_ctx, bool resolution_change);

//VOID
//HDMI_HDCP_Enable(
//    PDEVICE_CONTEXT DevCtx,
//    bool IsEnable
//    );

#endif // _FL2000_HDMI_H_

// eof: vid_HDMI.c
//
