#include <cstdint>

// This stores the initial value of the stack pointer. It is defined in the linker script. The stack pointer is 
// located at the very end of one of the RAM memories. In the case of the STM32H7, it is located at the end of the
// RAM_D1 memory.
extern char _estack;

// The entry point of the application on a power cycle or restart. It is defined in stm32h7_startup.cpp.
extern "C" void Reset_Handler();

/// The default handler is the default interrupt handler for all interrupts and exceptions in the system. You end up
/// here if an interrupt has triggered and you have no redefined the interrupt handler. You can then view the state
/// of the system with a debugger to figure out why you are here.
extern "C" __attribute__((interrupt, weak))
void Default_Handler() {
    while (true);
}

// Attributes of the default weak definitions of the interrupts handlers in STM32H7.
// https://gcc.gnu.org/onlinedocs/gcc-11.1.0/gcc/Common-Function-Attributes.html#Common-Function-Attributes
// TODO: By using the alias, it can be hard to tell which interrupt was executed if you end up in it. I suggest
// you should define a weak version of each interrupt each with its own while loop so you can tell which one you got
// trapped in.
#define DEFAULT_INTERRUPT extern "C" __attribute__((interrupt, weak, alias("Default_Handler")))

/*****************************************************************************/
/******** DEFAULT INTERRUPT DEFINITIONS **************************************/
/*****************************************************************************/     

DEFAULT_INTERRUPT void NMI_Handler();                       // NMI_Handler
DEFAULT_INTERRUPT void HardFault_Handler();                 // HardFault_Handler
DEFAULT_INTERRUPT void MemManage_Handler();                 // MemManage_Handler
DEFAULT_INTERRUPT void BusFault_Handler();                  // BusFault_Handler
DEFAULT_INTERRUPT void UsageFault_Handler();                // UsageFault_Handler
DEFAULT_INTERRUPT void SVC_Handler();                       // SVC_Handler
DEFAULT_INTERRUPT void DebugMon_Handler();                  // DebugMon_Handler
DEFAULT_INTERRUPT void PendSV_Handler();                    // PendSV_Handler
DEFAULT_INTERRUPT void SysTick_Handler();                   // SysTick_Handler

