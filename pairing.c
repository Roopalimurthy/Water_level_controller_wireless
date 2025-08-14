//void Reprogram_Handler()
//{
//   
//        Re_prog();
//}
//void Re_prog(void)
// { 
//     if(Uniq_Code_Gen())     /*start unique code generation*/
//     { // CLRWDT();
//         Secret_Key_Ack();   /*validate key and acknowledge it*/
//     }          
//     SYSTEM_Initialize();   
//     Nrf_TX_RX_Init(RECEIVE);
// }
///*-----------------------------------------------------------------------------*
// *  NAME
// *     Uniq_Code_Gen()
// *
// *  DESCRIPTION
// *     This function generates unique code while increment of timer IOC for every ms.
// *     If dedicated re-prog data receive then it holds unique code into buffer. 
// *     
// *  PARAMETERS
// *      void
// *
// *  RETURNS
// *      Returns nothing 
// *----------------------------------------------------------------------------*/
//bool  Uniq_Code_Gen( void)
//{
//    uint8_t *pv_Read_SPIQueue_u8r=NULL;
//    uint32_t v_RandomNum_u32r;
//    Reset_Delay_Start(&V_UCG_Delay_Start_10s_32r);      /*make sure delay start variable as to be zero, before passing arg*/
//    for(v_RandomNum_u32r=KEY_MIN_VALUE;v_RandomNum_u32r<=KEY_MAX_VALUE ;v_RandomNum_u32r++)
//    {  
//        if(get_DelayStatus(&V_UCG_Delay_Start_10s_32r,V_Delay_Count1_10s_16r)==false)
//        {
//            Blink_Module_Indication_LED(V_Delay_LED_Blink_16r);           /*Module indication blinking will start*/  
//            if(get_NRF_IRQ_IOCStatus())
//            { 
//                if(Remove_SPIQueue(&SPI2Queue,&pv_Read_SPIQueue_u8r))     /*check for IOC */
//                {
//                    pv_Read_SPIQueue_u8r+=11;                             /*Move to 12 byte*/
//                    if((*pv_Read_SPIQueue_u8r == 'S')||(*pv_Read_SPIQueue_u8r == 'U'))  /*If 'S'->indicates normal remote re-prog data*/
//                    {                                                                   /*If 'U'->indicates universal remote re-prog data*/
//                        #if(DEBUG==ENABLE)                              
//                        EUSART_Write('S');
//                        EUSART_Write(' ');
//                        #endif
//                        Updating_Random_Code( v_RandomNum_u32r);          /*Update recent value of v_RandomNum_u32r as a unique code*/           
//                        return true; 
//                    }   
//                
//                    
//                    
//                    
//                    
//                    
//                    
//                    
//                    
//                    
//            }
//        }
//        if(V_UCG_Delay_Start_10s_32r == 0)
//        {
//            return false;              
//        }
//       
//    }
//    return false;
//}
