/******************************************************************************
* SMSC LAN9118 definition                                                     *
*                                                                             *
*  Copyright (c) 2005, MiSPO Co., Ltd.                                        *
*  All rights reserved.                                                       *
*                                                                             *
* 19/Apr/2005 Created                                                    [OK] *
******************************************************************************/

#ifndef LAN9118_H
#define LAN9118_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef ALIGN
#define ALIGN(p,n)  ((((UW)(p)) + (((UW)(n)) - 1)) & (~(((UW)(n)) - 1)))
                   /* アラインメント調整マクロ
                    *  p: 計算前アドレス
                    *  n: 境界値(2のべき乗であること)
                    */
#endif


/* TX COMMAND 'A' field definition */

#define TX_CMDA_IOC         0x80000000UL  /* Interrupt on Completion */
#define TX_CMDA_EA4         (0UL<<24)     /* Buffer End Alignment( 4-Byte) */
#define TX_CMDA_EA16        (1UL<<24)     /* Buffer End Alignment(16-Byte) */
#define TX_CMDA_EA32        (2UL<<24)     /* Buffer End Alignment(32-Byte) */
#define TX_CMDA_DOFF_SHFT   16            /* Data Start Offset / SHIFT number */
#define TX_CMDA_F           0x00002000UL  /* First Segment */
#define TX_CMDA_L           0x00001000UL  /* Last Segment */
#define TX_CMDA_BSZ_MASK    0x000007ffUL  /* Buffer Size / MASK pattern */

/* TX COMMAND 'B' field definition */

#define TX_CMDB_TAG_MASK    0xffff0000UL  /* Packet Tag / MASK pattern */
#define TX_CMDB_CRC         0x00002000UL  /* Add CRC Disable */
#define TX_CMDB_PAD         0x00001000UL  /* Disable Ethernet Frame Padding */
#define TX_CMDB_LEN_MASK    0x000007ffUL  /* Packet Length / MASK pattern */

/* TX Status field definition */

#define TX_STS_TAG_MASK     0xffff0000UL  /* Packet Tag / MASK pattern */
#define TX_STS_ES           0x00008000UL  /* Error Status */
#define TX_STS_LOSS         0x00000800UL  /* Loss of Carrier */
#define TX_STS_NC           0x00000400UL  /* No Carrier */
#define TX_STS_LCOL         0x00000200UL  /* Late Collision */
#define TX_STS_ECOL         0x00000100UL  /* Excessive Collisions */
#define TX_STS_EDEF         0x00000004UL  /* Excessive Deferral */
#define TX_STS_UNDER        0x00000002UL  /* Underrun Error */
#define TX_STS_DEF          0x00000001UL  /* Deferred */

/* RX Status field definition */

#define RX_STS_FFAIL        0x40000000UL  /* Filtering Fail */
#define RX_STS_LEN_MASK     0x3fff0000UL  /* Packet Length / MASK pattern */
#define RX_STS_LEN_SHFT     16            /* Packet Length / SHIFT number */
#define RX_STS_ES           0x00008000UL  /* Error Status */
#define RX_STS_BF           0x00002000UL  /* Broadcast Frame */
#define RX_STS_LE           0x00001000UL  /* Length Error */
#define RX_STS_RF           0x00000800UL  /* Runt Frame */
#define RX_STS_MF           0x00000400UL  /* Multicast Frame */
#define RX_STS_LONG         0x00000080UL  /* Frame Too Long */
#define RX_STS_COL          0x00000040UL  /* Collision Seen */
#define RX_STS_TYPE         0x00000020UL  /* Frame Type */
#define RX_STS_WDOG         0x00000010UL  /* Receive Watchdog time-out */
#define RX_STS_MII          0x00000008UL  /* MII Error */
#define RX_STS_DBIT         0x00000004UL  /* Dribbling Bit */
#define RX_STS_CRC          0x00000002UL  /* CRC Error */


/*
 * Register address map
 */

