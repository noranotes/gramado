/*
 * File: sm\disk\disk.c 
 * 
 * Descri��o:
 *     Gerenciador de discos. 
 *     Arquivo principal do Disk Manager.
 *     MB - M�dulos inclu�dos no Kernel Base.
 *
 *     O Gerenciador de Discos � um m�dulo muito importante do Kernel.
 *
 * History:
 *     2015 - Created by Fred Nora.
 *     2018 - IDE COntroller by Nelson Cole.
 *            Revision.
 *
 */

 
//
// Obs: o foco est� na lista de discos. diskList
//

#include <kernel.h>


/*
 * disk_get_disk_handle:
 *     Obtem o ponteiro da estrutura dado o descritor.
 *
 */
void *disk_get_disk_handle( int number )
{
	//check limts
	if( number < 0 || number >= DISK_COUNT_MAX ){
		return NULL;
	}
	return (void *) diskList[number];
};



/*
 * disk_get_current_disk_info:
 *     Obtem o ponteiro da estrutura de informa��es do disco atual.
 *
 */
void *disk_get_current_disk_info()
{
	return (void *) CurrentDiskInfo; 
};


/*
 * disk_init:
 *     Inicializa o Disk Manager.
 *     Que � um m�dulo do tipo MB.
 *
 */
//int diskInit() 
int disk_init()
{
	
#ifdef KERNEL_VERBOSE	
    printf("disk_init: Initializing..\n");
#endif	
	
	//
	// Inicializando uma estrutura global de informa��es sobre o 
	// disco do sistema.
	// ?? Em que arquivo est� essa estrutura.
	//

	// disk info
    diskinfo_conductor = (void*) malloc( sizeof(struct diskinfo_d) );
	if( (void*) diskinfo_conductor == NULL )
	{
	    printf("sm-disk-disk_init:");
		die();
	}else{
		
	    diskinfo_conductor->id = 0;
	    
		diskinfo_conductor->used = 1;
	    diskinfo_conductor->magic = 1234;
	    
		diskinfo_conductor->name = "DISK INFO";
		
		//diskinfo_conductor->BytesPerSector = DISK_BYTES_PER_SECTOR;
	    //diskinfo_conductor->SectorsPerCluster = 0;
        //...		
	};
	
	
	// disk.
    disk_conductor = (void*) malloc( sizeof(struct disk_d) );
	if( (void*) disk_conductor == NULL )
	{
	    printf("sm-disk-disk_init:");
		die();
	}else{
		
		//@todo:
		//disk_conductor->objectType = ?;
		//disk_conductor->objectClass = ?;
		
		disk_conductor->diskType = DISK_TYPE_NULL;
		
	    disk_conductor->id = 0;
	    
		disk_conductor->used = 1;
	    disk_conductor->magic = 1234;
	    
		disk_conductor->name = "DISK 0";
		
		disk_conductor->disk_info = (struct diskinfo_d *) diskinfo_conductor;
	};	
		
    //
	//@todo: Nothing more ?!!
	//
	
done:
    printf("Done.\n");
    return (int) 0;
};


/*
 * init_test_disk:
 *     Testando opera��es com disco.
 *
 *     @todo: Essa rotina pode virar um aplicativo de teste.
 *
 * + Carrega o arquivo KCONFIG.TXT se estiver presente e 
 *   sonda informa��es de configura��o contidas nele, exibindo as
 *   informa��es.
 * 
 */
void init_test_disk()
{
	/*@todo: Teste desse tipo pode ser feito em user mode.*/
	
 	/*
    int Index = 0;
	//int fileSize = 0;
	void *addr;
	
	//Deve existir uma estrutura de propriedades do arquivo,
	//como tamanho ...

	
	//@todo: testando lock
	//taskswitch_lock();
	
	printf("init_test_disk: Testing..\n");
	
	//
	// Alocando no heap do kernel. (Root dir de 32 setores).
	//
	
	//@todo: Usar constantes ou vari�veis para esses valores.
	
	addr = (void *) malloc(512*32); 
	if( (void *) addr ==  NULL)
	{
	    printf("init_test_disk: error\n");
		
		free(addr);     //@todo *** Cuidado (testando free()).
		
		refresh_screen();
		//while(1){}
		return;
	}
	else
	{		
	    printf("init_test_disk: Loading file..\n");
		fsLoadFile("KCONFIG TXT", (unsigned long) addr);
		printf(addr);	   		
	};
	
	
	//
	// buffer. @todo: Usar ioBuffers.
	//
	
	
	unsigned char *buffer = (unsigned char *) addr;			

	
	
	//@todo: Checar esse valor m�ximo.
	
	Index = 0;
	while(Index < 128)
	{
	    if( buffer[Index] == 'U')
        {
			if( buffer[Index +1] == 'S' &&
                buffer[Index +2] == 'E' &&
                buffer[Index +3] == 'R' &&
                buffer[Index +4] == '='	)
			{
			    printf("UserName={%s}\n", &buffer[Index+5]);	
			};
			
		};			
		
		Index++;
	};
	
	
//checkingMBR:	
	fsCheckMbr();    //Check MBR.
	
	
	
	//read_lba( FAT16_ROOTDIR_ADDRESS + b, FAT16_ROOTDIR_LBA + i);  

	//@todo: Testando unlock
	//taskswitch_unlock();

done:
	printf("Done.\n");
	*/
    return;
};


/*
int init_disk_manager();
int init_disk_manager()
{
	disk_init();
    return (int) 0;
};


*/


//
// ==============================================================================================
// ATA  - TESTANDO O SUPORTE A ATA DO NELSON...
// ==============================================================================================
//


/* *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *
 *
 */


extern st_dev_t *current_dev;
static _u32 ata_irq_invoked = 1; 

_void ata_wait(_i32 val)
{ 
   val/=100;
   while(val--)io_delay();
   //while(val--)delay();
}



// TODO: Nelson, ao configurar os bits BUSY e DRQ 
// devemos verificar retornos de erros.
_u8 ata_wait_not_busy()
{
    while(ata_status_read() &ATA_SR_BSY)
    if(ata_status_read() &ATA_SR_ERR)
    return 1;

    return 0;

}

_u8 ata_wait_busy()
{
    while(!(ata_status_read() &ATA_SR_BSY))
    if(ata_status_read() &ATA_SR_ERR)
    return 1;

    return 0;

}


_u8 ata_wait_no_drq()
{
    while(ata_status_read() &ATA_SR_DRQ)
    if(ata_status_read() &ATA_SR_ERR)
    return 1;

    return 0;
}

_u8 ata_wait_drq()
{
    while(!(ata_status_read() &ATA_SR_DRQ))
    if(ata_status_read() &ATA_SR_ERR)
    return 1;

    return 0;
}




_void ata_irq_handler1(_void)
{

    ata_irq_invoked = 1;
   
}

_void ata_irq_handler2(_void)
{

    ata_irq_invoked = 1;
    
}


_u8 ata_wait_irq()
{
   _u32 tmp = 0x10000;
   _u8 data;
   while (!ata_irq_invoked){
    data = ata_status_read();
    if((data &ATA_SR_ERR)){
        ata_irq_invoked = 0;
        return -1;
        }
                         //ns
        if(--tmp)ata_wait(100);
        else{
        ata_irq_invoked = 0;
        return 0x80;
        }
    }
 
    ata_irq_invoked = 0;
    return 0;
}