DEFAULT_INTERRUPT void WWDG_IRQHandler();                   // Window WatchDog                                                
DEFAULT_INTERRUPT void PVD_AVD_IRQHandler();                // PVD/AVD through EXTI Line detection                      
DEFAULT_INTERRUPT void TAMP_STAMP_IRQHandler();             // Tamper and TimeStamps through the EXTI line        
DEFAULT_INTERRUPT void RTC_WKUP_IRQHandler();               // RTC Wakeup through the EXTI line                      
DEFAULT_INTERRUPT void FLASH_IRQHandler();                  // FLASH                                                                 
DEFAULT_INTERRUPT void RCC_IRQHandler();                    // RCC                                                                    
DEFAULT_INTERRUPT void EXTI0_IRQHandler();                  // EXTI Line0                                           
DEFAULT_INTERRUPT void EXTI1_IRQHandler ();                 // EXTI Line1                                             
DEFAULT_INTERRUPT void EXTI2_IRQHandler();                  // EXTI Line2                                             
DEFAULT_INTERRUPT void EXTI3_IRQHandler();                  // EXTI Line3                                             
DEFAULT_INTERRUPT void EXTI4_IRQHandler();                  // EXTI Line4                                             
DEFAULT_INTERRUPT void DMA1_Stream0_IRQHandler();           // DMA1 Stream 0                                  
DEFAULT_INTERRUPT void DMA1_Stream1_IRQHandler();           // DMA1 Stream 1                                   
DEFAULT_INTERRUPT void DMA1_Stream2_IRQHandler();           // DMA1 Stream 2                                   
DEFAULT_INTERRUPT void DMA1_Stream3_IRQHandler();           // DMA1 Stream 3                                   
DEFAULT_INTERRUPT void DMA1_Stream4_IRQHandler();           // DMA1 Stream 4                                   
DEFAULT_INTERRUPT void DMA1_Stream5_IRQHandler();           // DMA1 Stream 5                                   
DEFAULT_INTERRUPT void DMA1_Stream6_IRQHandler();           // DMA1 Stream 6                                   
DEFAULT_INTERRUPT void ADC_IRQHandler();                    // ADC1, ADC2 and ADC3s                            
DEFAULT_INTERRUPT void FDCAN1_IT0_IRQHandler();             // FDCAN1 interrupt line 0                          
DEFAULT_INTERRUPT void FDCAN2_IT0_IRQHandler();             // FDCAN2 interrupt line 0                          
DEFAULT_INTERRUPT void FDCAN1_IT1_IRQHandler();             // FDCAN1 interrupt line 1                          
DEFAULT_INTERRUPT void FDCAN2_IT1_IRQHandler();             // FDCAN2 interrupt line 1                          
DEFAULT_INTERRUPT void EXTI9_5_IRQHandler();                // External Line[9:5]s                                    
DEFAULT_INTERRUPT void TIM1_BRK_IRQHandler();               // TIM1 Break interrupt                  
DEFAULT_INTERRUPT void TIM1_UP_IRQHandler();                // TIM1 Update interrupt                 
DEFAULT_INTERRUPT void TIM1_TRG_COM_IRQHandler();           // TIM1 Trigger and Commutation interrupt 
DEFAULT_INTERRUPT void TIM1_CC_IRQHandler();                // TIM1 Capture Compare                                   
DEFAULT_INTERRUPT void TIM2_IRQHandler();                   // TIM2                                            
DEFAULT_INTERRUPT void TIM3_IRQHandler();                   // TIM3                                            
DEFAULT_INTERRUPT void TIM4_IRQHandler();                   // TIM4                                            
DEFAULT_INTERRUPT void I2C1_EV_IRQHandler();                // I2C1 Event                                             
DEFAULT_INTERRUPT void I2C1_ER_IRQHandler();                // I2C1 Error                                             
DEFAULT_INTERRUPT void I2C2_EV_IRQHandler();                // I2C2 Event                                             
DEFAULT_INTERRUPT void I2C2_ER_IRQHandler();                // I2C2 Error                                               
DEFAULT_INTERRUPT void SPI1_IRQHandler();                   // SPI1                                            
DEFAULT_INTERRUPT void SPI2_IRQHandler();                   // SPI2                                            
DEFAULT_INTERRUPT void USART1_IRQHandler();                 // USART1                                          
DEFAULT_INTERRUPT void USART2_IRQHandler();                 // USART2                                          
DEFAULT_INTERRUPT void USART3_IRQHandler();                 // USART3                                          
DEFAULT_INTERRUPT void EXTI15_10_IRQHandler();              // External Line[15:10]s                                  
DEFAULT_INTERRUPT void RTC_Alarm_IRQHandler();              // RTC Alarm (A and B) through EXTI Line                                      
DEFAULT_INTERRUPT void TIM8_BRK_TIM12_IRQHandler();         // TIM8 Break and TIM12                  
DEFAULT_INTERRUPT void TIM8_UP_TIM13_IRQHandler();          // TIM8 Update and TIM13                 
DEFAULT_INTERRUPT void TIM8_TRG_COM_TIM14_IRQHandler();     // TIM8 Trigger and Commutation and TIM14 
DEFAULT_INTERRUPT void TIM8_CC_IRQHandler();                // TIM8 Capture Compare                                   
DEFAULT_INTERRUPT void DMA1_Stream7_IRQHandler();           // DMA1 Stream7                                           
DEFAULT_INTERRUPT void FMC_IRQHandler();                    // FMC                                             
DEFAULT_INTERRUPT void SDMMC1_IRQHandler();                 // SDMMC1                                          
DEFAULT_INTERRUPT void TIM5_IRQHandler();                   // TIM5                                            
DEFAULT_INTERRUPT void SPI3_IRQHandler();                   // SPI3                                            
DEFAULT_INTERRUPT void UART4_IRQHandler();                  // UART4                                           
DEFAULT_INTERRUPT void UART5_IRQHandler();                  // UART5                                           
DEFAULT_INTERRUPT void TIM6_DAC_IRQHandler();               // TIM6 and DAC1&2 underrun errors                    
DEFAULT_INTERRUPT void TIM7_IRQHandler();                   // TIM7                         
DEFAULT_INTERRUPT void DMA2_Stream0_IRQHandler();           // DMA2 Stream 0                                   
DEFAULT_INTERRUPT void DMA2_Stream1_IRQHandler();           // DMA2 Stream 1                                   
DEFAULT_INTERRUPT void DMA2_Stream2_IRQHandler();           // DMA2 Stream 2                                   
DEFAULT_INTERRUPT void DMA2_Stream3_IRQHandler();           // DMA2 Stream 3                                   
DEFAULT_INTERRUPT void DMA2_Stream4_IRQHandler();           // DMA2 Stream 4                                   
DEFAULT_INTERRUPT void ETH_IRQHandler();                    // Ethernet                                        
DEFAULT_INTERRUPT void ETH_WKUP_IRQHandler();               // Ethernet Wakeup through EXTI line                      
DEFAULT_INTERRUPT void FDCAN_CAL_IRQHandler();              // FDCAN calibration unit interrupt                                                               
DEFAULT_INTERRUPT void DMA2_Stream5_IRQHandler();           // DMA2 Stream 5                                   
DEFAULT_INTERRUPT void DMA2_Stream6_IRQHandler();           // DMA2 Stream 6                                   
DEFAULT_INTERRUPT void DMA2_Stream7_IRQHandler();           // DMA2 Stream 7                                   
DEFAULT_INTERRUPT void USART6_IRQHandler();                 // USART6                                           
DEFAULT_INTERRUPT void I2C3_EV_IRQHandler();                // I2C3 event                                             
DEFAULT_INTERRUPT void I2C3_ER_IRQHandler();                // I2C3 error                                             
DEFAULT_INTERRUPT void OTG_HS_EP1_OUT_IRQHandler();         // USB OTG HS End Point 1 Out                      
DEFAULT_INTERRUPT void OTG_HS_EP1_IN_IRQHandler();          // USB OTG HS End Point 1 In                       
DEFAULT_INTERRUPT void OTG_HS_WKUP_IRQHandler();            // USB OTG HS Wakeup through EXTI                          
DEFAULT_INTERRUPT void OTG_HS_IRQHandler();                 // USB OTG HS                                      
DEFAULT_INTERRUPT void DCMI_IRQHandler();                   // DCMI                                                             
DEFAULT_INTERRUPT void RNG_IRQHandler();                    // Rng                          
DEFAULT_INTERRUPT void FPU_IRQHandler();                    // FPU                          
DEFAULT_INTERRUPT void UART7_IRQHandler();                  // UART7                              
DEFAULT_INTERRUPT void UART8_IRQHandler();                  // UART8                        
DEFAULT_INTERRUPT void SPI4_IRQHandler();                   // SPI4                         
DEFAULT_INTERRUPT void SPI5_IRQHandler();                   // SPI5                         
DEFAULT_INTERRUPT void SPI6_IRQHandler();                   // SPI6                         
DEFAULT_INTERRUPT void SAI1_IRQHandler();                   // SAI1                         
DEFAULT_INTERRUPT void LTDC_IRQHandler();                   // LTDC                         
DEFAULT_INTERRUPT void LTDC_ER_IRQHandler();                // LTDC error                   
DEFAULT_INTERRUPT void DMA2D_IRQHandler();                  // DMA2D                        
DEFAULT_INTERRUPT void SAI2_IRQHandler();                   // SAI2                         
DEFAULT_INTERRUPT void QUADSPI_IRQHandler();                // QUADSPI                      
DEFAULT_INTERRUPT void LPTIM1_IRQHandler();                 // LPTIM1                       
DEFAULT_INTERRUPT void CEC_IRQHandler();                    // HDMI_CEC                     
DEFAULT_INTERRUPT void I2C4_EV_IRQHandler();                // I2C4 Event                   
DEFAULT_INTERRUPT void I2C4_ER_IRQHandler();                // I2C4 Error                   
DEFAULT_INTERRUPT void SPDIF_RX_IRQHandler();               // SPDIF_RX                      
DEFAULT_INTERRUPT void OTG_FS_EP1_OUT_IRQHandler();         // USB OTG FS End Point 1 Out      
DEFAULT_INTERRUPT void OTG_FS_EP1_IN_IRQHandler();          // USB OTG FS End Point 1 In       
DEFAULT_INTERRUPT void OTG_FS_WKUP_IRQHandler();            // USB OTG FS Wakeup through EXTI   
DEFAULT_INTERRUPT void OTG_FS_IRQHandler();                 // USB OTG FS                   
DEFAULT_INTERRUPT void DMAMUX1_OVR_IRQHandler();            // DMAMUX1 Overrun interrupt      
DEFAULT_INTERRUPT void HRTIM1_Master_IRQHandler();          // HRTIM Master Timer global Interrupt 
DEFAULT_INTERRUPT void HRTIM1_TIMA_IRQHandler();            // HRTIM Timer A global Interrupt   
DEFAULT_INTERRUPT void HRTIM1_TIMB_IRQHandler();            // HRTIM Timer B global Interrupt   
DEFAULT_INTERRUPT void HRTIM1_TIMC_IRQHandler();            // HRTIM Timer C global Interrupt   
DEFAULT_INTERRUPT void HRTIM1_TIMD_IRQHandler();            // HRTIM Timer D global Interrupt   
DEFAULT_INTERRUPT void HRTIM1_TIME_IRQHandler();            // HRTIM Timer E global Interrupt   
DEFAULT_INTERRUPT void HRTIM1_FLT_IRQHandler();             // HRTIM Fault global Interrupt    
DEFAULT_INTERRUPT void DFSDM1_FLT0_IRQHandler();            // DFSDM Filter0 Interrupt        
DEFAULT_INTERRUPT void DFSDM1_FLT1_IRQHandler();            // DFSDM Filter1 Interrupt             
DEFAULT_INTERRUPT void DFSDM1_FLT2_IRQHandler();            // DFSDM Filter2 Interrupt            
DEFAULT_INTERRUPT void DFSDM1_FLT3_IRQHandler();            // DFSDM Filter3 Interrupt           
DEFAULT_INTERRUPT void SAI3_IRQHandler();                   // SAI3 global Interrupt             
DEFAULT_INTERRUPT void SWPMI1_IRQHandler();                 // Serial Wire Interface 1 global interrupt 
DEFAULT_INTERRUPT void TIM15_IRQHandler();                  // TIM15 global Interrupt              
DEFAULT_INTERRUPT void TIM16_IRQHandler();                  // TIM16 global Interrupt            
DEFAULT_INTERRUPT void TIM17_IRQHandler();                  // TIM17 global Interrupt            
DEFAULT_INTERRUPT void MDIOS_WKUP_IRQHandler();             // MDIOS Wakeup    reinterpret_cast<uintptr_t>(         
DEFAULT_INTERRUPT void MDIOS_IRQHandler();                  // MDIOS global Interrupt          
DEFAULT_INTERRUPT void JPEG_IRQHandler();                   // JPEG global Interrupt           
DEFAULT_INTERRUPT void MDMA_IRQHandler();                   // MDMA global Interrupt           
DEFAULT_INTERRUPT void SDMMC2_IRQHandler();                 // SDMMC2 global Interrupt         
DEFAULT_INTERRUPT void HSEM1_IRQHandler();                  // HSEM1 global Interrupt             
DEFAULT_INTERRUPT void ADC3_IRQHandler();                   // ADC3 global Interrupt           
DEFAULT_INTERRUPT void DMAMUX2_OVR_IRQHandler();            // DMAMUX Overrun interrupt        
DEFAULT_INTERRUPT void BDMA_Channel0_IRQHandler();          // BDMA Channel 0 global Interrupt 
DEFAULT_INTERRUPT void BDMA_Channel1_IRQHandler();          // BDMA Channel 1 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel2_IRQHandler();          // BDMA Channel 2 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel3_IRQHandler();          // BDMA Channel 3 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel4_IRQHandler();          // BDMA Channel 4 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel5_IRQHandler();          // BDMA Channel 5 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel6_IRQHandler();          // BDMA Channel 6 global Interrupt  
DEFAULT_INTERRUPT void BDMA_Channel7_IRQHandler();          // BDMA Channel 7 global Interrupt  
DEFAULT_INTERRUPT void COMP1_IRQHandler();                  // COMP1 global Interrupt               
DEFAULT_INTERRUPT void LPTIM2_IRQHandler();                 // LP TIM2 global interrupt         
DEFAULT_INTERRUPT void LPTIM3_IRQHandler();                 // LP TIM3 global interrupt        
DEFAULT_INTERRUPT void LPTIM4_IRQHandler();                 // LP TIM4 global interrupt        
DEFAULT_INTERRUPT void LPTIM5_IRQHandler();                 // LP TIM5 global interrupt        
DEFAULT_INTERRUPT void LPUART1_IRQHandler();                // LP UART1 interrupt                  
DEFAULT_INTERRUPT void CRS_IRQHandler();                    // Clock Recovery Global Interrupt  
DEFAULT_INTERRUPT void ECC_IRQHandler();                    // ECC diagnostic Global Interrupt  
DEFAULT_INTERRUPT void SAI4_IRQHandler();                   // SAI4 global interrupt                
DEFAULT_INTERRUPT void WAKEUP_PIN_IRQHandler();             // Interrupt for all 6 wake-up pins 