#define RX_DATA             0x00  /* ..0x1c:RX Data FIFO Port */
#define TX_DATA             0x20  /* ..0x3c:TX Data FIFO Port */
#define RX_STAT             0x40  /* RX Status FIFO Port */
#define RX_STAT_PEEK        0x44  /* RX Status FIFO PEEK */
#define TX_STAT             0x48  /* TX Status FIFO Port */
#define TX_STAT_PEEK        0x4c  /* TX Status FIFO PEEK */

#define ID_REV              0x50  /* Chip ID and Revision */
#define IRQ_CFG             0x54  /* Main Interrupt Configuration */
#define INT_STS             0x58  /* Interrupt Status */
#define INT_EN              0x5c  /* Interrupt Enable Register */
#define BYTE_TEST           0x64  /* Read-only byte order testing register */
#define FIFO_INT            0x68  /* FIFO Level interrupts */
#define RX_CFG              0x6c  /* Receive Configuration */
#define TX_CFG              0x70  /* Transmit Configuration */
#define HW_CFG              0x74  /* Hardware Configuration */
#define RX_DP_CTL           0x78  /* RX Datapath Control */
#define RX_FIFO_INF         0x7c  /* Receive FIFO Information */
#define TX_FIFO_INF         0x80  /* Transmit FIFO Information */
#define PMT_CTRL            0x84  /* Power Management Control */
#define GPIO_CFG            0x88  /* General Purpose IO Configuration */
#define GPT_CFG             0x8c  /* General Purpose Timer Configuration */
#define GPT_CNT             0x90  /* General Purpose Timer Counter */
#define ENDIAN_CTL          0x98  /* ENDIAN */
#define FREE_RUN            0x9c  /* Free Run Counter */
#define RX_DROP             0xa0  /* RX Dropped Frames Counter */
#define MAC_CSR_CMD         0xa4  /* MAC CSR Synchronizer Command */
#define MAC_CSR_DATA        0xa8  /* MAC CSR Synchronizer Data */
#define AFC_CFG             0xac  /* Automatic Flow Control Configuration */
#define E2P_CMD             0xb0  /* EEPROM command */
#define E2P_DATA            0xb4  /* EEPROM Data */

/*
 * bit assignment of each register
 */

/* IRQ_CFG */

#define IRQ_DEAS_MASK       0xff000000UL  /* Interrupt Deassertion Interval / MASK pattern */
#define IRQ_DEAS_CLR        0x00004000UL  /* Interrupt Deassertion Interval Clear */
#define IRQ_DEAS_STS        0x00002000UL  /* Interrupt Deassertion Status */
#define IRQ_INT             0x00001000UL  /* Master Interrupt */
#define IRQ_EN              0x00000100UL  /* IRQ Enable */
#define IRQ_POL             0x00000010UL  /* IRQ Polarity */
#define IRQ_TYPE            0x00000001UL  /* IRQ Buffer Type */

/* INT_STS / INT_EN */

