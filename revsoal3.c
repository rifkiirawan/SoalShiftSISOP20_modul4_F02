#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
char *dirpath = "/home/evelyn/Documents";
char simpan[20][500];
int flag=0;

void rsinkronisasi (char name1[500],char name2[500]){
    pid_t child_id;
    int status;
    child_id = fork();
    if (child_id < 0) {
        exit(EXIT_FAILURE);
    }
    if (child_id == 0) {
        char *argv[] = {"rsynz", "-avu","--delete",name1,name2, NULL};
        execv("/usr/bin/rsync", argv); 
    }
    else while ((wait(&status)) > 0);
}
bool checkDir(char* path1, char* path2){ //udh dipastikan path1 dan path2 ada
    DIR *dp = opendir(path1);
    struct dirent *de;
    while((de=readdir(dp))!=NULL){
        if(strcmp(de->d_name,".")==0) continue;
	    if(strcmp(de->d_name,"..")==0) continue;
        struct stat st1;
        struct stat st2;
        struct tm *foo1;
        struct tm *foo2;
        char name2[500];
        sprintf(name2,"%s/%s",path2,de->d_name);
        char name1[500];
        sprintf(name1,"%s/%s",path1,de->d_name);
        struct stat buffer;
        int exist = stat(name2,&buffer);
        if(de->d_type==4&&opendir(name2)!=NULL){
            if(checkDir(name1,name2)==false) return false;
        }
        else if(de->d_type==4){
            return false;
        }
        else if(exist>=0){
            stat(name1, &st1);
            foo1 = gmtime(&(st1.st_mtime));
            stat(name2, &st2);
            foo2 = gmtime(&(st2.st_mtime));
            int same=0;
            if(foo1->tm_year==foo2->tm_year&&foo1->tm_mon==foo2->tm_mon&&foo1->tm_mday==foo2->tm_mday&&
            foo1->tm_hour==foo2->tm_hour&&foo1->tm_min==foo2->tm_min&&(foo1->tm_sec-foo2->tm_sec<0.1&&
            foo2->tm_sec-foo1->tm_sec<0.1)) same=1;
            //bandingin file1 dan file 2
            if(same==0) return false;
        }
        else{
            return false;
        }
    }
    return true;
}
 
void sinkronisasi(char *fpath){
	DIR *dp;
	struct dirent *de;
    dp = opendir(fpath);
	if (dp == NULL)
		return;
    struct dirent *arr[500];
    int i=0;
    while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,".")==0) continue;
    	if(strcmp(de->d_name,"..")==0) continue;
        arr[i]=de;
        i++;
	}
    int j=0;
    while(j<i){
        struct stat st;
        int sinc=0;
        char next[500],namadir[500],namadir2[500];
        sprintf(namadir,"sync_%s",arr[j]->d_name);
        sprintf(next,"%s/%s",fpath,arr[j]->d_name);
        sprintf(namadir2,"%s/%s",fpath,namadir);
        memset(&st, 0, sizeof(st));
        int k=0;
        while (k<i) {
            if(strcmp(namadir,arr[k]->d_name)==0&&arr[k]->d_type==4&&arr[j]->d_type==4) sinc=1;
            k++;
        }
        if(sinc){
            bool sama=true;
            if(arr[j]->d_type==4){
                sama = sama&&checkDir(next,namadir2)&&checkDir(namadir2,next);
            }
            if(sama){
                sprintf(next,"%s/",next);
                strcpy(simpan[flag],next);
                flag+=1;
                sprintf(namadir2,"%s/",namadir2);
                strcpy(simpan[flag],namadir2);
                flag++;
            }
        }
        if(arr[j]->d_type == 4){
            sinkronisasi(next);
        }
        j++;
    }
}