_void ata_soft_reset()
{
    _u8 data =  inb(ata.ctrl_block_base_address + 2);
    outb(ata.ctrl_block_base_address, data | 0x4);
    outb(ata.ctrl_block_base_address, data &0xfb);    
}


_u8 ata_status_read()
{
   	return inb(ata.cmd_block_base_address + ATA_REG_STATUS);

}

_void ata_cmd_write(_i32 cmd_val)
{
           
    	// no_busy      	
	ata_wait_not_busy();
	outb(ata.cmd_block_base_address + ATA_REG_CMD,cmd_val);
	ata_wait(400);  // Esperamos 400ns

}



_u8 ata_assert_dever(_i8 nport)
{

    switch(nport){
    case 0:
        ata.channel = 0;
        ata.dev_num = 0;
    break;
    case 1:   
        ata.channel = 0;
        ata.dev_num = 1;
    break;
    case 2:
        ata.channel = 1;
        ata.dev_num = 0;
    break;
    case 3:
        ata.channel = 1;
        ata.dev_num = 1;
    break;
    default:
        kprintf("Port %d, volue not used\n",nport);
        return -1;
     break;
    }

    set_ata_addr(ata.channel);


    return 0;

}


/*
 * ide_identify_device:
 * 
 */
int ide_identify_device(uint8_t nport)
{
    _u8 status;
    _u8 lba1,lba2;

    ata_assert_dever(nport);

    // Ponto flutuante
    if(ata_status_read() == 0xff)//Sem unidade conectada ao barramento
        return -1;

    outb(ata.cmd_block_base_address + ATA_REG_SECCOUNT,0);	// Sector Count 7:0
	outb(ata.cmd_block_base_address + ATA_REG_LBA0,0);        // LBA 7-0   
	outb(ata.cmd_block_base_address + ATA_REG_LBA1,0);        // LBA 15-8
	outb(ata.cmd_block_base_address + ATA_REG_LBA2,0);        // LBA 23-16

    
    // Select device,
    outb(ata.cmd_block_base_address + ATA_REG_DEVSEL,0xE0| ata.dev_num << 4);
    ata_wait(400);

    // cmd
    ata_cmd_write(ATA_CMD_IDENTIFY_DEVICE); 
    // ata_wait_irq();
    // Nunca espere por um IRQ aqui
    // Devido unidades ATAPI, ao menos que pesquisamos pelo Bit ERROR
    // Melhor seria fazermos polling
     
    ata_wait(400);


    if(ata_status_read() == 0) //Sem unidade no canal 
        return -1;

   lba1 = inb(ata.cmd_block_base_address + ATA_REG_LBA1);
   lba2 = inb(ata.cmd_block_base_address + ATA_REG_LBA2);

   if(lba1 == 0x14 && lba2 == 0xEB){
        //kputs("Unidade PATAPI\n");   
        ata_cmd_write(ATA_CMD_IDENTIFY_PACKET_DEVICE);
        ata_wait(400);
        ata_wait_drq(); 
        ata_pio_read(ata_identify_dev_buf,512);
        ata_wait_not_busy();
        ata_wait_no_drq();
        
        return 0x80;
   }
   else if(lba1 == 0x69  && lba2 == 0x96){

        //kputs("Unidade SATAPI\n");   
        ata_cmd_write(ATA_CMD_IDENTIFY_PACKET_DEVICE);
        ata_wait(400);
        ata_wait_drq(); 
        ata_pio_read(ata_identify_dev_buf,512);
        ata_wait_not_busy();
        ata_wait_no_drq();
		
        return 0x80;

   }
   else if(lba1 == 0x3C && lba2 == 0xC3){
        //kputs("Unidade SATA\n");   
        // O dispositivo responde imediatamente um erro ao cmd Identify device
        // entao devemos esperar pelo DRQ ao invez de um BUSY
        // em seguida enviar 256 word de dados PIO.
        ata_wait_drq(); 
        ata_pio_read(ata_identify_dev_buf,512);
        ata_wait_not_busy();
        ata_wait_no_drq();
		
        return 0;
   }


   else if(lba1 == 0 && lba2 == 0){
        // kputs("Unidade PATA\n");
        // aqui esperamos pelo DRQ
        // e eviamoos 256 word de dados PIO
        ata_wait_drq();
        ata_pio_read(ata_identify_dev_buf,512);

        ata_wait_not_busy();
        ata_wait_no_drq();
		
        return 0;
    }


done:
	return 0;   
};


// TODO
// Nelson aqui, devemos destinguir se a Interface � IDE ou AHCI
// Por em quanto temos suporte a IDE
int read_sector(void *buf,uint64_t addr,uint16_t count,char dev)
{

    int i;
    if(nport_ajuste(dev) != 0)
    return -1;

    if(current_dev->dev_type == ATA_DEVICE_TYPE){
	for(i=0;i < count;i++){
        if((ide_read_sector(dev,1,addr + i ,buf + (current_dev->dev_byte_per_sector*i))) !=0)
        return -1;

	}

    }
    else if(current_dev->dev_type == ATAPI_DEVICE_TYPE){
	for(i=0;i < count;i++){
        if((atapi_read_sector(dev,count,addr + i,buf+ (current_dev->dev_byte_per_sector*i)))!=0)
        return -1;
    	}
	
    }
    return 0;

}

int write_sector(void *buf,uint64_t addr,uint16_t count,char dev)
{
    int i;
    if(nport_ajuste(dev) != 0)
    return 1;

    if(current_dev->dev_type == ATA_DEVICE_TYPE){
    for(i=0;i < count;i++){
        if((ide_write_sector(dev,1,addr + i ,buf + (current_dev->dev_byte_per_sector*i))) !=0)
        return -1;
    }    

    }
    else if(current_dev->dev_type == ATAPI_DEVICE_TYPE){
        kprintf("Acesso negado, apenas leitura devd%\n",dev);
        return -1;
    }

    return 0;

}




/* *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *
 *
 */
//#include "ata.h"

#define DISK1 1
#define DISK2 2
#define DISK3 3
#define DISK4 4

static const char *ata_sub_class_code_register_strings[] ={
    "Unknown",
    "IDE Controller",
    "Unknown",
    "Unknown",
    "RAID Controller",
    "Unknown",
    "AHCI Controller"
};


extern st_dev_t *current_dev;



// base address 
static _u32 ATA_BAR0;    // Primary Command Block Base Address
static _u32 ATA_BAR1;    // Primary Control Block Base Address
static _u32 ATA_BAR2;    // Secondary Command Block Base Address
static _u32 ATA_BAR3;    // Secondary Control Block Base Address
static _u32 ATA_BAR4;    // Legacy Bus Master Base Address
static _u32 ATA_BAR5;    // AHCI Base Address / SATA Index Data Pair Base Address



//
//   ## ATA INITIALIZE ##
//

/*
 ****************************************************************
 * ata_initialize:
 *     Inicializa o IDE e mostra informa��es sobre o disco.
 *
 */