#define INT_SW              0x80000000UL  /* Software Interrupt */
#define INT_TXSTOP          0x02000000UL  /* TX Stopped */
#define INT_RXSTOP          0x01000000UL  /* RX Stopped */
#define INT_RXDFH           0x00800000UL  /* RX Dropped Frame Counter Halfway */
#define INT_IOC             0x00200000UL  /* TX IOC Interrupt */
#define INT_RXD             0x00100000UL  /* RX DMA Interrupt */
#define INT_GPT             0x00080000UL  /* GP Timer Interrupt */
#define INT_PHY             0x00040000UL  /* PHY Interrupt */
#define INT_PME             0x00020000UL  /* Power Management Event Interrupt */
#define INT_TXSO            0x00010000UL  /* TX Status FIFO Overflow */
#define INT_RWT             0x00008000UL  /* Receive Watchdog Time-out */
#define INT_RXE             0x00004000UL  /* Receive Error */
#define INT_TXE             0x00002000UL  /* Transmitter Error */
#define INT_TDFU            0x00000800UL  /* TX Data FIFO Underrun Interrupt */
#define INT_TDFO            0x00000400UL  /* TX Data FIFO Overrun Interrupt */
#define INT_TDFA            0x00000200UL  /* TX Data FIFO Available Interrupt */
#define INT_TSFF            0x00000100UL  /* TX Status FIFO Full Interrupt */
#define INT_TSFL            0x00000080UL  /* TX Status FIFO Level Interrupt */
#define INT_RXDF            0x00000040UL  /* RX Dropped Frame Interrupt */
#define INT_RDFL            0x00000020UL  /* RX Data FIFO Level Interrupt */
#define INT_RSFF            0x00000010UL  /* RX Status FIFO Full Interrupt */
#define INT_RSFL            0x00000008UL  /* RX Status FIFO Level Interrupt */
#define INT_GPIO2           0x00000004UL  /* GPIO2 Interrupt */
#define INT_GPIO1           0x00000002UL  /* GPIO1 Interrupt */
#define INT_GPIO0           0x00000001UL  /* GPIO0 Interrupt */

/* BYTE_TEST */

#define BYTE_TEST_VAL       0x87654321UL  /* data pattern */

/* FIFO_INT */

#define FINT_TDA_SHFT       24            /* TX Data Available Level / SHIFT number */
#define FINT_TSL_SHFT       16            /* TX Status Level / SHIFT number */
#define FINT_RSA_SHFT       8             /* RX Space Available Level / SHIFT number */
#define FINT_RSL_SHFT       0             /* RX Status Level / SHIFT number */

/* RX_CFG */

#define RCFG_EA4            (0UL<<30)     /* RX End Alignment( 4-Byte) */
#define RCFG_EA16           (1UL<<30)     /* RX End Alignment(16-Byte) */
#define RCFG_EA32           (2UL<<30)     /* RX End Alignment(32-Byte) */
#define RCFG_DMA_SHFT       16            /* RX DMA Count / SHIFT number */
#define RCFG_DUMP           0x00008000UL  /* Force RX Discard */
#define RCFG_DOFF_SHFT      8             /* RX Data Offset / SHIFT number */

/* TX_CFG */

#define TCFG_SDUMP          0x00008000UL  /* Force TX Status Discard */
#define TCFG_DDUMP          0x00004000UL  /* Force TX Data Discard */
#define TCFG_TXSAO          0x00000004UL  /* TX Status Allow Overrun */
#define TCFG_TXON           0x00000002UL  /* Transmitter Enable */
#define TCFG_STOP           0x00000001UL  /* Stop Transmitter */

/* HW_CFG */

#define HWCFG_TTM           0x00200000UL  /* Transmit Threshold Mode */
#define HWCFG_SF            0x00100000UL  /* Store and Forward */
#define HWCFG_TFSZ2         (2UL<<16)     /* TXD: 1536 TXS:512 RXD:13440 RXS 896 */
#define HWCFG_TFSZ3         (3UL<<16)     /* TXD: 2560 TXS:512 RXD:12480 RXS 832 */
#define HWCFG_TFSZ4         (4UL<<16)     /* TXD: 3548 TXS:512 RXD:11520 RXS 768 */
#define HWCFG_TFSZ5         (5UL<<16)     /* TXD: 4608 TXS:512 RXD:10560 RXS 704 */
#define HWCFG_TFSZ6         (6UL<<16)     /* TXD: 5632 TXS:512 RXD: 9600 RXS 640 */
#define HWCFG_TFSZ7         (7UL<<16)     /* TXD: 6656 TXS:512 RXD: 8640 RXS 576 */
#define HWCFG_TFSZ8         (8UL<<16)     /* TXD: 7680 TXS:512 RXD: 7680 RXS 512 */
#define HWCFG_TFSZ9         (9UL<<16)     /* TXD: 8704 TXS:512 RXD: 6720 RXS 448 */
#define HWCFG_TFSZ10        (10UL<<16)    /* TXD: 9728 TXS:512 RXD: 5760 RXS 384 */
#define HWCFG_TFSZ11        (11UL<<16)    /* TXD:10752 TXS:512 RXD: 4800 RXS 320 */
#define HWCFG_TFSZ12        (12UL<<16)    /* TXD:11776 TXS:512 RXD: 3840 RXS 256 */
#define HWCFG_TFSZ13        (13UL<<16)    /* TXD:12800 TXS:512 RXD: 2880 RXS 192 */
#define HWCFG_TFSZ14        (14UL<<16)    /* TXD:13824 TXS:512 RXD: 1920 RXS 128 */
#define HWCFG_TR0           (0UL<<12)     /* */
#define HWCFG_TR1           (1UL<<12)     /* */
#define HWCFG_TR2           (2UL<<12)     /* */
#define HWCFG_TR3           (3UL<<12)     /* */
#define HWCFG_BUS32         0x00000004UL  /* 32/16-bit Mode (1:32bit) */
#define HWCFG_SRST_TO       0x00000002UL  /* Soft Reset Time-out */
#define HWCFG_SRST          0x00000001UL  /* Soft Reset */