/*****************************************************************************/
/******** VECTOR TABLE LOCATED AT THE START OF THE FLASH MEMORY **************/
/*****************************************************************************/

__attribute__((section(".isr_vector")))
const volatile uintptr_t g_pfnVectors[]{
    // stack pointer initialization
    reinterpret_cast<uintptr_t>(&_estack),

    // entry point
    reinterpret_cast<uintptr_t>(Reset_Handler),

    // exceptions
    reinterpret_cast<uintptr_t>(NMI_Handler),              // NMI_Handler
    reinterpret_cast<uintptr_t>(HardFault_Handler),        // HardFault_Handler
    reinterpret_cast<uintptr_t>(MemManage_Handler),        // MemManage_Handler
    reinterpret_cast<uintptr_t>(BusFault_Handler),         // BusFault_Handler
    reinterpret_cast<uintptr_t>(UsageFault_Handler),       // UsageFault_Handler
    reinterpret_cast<uintptr_t>(nullptr),                  // 0
    reinterpret_cast<uintptr_t>(nullptr),                  // 0
    reinterpret_cast<uintptr_t>(nullptr),                  // 0
    reinterpret_cast<uintptr_t>(nullptr),                  // 0
    reinterpret_cast<uintptr_t>(SVC_Handler),              // SVC_Handler
    reinterpret_cast<uintptr_t>(DebugMon_Handler),         // DebugMon_Handler
    reinterpret_cast<uintptr_t>(nullptr),                  // 0
    reinterpret_cast<uintptr_t>(PendSV_Handler),           // PendSV_Handler
    reinterpret_cast<uintptr_t>(SysTick_Handler),          // SysTick_Handler

    // External Interrupts
    reinterpret_cast<uintptr_t>(WWDG_IRQHandler),                   // Window WatchDog                                                      
    reinterpret_cast<uintptr_t>(PVD_AVD_IRQHandler),                // PVD/AVD through EXTI Line detection                         
    reinterpret_cast<uintptr_t>(TAMP_STAMP_IRQHandler),             // Tamper and TimeStamps through the EXTI line             
    reinterpret_cast<uintptr_t>(RTC_WKUP_IRQHandler),               // RTC Wakeup through the EXTI line                       
    reinterpret_cast<uintptr_t>(FLASH_IRQHandler),                  // FLASH                                                                  
    reinterpret_cast<uintptr_t>(RCC_IRQHandler),                    // RCC                                                                      
    reinterpret_cast<uintptr_t>(EXTI0_IRQHandler),                  // EXTI Line0                                           
    reinterpret_cast<uintptr_t>(EXTI1_IRQHandler),                  // EXTI Line1                                             
    reinterpret_cast<uintptr_t>(EXTI2_IRQHandler),                  // EXTI Line2                                             
    reinterpret_cast<uintptr_t>(EXTI3_IRQHandler),                  // EXTI Line3                                             
    reinterpret_cast<uintptr_t>(EXTI4_IRQHandler),                  // EXTI Line4                                             
    reinterpret_cast<uintptr_t>(DMA1_Stream0_IRQHandler),           // DMA1 Stream 0                                  
    reinterpret_cast<uintptr_t>(DMA1_Stream1_IRQHandler),           // DMA1 Stream 1                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream2_IRQHandler),           // DMA1 Stream 2                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream3_IRQHandler),           // DMA1 Stream 3                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream4_IRQHandler),           // DMA1 Stream 4                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream5_IRQHandler),           // DMA1 Stream 5                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream6_IRQHandler),           // DMA1 Stream 6                                   
    reinterpret_cast<uintptr_t>(ADC_IRQHandler),                    // ADC1, ADC2 and ADC3s                            
    reinterpret_cast<uintptr_t>(FDCAN1_IT0_IRQHandler),             // FDCAN1 interrupt line 0                          
    reinterpret_cast<uintptr_t>(FDCAN2_IT0_IRQHandler),             // FDCAN2 interrupt line 0                          
    reinterpret_cast<uintptr_t>(FDCAN1_IT1_IRQHandler),             // FDCAN1 interrupt line 1                          
    reinterpret_cast<uintptr_t>(FDCAN2_IT1_IRQHandler),             // FDCAN2 interrupt line 1                          
    reinterpret_cast<uintptr_t>(EXTI9_5_IRQHandler),                // External Line[9:5]s                                    
    reinterpret_cast<uintptr_t>(TIM1_BRK_IRQHandler),               // TIM1 Break interrupt                  
    reinterpret_cast<uintptr_t>(TIM1_UP_IRQHandler),                // TIM1 Update interrupt                 
    reinterpret_cast<uintptr_t>(TIM1_TRG_COM_IRQHandler),           // TIM1 Trigger and Commutation interrupt 
    reinterpret_cast<uintptr_t>(TIM1_CC_IRQHandler),                // TIM1 Capture Compare                                   
    reinterpret_cast<uintptr_t>(TIM2_IRQHandler),                   // TIM2                                            
    reinterpret_cast<uintptr_t>(TIM3_IRQHandler),                   // TIM3                                            
    reinterpret_cast<uintptr_t>(TIM4_IRQHandler),                   // TIM4                                            
    reinterpret_cast<uintptr_t>(I2C1_EV_IRQHandler),                // I2C1 Event                                             
    reinterpret_cast<uintptr_t>(I2C1_ER_IRQHandler),                // I2C1 Error                                             
    reinterpret_cast<uintptr_t>(I2C2_EV_IRQHandler),                // I2C2 Event                                             
    reinterpret_cast<uintptr_t>(I2C2_ER_IRQHandler),                // I2C2 Error                                               
    reinterpret_cast<uintptr_t>(SPI1_IRQHandler),                   // SPI1                                            
    reinterpret_cast<uintptr_t>(SPI2_IRQHandler),                   // SPI2                                            
    reinterpret_cast<uintptr_t>(USART1_IRQHandler),                 // USART1                                          
    reinterpret_cast<uintptr_t>(USART2_IRQHandler),                 // USART2                                          
    reinterpret_cast<uintptr_t>(USART3_IRQHandler),                 // USART3                                          
    reinterpret_cast<uintptr_t>(EXTI15_10_IRQHandler),              // External Line[15:10]s                                  
    reinterpret_cast<uintptr_t>(RTC_Alarm_IRQHandler),              // RTC Alarm (A and B) through EXTI Line                  
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                            
    reinterpret_cast<uintptr_t>(TIM8_BRK_TIM12_IRQHandler),         // TIM8 Break and TIM12                  
    reinterpret_cast<uintptr_t>(TIM8_UP_TIM13_IRQHandler),          // TIM8 Update and TIM13                 
    reinterpret_cast<uintptr_t>(TIM8_TRG_COM_TIM14_IRQHandler),     // TIM8 Trigger and Commutation and TIM14 
    reinterpret_cast<uintptr_t>(TIM8_CC_IRQHandler),                // TIM8 Capture Compare                                   
    reinterpret_cast<uintptr_t>(DMA1_Stream7_IRQHandler),           // DMA1 Stream7                                           
    reinterpret_cast<uintptr_t>(FMC_IRQHandler),                    // FMC                                             
    reinterpret_cast<uintptr_t>(SDMMC1_IRQHandler),                 // SDMMC1                                          
    reinterpret_cast<uintptr_t>(TIM5_IRQHandler),                   // TIM5                                            
    reinterpret_cast<uintptr_t>(SPI3_IRQHandler),                   // SPI3                                            
    reinterpret_cast<uintptr_t>(UART4_IRQHandler),                  // UART4                                           
    reinterpret_cast<uintptr_t>(UART5_IRQHandler),                  // UART5                                           
    reinterpret_cast<uintptr_t>(TIM6_DAC_IRQHandler),               // TIM6 and DAC1&2 underrun errors                    
    reinterpret_cast<uintptr_t>(TIM7_IRQHandler),                   // TIM7                         
    reinterpret_cast<uintptr_t>(DMA2_Stream0_IRQHandler),           // DMA2 Stream 0                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream1_IRQHandler),           // DMA2 Stream 1                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream2_IRQHandler),           // DMA2 Stream 2                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream3_IRQHandler),           // DMA2 Stream 3                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream4_IRQHandler),           // DMA2 Stream 4                                   
    reinterpret_cast<uintptr_t>(ETH_IRQHandler),                    // Ethernet                                        
    reinterpret_cast<uintptr_t>(ETH_WKUP_IRQHandler),               // Ethernet Wakeup through EXTI line                      
    reinterpret_cast<uintptr_t>(FDCAN_CAL_IRQHandler),              // FDCAN calibration unit interrupt                                               
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                               
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                            
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                                
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                       
    reinterpret_cast<uintptr_t>(DMA2_Stream5_IRQHandler),           // DMA2 Stream 5                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream6_IRQHandler),           // DMA2 Stream 6                                   
    reinterpret_cast<uintptr_t>(DMA2_Stream7_IRQHandler),           // DMA2 Stream 7                                   
    reinterpret_cast<uintptr_t>(USART6_IRQHandler),                 // USART6                                           
    reinterpret_cast<uintptr_t>(I2C3_EV_IRQHandler),                // I2C3 event                                             
    reinterpret_cast<uintptr_t>(I2C3_ER_IRQHandler),                // I2C3 error                                             
    reinterpret_cast<uintptr_t>(OTG_HS_EP1_OUT_IRQHandler),         // USB OTG HS End Point 1 Out                      
    reinterpret_cast<uintptr_t>(OTG_HS_EP1_IN_IRQHandler),          // USB OTG HS End Point 1 In                       
    reinterpret_cast<uintptr_t>(OTG_HS_WKUP_IRQHandler),            // USB OTG HS Wakeup through EXTI                          
    reinterpret_cast<uintptr_t>(OTG_HS_IRQHandler),                 // USB OTG HS                                      
    reinterpret_cast<uintptr_t>(DCMI_IRQHandler),                   // DCMI                                            
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                                        
    reinterpret_cast<uintptr_t>(RNG_IRQHandler),                    // Rng                          
    reinterpret_cast<uintptr_t>(FPU_IRQHandler),                    // FPU                          
    reinterpret_cast<uintptr_t>(UART7_IRQHandler),                  // UART7                              
    reinterpret_cast<uintptr_t>(UART8_IRQHandler),                  // UART8                        
    reinterpret_cast<uintptr_t>(SPI4_IRQHandler),                   // SPI4                         
    reinterpret_cast<uintptr_t>(SPI5_IRQHandler),                   // SPI5                         
    reinterpret_cast<uintptr_t>(SPI6_IRQHandler),                   // SPI6                         
    reinterpret_cast<uintptr_t>(SAI1_IRQHandler),                   // SAI1                         
    reinterpret_cast<uintptr_t>(LTDC_IRQHandler),                   // LTDC                         
    reinterpret_cast<uintptr_t>(LTDC_ER_IRQHandler),                // LTDC error                   
    reinterpret_cast<uintptr_t>(DMA2D_IRQHandler),                  // DMA2D                        
    reinterpret_cast<uintptr_t>(SAI2_IRQHandler),                   // SAI2                         
    reinterpret_cast<uintptr_t>(QUADSPI_IRQHandler),                // QUADSPI                      
    reinterpret_cast<uintptr_t>(LPTIM1_IRQHandler),                 // LPTIM1                       
    reinterpret_cast<uintptr_t>(CEC_IRQHandler),                    // HDMI_CEC                     
    reinterpret_cast<uintptr_t>(I2C4_EV_IRQHandler),                // I2C4 Event                   
    reinterpret_cast<uintptr_t>(I2C4_ER_IRQHandler),                // I2C4 Error                   
    reinterpret_cast<uintptr_t>(SPDIF_RX_IRQHandler),               // SPDIF_RX                      
    reinterpret_cast<uintptr_t>(OTG_FS_EP1_OUT_IRQHandler),         // USB OTG FS End Point 1 Out      
    reinterpret_cast<uintptr_t>(OTG_FS_EP1_IN_IRQHandler),          // USB OTG FS End Point 1 In       
    reinterpret_cast<uintptr_t>(OTG_FS_WKUP_IRQHandler),            // USB OTG FS Wakeup through EXTI   
    reinterpret_cast<uintptr_t>(OTG_FS_IRQHandler),                 // USB OTG FS                   
    reinterpret_cast<uintptr_t>(DMAMUX1_OVR_IRQHandler),            // DMAMUX1 Overrun interrupt      
    reinterpret_cast<uintptr_t>(HRTIM1_Master_IRQHandler),          // HRTIM Master Timer global Interrupt 
    reinterpret_cast<uintptr_t>(HRTIM1_TIMA_IRQHandler),            // HRTIM Timer A global Interrupt   
    reinterpret_cast<uintptr_t>(HRTIM1_TIMB_IRQHandler),            // HRTIM Timer B global Interrupt   
    reinterpret_cast<uintptr_t>(HRTIM1_TIMC_IRQHandler),            // HRTIM Timer C global Interrupt   
    reinterpret_cast<uintptr_t>(HRTIM1_TIMD_IRQHandler),            // HRTIM Timer D global Interrupt   
    reinterpret_cast<uintptr_t>(HRTIM1_TIME_IRQHandler),            // HRTIM Timer E global Interrupt   
    reinterpret_cast<uintptr_t>(HRTIM1_FLT_IRQHandler),             // HRTIM Fault global Interrupt    
    reinterpret_cast<uintptr_t>(DFSDM1_FLT0_IRQHandler),            // DFSDM Filter0 Interrupt        
    reinterpret_cast<uintptr_t>(DFSDM1_FLT1_IRQHandler),            // DFSDM Filter1 Interrupt             
    reinterpret_cast<uintptr_t>(DFSDM1_FLT2_IRQHandler),            // DFSDM Filter2 Interrupt            
    reinterpret_cast<uintptr_t>(DFSDM1_FLT3_IRQHandler),            // DFSDM Filter3 Interrupt           
    reinterpret_cast<uintptr_t>(SAI3_IRQHandler),                   // SAI3 global Interrupt             
    reinterpret_cast<uintptr_t>(SWPMI1_IRQHandler),                 // Serial Wire Interface 1 global interrupt 
    reinterpret_cast<uintptr_t>(TIM15_IRQHandler),                  // TIM15 global Interrupt              
    reinterpret_cast<uintptr_t>(TIM16_IRQHandler),                  // TIM16 global Interrupt            
    reinterpret_cast<uintptr_t>(TIM17_IRQHandler),                  // TIM17 global Interrupt            
    reinterpret_cast<uintptr_t>(MDIOS_WKUP_IRQHandler),             // MDIOS Wakeup Interrupt          
    reinterpret_cast<uintptr_t>(MDIOS_IRQHandler),                  // MDIOS global Interrupt          
    reinterpret_cast<uintptr_t>(JPEG_IRQHandler),                   // JPEG global Interrupt           
    reinterpret_cast<uintptr_t>(MDMA_IRQHandler),                   // MDMA global Interrupt           
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                        
    reinterpret_cast<uintptr_t>(SDMMC2_IRQHandler),                 // SDMMC2 global Interrupt         
    reinterpret_cast<uintptr_t>(HSEM1_IRQHandler),                  // HSEM1 global Interrupt          
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                        
    reinterpret_cast<uintptr_t>(ADC3_IRQHandler),                   // ADC3 global Interrupt           
    reinterpret_cast<uintptr_t>(DMAMUX2_OVR_IRQHandler),            // DMAMUX Overrun interrupt        
    reinterpret_cast<uintptr_t>(BDMA_Channel0_IRQHandler),          // BDMA Channel 0 global Interrupt 
    reinterpret_cast<uintptr_t>(BDMA_Channel1_IRQHandler),          // BDMA Channel 1 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel2_IRQHandler),          // BDMA Channel 2 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel3_IRQHandler),          // BDMA Channel 3 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel4_IRQHandler),          // BDMA Channel 4 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel5_IRQHandler),          // BDMA Channel 5 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel6_IRQHandler),          // BDMA Channel 6 global Interrupt  
    reinterpret_cast<uintptr_t>(BDMA_Channel7_IRQHandler),          // BDMA Channel 7 global Interrupt  
    reinterpret_cast<uintptr_t>(COMP1_IRQHandler),                  // COMP1 global Interrupt               
    reinterpret_cast<uintptr_t>(LPTIM2_IRQHandler),                 // LP TIM2 global interrupt         
    reinterpret_cast<uintptr_t>(LPTIM3_IRQHandler),                 // LP TIM3 global interrupt        
    reinterpret_cast<uintptr_t>(LPTIM4_IRQHandler),                 // LP TIM4 global interrupt        
    reinterpret_cast<uintptr_t>(LPTIM5_IRQHandler),                 // LP TIM5 global interrupt        
    reinterpret_cast<uintptr_t>(LPUART1_IRQHandler),                // LP UART1 interrupt              
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                        
    reinterpret_cast<uintptr_t>(CRS_IRQHandler),                    // Clock Recovery Global Interrupt  
    reinterpret_cast<uintptr_t>(ECC_IRQHandler),                    // ECC diagnostic Global Interrupt  
    reinterpret_cast<uintptr_t>(SAI4_IRQHandler),                   // SAI4 global interrupt            
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                         
    reinterpret_cast<uintptr_t>(nullptr),                           // Reserved                         
    reinterpret_cast<uintptr_t>(WAKEUP_PIN_IRQHandler)              // Interrupt for all 6 wake-up pins 
};
