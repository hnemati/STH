
/*
 * this file will load ELF symbols so we can enter variable and functions names
 * in the debugger instead of using their addresses
 */ 
    
#include <stdio.h>
#include <stdlib.h>
    
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char
    
	
	
	
	
	
	
	
	
	
	
	


	
	
	
	
	
	


	
	
	
	
	
	
	


	
	
	


	
	
	




// #define IS_INTERNAL_LABEL(name) (name[0] == '$' || strstr(name, ".isra.1") != 0)
#define IS_INTERNAL_LABEL(name) (name[0] == '$')
    
/************************************************************************** 
 * 
 **************************************************************************/ 
void do_read_symbol_table(FILE * fp, uint32_t offset, uint32_t size,
			  uint32_t entsize) 
{
	
	
	
	
		
			sizeof(symbol));
		
	
	
	
	
		
		
		
		
		
		    /* only intrested in objects and functions */ 
		    if (type != 0 && type != 1 & type != 2)
			continue;
		
		    /* null name */ 
		    if (symbol.name == 0)
			continue;
		
//        if(symbol.size == 0) continue;
		    
//        printf("SYMBOL %d adr=%08lx size = %d, type=%d, bind=%d, idx=%d\n", symbol.name, symbol.value, symbol.size, type, bind, symbol.index); // DEBUG
		    
		    // allocate and put it on the list
		    s = (symbol_chain *) malloc(sizeof(symbol_chain));
		
		
		
		
		
	
	



			     uint32_t entsize) 
{
	
		
		
	
	
	
	


/************************************************************************** 
 * 
 **************************************************************************/ 
void doLoadSymbols(const char *filename) 
{
	
	
	
	
	
	
		
		
	
	
	
		
		
	
	
	
	    /*
	       printf("ELF ID = %x%c%c%c\n", header.id[0], header.id[1], header.id[2], header.id[3]);
	       printf("ELF type = %d, machine = %d, version = %d, entry = %08lx\n",
	       header.type, header.machine, header.version, header.entry);
	       
	       printf("ELF %ph/sh-off=%08lx/%08lx, flags=%08lx\n", header.phoff, header.shoff, header.flags);
	       printf("ELF: eh_size=%d, ph=%d/%d, sh=%d/%d, string_index=%d\n",
	       header.eh_size,
	       header.ph_size, header.ph_count,
	       header.sh_size, header.sh_count,
	       header.string_index
	       );
	     */ 
	    if (header.id[0] == 0x7F && header.id[1] == 'E'
		&& header.id[2] == 'L' && header.id[3] == 'F') {
		
			
			       SEEK_SET);
			
			
			
				
						      section.size,
						      section.entsize);
				
			
				
//                if(i == header.string_index) 
				    do_read_string_table(fp, section.offset,
							 section.size,
							 section.entsize);
				
			
		
	
		printf("ERROR: file is not an ELF\n");
	
	
		
		
	



{
	
	
		return 0;
	
		return 0;
	
		
		
			
			
		
		
	
	



{
	
	
		return 0;
	
		
			
			
				
		
		
	
	



/* ------------------------------------------------------------------ */ 

{
	



{
	



			  int before) 
{
	
	
	
		return 0;
	
		adr++;		/* dont give us our own! */
	
		
		
			delta = -delta;
		
			
			
				
				
					*out_adr = s->addr;
				
					*out_label = name;
			
		
		
	
	


