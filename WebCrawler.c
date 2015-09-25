/* Author Bharath Chandra Elluru.
 *This program searches for a specific type of files in specified folders and subfolders
 *and lists them in a file.
 *Date:05/27/2014, Version:1.0.
 */
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
//Structure used to store the path and name of the file.
struct bbFile_Attr{
  char *bbFilePath;
  char *bbFileName;
  char *bbFileRevisionNo;
  char *bbFileVersionNo;
  char *bbFilemd5Sum;
  char *bbFilesha256Sum;
};
//Defining a structure that can hold up to 2500 files.
//Change the length here if you have more than 2500 lines
struct bbFile_Attr bbFile_list[2500];
char *path="";
char *temppath[70];
int flag=0,count=0;
int totalNoOfBbFiles=0;
//This method is used to search through the folders and subfolders to find for a file of type mentioned in my_pattern
int file_select(const struct dirent *entry)
{
  int  r = 1;
//The pattern of the file which we are searching for, *.bb or *.inc
  char my_pattern[] = "*.bb";
  if(entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0  && strcmp(entry->d_name, "..") != 0){
    flag++;
    struct dirent **namelist;
    char *new_path="";
    int n,count1=0;
    new_path=malloc(sizeof(char) * strlen(path) * 30);
    strcpy(new_path,temppath[flag]);
    strcat(new_path,"/");
    strcat(new_path,entry->d_name);
    strcpy(temppath[flag+1],new_path);
    n=scandir(new_path, &namelist, file_select, alphasort);
    flag--;
    if (n < 0)
      perror("scandir");
    else {
      while (n-- && strcmp(namelist[n]->d_name, ".") != 0  && strcmp(namelist[n]->d_name, "..") != 0 && namelist[n]->d_type != DT_DIR) {			
	bbFile_list[count].bbFilePath=malloc(sizeof(char) * strlen(new_path) + 1);
	bbFile_list[count].bbFileName=	malloc(sizeof(char) * strlen(namelist[n]->d_name) + 1);		
	strcpy(bbFile_list[count].bbFilePath,new_path);
	bbFile_list[count].bbFileName=namelist[n]->d_name;
	count++;	
	count1++;
      }
    }
    totalNoOfBbFiles=totalNoOfBbFiles+count1;
    free(new_path);
  }
  else{
    r = fnmatch(my_pattern, entry->d_name, FNM_PERIOD);
  }
  return (r==0)?1:0;
}

void split(char* cmd, char *args[],char *arg){
  char *p;
  int j=0;
  p=strtok(cmd,arg);  	
  while(p != NULL)
    {
      args[j]=p;
      j++;
      p=strtok(NULL,arg);		
    }
  args[j]='\0';
}


int main(void)
{
  struct dirent **namelist;
  int k,j;
  char str1[20], str2[20], str3[20];
  char temp[512];
  char *filePath="";
  char *fileName="";
  char *revision[10];
  char *versiontemp[10];
  char *version[10];
  char *md5CheckSum[10];
  char *md5CheckSumtemp[10];
  char *sha256CheckSum[10];
  char *sha256CheckSumtemp[10];
  int n,count=0,i;
  FILE *fp,*fw;
//Change the path here
  path="/usr/local/bin/poky";
  for(k=0;k<70;k++){
    temppath[k]=malloc(sizeof(char) * strlen(path) * 30);
  }		
  strcpy(temppath[1],path);
//Change the path here
  n = scandir("/usr/local/bin/poky", &namelist, file_select, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while (n-- && namelist[n]->d_type != DT_DIR) {
      count++;
    }
  }
  
  totalNoOfBbFiles=totalNoOfBbFiles+count;
  printf("totalNoOfBbFiles are %d\n", totalNoOfBbFiles);
  for(j=0;j<70;j++){
    free(temppath[j]);
  }
  fw=fopen("ListOfBBFiles.txt", "w");
//Listing down all the files. 
//Change the length here if you have more than 2500 lines
  for(i=0;i<2500;i++){
    if(bbFile_list[i].bbFilePath == NULL)
	break;
    printf("%d %s %s\n",i,bbFile_list[i].bbFilePath,bbFile_list[i].bbFileName);
    filePath=malloc(sizeof(char) * strlen(path) * 30);
    strcpy(filePath,bbFile_list[i].bbFilePath);
    strcat(filePath,"/");
    strcat(filePath,bbFile_list[i].bbFileName);
    fprintf(fw, filePath);
    fprintf(fw, "\n");
    fp=fopen(filePath, "r");
    while(fgets(temp, 512, fp) != NULL) {
      if((strstr(temp, "PR = ")) != NULL) {
	split(strstr(temp, "PR = "),revision," ");
	bbFile_list[i].bbFileRevisionNo=malloc(sizeof(char) * strlen(revision[2]) + 1);
	strncpy(bbFile_list[i].bbFileRevisionNo,revision[2]+1,strlen(revision[2]));
	bbFile_list[i].bbFileRevisionNo[strlen(bbFile_list[i].bbFileRevisionNo)-2]='\0';
      }
      if((strstr(temp, "md5sum]")) != NULL){
	split(strstr(temp,"md5sum]"),md5CheckSumtemp,"=");
	split(md5CheckSumtemp[1],md5CheckSum,"\"");
	bbFile_list[i].bbFilemd5Sum=malloc(sizeof(char) * strlen(md5CheckSum[1]) + 1);
	strcpy(bbFile_list[i].bbFilemd5Sum,md5CheckSum[1]);
      }
      if((strstr(temp, "sha256sum]")) != NULL){
	split(strstr(temp,"sha256sum]"),sha256CheckSumtemp,"=");
	split(sha256CheckSumtemp[1],sha256CheckSum,"\"");
	bbFile_list[i].bbFilesha256Sum=malloc(sizeof(char) * strlen(sha256CheckSum[1]) + 1);
	strcpy(bbFile_list[i].bbFilesha256Sum,sha256CheckSum[1]);
      }
    }
    fclose(fp);
    free(filePath);
    fileName=malloc(sizeof(char) * strlen(bbFile_list[i].bbFileName) + 1);
    strcpy(fileName,bbFile_list[i].bbFileName);
    if((strstr(fileName, "_")) != NULL) {
      split(fileName,versiontemp,"_");
      split(versiontemp[1],version,"bb");
      bbFile_list[i].bbFileVersionNo=malloc(sizeof(char) * strlen(version[0]) + 1);
      strcpy(bbFile_list[i].bbFileVersionNo,version[0]);
      bbFile_list[i].bbFileVersionNo[strlen(bbFile_list[i].bbFileVersionNo)-1]='\0';
    }
    free(fileName);
  }
  fclose(fw);
  
  
}