int ata_initialize()
{
	//int Status = 0;
	
	_u32 data;
	
	_u8 bus;
	_u8 dev;
	_u8 fun;
	
	//
	// Messages
	//
	
#ifdef KERNEL_VERBOSE
    kprintf("sm-disk-disk-ata_initalize:\n");
    kprintf("Initializing IDE/AHCI support ...\n");
	//refresh_screen();
#endif 
	
    data = (_u32) pci_scan_device(PCI_CLASSE_MASS);
    if( data == -1 )
	{
		// Error.
		kprintf("sm-disk-disk-ata_initalize: pci_scan_device fail. ret={%d} \n", (_u32) data );
		refresh_screen();
		
	    // Abortar.
		return (int) (PCI_MSG_ERROR); 	
	};
    
    bus = ( data >> 8 &0xff );
    dev = ( data >> 3 &31 );
    fun = ( data &7 );

	//
	// Configuration space.
	//
	
    data = (_u32) ata_pci_configuration_space( bus, dev, fun );

    if( data == PCI_MSG_ERROR )
	{
		// Error.
        kprintf("sm-disk-disk-ata_initalize: Error Driver [%X]\n",data);
        refresh_screen();
		return (int) 1;    
	
	}else if( data == PCI_MSG_AVALIABLE )
	      {
              kprintf("sm-disk-disk-ata_initalize: RAID Controller Not supported.\n");
		      refresh_screen();
              return (int) 1;       
          };
		  
	//
    // Salvando informa��es.
    //	

    // Initialize base address
    // AHCI/IDE Compativel com portas IO IDE legado
    ATA_BAR0 = ( ata_pci.bar0 & ~7   ) + ATA_IDE_BAR0 * ( !ata_pci.bar0 );
    ATA_BAR1 = ( ata_pci.bar1 & ~3   ) + ATA_IDE_BAR1 * ( !ata_pci.bar1 );       
    ATA_BAR2 = ( ata_pci.bar2 & ~7   ) + ATA_IDE_BAR2 * ( !ata_pci.bar2 );
    ATA_BAR3 = ( ata_pci.bar3 & ~3   ) + ATA_IDE_BAR3 * ( !ata_pci.bar3 );
    ATA_BAR4 = ( ata_pci.bar4 & ~0x7 ) + ATA_IDE_BAR4 * ( !ata_pci.bar4 );
    ATA_BAR5 = ( ata_pci.bar5 & ~0xf ) + ATA_IDE_BAR5 * ( !ata_pci.bar5 );

	
	// Type
    if( ata.chip_control_type == ATA_IDE_CONTROLLER )
	{

    //Soft Reset, defina IRQ
    outb( ATA_BAR1, 0xff );
    outb( ATA_BAR3, 0xff );
    outb( ATA_BAR1, 0x00 );
    outb( ATA_BAR3, 0x00 );

    ata_record_dev = 0xff;
    ata_record_channel = 0xff;

#ifdef KERNEL_VERBOSE	
	printf("Initializing IDE Mass Storage device ...\n");
	refresh_screen();
#endif    
	
	//
	// As estruturas de disco ser�o colocadas em uma lista encadeada.
	//
	
	ide_mass_storage_initialize();


    }else if( ata.chip_control_type == ATA_AHCI_CONTROLLER )
	      {
			  
		      //
              // Nothing for now !!!
              //			  

              // Aqui, vamos mapear o BAR5
              // Estou colocando na marca 28MB
    
//#ifdef KERNEL_VERBOSE	
              // printf("ata_initialize: mem_map para ahci\n");
              // refresh_screen();
//#endif
	
	          //mem_map( (uint32_t*)0x01C00000, (uint32_t*) ATA_BAR5, 0x13, 2);

              //kputs("[ AHCI Mass Storage initialize ]\n");
              //ahci_mass_storage_init();

          }else{
			  
			   //
			   // Panic !!
			   //
               
			   kprintf("sm-disk-disk-ata_initalize: Panic!\n");
               kprintf("IDE controller not found!\n");
		       kprintf("AHCI controller not found!\n");
			   die();
          };
	
	
 done:

#ifdef KERNEL_VERBOSE 
    printf("done!\n");
    refresh_screen();
#endif 
	
    return (int) 0;	
};



void set_ata_addr(int channel)
{
	//
	// @todo:
	// Checar a validade da estrutura.
	//

    switch(channel)
	{
        case ATA_PRIMARY:
            ata.cmd_block_base_address  = ATA_BAR0;
            ata.ctrl_block_base_address = ATA_BAR1;
            ata.bus_master_base_address = ATA_BAR4;
            break;
			
        case ATA_SECONDARY:
            ata.cmd_block_base_address  = ATA_BAR2;
            ata.ctrl_block_base_address = ATA_BAR3;
            ata.bus_master_base_address = ATA_BAR4 + 8;
            break;
			
        default:
            //PANIC
            break;
    };

done:
    return;
};


/*
 *************************************************************
 * ata_pci_configuration_space:
 *     Espa�o de configura�ao PCI Mass Storage
 */
