#include "hyper_config_base.h"
 
/*
 * Bitmask constants for specifying guest mode
 * contexts that can be get/set.
 */ 
#define HC_GM_TRUSTED_MASK   (1 << HC_GM_TRUSTED)
#define HC_GM_KERNEL_MASK    (1 << HC_GM_KERNEL)
#define HC_GM_INTERRUPT_MASK (1 << HC_GM_INTERRUPT)
#define HC_GM_TASK_MASK      (1 << HC_GM_TASK)
    
/*
 * Guest mode access to certain domains
 * ********************************************************/ 
    
#define HC_DOMAC_ALL \
    ((1 << (2 * HC_DOM_DEFAULT)) | \
     (1 << (2 * HC_DOM_TASK)) | \
     (1 << (2 * HC_DOM_KERNEL)) | \(1 << (2 * HC_DOM_TRUSTED)))  
#define HC_DOMAC_KERNEL  \
    ((1 << (2 * HC_DOM_DEFAULT)) | \(1 << (2 * HC_DOM_KERNEL)))  
#define HC_DOMAC_TRUSTED \
    ((1 << (2 * HC_DOM_DEFAULT)) | \(1 << (2 * HC_DOM_TRUSTED)))  
#define HC_DOMAC_INTERRUPT HC_DOMAC_ALL
    
#define HC_DOMAC_TASK \
    ((1 << (2 * HC_DOM_DEFAULT)) | \(1 << (2 * HC_DOM_TASK)))  
/*************************************************************/ 
    
/*
 * Guest mode capabilities
 */ 
#define HC_CAP_TASK 0		// No capabilities
    
#define HC_CAP_INTERRUPT \
    (HC_CAP_GET_MODE_CONTEXT | \HC_CAP_SET_MODE_CONTEXT)  
#define HC_CAP_KERNEL \
    (HC_CAP_SET_MODE_CONTEXT | \HC_CAP_GET_MODE_CONTEXT)  
#define HC_CAP_TRUSTED 0	// No capabilities
    
/*************************************************************/ 
    
/*
 * Configuration for guest modes
 */ 
static const hc_guest_mode 
    gm_trusted = { .name = "trusted", .domain_ac = HC_DOMAC_TRUSTED, 
}, gm_kernel = {
.name = "kernel", .domain_ac = HC_DOMAC_KERNEL, }, gm_task = {
.name = "application", .domain_ac = HC_DOMAC_TASK, }, gm_interrupt = {
.name = "interrupt", .domain_ac = HC_DOMAC_INTERRUPT, } ;

 
/*
 * Guest configuration structure
 */ 
    hc_config minimal_config = {
	.guest_entry_point = (0xc0000000), .guest_modes = {
	&gm_trusted, &gm_kernel, &gm_task, &gm_interrupt}, .reserved_va_for_pt_access_start = 0x0, .reserved_va_for_pt_access_end = 0x002fffff, .pa_for_pt_access_start = HAL_PHYS_START + 0x01000000, .pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff, .pa_initial_l1 = HAL_PHYS_START + 0x01000000 + 0x00200000	// Initial address + 2MB
};


