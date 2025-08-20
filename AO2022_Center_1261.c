/****************************************************************************
 * @file     AO2022_Center_1261.c
 * @version  V1.23
 * @Date     2022/05/30-06:10:25 
 * @brief    NuMicro generated code file
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) 2013-2022 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/********************
MCU:NUC126NE4AE(QFN48)
Pin Configuration:
Pin1:PB.5
Pin2:PB.6
Pin3:PB.7
Pin5:UART0_RXD
Pin7:UART0_TXD
Pin8:PD.2
Pin9:PD.3
Pin11:X32_OUT
Pin12:X32_IN
Pin13:PF.2
Pin14:PD.7
Pin15:I2C1_SCL
Pin16:I2C1_SDA
Pin19:UART2_nCTS
Pin20:UART2_nRTS
Pin21:UART2_TXD
Pin22:UART2_RXD
Pin23:PC.4
Pin24:PE.0
Pin25:ICE_CLK
Pin26:ICE_DAT
Pin27:UART1_nCTS
Pin28:UART1_nRTS
Pin29:UART1_TXD
Pin30:UART1_RXD
Pin35:PF.7
Pin37:PA.3
Pin38:PA.2
Pin39:PA.1
Pin40:PA.0
Pin44:PB.0
Pin45:PB.1
Pin46:PB.2
Pin47:PB.3
Pin48:PB.4
********************/

#include "NUC1261.h"

void AO2022_Center_1261_init_i2c1(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF4MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF4MFP_I2C1_SDA | SYS_GPF_MFPL_PF3MFP_I2C1_SCL);

    return;
}

void AO2022_Center_1261_deinit_i2c1(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF4MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk);

    return;
}

void AO2022_Center_1261_init_ice(void)
{
    SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE7MFP_Msk | SYS_GPE_MFPL_PE6MFP_Msk);
    SYS->GPE_MFPL |= (SYS_GPE_MFPL_PE7MFP_ICE_DAT | SYS_GPE_MFPL_PE6MFP_ICE_CLK);

    return;
}

void AO2022_Center_1261_deinit_ice(void)
{
    SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE7MFP_Msk | SYS_GPE_MFPL_PE6MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pa(void)
{
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA0MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_GPIO | SYS_GPA_MFPL_PA2MFP_GPIO | SYS_GPA_MFPL_PA1MFP_GPIO | SYS_GPA_MFPL_PA0MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pa(void)
{
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA0MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pb(void)
{
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB7MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk | SYS_GPB_MFPL_PB0MFP_Msk);
    SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB7MFP_GPIO | SYS_GPB_MFPL_PB6MFP_GPIO | SYS_GPB_MFPL_PB5MFP_GPIO | SYS_GPB_MFPL_PB4MFP_GPIO | SYS_GPB_MFPL_PB3MFP_GPIO | SYS_GPB_MFPL_PB2MFP_GPIO | SYS_GPB_MFPL_PB1MFP_GPIO | SYS_GPB_MFPL_PB0MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pb(void)
{
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB7MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk | SYS_GPB_MFPL_PB0MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pc(void)
{
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC4MFP_Msk);
    SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC4MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pc(void)
{
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC4MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pd(void)
{
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD7MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD2MFP_Msk);
    SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD7MFP_GPIO | SYS_GPD_MFPL_PD3MFP_GPIO | SYS_GPD_MFPL_PD2MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pd(void)
{
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD7MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD2MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pe(void)
{
    SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE0MFP_Msk);
    SYS->GPE_MFPL |= (SYS_GPE_MFPL_PE0MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pe(void)
{
    SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE0MFP_Msk);

    return;
}

void AO2022_Center_1261_init_pf(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF7MFP_Msk | SYS_GPF_MFPL_PF4MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF7MFP_GPIO | SYS_GPF_MFPL_PF4MFP_GPIO | SYS_GPF_MFPL_PF3MFP_GPIO | SYS_GPF_MFPL_PF2MFP_GPIO);

    return;
}

void AO2022_Center_1261_deinit_pf(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF7MFP_Msk | SYS_GPF_MFPL_PF4MFP_Msk | SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);

    return;
}

void AO2022_Center_1261_init_uart0(void)
{
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk | SYS_GPD_MFPL_PD0MFP_Msk);
    SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD1MFP_UART0_TXD | SYS_GPD_MFPL_PD0MFP_UART0_RXD);

    return;
}

void AO2022_Center_1261_deinit_uart0(void)
{
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk | SYS_GPD_MFPL_PD0MFP_Msk);

    return;
}

void AO2022_Center_1261_init_uart1(void)
{
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE13MFP_Msk | SYS_GPE_MFPH_PE12MFP_Msk | SYS_GPE_MFPH_PE11MFP_Msk | SYS_GPE_MFPH_PE10MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE13MFP_UART1_RXD | SYS_GPE_MFPH_PE12MFP_UART1_TXD | SYS_GPE_MFPH_PE11MFP_UART1_nRTS | SYS_GPE_MFPH_PE10MFP_UART1_nCTS);

    return;
}