int ata_pci_configuration_space( char bus, char dev, char fun )
{
    uint32_t data;

#ifdef KERNEL_VERBOSE	
	kprintf("ata_pci_configuration_space:\n");
    kprintf("Initializing PCI Mass Storage support ...\n");
#endif

    // Indentification Device
    data = (uint32_t) read_pci_config_addr( bus, dev, fun, 0 );
	
	//
	// Salvando configura��es.
	//
    
    ata_pci.vendor_id = data &0xffff;
    ata_pci.device_id = data >> 16 &0xffff;
	
#ifdef KERNEL_VERBOSE	
	kprintf("\nDisk info:\n");
    kprintf("[ Vendor ID: %X,Device ID: %X ]\n", ata_pci.vendor_id, 
	    ata_pci.device_id );
#endif	
	
    // Classe code, programming interface, revision id
    data  = (uint32_t) read_pci_config_addr( bus, dev, fun, 8 );
    
	
	//
	// Salvando configura��es.
	//
	
	// Classe e sub-classe.
    ata_pci.classe = data >> 24 &0xff;
    ata_pci.subclasse = data >> 16 &0xff;
    
	// prog if.
	ata_pci.prog_if = data >> 8 &0xff;
   
    // Revision.
    ata_pci.revision_id = data &0xff;

	
    if( ata_pci.classe == 1 && 
	    ata_pci.subclasse == 1 )
	{
        ata.chip_control_type = ATA_IDE_CONTROLLER; 
                   
        // IDE
	
        // Compatibilidade e nativo, primary.
        data  = read_pci_config_addr( bus, dev, fun, 8 );
        if( data &0x200 )
		{ 
	        write_pci_config_addr( bus, dev, fun, 8, data | 0x100 ); 
		};        

        // Compatibilidade e nativo, secundary.
        data = read_pci_config_addr( bus, dev, fun, 8 );
        if( data &0x800 )
		{ 
	        write_pci_config_addr( bus, dev, fun, 8, data | 0x400 ); 
		};        

        data = read_pci_config_addr( bus, dev, fun, 8 );
        if( data &0x8000 )
		{    
            // Bus Master Enable
            data  = read_pci_config_addr(bus,dev,fun,4);
            write_pci_config_addr(bus,dev,fun,4,data | 0x4);
        }; 

	    // Habilitar interrupcao (INTx#)
        data = read_pci_config_addr( bus, dev, fun, 4 );
        write_pci_config_addr( bus, dev, fun, 4, data & ~0x400);

       	// IDE Decode Enable
       	data = read_pci_config_addr( bus, dev, fun, 0x40 );
       	write_pci_config_addr( bus, dev, fun, 0x40, data | 0x80008000 );

        // Synchronous DMA Control Register
	    // Enable UDMA
	    data = read_pci_config_addr( bus, dev, fun, 0x48 );
	    write_pci_config_addr( bus, dev, fun, 0x48, data | 0xf);

#ifdef KERNEL_VERBOSE 		
        kprintf("[ Sub Class Code %s Programming Interface %d Revision ID %d ]\n",\
            ata_sub_class_code_register_strings[ata.chip_control_type],
	        ata_pci.prog_if,
			ata_pci.revision_id );
#endif
            
    }else if( ata_pci.classe == 1 && ata_pci.subclasse == 4 )
	      {
              //RAID
              ata.chip_control_type = ATA_RAID_CONTROLLER;
			  
#ifdef KERNEL_VERBOSE              
			  kprintf("[ Sub Class Code %s Programming Interface %d Revision ID %d ]\n",\
                  ata_sub_class_code_register_strings[ata.chip_control_type], 
				  ata_pci.prog_if,
				  ata_pci.revision_id );
#endif
    
			  // Em avaliacao
              return PCI_MSG_AVALIABLE;
			  
          }else if( ata_pci.classe == 1 && 
	                ata_pci.subclasse == 6)
		        {
              
			        ata.chip_control_type = ATA_AHCI_CONTROLLER;
       
	                //
                    // ACHI.
                    //
		
                    // Compatibilidade e nativo, primary.
                    data = read_pci_config_addr( bus, dev, fun, 8 );
                    if( data &0x200 ){ 
		                write_pci_config_addr( bus, dev, fun, 8, data | 0x100 ); 
		            }        

                    // Compatibilidade e nativo, secundary.
                    data = read_pci_config_addr( bus, dev, fun, 8 );
                    if( data &0x800 ){ 
	                    write_pci_config_addr( bus, dev, fun, 8, data | 0x400 ); 
		            }        

		            // ??
                    data = read_pci_config_addr(bus,dev,fun,8);
                    if( data &0x8000 ) 
		            {    
                        // Bus Master Enable.
                        data = read_pci_config_addr( bus, dev, fun, 4 );
                        write_pci_config_addr( bus, dev, fun, 4, data | 0x4 );
                    } 

                    // IDE Decode Enable
                    data = read_pci_config_addr( bus, dev, fun, 0x40 );
                    write_pci_config_addr( bus, dev, fun, 0x40, data | 0x80008000 );

                    // Habilitar interrupcao (INTx#)
                    data = read_pci_config_addr( bus, dev, fun, 4 );
                    write_pci_config_addr( bus, dev, fun, 4, data & ~0x400);

#ifdef KERNEL_VERBOSE
                    kprintf("[ Sub Class Code %s Programming Interface %d Revision ID %d ]\n",\
                        ata_sub_class_code_register_strings[ata.chip_control_type], 
		                ata_pci.prog_if,
			            ata_pci.revision_id );
#endif			

                    //Ok.
                }else{
					
					 //
                     // PANIC! 
                     //
					 
					 kprintf("sm-disk-disk-ata_pci_configuration_space: PANIC DRIVER BLOCK!");
		             die();
                 };



    // PCI cacheline, Latancy, Headr type, end BIST
    data = read_pci_config_addr( bus, dev, fun, 0xC );

	//
	// Salvando configura��es.
	//    
	
	// ??
	ata_pci.primary_master_latency_timer = data >>8 &0xff;
    
	ata_pci.header_type = data >>16 &0xff;
	
    ata_pci.BIST = data >>24 &0xff;
    
	// BARs
    ata_pci.bar0 = read_pci_config_addr( bus, dev, fun, 0x10 );
    ata_pci.bar1 = read_pci_config_addr( bus, dev, fun, 0x14 );
    ata_pci.bar2 = read_pci_config_addr( bus, dev, fun, 0x18 );
    ata_pci.bar3 = read_pci_config_addr( bus, dev, fun, 0x1C );
    ata_pci.bar4 = read_pci_config_addr( bus, dev, fun, 0x20 );
    ata_pci.bar5 = read_pci_config_addr( bus, dev, fun, 0x24 );
	
    //--------------
    
    // Interrupt
    data = read_pci_config_addr( bus, dev, fun, 0x3C );
    
    //
    // Salvando configura��es.
    //		
	
	ata_pci.interrupt_line = data &0xff;
    ata_pci.interrupt_pin  = data >> 8 &0xff;


    // PCI command and status
    data = read_pci_config_addr( bus, dev, fun, 4 );
    
    //
    // Salvando configura��es.
    //	
	
	ata_pci.command = data &0xffff; 
    ata_pci.status  = data >>16 &0xffff;
	
	
#ifdef KERNEL_VERBOSE	
    kprintf("[ Command %x Status %x ]\n", ata_pci.command, 
	    ata_pci.status );
		
    kprintf("[ Interrupt Line %x Interrupt Pin %x ]\n", ata_pci.interrupt_pin, 
	    ata_pci.interrupt_line );
#endif		
	
    data = read_pci_config_addr(bus,dev,fun,0x48);
	
#ifdef KERNEL_VERBOSE		
    kprintf("[ Synchronous DMA Control Register %X ]\n", data );
#endif
	
done:

#ifdef KERNEL_VERBOSE	
    refresh_screen();
#endif 
 	
    return (PCI_MSG_SUCCESSFUL);
};


/* *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *
 * Obs: O que segue s�o rotinas de suporte ao controlador IDE.
 *
 */


const char *dev_type[] = {
    "ATA",
    "ATAPI"
};

st_dev_t *current_dev;       // A unidade atualmente selecionada.
st_dev_t *ready_queue_dev;   // O in�cio da lista.
uint32_t  dev_next_pid = 0;  // O pr�ximo ID de unidade dispon�vel. 


/*
 ********************************************************
 * ide_mass_storage_initialize:
 *     Rotina de inicializa��o de dispositivo de 
 * armazenamento de dados.
 *
 */
void ide_mass_storage_initialize()
{
	int port;
	
	// Iniciando a lista.
	ready_queue_dev = ( struct st_dev * ) kmalloc( sizeof( struct st_dev) );
	
	current_dev = ( struct st_dev * ) ready_queue_dev;
    current_dev->dev_id      = dev_next_pid++;
    current_dev->dev_type    = -1;
    current_dev->dev_num     = -1;
    current_dev->dev_channel = -1;
    current_dev->dev_nport   = -1;
    current_dev->next        = NULL;

    // ??
	ata_identify_dev_buf = ( _u16 * ) kmalloc(4096);


    // As primeiras quatro portas do controlador IDE.    
	for( port=0; port < 4; port++ )
	{
		// ??
        ide_dev_init(port);
	};
    
done:
    return;
};


/*
 *******************************************************************
 * ide_dev_init:
 *     ?? Alguma rotina de configura��o de dispositivos.
 *
 */