void logs (char* cmd, char* deskripsi){
    char level[20],backupname[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *file = fopen("/home/evelyn/fs.log","a");
    if(strcmp(cmd,"RMDIR")==0||strcmp(cmd,"UNLINK")==0){
        strcpy(level,"WARNING");
    }
    else{
        strcpy(level,"INFO");
    }
    int year=tm.tm_year+1900-2000;
    sprintf(backupname, "%s::%02d%02d%02d-%02d:%02d:%02d::%s::%s", level,year, tm.tm_mon + 1, tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec, cmd,deskripsi);
    fprintf(file,"%s",backupname);
    fprintf(file,"\n");
    fclose(file);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
    
    // char deskrip[1000];
    // sprintf(deskrip,"%s::%d",fpath,mask);
    // logs("ACCESS",deskrip);
	
	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	
	res = readlink(fpath, buf, size - 1);

	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s",fpath,buf);
        logs("READLINK",deskrip);
    }

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	
	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode)){
        res = mkfifo(fpath, mode);
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("MKFIFO",deskrip);
    }
		
	else{
        res = mknod(fpath, mode, rdev);
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("MKNOD",deskrip);
    }
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("MKDIR",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = unlink(fpath);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("UNLINK",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
    return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = rmdir(fpath);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("RMDIR",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;
    char fpath[1000],tpath[1000];
    if(strcmp(from,"/") == 0){
        from=dirpath;
        sprintf(fpath,"%s",from);
    }
    else sprintf(fpath, "%s%s",dirpath,from);
	if(strcmp(to,"/") == 0){
        to=dirpath;
        sprintf(tpath,"%s",to);
    }
    else sprintf(tpath, "%s%s",dirpath,to);
	res = symlink(fpath, tpath);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s",fpath, tpath);
        logs("SYMLINK",deskrip);
    }

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
    char fpath[1000],tpath[1000];
    if(strcmp(from,"/") == 0){
        from=dirpath;
        sprintf(fpath,"%s",from);
    }
    else sprintf(fpath, "%s%s",dirpath,from);
	if(strcmp(to,"/") == 0){
        to=dirpath;
        sprintf(tpath,"%s",to);
    }
    else sprintf(tpath, "%s%s",dirpath,to);
	res = rename(fpath, tpath);
    
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s",fpath, tpath);
        logs("RENAME",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;
    char fpath[1000],tpath[1000];
    if(strcmp(from,"/") == 0){
        from=dirpath;
        sprintf(fpath,"%s",from);
    }
    else sprintf(fpath, "%s%s",dirpath,from);
	if(strcmp(to,"/") == 0){
        to=dirpath;
        sprintf(tpath,"%s",to);
    }
    else sprintf(tpath, "%s%s",dirpath,to);
	res = link(fpath, tpath);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s",fpath, tpath);
        logs("LINK",deskrip);
    }

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("CHMOD",deskrip);
    }

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("LCHOWN",deskrip);
    }

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("TRUNCATE",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
    return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;
	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("UTIMES",deskrip);
    }

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("OPEN",deskrip);
    }

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

    int res;
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s",fpath);
        logs("CREAT",deskrip);
    }
    close(res);

    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	(void) fpath;
	(void) fi;
    char deskrip[1000];
    sprintf(deskrip,"%s",fpath);
    logs("RELEASE",deskrip);
    
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	(void) fpath;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	int res = lsetxattr(fpath, name, value, size, flags);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s::%s::%d",fpath,name,value,flags);
        logs("LSETXATTR",deskrip);
    }
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	int res = lgetxattr(fpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	int res = llistxattr(fpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
	int res = lremovexattr(fpath, name);
	if (res == -1)
		return -errno;
    else{
        char deskrip[1000];
        sprintf(deskrip,"%s::%s",fpath,name);
        logs("REMOVE",deskrip);
    }
    int i,j;
    char lokasi[500];
    for(i=0;i<strlen(fpath);i++){
        strncpy(lokasi,fpath,i);
        lokasi[i+1]='\0';
        lokasi[i]='\0';
        if(lokasi[i-1]=='/') {
            for(j=0;j<flag;j++){
                if(strcmp(lokasi,simpan[j])==0&&j%2==0){
                    rsinkronisasi(lokasi,simpan[j+1]);
                }
                else if(strcmp(lokasi,simpan[j])==0&&j%2==1) {
                    rsinkronisasi(lokasi,simpan[j-1]);
                }
            }
        }
    }
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create         = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
    sinkronisasi(dirpath);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}