/* RX_DP_CTRL */

#define RXDP_FFWD           0x80000000UL  /* RX Data FIFO Fast Forward */

/* RX_FIFO_INF */

#define RINF_SUSED_MASK     0x00ff0000UL  /* RX Status FIFO Used Space / MASK pattern */
#define RINF_SUSED_SHFT     16            /* RX Status FIFO Used Space / SHIFT number */
#define RINF_DUSED_MASK     0x0000ffffUL  /* RX Data FIFO Used Space / MASK pattern */

/* TX_FIFO_INF */

#define TINF_SUSED_MASK     0x00ff0000UL  /* TX Status FIFO Used Space / MASK pattern */
#define TINF_SUSED_SHFT     16            /* TX Status FIFO Used Space / SHIFT number */
#define TINF_DFREE_MASK     0x0000ffffUL  /* TX Data FIFO Free Space / MASK pattern */

/* PMT_CTRL */

#define PMT_MODE0           (0UL<<12)     /* Power Management Mode D0(normal operation) */
#define PMT_MODE1           (1UL<<12)     /* Power Management Mode D1(wake-up & magic enable) */
#define PMT_MODE2           (2UL<<12)     /* Power Management Mode D2(can perform energy detect) */
#define PMT_PHY_RST         0x00000400UL  /* PHY Reset */
#define PMT_WOL_EN          0x00000200UL  /* Wake-On-Lan Enable */
#define PMT_ED_EN           0x00000100UL  /* Energy-Detect Enable */
#define PMT_PME_TYPE        0x00000040UL  /* PME Buffer Type */
#define PMT_WUPS0           (0UL<<4)      /* WAKE-UP Status 0(No wake-up) */
#define PMT_WUPS1           (1UL<<4)      /* WAKE-UP Status 1(Energy detected) */
#define PMT_WUPS2           (2UL<<4)      /* WAKE-UP Status 2(wake-up or magic detected) */
#define PMT_WUPS3           (3UL<<4)      /* WAKE-UP Status 3(Indicates multiple events occurred) */
#define PMT_PME_IND         0x00000008UL  /* PME indicateion */
#define PMT_PME_POL         0x00000004UL  /* PME Polarity */
#define PMT_PME_EN          0x00000002UL  /* PME Enable */
#define PMT_READY           0x00000001UL  /* Device Readt */

/* GPIO_CFG */