int ide_dev_init(char port)
{
    int data;

    st_dev_t *new_dev;	
	
    new_dev = ( struct st_dev * ) kmalloc( sizeof( struct st_dev) );
    
	//@todo: Checar a validade da estrutura.
	
	
	data = (int) ide_identify_device(port);
	
    if( data == -1 )
	{
		//@todo: Message.
        return (int) 1;
	};
	
	if( data == 0 )
	{
        // Unidades ATA.

        new_dev->dev_type            = (ata_identify_dev_buf[0] &0x8000)?       0xffff: ATA_DEVICE_TYPE;
        new_dev->dev_access          = (ata_identify_dev_buf[83]&0x0400)?    ATA_LBA48: ATA_LBA28;
        new_dev->dev_modo_transfere  = (ata_identify_dev_buf[49]&0x0100)? ATA_DMA_MODO: ATA_PIO_MODO;
        
		new_dev->dev_total_num_sector   = ata_identify_dev_buf[60];
        new_dev->dev_total_num_sector  += ata_identify_dev_buf[61];
		
        new_dev->dev_byte_per_sector = 512;
		
        new_dev->dev_total_num_sector_lba48  = ata_identify_dev_buf[100];
        new_dev->dev_total_num_sector_lba48 += ata_identify_dev_buf[101];
        new_dev->dev_total_num_sector_lba48 += ata_identify_dev_buf[102];
        new_dev->dev_total_num_sector_lba48 += ata_identify_dev_buf[103];
		
        new_dev->dev_size = (new_dev->dev_total_num_sector_lba48 * 512);

        
    }else if( data == 0x80 )
	      {

              // Unidades ATAPI.

              new_dev->dev_type = (ata_identify_dev_buf[0]&0x8000)? ATAPI_DEVICE_TYPE: 0xffff;
              
			  new_dev->dev_access = ATA_LBA28;
              
			  new_dev->dev_modo_transfere = (ata_identify_dev_buf[49]&0x0100)? ATA_DMA_MODO: ATA_PIO_MODO;
              
			  new_dev->dev_total_num_sector  = 0;
              new_dev->dev_total_num_sector += 0;
              
			  new_dev->dev_byte_per_sector = 2048; 
              
			  new_dev->dev_total_num_sector_lba48  = 0;
              new_dev->dev_total_num_sector_lba48 += 0;
              new_dev->dev_total_num_sector_lba48 += 0;
              new_dev->dev_total_num_sector_lba48 += 0;
              
			  new_dev->dev_size = (new_dev->dev_total_num_sector_lba48 * 2048);

    
          }else{
               
			   // @todo: Message.
			   // Identificar o erro
               return (int) 1;
		  };

    //Dados em comum.

    new_dev->dev_id = dev_next_pid++;
	
    new_dev->dev_num = ata.dev_num;
    
	new_dev->dev_channel= ata.channel;

    new_dev->dev_nport = port;
    
	//
	// port
	//
	
	switch( port )
	{
        case 0:
            dev_nport.dev0 = 0x81;
            break;
            
		case 1:
            dev_nport.dev1 = 0x82;
            break;
        
		case 2:
            dev_nport.dev2 = 0x83;
            break;
            
		case 3:
            dev_nport.dev3 = 0x84;
            break;

        //?? default ?? 			
    };


#ifdef KERNEL_VERBOSE
    kprintf("[ Detected Disk type: %s ]\n", dev_type[new_dev->dev_type] );
	refresh_screen();
#endif

    new_dev->next = NULL;

    //
    // Add no fim da lista (ready_queue_dev).
	//
	
    st_dev_t *tmp_dev; 
	
	tmp_dev = ( struct st_dev * ) ready_queue_dev;

    while( tmp_dev->next )
	{
        tmp_dev = tmp_dev->next;
    };
    
    tmp_dev->next = new_dev;
    
done:	
	return (int) 0;
};


/*
 * dev_switch:
 *     ?? Porque esse tipo ??
 */
static inline void dev_switch(void)
{
	// ??
    // Pula, se ainda n�o tiver nenhuma unidade.
    if( !current_dev ){
        return;
	}
	
    // Obter a pr�xima tarefa a ser executada.
    current_dev = current_dev->next;
    
    // Se ca�mos no final da lista vinculada, 
    // comece novamente do in�cio.
    if( !current_dev ){
        current_dev = ready_queue_dev;
    }
};


/*
 * getpid_dev:
 *     ?? Deve ser algum suporte a Processos.
 */
static inline int getpid_dev()
{
    return current_dev->dev_id;
};


/*
 * getnport_dev:
 *
 */
static inline int getnport_dev()
{
    return current_dev->dev_nport;
};


/*
 * nport_ajuste:
 *
 *
 */
int nport_ajuste( char nport )
{
    _i8 i = 0;
	
    while( nport != getnport_dev() )
	{
        if( i == 4 ){ 
		    return (int) 1; 
		}
        
		dev_switch();
        i++;
    };
	
    if( getnport_dev() == -1 ){ 
	    return (int) 1; 
	}
	
	
done:
    return (int) 0;
};



/**
 * Credits:
 *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *  
 * Obs: O que segue s�o rotinas de suporte a ATA.
 *
 */

void ata_pio_read( void *buffer, _i32 bytes )
{
    __asm__ __volatile__(\
                "cld;\
                rep; insw"::"D"(buffer),\
                "d"(ata.cmd_block_base_address + ATA_REG_DATA),\
                "c"(bytes/2));
				
};


void ata_pio_write( void *buffer, _i32 bytes )
{
    __asm__ __volatile__(\
                "cld;\
                rep; outsw"::"S"(buffer),\
                "d"(ata.cmd_block_base_address + ATA_REG_DATA),\
                "c"(bytes/2));

};



/**
 * Credits:
 *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *
 * Obs: O que segue s�o rotinas de suporte a IDE ATAPI.
 */

extern st_dev_t *current_dev;
extern uint8_t *dma_addr;


/*
 * atapi_pio_read:
 *
 *
 */
static inline void atapi_pio_read( void *buffer, uint32_t bytes )
{
    __asm__ __volatile__(\
                "cld;\
                rep; insw"::"D"(buffer),\
                "d"(ata.cmd_block_base_address + ATA_REG_DATA),\
                "c"(bytes/2));

};


/*
 ****************************************************************
 * atapi_read_sector:
 *
 */