void AO2022_Center_1261_deinit_uart1(void)
{
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE13MFP_Msk | SYS_GPE_MFPH_PE12MFP_Msk | SYS_GPE_MFPH_PE11MFP_Msk | SYS_GPE_MFPH_PE10MFP_Msk);

    return;
}

void AO2022_Center_1261_init_uart2(void)
{
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC3MFP_Msk | SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk | SYS_GPC_MFPL_PC0MFP_Msk);
    SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC3MFP_UART2_RXD | SYS_GPC_MFPL_PC2MFP_UART2_TXD | SYS_GPC_MFPL_PC1MFP_UART2_nRTS | SYS_GPC_MFPL_PC0MFP_UART2_nCTS);

    return;
}

void AO2022_Center_1261_deinit_uart2(void)
{
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC3MFP_Msk | SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk | SYS_GPC_MFPL_PC0MFP_Msk);

    return;
}

void AO2022_Center_1261_init_x32(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF1MFP_Msk | SYS_GPF_MFPL_PF0MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF1MFP_X32_IN | SYS_GPF_MFPL_PF0MFP_X32_OUT);

    return;
}

void AO2022_Center_1261_deinit_x32(void)
{
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF1MFP_Msk | SYS_GPF_MFPL_PF0MFP_Msk);

    return;
}

void AO2022_Center_1261_init(void)
{
    //SYS->GPA_MFPL = 0x00000000UL;
    //SYS->GPB_MFPL = 0x00000000UL;
    //SYS->GPC_MFPL = 0x00003333UL;
    //SYS->GPD_MFPL = 0x00000033UL;
    //SYS->GPE_MFPH = 0x00333300UL;
    //SYS->GPE_MFPL = 0x11000000UL;
    //SYS->GPF_MFPL = 0x00033011UL;
    AO2022_Center_1261_init_ice();
    AO2022_Center_1261_init_pa();
    AO2022_Center_1261_init_pb();
    AO2022_Center_1261_init_pc();
    AO2022_Center_1261_init_pd();
    AO2022_Center_1261_init_pe();
    AO2022_Center_1261_init_pf();
    AO2022_Center_1261_init_uart0();
    AO2022_Center_1261_init_uart1();
    AO2022_Center_1261_init_uart2();
    AO2022_Center_1261_init_x32();

    return;
}

void AO2022_Center_1261_deinit(void)
{
    AO2022_Center_1261_deinit_i2c1();
    AO2022_Center_1261_deinit_ice();
    AO2022_Center_1261_deinit_pa();
    AO2022_Center_1261_deinit_pb();
    AO2022_Center_1261_deinit_pc();
    AO2022_Center_1261_deinit_pd();
    AO2022_Center_1261_deinit_pe();
    AO2022_Center_1261_deinit_pf();
    AO2022_Center_1261_deinit_uart0();
    AO2022_Center_1261_deinit_uart1();
    AO2022_Center_1261_deinit_uart2();
    AO2022_Center_1261_deinit_x32();

    return;
}
/*** (C) COPYRIGHT 2013-2022 Nuvoton Technology Corp. ***/
