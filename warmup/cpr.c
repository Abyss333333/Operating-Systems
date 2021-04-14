#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>




/* make sure to use syserror() when a system call fails. see common.h */

void
usage()
{
	fprintf(stderr, "Usage: cpr srcdir dstdir\n");
	exit(1);
}

void copy_file (char *cp_file, char *dest_dir, char *new_file_name){
     //open file
    int fd;
    int flags = 0;
   // char pathname [50] = "/homes/s/siddi558/ece344/warmup/test/Temp.txt";
    char pathname [50];
    strcpy(pathname, cp_file);
    
    fd = open (pathname, flags);
    
    if (fd<0) {
          syserror(open, pathname);
  }
    
    // read file 
    char buf[100000];
    int ret = 1;
    
    
        ret = read(fd,buf,100000);
       
    if (ret<0) {
          syserror(read, pathname);
  }
        

    


    //check stat of file to get permission
    struct stat sfile;
    int st;
    st = stat( pathname, &sfile);
    if (st<0){
        syserror(stat, pathname);
    }
    mode_t val = sfile.st_mode & ~S_IFMT;
    
    // create a file
    //char new_file [50] = "/homes/s/siddi558/ece344/warmup/test/New.txt";
    char new_file [50]; 
    strcpy(new_file,dest_dir);
    strcat(new_file,"/");
    strcat(new_file, new_file_name);
    int cf; //created file
    cf= creat (new_file, 0777);
    
    if (cf<0){
        syserror(creat, new_file);
    }

    
    

   // write to a file 
    int wf; //write to file
    wf= write(cf,buf,ret );
    
    
    if (wf<0){
        syserror(write,new_file);
    }
     
    //setting permissions for new file
    int ch_mod;
    ch_mod = chmod(new_file, val);
    
    if (ch_mod < 0){
        syserror(chmod, new_file);
    }
    
    // close a file, fd doesnt point to that file anymore and can be reused
    int cl_f;
    cl_f = close(fd);
    if (cl_f<0){
        syserror(close, pathname);
    }
    cl_f = close(cf);
    if (cl_f<0){
        syserror(close, new_file);
    }
    memset(buf, 0, strlen(buf));
}

void make_dir(char *src, char *dest){
    
    
    

    //printf("%s\n", dest);
    // making the directory
    int mk_dir;
    mk_dir = mkdir(dest, 0777);
    
    if (mk_dir < 0){
        syserror(mkdir, dest);
    }
    
   
    


}

void copy_dir(char *src, char *dest){
    
   //Opening a Directory
    
    DIR*dir= NULL;
    dir = opendir(src);
    
    if (dir == NULL) {
        syserror(opendir, src);
    }

    //reading a directoy
    struct dirent *rd;
    struct stat sfile;
    while ( (rd = readdir(dir)) != NULL){
        //printf("%s\n", rd->d_name);
        char aa[50];
        strcpy(aa,src);
        if (strcmp(rd->d_name,".") == 0 || strcmp(rd->d_name,"..")==0){
            strcpy(aa, rd->d_name);
        }
        else{
            strcat(aa,"/");
            strcat(aa,rd->d_name);
        }
            
        int st;
        
        st = stat(aa, &sfile);
        
        if (st<0){
            syserror(stat, aa);
        }
        if (S_ISREG(sfile.st_mode)){
            //printf("%s is a file\n", aa);
            copy_file(aa, dest, rd->d_name);
         }
        if (S_ISDIR(sfile.st_mode)){
              //printf("%s is a dir\n", aa);
           // copy_dir(
             char new[50];
             strcpy(new,dest);
             strcat(new,"/");
             strcat(new, rd->d_name);
             if (strcmp(rd->d_name,".") != 0 && strcmp(rd->d_name,"..")!=0){
                //printf("%s www \n", rd->d_name);
                make_dir(aa,new);
                copy_dir(aa,new);
                
                //setting permission                
                
                mode_t val = sfile.st_mode & ~S_IFMT;
                //setting permission for initial directory 
                int ch_mod;
                ch_mod = chmod(new, val);
    
                if (ch_mod < 0){
                    syserror(chmod, dest);
                }
             }      
             
             
        }
        
    }
    int cl_d;
    cl_d = closedir(dir);
    if (cl_d<0){
        syserror(closedir, src);
    }
    
    

}

int
main(int argc, char *argv[])
{
	//if (argc != 3) {
		//usage();
	//}
	//TBD();

   

    
    make_dir(argv[1],argv[2]);
   

    //open_read_close_dir(argv[1]);
    copy_dir(argv[1],argv[2]);

    
    
   
     //check stat
    //struct stat sfile;
    //int st;
    //st = stat( src, &sfile);

    //if (st<0){
     //   syserror(stat, src);
   // }
    //if (S_ISREG(sfile.st_mode)){
      //  printf("file\n");
   // }
    //if (S_ISDIR(sfile.st_mode)){
     //   printf("dir\n");
    //}
    
    
    
    
    
    
   
    
	return 0;
} 