int atapi_read_sector( char nport, 
                       uint16_t count, 
                       uint64_t addr, 
                       void *buffer )
{
    uint32_t sector_size = current_dev->dev_byte_per_sector;
    uint8_t  modo_atapi  = 0;
	
	//
	// package.
	//
	
    char atapi_packet [] = { 
	    ATAPI_CMD_READ, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0 
	};

    
	// #bugbug
	// ?? N�o sei qual � o problema aqui.
	// FIXME
    ata_assert_dever(nport); 
	
    if( current_dev->dev_modo_transfere == ATA_DMA_MODO )
	{
        switch( nport )
		{
            case 0: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR0;
                break;
				
            case 1: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR1;
                break;
				
            case 2: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR2;
                break;
				
            case 3: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR3;
                break;
				
            // ?? default
        };
	};
    
    
    // Configurar o pacote SCSI, 
    // 12 bytes de tamanho.
    // Use LBA28.
    atapi_packet[0x02] = ( addr >> 24 ) & 0xff;
    atapi_packet[0x03] = ( addr >> 16 ) & 0xff;
    atapi_packet[0x04] = ( addr >> 8  ) & 0xff;
    atapi_packet[0x05] = ( addr) & 0xff;
    atapi_packet[0x09] = (count);
    
    // Seleccionar a unidade.
    outb( ata.cmd_block_base_address + ATA_REG_DEVSEL, 0x40 | ata.dev_num << 4 );
	
    // Verifique se � a mesma unidade para n�o esperar.
    if( ata_record_dev != ata.dev_num && 
	    ata_record_channel != ata.channel )
	{
        ata_wait(400);    // 400 ns.
        ata_record_dev      = ata.dev_num;
        ata_record_channel  = ata.channel;
    };

    // Selecionar modo ATAPI
    // 0 = Modo PIO.
    // 1 = DMA or UDMA
	
    if( current_dev->dev_modo_transfere == ATA_DMA_MODO )
	{
        modo_atapi = 1;
        sector_size = 0;
    };
	
	// Modo PIO or DMA.
    outb( ata.cmd_block_base_address + ATA_REG_FEATURES,modo_atapi ); 

    // Defina o tamanho do buffer em LBA0 e LBA1.
    // O tamanho do bytes por sector.
     outb( ata.cmd_block_base_address + ATA_REG_LBA0, sector_size &0xff);  // LBA 7-0   
	 outb( ata.cmd_block_base_address + ATA_REG_LBA1, sector_size >> 8);   // LBA 15-8

    // Enviar comando PACKET.
    ata_cmd_write(ATA_CMD_PACKET);
    
	if( modo_atapi == 0 ) 
	{ 
        // #bugbug
        // FIXME IRQs
        ata_wait_irq();  
	
	}else{ 
	
        if( ata_wait_not_busy() != 0 )
		{ 
	        // Message.
		    return (int) -1; 
		};
		
    };

	
    if( ata_wait_drq() != 0 ){ 
	    return (int) -1; 
	}
    
	// Enviar os dados do pacote.
	
    __asm__ __volatile__("cld; rep; outsw" ::"c" (6) , "d" (\
    (ata.cmd_block_base_address +ATA_REG_DATA)),"S"(atapi_packet));
 
    
	//
	// @todo:
    // ?? Nelson � mesmo correcto DRQ aqui ??
	//
	
    if( ata_wait_drq() != 0 )
	{
		//Message
		return (int) -1;
    }		

    if( current_dev->dev_modo_transfere == ATA_DMA_MODO )
	{
        ide_dma_data( buffer, 
		              current_dev->dev_byte_per_sector*count, 
					  0x0, 
					  nport );    

        ide_dma_start();

        ata_wait_irq();

        while( ide_dma_read_status() &1 )
		{
            if( ide_dma_read_status() &2 )
			{
				// ?? Message.
                ide_dma_stop();
				
                return (int) -1;
            };
        };			

        ide_dma_stop();
        
		__asm__ __volatile__("cld; rep; movsd;"::"D"(buffer),\
                "S"(dma_addr),"c"((current_dev->dev_byte_per_sector*count)/4));
     

        return (int) 0;
    };
    
    atapi_pio_read(buffer,sector_size);
    ata_wait_irq();
    
	// Verifica erros
done:
    return (int) 0;
};



/* *
 * Credits:
 *
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 *
 * Obs:
 *     O que segue s�o rotinas de suporte a IDE ATA.
 */

extern st_dev_t *current_dev;
_u8 *dma_addr;


/*
 * ata_set_device_and_sector:
 *
 *
 */
static inline _void ata_set_device_and_sector( _u32 count, _u64 addr,\
                                               _i32 access_type, _i8 nport )
{
    ata_assert_dever(nport);

	//
	// Access type.
	//
	
    switch( access_type )
	{
	    case 28:
            //Mode LBA28
	        outb( ata.cmd_block_base_address + ATA_REG_SECCOUNT,count);	// Sector Count 7:0
	        outb( ata.cmd_block_base_address + ATA_REG_LBA0,addr);		        // LBA 7-0   
	        outb( ata.cmd_block_base_address + ATA_REG_LBA1,addr >> 8);          // LBA 15-8
	        outb( ata.cmd_block_base_address + ATA_REG_LBA2,addr >> 16);	        // LBA 23-16
            // Modo LBA active, Select device, and LBA 27-24
            outb( ata.cmd_block_base_address + ATA_REG_DEVSEL,0x40 |(ata.dev_num << 4) | (addr >> 24 &0x0f));
            
			// Verifique se e a mesma unidade para n�o esperar pelos 400ns.
            if( ata_record_dev != ata.dev_num && 
			    ata_record_channel != ata.channel )
			{
                ata_wait(400);
                //verifique erro
                ata_record_dev      = ata.dev_num;
                ata_record_channel  = ata.channel;
			};
            break;
			
        case 48:
            //Mode LBA48
            outb( ata.cmd_block_base_address + ATA_REG_SECCOUNT,0);	      // Sector Count 15:8
	        outb( ata.cmd_block_base_address + ATA_REG_LBA0,addr >> 24);  // LBA 31-24   
	        outb( ata.cmd_block_base_address + ATA_REG_LBA1,addr >> 32);  // LBA 39-32
	        outb( ata.cmd_block_base_address + ATA_REG_LBA2,addr >> 40);  // LBA 47-40
	        outb( ata.cmd_block_base_address + ATA_REG_SECCOUNT,count);   // Sector Count 7:0
	        outb( ata.cmd_block_base_address + ATA_REG_LBA0,addr);		  // LBA 7-0   
	        outb( ata.cmd_block_base_address + ATA_REG_LBA1,addr >> 8);   // LBA 15-8
	        outb( ata.cmd_block_base_address + ATA_REG_LBA2,addr >> 16);  // LBA 23-16
            outb( ata.cmd_block_base_address + ATA_REG_DEVSEL,0x40 | ata.dev_num << 4);   // Modo LBA active, Select device,        
            
			// Verifique se e a mesma unidade para n�o esperar pelos 400ns.
            if( ata_record_dev != ata.dev_num && 
			    ata_record_channel != ata.channel )
			{
                ata_wait(400);
                ata_record_dev     = ata.dev_num;
                ata_record_channel = ata.channel;
			};
            break;
			
        case 0:
            // Modo CHS
            // not supported
            break;

        // Default ??

    };
       
//done:
    // ??
};


/*
 ***********************************************************
 * ide_read_sector:
 *     READ SECTOR
 *
 */
