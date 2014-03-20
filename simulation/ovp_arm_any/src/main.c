#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>
    
#include "icm/icmCpuManager.h"
    
#define MAX_CYCLES 0x0FFFFFFF
#define MEMORY_DUMP_SIZE 32
    
#define DEFAULT_RAM_BASE 0x00000000
#define DEFAULT_RAM_SIZE 0x10000000

			    
			    icmBusP * bus_);





/***********************************************************/ 
#define REGISTER_INDEX_CPSR 16
#define REGISTER_INDEX_SPSR 17
#define REGISTER_COUNT_NORMAL 16
#define REGISTER_COUNT_ALL (17 + 24)
#define REGISTERS_BANKED_START (REGISTER_INDEX_SPSR + 1)
#define REGISTERS_BANKED_END REGISTER_COUNT_ALL

    { 
"R11", "R12", "SP", "LR", "PC", "CPSR", "SPSR", 

"SPSR_UNDEF", 
"R9_FIQ", "R10_FIQ", "R11_FIQ", "R12_FIQ", 
};


    { 
"r11", "r12", "sp", "lr", "pc", "cpsr", "spsr", 
"lr_svc", "spsr_svc", 

"r11_fiq", "r12_fiq", 
};





/**********************************************************
 * number parser
 **********************************************************/ 

