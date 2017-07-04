#ifndef __WISOL_H_
#define __WISOL_H_


#define CID_SET_ACTIVATION                                      "30"
#define CID_SET_ACTIVATION_OPTION1_OTAA                         "otaa"
#define CID_SET_ACTIVATION_OPTION1_ABP                          "abp"

#define CID_TX_MESSAGE                      					"31"
#define CID_TX_MESSAGE_OPTION1_DR0          					"DR 0"
#define CID_TX_MESSAGE_OPTION1_DR1          					"DR 1"
#define CID_TX_MESSAGE_OPTION1_DR2          					"DR 2"
#define CID_TX_MESSAGE_OPTION2_CNF          					"cnf"
#define CID_TX_MESSAGE_OPTION2_UNCNF        					"uncnf"
#define CID_TX_MESSAGE_OPTION3              					"Fport:%d" // 1~221

#define CID_SET_REPORT_TIME                 					"32"
#define CID_SET_REPORT_TIME_OPTION1         					"Report time:%d" // 1~2^32

#define CID_SET_APP_EUI                     					"33"
#define CID_SET_APP_EUI_OPTION1             					"%s"    // 8byte Hex-decimal AppEUI

#define CID_GET_REPORT_TIME                 					"34"
#define CID_SET_DATA_RATE                   					"35"
#define CID_SET_DATA_RATE_OPTION1_0         					"0" //SF12/125Khz
#define CID_SET_DATA_RATE_OPTION1_1         					"1" //SF11/125Khz
#define CID_SET_DATA_RATE_OPTION1_2         					"2" //SF10/125Khz
#define CID_SET_DATA_RATE_OPTION1_3         					"3" //SF9/125Khz
#define CID_SET_DATA_RATE_OPTION1_4         					"4" //SF8/125Khz
#define CID_SET_DATA_RATE_OPTION1_5         					"5" //SF7/125Khz    

#define CID_SET_ADR                         					"36"
#define CID_SET_ADR_OPTION1_ON              					"on" 
#define CID_SET_ADR_OPTION1_OFF             					"off" 

#define CID_SET_RETX                        					"37"
#define CID_SET_RETX_OPTION1_ON             					"ReTx:%d" 

#define CID_SET_LINK_CHECK_REQUEST          					"38"

#define CID_ENHANCE_PROVISIONING_ONOFF            				"3B"
#define CID_ENHANCE_PROVISIONING_ONOFF_OPTION1_0  				"0"     // provisioning off
#define CID_ENHANCE_PROVISIONING_ONOFF_OPTION1_1  				"1"     // provisioning on

#define CID_ENHANCE_PROVISIONING_NONEDONE            			"3c"
#define CID_ENHANCE_PROVISIONING_NONEDONE_OPTION1_0  			"0"     // provisioning none
#define CID_ENHANCE_PROVISIONING_NONEDONE_OPTION1_1  			"1"     // provisioning done

#define CID_GET_DEVICE_EUI                      				"3F"
#define CID_GET_APP_EUI                         				"40"
#define CID_GET_TX_DATA_RATE                    				"42"
#define CID_GET_ADR                             				"44"
#define CID_GET_RETX                            				"45"
#define CID_GET_RX1_DELAY                       				"46"
#define CID_CHECK_CONNECTION                    				"49"
#define CID_GET_THE_LAST_RSSISNR                				"4A"

#define CID_SET_CLASS_TYPE                      				"4B"
#define CID_SET_CLASS_TYPE_OPTION1_0            				"0"     // Class A
#define CID_SET_CLASS_TYPE_OPTION1_1            				"1"     // Class B
#define CID_SET_CLASS_TYPE_OPTION1_2            				"2"     // Class C

#define CID_GET_CLASS_TYPE                      				"4C"
#define CID_GET_FIRMWARE_VERSION                				"4F"

#define CID_DEBUG_MESSAGE_ONOFF                 				"50"
#define CID_DEBUG_MESSAGE_ONOFF_OPTION1_0       				"0"     // Debug message off
#define CID_DEBUG_MESSAGE_ONOFF_OPTION1_1       				"1"     // Debug message on

#define CID_SET_APPLICATION_KEY                 				"51"
#define CID_SET_APPLICATION_KEY_OPTION1         				"%s"    // 8byte Hex-decimal AppEUI

#define CID_GET_APPLICATION_KEY                 				"52"
#define CID_SET_ATTEN                           				"53"
#define CID_SET_ATTEN_OPTION1                   				"atten:%d"   // antenna gain 0~6

#define CID_GET_ATTEN                           				"63"

#define CID_SET_UNCONFIRMED_MSG_TRANSMISSION_NUMBER             "54"
#define CID_SET_UNCONFIRMED_MSG_TRANSMISSION_NUMBER_OPTION1     "ReTx:%d"   //  1~8

#define CID_GET_UNCONFIRMED_MSG_TRANSMISSION_NUMBER             "55"
#define CID_GET_RX1_DATARATE_OFFSET                 			"56"

#define CID_SET_UPLINK_CYCLE_ONOFF                  			"58"
#define CID_SET_UPLINK_CYCLE_ONOFF_OPTION1_0        			"0"  // Cycle Send message off
#define CID_SET_UPLINK_CYCLE_ONOFF_OPTION1_1        			"1"  // Cycle Send message on

#define CID_GET_UPLINK_CYCLE_ONOFF                  			"59"
#define CID_SET_SLEEP_MODE                          			"60"
#define CID_SYSTEM_SOFTWARE_RESET                   			"70"
			
#define CID_SET_CHANNEL_TX_POWER                    			"5E"
#define CID_SET_CHANNEL_TX_POWER_OPTION1            			"Index %d"  // 1~8 (index 1: 922.1Mhz, index 8: 923.3Mhz)
#define CID_SET_CHANNEL_TX_POWER_OPTION2            			"Index %d"  // 0~14 ( index 0: 14dBm, index 14: 0 dBm )
			
#define CID_GET_CHANNEL_TX_POWER                    			"5F"
#define CID_SET_TIME_SYNC_REQUEST                   			"39"

#define CID_TX_MESSAGE_BINARY_DATA                      		" 4D " // the space must be included. 
#define CID_TX_MESSAGE_BINARY_DATA_MTYPE                		"%d"   // 1 : confirmed data, 0 : unconfirmed data 
#define CID_TX_MESSAGE_BINARY_DATA_FPORT                		"%d"   // 0x1 ~ 0xDD
#define CID_TX_MESSAGE_BINARY_DATA_LENGTH               		"%d"   // 0x1 ~ 0xF2


#define CID_TX_BYPASS_PROCESS_FOR_SKIOT                         0xff

typedef void (*callbackFunc)(const char *cmd, int size); 
void initWisol(callbackFunc cb);
bool writeLRW(const char *msg, int size, int bypasscmd);
bool parser_Wisol(char *cmd, int size);

#endif //__WISOL_H_