#define GPIO_LED3           0x40000000UL  /* LED3(GPIO2) enable */
#define GPIO_LED2           0x20000000UL  /* LED2(GPIO1) enable */
#define GPIO_LED1           0x10000000UL  /* LED1(GPIO0) enable */
#define GPIO_POL2           0x04000000UL  /* GPIO2 Interrupt Polarity */
#define GPIO_POL1           0x02000000UL  /* GPIO1 Interrupt Polarity */
#define GPIO_POL0           0x01000000UL  /* GPIO0 Interrupt Polarity */
#define GPIO_EEPR0          (0<<20)       /* EEDIO  EECLK */
#define GPIO_EEPR1          (1<<20)       /* GPO3   GPO4  */
#define GPIO_EEPR3          (3<<20)       /* GPO3   RX_DV */
#define GPIO_EEPR5          (5<<20)       /* TX_EN  GPO4  */
#define GPIO_EEPR6          (6<<20)       /* TX_EN  RX_DV */
#define GPIO_EEPR7          (7<<20)       /* TX_CLK RX_CLK */
#define GPIO_BUF2           0x00040000UL  /* GPIO2 Buffer Type */
#define GPIO_BUF1           0x00020000UL  /* GPIO1 Buffer Type */
#define GPIO_BUF0           0x00010000UL  /* GPIO0 Buffer Type */
#define GPIO_DIR2           0x00000400UL  /* GPIO2 Direction */
#define GPIO_DIR1           0x00000200UL  /* GPIO1 Direction */
#define GPIO_DIR0           0x00000100UL  /* GPIO0 Direction */
#define GPIO_OD4            0x00000010UL  /* GPO Data 4 */
#define GPIO_OD3            0x00000008UL  /* GPO Data 3 */
#define GPIO_IOD2           0x00000004UL  /* GPIO Data 2 */
#define GPIO_IOD1           0x00000002UL  /* GPIO Data 1 */
#define GPIO_IOD0           0x00000001UL  /* GPIO Data 0 */

/* GPT_CFG */

#define GPT_TIMER_EN        0x20000000UL  /* GP Timer Enable */
#define GPT_LOAD_MASK       0x0000ffffUL  /* General Purpose Timer Pre-Load / MASK pattern */

/* ENDIAN */

#define ENDIAN_SET          0xffffffffUL  /* BIG-ENDIAN for 16bit-bus */

/* MAC_CSR_CMD */

#define CSR_BUSY            0x80000000UL  /* CSR Busy */
#define CSR_RD              0x40000000UL  /* CSR Read */

/* AFC_CFG */

#define AFC_HI_SHFT         16            /* Automatic Flow Control High Level / SHIFT number */
#define AFC_LO_SHFT         8             /* Automatic Flow Control Low Level / SHIFT number */
#define AFC_BDUR_SHFT       4             /* Backpressure Duration / SHIFT number */
#define AFC_FCMULT          0x00000008UL  /* Flow Control on Multicast Frame */
#define AFC_FCBRD           0x00000004UL  /* Flow Control on Broadcast Frame */
#define AFC_FCADD           0x00000002UL  /* Flow Control on Address Decorde */
#define AFC_FCANY           0x00000001UL  /* Flow Control on Any Frame */

/* E2P_CMD */

#define E2P_BUSY            0x80000000UL  /* EPC Busy */
#define E2P_READ            (0UL<<28)     /* EPC command(Read Location) */
#define E2P_EWDS            (1UL<<28)     /* EPC command(Erase/Write disable) */
#define E2P_EWEN            (2UL<<28)     /* EPC command(Erase/Write enable) */
#define E2P_WRITE           (3UL<<28)     /* EPC command(Write Location) */
#define E2P_WRAL            (4UL<<28)     /* EPC command(Write All) */
#define E2P_ERASE           (5UL<<28)     /* EPC command(Erase Location) */
#define E2P_ERAL            (6UL<<28)     /* EPC command(Erase All) */
#define E2P_RELO            (7UL<<28)     /* EPC command(MAC Address Reload) */
#define E2P_TMO             0x00000200UL  /* EPC Time-out */
#define E2P_MACLO           0x00000100UL  /* MAC Address Loaded */


/*
 * MAC Control and Status Register
 */

/* Register Index */

