#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>
    
#include "icm/icmCpuManager.h"
    
/***********************************************************
 * command history
 ***********************************************************/ 
    
	
	
	




{
	
	    malloc(sizeof(command_history_t) + strlen(cmd) + 1);
	
	    /* dont grow too much */ 
	    if (history_cnt++ > 50) {
		
		
		
	
	
	
	
	
		
		
	
		
	



{
	
	
		
			return tmp;
		
	
	



{
	



{
	
	
		
		
	



{
	
	
	
		
	
		
			
		
			
		
	
		
			
				
			
				
				    ("ERROR: could not find history '%d'\n",
				     id);
			
		
			
		
	



{
	
		
		
			
			
				
			
			
		
			fprintf(stderr,
				"ERROR: unable to open history file for writing!\n");
	
		fprintf(stderr, "WARNING: found no history to save!\n");



{
	
	
	
	
		
		
			
			
			
				continue;	/* empty string */
			
				
		
		
	
		fprintf(stderr,
			"ERROR: unable to open history file '%s' for reading!\n",
			filename);


