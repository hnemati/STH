#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>
    
/**********************************************************
 * parsing and IO
 **********************************************************/ 
int is_space(char c) 
{
	



{
	
	
	
		
		     || buffer[count] == '\n')
			
	
	
	
	    /* empty line ? */ 
	    if (count == 0)
		return 0;
	



{
	
	
		return 0;
	
		buffer++;
	
	
		a++;
	
	
		
	



{
	
		return 0;
	
		buffer++;
	
		return 0;
	
	
		buffer++;
	
		return 0;
	



/* number parsing: since some ugly hacks are just to cute...
 * handle digits and hex startitg with 0x 
 */ 
int getNumberFromString(char *str, int *result) 
{
	
	
		return 0;
	
		str++;
	
		return 0;	/* empty */
	
		
		
	
		
	
	
	
	
		
		
			break;	/* end of the number */
		
			n = c - '0';
		
		else if (c >= 'A' && c <= 'F')
			n = c - 'A' + 10;
		
		else if (c >= 'a' && c <= 'f')
			n = c - 'a' + 10;
		
		else
			n = -1;
		
			return 0;	/* bad char */
		
		
	
	
		return 0;	/* nothing */
	
	



/***********************************************************
 * params
 ***********************************************************/ 
    
#define PARAM_COUNT 10
char *params[PARAM_COUNT];



{
	
	
	
	    /* get parameters */ 
	    for (params_count = 0; params_count < PARAM_COUNT;) {
		
		
			break;
		
		
	
	
	    /* get flags */ 
	    for (i = 0; i < 256; i++)
		params_flags[i] = 0;
	
		
		
			
			
				
		
			
		
	
	



{
	



{
	



{
	



{
	



{
	
	
	
		return default_;
	
	
	
		
	