#define MAC_CR              0x01  /* MAC Control */
#define MAC_ADDRH           0x02  /* MAC Address High */
#define MAC_ADDRL           0x03  /* MAC Address Low */
#define MAC_HASHH           0x04  /* Multicast Hash Table High */
#define MAC_HASHL           0x05  /* Multicast Hash Table Low */
#define MAC_MII_ACC         0x06  /* MII Access */
#define MAC_MII_DATA        0x07  /* MII Data */
#define MAC_FLOW            0x08  /* Flow Control */
#define MAC_VLAN1           0x09  /* VLAN1 Tag */
#define MAC_VLAN2           0x0a  /* VLAN2 Tag */
#define MAC_WUFF            0x0b  /* Wake-up Frames Filter */
#define MAC_WUCSR           0x0c  /* Wake-up Control and Status */


/* MAC_CR */

#define MAC_CR_RXALL        0x80000000UL  /* Receive All Mode */
#define MAC_CR_RCVOWN       0x00800000UL  /* Disable Receive Own */
#define MAC_CR_LOOPBK       0x00200000UL  /* Loopback operation Mode */
#define MAC_CR_FDPX         0x00100000UL  /* Full Duplex Mode */
#define MAC_CR_MCPAS        0x00080000UL  /* Pass All Multicast */
#define MAC_CR_PRMS         0x00040000UL  /* Promiscuous Mode */
#define MAC_CR_INVFILT      0x00020000UL  /* Inverse filtering */
#define MAC_CR_PASSBAD      0x00010000UL  /* Pass Bad Frame */
#define MAC_CR_HO           0x00008000UL  /* Hash Only Filtering mode */
#define MAC_CR_HPFILT       0x00002000UL  /* Hash/Perfect Filtering mode */
#define MAC_CR_LCOLL        0x00001000UL  /* Late Collision Control */
#define MAC_CR_BCAST        0x00000800UL  /* Disable Broadcast Frames */
#define MAC_CR_DISRTY       0x00000400UL  /* Disable Retry */
#define MAC_CR_PADSTR       0x00000100UL  /* Automatic PAD Stripping */
#define MAC_CR_BOLMT0       (0UL<<6)      /* BackOff Limit (00b) */
#define MAC_CR_BOLMT1       (1UL<<6)      /* BackOff Limit (01b) */
#define MAC_CR_BOLMT2       (2UL<<6)      /* BackOff Limit (10b) */
#define MAC_CR_BOLMT3       (3UL<<6)      /* BackOff Limit (11b) */
#define MAC_CR_DFCHK        0x00000020UL  /* Deferral Check */
#define MAC_CR_TXEN         0x00000008UL  /* Transmitter enable */
#define MAC_CR_RXEN         0x00000004UL  /* Receiver enable */

/* MII_ACC */

#define MII_PHYAD_SHFT      11            /* PHY Address / SHIFT number */
#define MII_INDEX_SHFT      6             /* Register Index / SHIFT number */
#define MII_WR              0x00000002UL  /* MII Write */
#define MII_BUSY            0x00000001UL  /* MII Busy */

/* FLOW */

#define FLOW_PT_SHFT        16            /* Pause Time / SHIFT number */
#define FLOW_PASS           0x00000004UL  /* Pass Control Frames */
#define FLOW_EN             0x00000002UL  /* Flow Control Enable */
#define FLOW_BUSY           0x00000001UL  /* Flow Control Busy */

/* WUCSR */

#define WUCSR_GUE           0x00000200UL  /* Global Unicast Enable */
#define WUCSR_FR            0x00000040UL  /* Remote Wake-up Frame Received */
#define WUCSR_MPR           0x00000020UL  /* Magic Packet Received */
#define WUCSR_WUEN          0x00000004UL  /* Wake-up Frame enabled */
#define WUCSR_MPEN          0x00000002UL  /* Magic Packet Enable */


/*
 * PHY register definition
 */

/* Basic register */