_i32 ide_read_sector( _i8 nport, 
                      _u16 count, 
					  _u64 addr, 
					  _void *buffer )
{ 
    
    // Select device, lba, count
    ata_set_device_and_sector( count, addr, current_dev->dev_access, nport );

    if( current_dev->dev_modo_transfere == ATA_DMA_MODO )
    {
	    switch( nport )
		{
            case 0: 
			    dma_addr = (_u8*) DMA_PHYS_ADDR0;
                break;
				
            case 1: 
			    dma_addr = (_u8*) DMA_PHYS_ADDR1;
                break;
				
            case 2: 
			    dma_addr = (_u8*) DMA_PHYS_ADDR2;
                break;
				
            case 3: 
			    dma_addr = (_u8*) DMA_PHYS_ADDR3;
                break;
				
            // ?? default
        };
	};

    // ??
	
    if( current_dev->dev_access == ATA_LBA28 && 
	    current_dev->dev_modo_transfere == ATA_PIO_MODO )
	{
        ata_cmd_write( ATA_CMD_READ_SECTORS );
		
        // #bugbug 
		// @todo:
		// //FIXME IRQs
		// ata_wait_irq(); 
        
		ata_wait_not_busy();
		
        if( ata_wait_drq() != 0 )
		{
            // ?? Message			
		    return -1;
		}
        
		ata_pio_read( buffer, current_dev->dev_byte_per_sector );
        ata_wait_not_busy();
        
		if( ata_wait_no_drq() != 0 )
		{

            // ?? Message
            return -1;
        }

    }else if( current_dev->dev_access == ATA_LBA48 && 
	          current_dev->dev_modo_transfere == ATA_PIO_MODO)
		  {

            ata_cmd_write(ATA_CMD_READ_SECTORS_EXT);
            
			// #bugbug 
			// @todo
			// FIXME IRQs
			// ata_wait_irq(); 
            
			ata_wait_not_busy();
            
			if( ata_wait_drq() != 0 )
			{ 
		        return -1; 
			};
			
            ata_pio_read(buffer,current_dev->dev_byte_per_sector);
            ata_wait_not_busy();
            
			if( ata_wait_no_drq() != 0 )
			{
                // ?? Message
                return -1;
            }
            
          }else if( current_dev->dev_access == ATA_LBA28 && 
		            current_dev->dev_modo_transfere == ATA_DMA_MODO )
		        {

                    ide_dma_data( dma_addr, 
			                      current_dev->dev_byte_per_sector*count, 
							      0x0, 
							      nport );
							 
                    ata_cmd_write(ATA_CMD_READ_DMA);
                    ide_dma_start();
                    ata_wait_irq();
            
                    while( ide_dma_read_status() &1 )
			        {
                        if(ide_dma_read_status() &2)
			            {
                            ide_dma_stop();
                            // ?? Message 					
                            return -1;
                        }
			        };
                    ide_dma_stop();
            

                __asm__ __volatile__("cld; rep; movsd;"::"D"(buffer),\
                    "S"(dma_addr),"c"((current_dev->dev_byte_per_sector*count)/4));
            
                }else if( current_dev->dev_access == ATA_LBA48 && 
		                  current_dev->dev_modo_transfere == ATA_DMA_MODO )
		              {
                          ide_dma_data( dma_addr,
				                        current_dev->dev_byte_per_sector*count, 
							            0x0,
							            nport );    
                
				          ata_cmd_write( ATA_CMD_READ_DMA_EXT );
                          ide_dma_start();
                          ata_wait_irq();
            
                          while(ide_dma_read_status() &1 )
			              {
                              if( ide_dma_read_status() &2 )
				              {
                                  ide_dma_stop();
                                  // Message 
                                  return -1;
                              }
			              };

                          ide_dma_stop();
          
                          __asm__ __volatile__("cld; rep; movsd;"::"D"(buffer),\
                                  "S"(dma_addr),"c"((current_dev->dev_byte_per_sector*count)/4));
         
                     };

done:        
    return 0;
};


/*
 ***********************************************************
 * ide_write_sector:
 *     WRITE SECTOR
 *
 */
_i32 ide_write_sector( _i8 nport, 
                       _u16 count, 
					   _u64 addr, 
					   _void *buffer )
{    
    //select device, lba, count
    ata_set_device_and_sector( count,addr,current_dev->dev_access, nport );

    if( current_dev->dev_modo_transfere == ATA_DMA_MODO )
	{
        switch(nport)
		{
            case 0: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR0;
                break;
              
			case 1: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR1;
                break;
                
			case 2: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR2;
                break;
                
		    case 3: 
			    dma_addr = (_u8*)DMA_PHYS_ADDR3;
                break;
                
			// ?? default
        };
    };

        
    if( current_dev->dev_access == ATA_LBA28 && 
	    current_dev->dev_modo_transfere == ATA_PIO_MODO )
	{
        ata_cmd_write(ATA_CMD_WRITE_SECTORS);
        //ata_wait_irq(); //FIXME IRQs
        ata_wait_not_busy();
        if(ata_wait_drq() != 0)return -1;
        ata_pio_write(buffer,current_dev->dev_byte_per_sector);

        //Flush Cache
        ata_cmd_write(ATA_CMD_FLUSH_CACHE);
        ata_wait_not_busy();
        if(ata_wait_no_drq() != 0)
		{
            return -1;
        }
    }else if( current_dev->dev_access == ATA_LBA48 && 
	          current_dev->dev_modo_transfere == ATA_PIO_MODO )
	      {

                  ata_cmd_write(ATA_CMD_WRITE_SECTORS_EXT);
                  //ata_wait_irq(); //FIXME IRQs
                  ata_wait_not_busy();
                  if( ata_wait_drq() != 0 )return -1;
                  ata_pio_write(buffer,current_dev->dev_byte_per_sector);

                  //Flush Cache
                  ata_cmd_write(ATA_CMD_FLUSH_CACHE_EXT);
                  ata_wait_not_busy();
                   if( ata_wait_no_drq() != 0 )
				   {
                       //Message ??                
                       return -1;
                   }

          }else if( current_dev->dev_access == ATA_LBA28 && 
			            current_dev->dev_modo_transfere == ATA_DMA_MODO )
			    {
            
                        __asm__ __volatile__("cld; rep; movsd;"::"S"(buffer),\
                        "D"(dma_addr),"c"((current_dev->dev_byte_per_sector*count)/4));


                        ide_dma_data(dma_addr,current_dev->dev_byte_per_sector*count,0x1,nport);    
                        ata_cmd_write(ATA_CMD_WRITE_DMA);

                        ide_dma_start();

                        ata_wait_irq();
						
                        while( ide_dma_read_status() &1 )
                        {
		                    if( ide_dma_read_status() &2 )
				            {
                
				                ide_dma_stop();
                                //Message 
                                return -1;
                            }
			            }			
                        ide_dma_stop();
                        //Flush Cache
                        ata_cmd_write(ATA_CMD_FLUSH_CACHE); 

                }else if( current_dev->dev_access == ATA_LBA48 && 
					          current_dev->dev_modo_transfere == ATA_DMA_MODO )
					  {

                              __asm__ __volatile__("cld; rep; movsd;"::"S"(buffer),\
                              "D"(dma_addr),"c"((current_dev->dev_byte_per_sector*count)/4));            

                              ide_dma_data(dma_addr,current_dev->dev_byte_per_sector*count,0x1,nport);    
                              ata_cmd_write(ATA_CMD_WRITE_DMA_EXT);

                              ide_dma_start();

                               ata_wait_irq();
							   
                               while( ide_dma_read_status() &1 )
			                   {
                                   if( ide_dma_read_status() &2 )
				                   {
                                       ide_dma_stop(); 
                                       return -1;
                                   }
			                   }
                               
							   ide_dma_stop();
                               //Flush Cache
                               ata_cmd_write(ATA_CMD_FLUSH_CACHE_EXT);  
			
                       };


done:
    return 0;
};



/**
 * Credits:
 * Copyright (C) 2017-2018 (Nelson Sapalo da Silva Cole)
 * Khole OS v0.3
 * Legacy Bus Master Base Address
 *
 *  TODO Nelson, j� mais se esque�a de habiliatar o // Bus Master Enable
 *  no espa�o de configura�ao PCI (offset 0x4 Command Register)
 *
 * Obs: O que segue � um suporte ao controlador de DMA para uso nas
 * rotinas de IDE.
 *
 */

 
//
// DMA support
//

// Commands dma 
#define dma_bus_start   1
#define dma_bus_stop    0
#define dma_bus_read    0
#define dma_bus_write   1


// Status dma
#define ide_dma_sr_err     0x02


