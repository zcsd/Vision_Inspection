#include "rfidtool.h"

RFIDTool::RFIDTool(QObject *parent) : QObject(parent)
{

}

int RFIDTool::initDevice()
{
    int icdev,st,number;
    unsigned char szVer[128];
    int iNode = 0;
    char szNode[128];

    //usb port ...
/*
    do
    {
        sprintf(szNode, "/dev/usb/hiddev%d", iNode);
        iNode++;
        if((icdev=fw_init_ex(2,szNode,115200))== -1)
        {
            printf("fw_init_ex ERR %d\n",icdev);
            return 0;
        }
        else
        {
            st = fw_getver(icdev, szVer);
            if(st ==0)
            {
                break;
            }
            else
            {
                printf("%s init error\n",szNode);
            }
        }
    }while(icdev != -1);
*/

    ////serila port ttyUSB0 115200...

    do
    {
        sprintf(szNode, "/dev/ttyUSB%d", iNode);
        iNode++;
        if((icdev=fw_init_ex(1,szNode,115200))== -1)
        {
            printf("fw_init_ex ERR %d\n",icdev);
            return 0;
        }
        else
        {
            st = fw_getver(icdev, szVer);
            if(st ==0)
            {
                break;
            }
            else
            {
                printf("%s init error\n",szNode);
            }
        }
    }while(icdev != -1);


    printf("%s init ok\n",szNode);

    fw_beep(icdev,10);

    printf("1---Device\n");
    printf("2--ICODE2\n");
    printf("0---Quit\n");

    scanf("%d",&number);

    switch(number)
    {
    case 1:
        st = testDevice(icdev);
        break;
    case 2:
        st = icode2(icdev);
        break;
        default:
        break;
    }

       fw_exit(icdev);
       return 0;
}

int RFIDTool::testDevice(int icdev)
{
    int st;//the state of each operation
    unsigned char rdata[1024];
    unsigned char wdata[1024];
    int rwlen = 200;
    int i;

    for(i=0; i< rwlen; i++) wdata[i] = 1+i;

    st = fw_swr_eeprom(icdev, 0, rwlen, wdata);
    if(st)
    {
        printf("fw_swr_eeprom error:0x%x\n", st);
        goto DO_EXIT;
    }

    st =fw_srd_eeprom(icdev, 0, rwlen, rdata);
    if(st)
    {
        printf("fw_srd_eep error:0x%x\n",st);
        goto DO_EXIT;
    }
    printf("fw_srd_eep ok\n");

    for(i=0; i<rwlen; i++)
        printf("%02X ", rdata[i]);
    printf("\n");


    //st = fw_lcd_dispclear(icdev);
    //if(st)printf("fw_lcd_dispclear error:0x%x\n",st);

    //st = fw_lcd_dispstr(icdev, L"字符串测试123ab完毕", 1);
    //if(st)printf("fw_lcd_dispstr error:0x%x\n",st);
DO_EXIT:
    return st;
}

int RFIDTool::icode2(int icdev)
{
    int st;
    unsigned char rlen[17]={0};
    unsigned char rbuffer[256];
    unsigned char szCardSn[512] ={0};
    unsigned char UID[16];
    unsigned char m_StaAddr = 12;
    unsigned char m_Blockno = 1;
    unsigned char tmp[256];
    int i;

    fw_config_card(icdev, 0x31);

    st= fw_inventory(icdev,0x36,0,0,rlen,rbuffer);        //find single card
    if(st)
    {
        printf("Find single card ERROR!\n");
        return 0;
    }

    hex_a(szCardSn, &rbuffer[0], 2* rlen[0]);

    printf("Find card %s \n", (char *)szCardSn);

    memcpy(UID,(char*)&rbuffer[0], 8);


    st=fw_select_uid(icdev,0x22,&UID[0]);
    if(st)
    {
        printf("fw_select_uid ERROR!\n");
        return 0;
    }

        st=fw_reset_to_ready(icdev,0x22,&UID[0]);
        if(st)
    {
        printf("fw_reset_to_ready ERROR!\n");
        return 0;
    }


    st = fw_get_securityinfo(icdev,0x22,0x04,0x02, &UID[0],rlen,rbuffer);
    if(st)
    {
        printf("fw_get_securityinfo ERROR!\n");
        return 0;
    }

    st=fw_readblock(icdev,0x22,m_StaAddr,m_Blockno,&UID[0],rlen,rbuffer);  //read block data
    if(st)
    {
        printf("Read data ERROR! \n");
        return 0;
    }

    for(i=0;i<m_Blockno;i++)
    {
        sprintf((char *)tmp,"BlockAddr:[%2d] Data:[%02X %02X %02X %02X]",m_StaAddr+i,rbuffer[i*4],rbuffer[i*4+1],rbuffer[i*4+2],rbuffer[i*4+3]);
        printf("%s\n", (const char *)tmp);
    }


    return 0;
}
