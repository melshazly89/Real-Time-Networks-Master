// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// August 29, 2016
#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512]; // temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active

// Return the larger of two integers.
int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}
//*****MountDirectory******
// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
void MountDirectory(void){ 
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
// **write this function**
	if(bDirectoryLoaded==0)
	{
		eDisk_ReadSector(Buff,255);
		
		for(int i=0;i<=255;i++)
		{
		Directory[i]=Buff[i];
		FAT[i]=Buff[i+256];
		}
		bDirectoryLoaded=1;
	}
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
// **write this function**

	uint8_t m=0;
  if(start==255)        //Searching of Last Sector           
	{
		return 255;
	}
	else
	{
		m=FAT[start];
		while(m!=255)	
		{
			start=m;
			m=FAT[start];
		}
			return start;
		
	}	
   
	}
        


// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
// **write this function**
  int16_t fs=-1;
	uint8_t i=0;
	uint8_t ls=0;
	
	while(1)
	{
		 ls=lastsector(Directory[i]);
		 if(ls==255){return fs+1;}
		   fs=max(fs,ls);
			 i=i+1;
			 
		 
			 
    }
	
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
// **write this function**
  uint8_t i=0;
	uint8_t m=0;
	i=Directory[num];      //Current File Number
	if(i==255)
	{
		Directory[num]=n;
	}
	else if(i!=255)
	{
		m=FAT[i];  
		while(m!=255)
		{
			i=m;
			m=FAT[i];
		}
		FAT[i]=n;
		
	}
	
  return 0; // replace this line
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void){
// **write this function**
	uint8_t i=0;
	MountDirectory();
	while(Directory[i]!=255)
	{
		i++;
		if(i==255)
		{
			return 255;
		}
	}
 
  return i;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
// **write this function**
  uint8_t i;
	uint8_t m;
	uint8_t Sector=0;
	i=Directory[num];
 if(i!=255)
 {
	 m=FAT[i];
	 while(m!=255)
	 {
		 i=m;
		 m=FAT[i];
		 Sector++;
	 }
	 return Sector+1;
 }
	return 0;
 
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
// **write this function**
	int32_t n;   //Sector Number
	if (bDirectoryLoaded == 0)
	{
  MountDirectory();
	}
  n=findfreesector();
	if(n!=255)
	{
		eDisk_WriteSector(buf,n);
		appendfat(num,n);
		return 0;
	}
	else
	{
	return 255;
	}
}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
// **write this function**
		
	uint8_t sector;
	sector=Directory[num];	
  while(location!=0)
  {
		
		sector=FAT[sector];
		location--;
		if(sector==255)
			{
			return 255;
			}			
	}
	eDisk_ReadSector(&buf[0],sector);
	return 0;
    

		
		
	}




//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){        //Storing Data From RAM to ROM
// **write this function**
int iRC;
int16_t i;
for(i=0;i<512;i++)
	{
		if(i<256)
		{
		Buff[i]=Directory[i];
		}
		else
		{
	   Buff[i]=FAT[i-256];
		}
	}
	iRC=eDisk_WriteSector(Buff,255);
	if(iRC==RES_ERROR)
	{
	return 255;
	}
	else
	{
  return 0; // replace this line
	}
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero
// **write this function**


int iRC;
int16_t i;
iRC=eDisk_Format();
if(iRC==RES_OK)
{
	for(i=0;i<=255;i++)
	{
	Directory[i]=0xFF;
	FAT[i]=0xFF;
	}
}
	bDirectoryLoaded=0;
	
		

  return iRC; // replace this line
}