#define PHY_CR              0  /* Basic Control */
#define PHY_SR              1  /* Basic Status */
#define PHY_ID1             2  /* PHY Identifier 1 */
#define PHY_ID2             3  /* PHY Identifier 2 */
#define PHY_ANA             4  /* Auto-Negotiation Advertisement */
#define PHY_ANLPA           5  /* Auto-Negotiation Link Partner Ability */
#define PHY_ANE             6  /* Auto-Negoriation Expansion */

/* Vendor register */

#define PHY_MCR             17 /* Mode Control/Status */
#define PHY_SMD             18 /* Special Modes */
#define PHY_SCI             27 /* Special Control/Status Indications */
#define PHY_ISR             29 /* Interrupt Source */
#define PHY_IMR             30 /* Interrupt Mask */
#define PHY_SCR             31 /* PHY Special Control/Status */


/*
 * bit assignment of each register
 */

/* Basic Control Register */

#define PHY_CR_RST          0x8000  /* Reset */
#define PHY_CR_LPBK         0x4000  /* Loopback */
#define PHY_CR_SPEED        0x2000  /* Speed Selection */
#define PHY_CR_ANEG_EN      0x1000  /* Auto-Negotiation Enable */
#define PHY_CR_PDN          0x0800  /* Power Down */
#define PHY_CR_ANEG_RST     0x0200  /* Auto-Negotiation Restart */
#define PHY_CR_DPLX         0x0100  /* Duplex Selection (1=Full Duplex) */
#define PHY_CR_COLTST       0x0080  /* Collision Test */

/* Basic Status Register */

#define PHY_SR_T4           0x8000  /* 100BASE-T4 Capable */
#define PHY_SR_TXF          0x4000  /* 100BASE-TX Full Duplex Capable */
#define PHY_SR_TXH          0x2000  /* 100BASE-TX Half Duplex Capable */
#define PHY_SR_10F          0x1000  /* 10BASE-T Full Duplex Capable */
#define PHY_SR_10H          0x0800  /* 10BASE-T Half Duplex Capable */

#define PHY_SR_AN_OK        0x0020  /* Auto-Negotiation Complete */
#define PHY_SR_RF           0x0010  /* Remote Fault Detect */
#define PHY_SR_ANA          0x0008  /* Auto-Negotiation Ability */
#define PHY_SR_LINK         0x0004  /* Link Status */
#define PHY_SR_JAB          0x0002  /* Jabber Detect */
#define PHY_SR_EXT          0x0001  /* Extended Capabilities */

/* Auto-Negotiation Advertisement */
/* Auto-Negotiation Link Partner Ability */

#define PHY_AN_NP           0x8000  /* Next Page capable */
#define PHY_AN_ACK          0x4000  /* Acknowldge (ANLPA-only) */
#define PHY_AN_RF           0x2000  /* Remote Fault */
#define PHY_AN_APAUSE       0x0800  /* Asymmetric PAUSE operation for Full Duplex Links */
#define PHY_AN_PAUSE        0x0400  /* Symmetric PAUSE operation for Full Duplex Links */
#define PHY_AN_T4           0x0200  /* 100BASE-T4 */
#define PHY_AN_TX_FDX       0x0100  /* 100BASE-TX Full Duplex Capable */
#define PHY_AN_TX_HDX       0x0080  /* 100BASE-TX Half Duplex Capable */
#define PHY_AN_10_FDX       0x0040  /* 10BASE-T Full Duplex Capable */
#define PHY_AN_10_HDX       0x0020  /* 10BASE-T Half Duplex Capable */
#define PHY_AN_802_3        0x0001  /* Selector Field: IEEE 802.3 */

/* Control Register 設定データ */

#define HD10    0x0000         /* 10BASE-T   Half Duplex */
#define FD10    PHY_CR_DPLX    /* 10BASE-T   Full Duplex */
#define HD100   PHY_CR_SPEED   /* 100BASE-TX Half Duplex */
#define FD100  (PHY_CR_SPEED|PHY_CR_DPLX)/* 100BASE-TX Full Duplex */

/* Auto-Negotiation Advertisement Register 設定データ */

