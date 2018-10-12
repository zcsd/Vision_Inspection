
#ifndef _S8_H
#define _S8_H

#include <stdlib.h>

#define __int16   short

#ifdef __cplusplus
extern "C"
{
#endif
//===============================================
/*----------basic function--------*/
//===============================================

int fw_init(int port,long baud);

int fw_init_ex(int comType, char* szPathName,long baud);

int fw_exit(int icdev);

void hex_a(unsigned char *hex,unsigned char *a,__int16 len);

int a_hex(unsigned char *a,unsigned char *hex,__int16 len);

int fw_des(unsigned char *key,unsigned char *sour,unsigned char *dest,__int16 m);

int fw_config_card(int icdev,unsigned char flags);

//===============================================
/*----------LCD operation function--------*/
//===============================================
int fw_lcd_setbright(int icdev,unsigned char bright);

int fw_lcd_dispclear(int icdev);

int fw_lcd_dispstr(int icdev,char *digit, int flag);

//===============================================
/*----------Device operation function--------*/
//===============================================

int fw_beep(int icdev,unsigned int _Msec);

int fw_getver(int icdev,unsigned char *buff);

int fw_srd_eeprom(int icdev,int offset,int length, unsigned char *rec_buffer);

int fw_srd_eepromhex(int icdev,int offset,int length,unsigned char *rec_buffer);

int fw_swr_eeprom(int icdev,int offset,int length, unsigned char* buffer);

int fw_swr_eepromhex(int icdev,int offset,int length,unsigned char* buffer);

int fw_reset(int icdev,unsigned __int16 _Msec);
//===============================================
/*----------S50 Commond Functions--------*/
//===============================================

int fw_card(int icdev,unsigned char _Mode,unsigned long *_Snr);

int fw_card_hex(int icdev,unsigned char _Mode,unsigned char* Snrbuf);

int fw_request(int icdev,unsigned char _Mode,unsigned int *TagType);

int fw_anticoll(int icdev,unsigned char _Bcnt,unsigned long *_Snr);

int fw_select(int icdev,unsigned long _Snr,unsigned char *_Size);

int  fw_anticoll2(int icdev,unsigned char _Bcnt,unsigned long *_Snr);
int  fw_select2(int icdev,unsigned long _Snr);

int fw_load_key(int icdev,unsigned char _Mode,
				unsigned char _SecNr,unsigned char *_NKey);

int fw_load_key_hex(int icdev,unsigned char _Mode,unsigned char _SecNr,unsigned char *_NKey);

int fw_authentication(int icdev,unsigned char _Mode,
					  unsigned char _SecNr);

int fw_authentication_pass(int icdev, unsigned char _Mode, 
						   unsigned char SecNr,
						   unsigned char *passbuff);

int fw_authentication_pass_hex(int icdev, unsigned char _Mode, 
									unsigned char SecNr,
									unsigned char *passbuff);

int fw_read(int icdev,unsigned char _Adr,unsigned char *_Data);

int fw_read_hex(int icdev,unsigned char _Adr,char *_Data); 

int fw_write(int icdev,unsigned char _Adr,unsigned char *_Data);

int fw_write_hex(int icdev,unsigned char _Adr,char *_Data);

int  fw_initval(int icdev,unsigned char _Adr,unsigned long _Value);

int  fw_readval(int icdev,unsigned char _Adr,unsigned long *_Value);

int  fw_increment(int icdev,unsigned char _Adr,unsigned long _Value);

int  fw_decrement(int icdev,unsigned char _Adr,unsigned long _Value);

int  fw_restore(int icdev,unsigned char _Adr);

int  fw_transfer(int icdev,unsigned char _Adr);

int fw_halt(int icdev);

int fw_des(unsigned char *key,unsigned char *sour,unsigned char *dest,__int16 m);

int fw_changeb3(int icdev,unsigned char _SecNr,unsigned char *_KeyA,
				unsigned char *_CtrlW,unsigned char _Bk,
				unsigned char *_KeyB);



//===============================================
/*----------S70 Read/Write function--------*/
//===============================================
int fw_read_S70(int icdev,unsigned char _Adr,unsigned char *_Data);

int fw_write_S70(int icdev,unsigned char _Adr,unsigned char *_Data);

int fw_read_S70_hex(int icdev,unsigned char _Adr, char *_Data);

int fw_write_S70_hex(int icdev,unsigned char _Adr,char *_Data);

//================================================
//     UltraLight card  Functions
//================================================
#define PAGE16_19	16
#define PAGE20_23	20
#define PAGE24_27	24
#define PAGE28_31	28
#define PAGE32_35	32
#define PAGE36_39	36
#define PAGE44_47	44

int  fw_request_ultralt(int icdev,unsigned char _Mode);
int  fw_anticall_ultralt(int icdev,unsigned long *_Snr);
int  fw_select_ultralt(int icdev,unsigned long _Snr);
int  fw_reqa_ultralt(int icdev,unsigned char _Bcnt,unsigned long _Snr);
int  fw_wakeup_ultralt(int icdev,unsigned char _Snr);
int  fw_read_ultralt(int icdev,unsigned char iPage,unsigned char *redata);
int  fw_write_ultralt(int icdev,unsigned char iPage,unsigned char *sdata);
int  fw_halt_ultralt(int icdev);
int  fw_ultralt_C_authen(int icdev, unsigned char* key);

//===============================================
/*----------Cpu operation function--------*/
//===============================================
int fw_cpuapdu(int icdev,unsigned char slen,unsigned char * sbuff,
			   unsigned char *rlen,unsigned char * rbuff);

int fw_cpuapdu_hex(int icdev,unsigned char slen, char * sendbuffer,unsigned char *rlen,unsigned char * rbuff);

int fw_cpureset(int icdev,unsigned char *rlen, unsigned char *rbuff);

int fw_cpureset_hex(int icdev,unsigned char *rlen, char *databuffer);

int fw_setcpu(int icdev,unsigned char fcard);

int fw_setcpupara(int icdev,unsigned char cputype,unsigned char cpupro,unsigned char cpuetu);


//===============================================
/*----------contact-less Cpu operation function--------*/
//===============================================
//1
int fw_pro_reset
(
 int ICDev,//端口标识符
 unsigned char *rlen,//返回复位信息的长度
 unsigned char * rbuff//存放返回的复位信息
 );

//2
int fw_pro_commandlink
(
 int ICDev,//端口标识符
 unsigned int slen,//发送的信息长度
 unsigned char * sbuff,//存放要发送的信息
 unsigned int *rlen,//返回信息的长度
 unsigned char * rbuff,//存放返回的信息
 unsigned char tt,//延迟时间，单位为：10ms
 unsigned char FG//分割长度。建议此值小于64
 );

//===============================================
//       Icode2 card  Functions
//===============================================

int fw_inventory(int icdev,unsigned char flags,
							   unsigned char AFI, 
							   unsigned char masklen, 
							   unsigned char *rlen,unsigned char *rbuffer);
int fw_stay_quiet(int icdev,unsigned char flags,unsigned char *UID);
int fw_select_uid(int icdev,unsigned char flags,unsigned char *UID);
int fw_reset_to_ready(int icdev,unsigned char flags,unsigned char *UID);
int fw_readblock(int icdev,unsigned char flags,
							   unsigned char startblock,unsigned char blocknum, 
							   unsigned char *UID, 
							   unsigned char *rlen,unsigned char *rbuffer);
int fw_writeblock(int icdev,unsigned char flags,
								unsigned char startblock,unsigned char blocknum, 
								unsigned char *UID, 
								unsigned char wlen,unsigned char *rbuffer);
int fw_lock_block(int icdev,unsigned char flags,unsigned char block,
								unsigned char *UID);
int fw_write_afi(int icdev,unsigned char flags,unsigned char AFI,
							   unsigned char *UID);
int fw_lock_afi(int icdev,unsigned char flags,unsigned char AFI,
							  unsigned char *UID);
int fw_write_dsfid(int icdev,unsigned char flags,unsigned char DSFID,
								 unsigned char *UID);
int fw_lock_dsfid(int icdev,unsigned char flags,unsigned char DSFID,
								unsigned char *UID);
int fw_get_systeminfo(int icdev,unsigned char flags,
									unsigned char *UID, 
									unsigned char *rlen,unsigned char *rbuffer);
int fw_get_securityinfo(int icdev,unsigned char flags,
									  unsigned char startblock,unsigned char blocknum, 
									  unsigned char *UID, 
									  unsigned char *rlen,unsigned char *rbuffer);



#ifdef __cplusplus
}
#endif


#endif