{
	
	
		
		
	
	



{
	
	    /* get valud from within pointer, eg [r0] [0x1234] */ 
	int adr, len = strlen(str);
	
		return 0;
	
	
	
		
	
	       (processor, (Addr) (0xFFFFFFFF & adr), result, 4, 1,
		ICM_HOSTENDIAN_HOST))
		
	



{
	
		return 0;
	
		return 1;
	
		return 1;
	
		return 1;
	
		return 1;
	
		return 1;
	



/**********************************************************
 * printing
 **********************************************************/ 

{
	
	
		
	
		  
	
	    /* instruction symbol ? */ 
	    symbol = symbol_in_instruction(processor, adr);
	
		
	



{
	
	
	
		printf("\n%s:\n", message);
	
		
	
	
	
		
	



			    int count) 
{
	
	
		return;
	
	    /* address */ 
	    printf("%08x: ", adr);
	
	    /* values */ 
	    for (i = 0; i < count; i++) {
		
		
	
	
		printf("          ");	/* space filler if less than 4 */
	
	    /* ascii */ 
	    printf("    ");
	
		
		
			
			
			
				&& (c >= 32 && c < 128) ? c : '.');
		
	

{
	
	
	
	
		  (value & (1L << 31)) ? 'N' : '-', 
		  (value & (1L << 30)) ? 'Z' : '-', 
		  (value & (1L << 29)) ? 'C' : '-', 
		  (value & (1L << 28)) ? 'V' : '-', 
		  (value & (1L << 5)) ? 'T' : '-' 
	
	
	
	
	
		tmp2 = "user";
		break;
	
		tmp2 = "FIQ";
		break;
	
		tmp2 = "IRQ";
		break;
	
		tmp2 = "supervisor";
		break;
	
		tmp2 = "abort";
		break;
	
		tmp2 = "undefined";
		break;
	
		tmp2 = "system";
		break;
	
		tmp2 = "??";
	
	
	



    work_regs2[REGISTER_COUNT_ALL];



{
	
	
	
	
	
	
		
	



{
	
	
		
			
		
	



{
	
	
	
	
		
		       (processor, (Addr) (0xFFFFFFFF & sp + i * 4), 
			4, 1, ICM_HOSTENDIAN_HOST))
			
		
			
			    /* ignore possible crap or __xxx__ address */ 
			    if (adr == 0)
				continue;
			
			
			     && (sym[0] | sym[1] | sym[len - 2] |
				 sym[len - 1]) == '_')
				
			
			
			    /* not to far ? */ 
			    if (adr > data - 1024) {
				
				    (" S[%03d] = 0x%08lx  ==> (0x%08lx) %s\n",
				     i, data, adr, sym);
			
		
	
	



/***********************************************************
 * Simulator functions
 ***********************************************************/ 
int reason_to_stop(int c) 
{
	
	
	
	
		
	
		
	



{
	
	
	
	
	
		
	
		
	



{
	
	
	     (processor, adr, &data, 4, 1, ICM_HOSTENDIAN_HOST))
		
	
		
		    /* B or BL or BLX */ 
		    rel_adr = data & 0x00FFFFFF;
		
			rel_adr |= 0xFF000000;	/* sign extends */
		
		
	
	
	    // LDR/STR rn, [PC, #n]
	    if ((data & 0x0E0F0000) == 0x040F0000) {
		
		
			rel_adr = -rel_adr;	/* up/down */
		
		
		       (processor, (Addr) (0xFFFFFFFF & abs_adr), 
			ICM_HOSTENDIAN_HOST)) {
			
		
	
	



{
	
	
	
	
	
		
	
	
	    // try to run the instruction again, now without the breakpoint
	    if (reason == ICM_SR_BP_ADDRESS && bypass_breakpoints) {
		
		
		
			setBreakpoint(processor, pc);
	
	



/* -- */ 

{
	
	
		
		
			
		
			
		
		
	
	
	
	
	
	
		
	
	    /* show frame */ 
	    icmReadReg(processor, "sp", &tmp);
	
	
		
		
				   "Possible user stack frame");
	
	
		
		
				   "Possible svc stack frame");
	



			Uns32 mark) 
{
	
	
	
		
		    (pos == pc) ? ">>>" : ((pos == mark) ? "-->" : " : ");
		
		

{
	
	

{
	
	
	
	
	
		
			
				
		
			
			
		
	
	
	
		
		    /* try to show the function we are in */ 
		int adr1, adr2, len2;
		
			adr1 = pos;
		
		else if ((unsigned int)(pos - adr1) > 256)
			adr1 = pos - 256;
		
			adr2 = pos + 12;
		
		else if ((unsigned int)(adr2 - pos) > 128)
			adr2 = pos + 128;
		
		
		
			len = len2;
	
	
		len = 12;
	
	
	



{
	
	
	
	
	
	
		
		
	
	
	
		
	
	    /* show it as ascii */ 
	    if (params_has_flag('a')) {
		
			
			     (processor, (Addr) (0xFFFFFFFF & adr + i), &data,
			      1, 1, ICM_HOSTENDIAN_HOST))
				
			
			
				break;
			
		
		
		
	
	
		
	
	
	
	
	    /* make it start at x 16 address */ 
	    count += (adr & 15) / 4;
	
	
	    /* show previous line too */ 
	    adr -= 16;
	
	
	    /* show full lines */ 
	    count = (count + 3) & (~3);
	
		
		
			
			
			
		
		
		
		       (processor, (Addr) (0xFFFFFFFF & adr), &data, 4, 1,
			ICM_HOSTENDIAN_HOST)) {
			
		
		
		
		
			
			
		
	
	



{
	
	
	
		
		
	
	
	
		
		
		
			return;
		
			return;
		
			
		
			
			     (processor, (Addr) (0xFFFFFFFF & adr), &data, 4,
			      1, ICM_HOSTENDIAN_HOST)) {
				
				    ("ERROR: could not write 0x%08lx to address 0x%08lx\n",
				     data, adr);
				
			
			
			    // readback:
			    if (!icmDebugReadProcessorMemory
				(processor, (Addr) (0xFFFFFFFF & adr), &data,
				 4, 1, ICM_HOSTENDIAN_HOST)) {
				
			
				
			
			
		
	



{
	
	
	
	    // first, update all registers
	    for (i = 0; i < REGISTER_COUNT_ALL; i++) {
		
		
		
		
			
				
			
			else
				
		
	
	
		
			
			
	
	
				 
				 reg_names_lower[REGISTER_INDEX_CPSR],
				 
	
		
		
			i++) {
			
				reg_changed[i] ? reg_names[i] :
				
			
				
			
			else
				
			
		
		
		
					 
					 reg_names_lower[REGISTER_INDEX_SPSR],
					 
	
	



{
	
	
	
	
		
		
	
	
		
		
	
	
	
		physical = 1;
	
		write_access = 1;
	
		
			
			
		
	
	



{
	
	
		
		
	
	
	    /* XXX: this wont work on thumb */ 
	    adr &= ~3;
	
		
		
	
		
		
	
	



{
	
	
	
	
	
	
	
	
		
		
		
		
	
		
		
		
		
			pc_min = 0;
		
			pc_max = -1;
		
		
		
		
			pc_max);
	
		
	
	
		
			
			
		
	
	
		
	
	
		
		
		
		
			count = 0;	// break
		
			
			
				
					count = i;
			
				
					count = i;
			
		
		
		    /* print register changes */ 
		    if (show_changes) {
			
			
		
	
	
		
	
	
		
		
			
			
			    /* if PC = LR, we assume this was a return from function */ 
			    icmReadReg(processor, "LR", &lr);
			
				
				
				    ("INFO: %s returned 0x%08lX to %s...\n",
				     from, r0, to);
			
				
				
				
				
				
				    ("INFO: Called %s(0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, ...) from %s...\n",
				     
			
				
				    ("INFO: Leaving %s and entering %s...\n",
				     from, to);
			
			
		
	
	
	    /* show the listing */ 
	    if (count > 1 || execute_function) {
		
		
	
	



{
	
	
	
	
	
	do {
		
		
		
		
	
	
		printf("Mode switch: %02x -> %02x\n", mode, tmp);
	



{
	
	
	
		
	
	
		
		
		
		
		
			
				
				    // first instruction was on a breakpoint :(
				    reason = run_one_cycle(processor, 1);
			
		
		
	
	
	
	
		
			
				steps);
		
			
			
			
				
				
				
				
				
				
				    ("Stopped at breakpoint %s(0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, ...)",
				     
				
				
				
					
				
				
			
				
			
		
	
	




{
	
		
		
		
		
			
				 filename);
			
		
		
		    /* see if a load address is given */ 
		    at = strchr(type, ':');
		
			
			
				
					
					
					    ("ERROR: invalid load address: %s\n",
					     at);
				
			
		
		
		    /* load based on extension */ 
		    if (!strcasecmp(type, ".fw") || !strcasecmp(type, ".bin")) {
			
			
			
				
					 "Could not open '%s' for reading\n",
					 filename);
				
			
			
				  adr);
			
				
				
				    // icmWriteReg(processor, "R15", (Addr) (0xFFFFFFFF & adr));
				    icmSetPC(processor,
					     (Addr) (0xFFFFFFFF & adr));
				
				    ("INFO: forcing cpu to start at 0x%08lx\n",
				     adr);
			
			
				
				
				
				
				
				
				     (processor, (Addr) (0xFFFFFFFF & adr),
				      &c, 4, 1, ICM_HOSTENDIAN_HOST)) {
					
						 "Could not write '%s' to address %08lx\n",
						 filename, adr);
					
				
				
			
			
			
		
			
			    //processor, ELF file, virtual address, enable debug, start execution spec in object file
			int ret =
			    icmLoadProcessorMemory(processor, filename, False,
						   True, need_start_binary);
			
				return 0;
			
			
			
		
			
				 filename);
			
		
	
		
	
	



{
	
	
	
	
		
		
	
	
	
	
		
	
		
	
		
			
				
				
					
						   &size);
				
				
				
			
		
		
		    // FAILED:
		    fprintf(stderr,
			    "Usage: d save <filename> <start> <size>\n");
	
		
		
			
				
			
		
		
	
		
		
			
				
			
		
		
	
		
	



{
	
		"  n [c | -f | -r]          Next c instructions, out of function, until return\n"
		
		"  c                        Next instruction until an ARM mode change is detected\n"
		
		"  b adr                    Set execution breakpoint\n" 
		"  B adr [adr] [-r | -p]    Set memory breakpoint\n" 
		"  r                        Show registers\n" 
		"  W [adr]                  Where am I in the code?\n" 
		"  m adr [len] [ -s | -a]   Dump memory contecnts. Example: 'm 0xFF00'\n"
		
		"  w adr                    Write data to memory starting at address adr.\n"
		
		"  s [-f] [adr] [len]       Show code, -f tries to show the whole function.\n"
		
		"  ! [number]               History\n" 
		"  < [filename]             Save history\n" 
		"  > [filename]             Load and replay history\n" 
		"  = [var [value]]          List/get/set variables\n" 
		"  q                        Quit\n" 

{
	
	
	


/***********************************************************
 * signal handlers
 ***********************************************************/ 

{
	
	
	


/***********************************************************
 * execute one command
 ***********************************************************/ 

{
	
	
	
	
	    /* multiple commands on the same line? */ 
	    tmp = strchr(buffer, ';');
	
		
		
			return True;
		
	
	
	
	
		return False;
	
	    /* history buffer is handled separatly before any parsing */ 
	    switch (cmd[0]) {
	
		
		
		
	
		
		
		
	
		
		
		
	
		
	
	
	    /* parse the line and execute the command */ 
	    params_set_line(cmd);
	
	
	
	
		
	
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
	
		
		
	
		
	
	



#if 0
    
/* stop excution if we see too many exceptions */ 
void notify_exception() 
{
	
#define EXCEPTION_STOP_COUNT 5
	static int cnt = 0;
	
		
		
			EXCEPTION_STOP_COUNT);
		
		
//        icmYield(processor);
	}



#endif	/* 

{
	
		 
		 
		 "\t-b <address>   set RAM base address\n" 
		 "\t-s <address>   set RAM size\n" 
		 "\t-d             use GDB\n" 
		 
	


/***********************************************************
 * main
 ***********************************************************/ 

{
	
	
	
	
	
	
	
#define LINE_SIZE (1024 * 2)    
	char buffer[LINE_SIZE];
	
	
	
	    /* parse args */ 
	    for (i = 1; i < argc; i++) {
		
		
			
			     || !strcmp(arg, "-d"))
				
			
			else if (!strcmp(arg, "-verbose")
				 || !strcmp(arg, "--verbose")
				 || !strcmp(arg, "-V"))
				
			
			else if (!strcmp(arg, "-normalrun")
				 || !strcmp(arg, "-n"))
				
			
			else if (!strcmp(arg, "-cpu") || !strcmp(arg, "-p"))
				
			
			else if (!strcmp(arg, "-b"))
				
			
			else if (!strcmp(arg, "-s"))
				
			
			else {
				
				
			
		
			
				
		
	
	
		
	
	
	
	
		  "***********************************************\n" 
		  "Configuration:\n" 
		  " Binary = %s\n" 
		  " Debug = %d, Verbose = %d\n" 
		  " CPU = %s. RAM = %08lx - %08lx\n" 
		  "***********************************************\n",
		  
		  ram_base + ram_size 
	
	    /* load the platform with the first binary */ 
	    createPlatform(cpu_name, ram_base, ram_size, debug, verbose,
			   &processor, &bus);
	
	    /* load the remaining binaries */ 
	    for (i = 0; i < binaries_count; i++) {
		
			
				 "ERROR: unable to load the binary '%s'\n",
				 binaries[i]);
			
		
	
	
	
		
		
	
	
	    // dealing with warnings
	    //    icmIgnoreMessage("ARM_MORPH_UCA");
	    
		
	
		
		
			  "***********************************************\n"
			  
			  " keep things _really_ simple...\n" 
			  "***********************************************\n"
			  
		
		    /*
		     * set this as late as possible so OVP can't override
		     */ 
		    signal(SIGINT, int_handler);
		
#if 0        
		    /* exception callback */ 
		    icmSetExceptionWatchPoint(processor, processor,
					      notify_exception);
		
#endif	/* 
		    
		    /* load startup file */ 
		    historyReplay(processor, "debugger.startup");
		
		    /*
		     * the command loop
		     */ 
		    
		
			
			
			
				
					
					    /* repeat last command */ 
					    strcpy(buffer, buffer_last);
				
					
				
			
				
			
			
		
	
	
	