#define AHD10   PHY_AN_10_HDX  /* 10BASE-T   Half Duplex, Auto-Negotiation */
#define AFD10   PHY_AN_10_FDX  /* 10BASE-T   Full Duplex, Auto-Negotiation */
#define AHD100  PHY_AN_TX_HDX  /* 100BASE-TX Half Duplex, Auto-Negotiation */
#define AFD100  PHY_AN_TX_FDX  /* 100BASE-TX Full Duplex, Auto-Negotiation */

/* Auto-Negoriation Expansion */

#define PHY_ANE_PDF         0x0010  /* Parallel Detection Fault */
#define PHY_ANE_LPNP        0x0008  /* Link Partner Next Page Able */
#define PHY_ANE_NP          0x0004  /* Next Page Able */
#define PHY_ANE_PAGE        0x0002  /* Page Received */
#define PHY_ANE_LPAN        0x0001  /* Link Partner Auto-Negotiation Able */

/* Mode Control/Status */

#define PHY_MCR_EDPWRDOWN   0x2000  /* Enable the Energy Detect Power-Down mode */
#define PHY_MCR_ENERGYON    0x0002  /* Indicates whether energy is detected */

/* Special Modes */

#define PHY_SMD_10H         (0<<5)  /* 10Base-T Half Duplex */
#define PHY_SMD_10F         (1<<5)  /* 10Base-T Full Duplex */
#define PHY_SMD_100H        (2<<5)  /* 100Base-TX Half Duplex */
#define PHY_SMD_100F        (3<<5)  /* 100Base-TX Full Duplex */
#define PHY_SMD_100H_AN     (4<<5)  /* 100Base-TX Half Duplex, Auto-Negotiation enabled */
#define PHY_SMD_REPEATER    (5<<5)  /* Repeater mode */
#define PHY_SMD_ALL         (7<<5)  /* All capable, Auto-Negotiation enabled */
#define PHY_SMD_PHYAD_MASK  0x001f  /* PHY Address / MASK pattern */

/* Special Control/Status Indication */

#define PHY_SCI_VCOFF_LP    0x0400  /* Forces the Receive PLL 10M to lock */
#define PHY_SCI_XPOL        0x0010  /* Polarity state of the 10Base-T */

/* Interrupt Source Flag */
/* Interrupt Mask */

#define PHY_INT_ENERGYON    0x0080  /* ENERGYON genareted */
#define PHY_INT_AN_OK       0x0040  /* Auto-Negotiation complete */
#define PHY_INT_RF          0x0020  /* Remote Fault Detected */
#define PHY_INT_LINK        0x0010  /* Link down */
#define PHY_INT_ANLP_ACK    0x0008  /* Auto-Negotiation LP Acknowledge */
#define PHY_INT_PDF         0x0004  /* Parallel Detection Fault */
#define PHY_INT_AN_PR       0x0002  /* Auto-Negotiation Page Receive */

/* PHY Special Control/Status */

#define PHY_SCR_ANDONE      0x1000  /* Auto-Negotiation done indication */
#define PHY_SCR_10H         (1<<2)  /* 10Mbps half-duplex */
#define PHY_SCR_10F         (5<<2)  /* 10Mbps full-duplex */
#define PHY_SCR_100H        (2<<2)  /* 100Mbps half-duplex */
#define PHY_SCR_100F        (6<<2)  /* 100Mbps full-duplex */

/***************************************
* リンク状態コールバックパラメータ
****************************************/

#define LAN_LNK_OFF     0
#define LAN_LNK_10H     1
#define LAN_LNK_10F     2
#define LAN_LNK_100H    3
#define LAN_LNK_100F    4

/***************************************
* 関数プロトタイプ
****************************************/

ER lan_def_cbk(FP);
ER lan_get_eep(UB, UB *);
ER lan_set_eep(UB, UB);
ER lan_get_mac(UB *);
ER lan_set_mac(UB *);

#ifdef __cplusplus
}
#endif
#endif /* LAN9118_H */