// Registros bus master base address
#define ide_dma_reg_cmd     0x00
#define ide_dma_reg_status  0x02
#define ide_dma_reg_addr    0x04

// channel
#define ide_dma_primary     0x00
#define ide_dma_secundary   0x01


/*
 * ide_dma_prdt:
 *
 */
struct {
    uint32_t addr;
    uint32_t len;
}ide_dma_prdt[4];


/*
 * ide_dma_data:
 *     ??
 *
 */
void ide_dma_data( void *addr, 
                   uint16_t byte_count,
				   uint8_t flg,
				   uint8_t nport )
{
    _u8 data;
    uint32_t phy;
	
    //
    // @todo: Check limits.
    //
	
    ide_dma_prdt[nport].addr = (_u32) addr;  //@todo: (&~1)sera que e necessario?
    ide_dma_prdt[nport].len  = byte_count |0x80000000;

    phy = (uint32_t) &ide_dma_prdt[nport];

    // prds physical.
    outportl( ata.bus_master_base_address + ide_dma_reg_addr, phy );
 
    // (bit 3 read/write)
    // 0 = Memory reads.
    // 1 = Memory writes.
	
    data = inb( ata.bus_master_base_address + ide_dma_reg_cmd ) &~8;

	//
	// TODO bit 8 Confilito no Oracle VirtualBox
	// Obs: Isso foi enviado via argumento e agora foi alerado.
	//
	
    flg = 1;  
	
    outb( ata.bus_master_base_address + ide_dma_reg_cmd, data | flg << 3 );
	
    // Limpar o bit de interrup��o e 
	// o bit de erro no registo de status.
	
    data = inb( ata.bus_master_base_address + ide_dma_reg_status );
    outb( ata.bus_master_base_address + ide_dma_reg_status, data &~6 );

done:
    return;	
};


/*
 * ide_dma_start:
 *
 */
void ide_dma_start()
{
    _u8 data = inb( ata.bus_master_base_address + ide_dma_reg_cmd );
    outb( ata.bus_master_base_address + ide_dma_reg_cmd, data | 1);
};


/*
 * ide_dma_stop:
 *
 */
void ide_dma_stop()
{
    _u8 data = inb( ata.bus_master_base_address + ide_dma_reg_cmd );  
	outb( ata.bus_master_base_address + ide_dma_reg_cmd, data &~1);
	
    data = inb( ata.bus_master_base_address + ide_dma_reg_status );
    outb( ata.bus_master_base_address + ide_dma_reg_status, data &~6);
	
done:	
    return;	
};


/*
 * ide_dma_read_status:
 *     DMA read status.
 */
int ide_dma_read_status()
{
    return inb( ata.bus_master_base_address + ide_dma_reg_status );
};


//
// pci support
//


#define PCI_PORT_ADDR 0xCF8
#define PCI_PORT_DATA 0xCFC

#define CONFIG_ADDR(bus,device,fn,offset)\
                       (\
                       (((uint32_t)(bus) &0xff) << 16)|\
                       (((uint32_t)(device) &0x3f) << 11)|\
                       (((uint32_t)(fn) &0x07) << 8)|\
                       ((uint32_t)(offset) &0xfc)|0x80000000)
					   
					   
const char *pci_classes[] = {
    "Unknown [old]",
    "Mass storage",
    "Network",
    "Display",
    "Multimedia device",
    "Memory",
    "Bridge device",
    "Simple Communication",
    "Base System Peripheral",
    "Input Device",
    "Docking Station",
    "Processor",
    "Serial Bus",
    "Wireless",
    "Inteligent I/O",
    "Satellite Communications",
    "Encrypt/Decrypt",
    "Data acquisition and signal processing",
    [255]="Unknown"
};
					   
					   
/*
 * read_pci_config_addr:
 *     READ
 */
uint32_t read_pci_config_addr( int bus, 
                               int dev,
							   int fun, 
							   int offset )
{
    outportl( PCI_PORT_ADDR, CONFIG_ADDR( bus, dev, fun, offset ) );
	
done:
    return (uint32_t) inportl(PCI_PORT_DATA);
};


/*
 * write_pci_config_addr:
 *     WRITE
 */
void write_pci_config_addr( int bus, 
                            int dev,
							int fun, 
							int offset, 
							int data )
{
    outportl(PCI_PORT_ADDR,CONFIG_ADDR(bus,dev,fun, offset));
    outportl(PCI_PORT_DATA,data);
};





/*
 ********************************************************************
 * pci_scan_device:
 *
 *     Esta fun��o deve retornar o n�mero de barramento, 
 * o dispositivo e a fun��o do dispositivo conectado ao barramento PCI 
 * de acordo a classe.
 *
 */
uint32_t pci_scan_device(int classe)
{
    int bus, dev, fun;
    
	uint32_t data = -1;

	
#ifdef KERNEL_VERBOSE		
	printf("sm-disk-disk-pci_scan_device:\n");
    refresh_screen();
#endif
    
    //
	// Probe.
	//
	
	for( bus=0; bus < 256; bus++ )
	{
        for( dev=0; dev < 32; dev++ )
		{
            for( fun=0; fun < 8; fun++ )
			{
                outportl( PCI_PORT_ADDR, CONFIG_ADDR( bus, dev, fun, 0x8) );
                data = inportl(PCI_PORT_DATA);
                
				if( ( data >> 24 &0xff ) == classe )
				{
					
					
                    // Message.
#ifdef KERNEL_VERBOSE							
					kprintf( "[ Detected PCI device: %s ]\n", 
					         pci_classes[classe] );
#endif
							 
					//
					// Done !
					//
                    
					return (uint32_t) ( fun + (dev*8) + (bus*32) );
                }
            };
     
        };
    };

	//
	// Fail !
	//
	
fail:	
    
	kprintf("[ PCI device NOT detected ]\n");		
	refresh_screen();
	
    return (uint32_t) (-1);
};


/*
 **********************************************************
 * show_ide_info:
 *     Mostrar as informa��es obtidas na inicializa��es 
 * do controlador.
 */
void show_ide_info()
{
	printf("sm-disk-disk-show_ide_info:\n");
	
	//
	// Estrutura 'ata'
	// Qual lista ??
	//
	
	//pegar a estrutura de uma lista.
	
	//if( ata != NULL )
	//{
		printf("ata:\n");
 	    printf("type={%d}", (int) ata.chip_control_type);
	    printf("channel={%d}", (int) ata.channel);
	    printf("devType={%d}", (int) ata.dev_type);
	    printf("devNum={%d}", (int) ata.dev_num);
	    printf("accessType={%d}", (int) ata.access_type);
	    printf("cmdReadMode={%d}", (int) ata.cmd_read_modo);
	    printf("cmdBlockBaseAddress={%d}", (int) ata.cmd_block_base_address);
	    printf("controlBlockBaseAddress={%d}", (int) ata.ctrl_block_base_address);
		printf("busMasterBaseAddress={%d}", (int) ata.bus_master_base_address);
		printf("ahciBaseAddress={%d}", (int) ata.ahci_base_address);
	//};
	
	
	//
	// Estrutura 'atapi'
	// Qual lista ??
	//	
	
	
	//
	// Estrutura 'st_dev'
	// Est�o na lista 'ready_queue_dev'
	//		
	
	

   //...
	
done:
    refresh_screen();
    return;	
};

//
// End.
//

