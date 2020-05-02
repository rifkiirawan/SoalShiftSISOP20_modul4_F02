# SoalShiftSISOP20_modul4_F02
Praktikum Modul 4 Sisop 2020

Kelompok F02 (0099 &amp; 0142)

# Pembahasan Jawaban

### Source Code : [ssfs.c](https://github.com/rifkiirawan/SoalShiftSISOP20_modul4_F02/blob/master/ssfs.c)

## No 1 Program Enkripsi V1
Pada no.1 diminta untuk mengenkripsi isi dari direktori dengan menggunakan metode caesar cipher dengan key = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO" dengan key = 10. Jika sebuah direktori memiliki diberi nama "encv1_", maka isi dari direktori tersebut akan dienkripsi, dan jika direktori yang telah dienkripsi di-rename, maka direktori tersebut akan di dekripsi.
Untuk pengimplementasian caesar cipher seperti berikut :
```
void encrypt(char *x)
{
    int xlength = strlen(x), xbegin = 0;
    int i;
    for (i = strlen(x); i >= 0; i--){
        if (x[i] == '/'){
            break;
        }
        if (x[i] == '.'){
            xlength = i - 1;
        }
    }
    for (int i = 1; i < xlength; i++){
        if (x[i] == '/'){
            xbegin = i;
        }
    }
    int ind;
    char *ptr;
    for (i = xbegin; i < xlength; i++){
        if (x[i] == '/'){
            continue;
        }
        ptr = strchr(cipher, x[i]);
        if (ptr){
            ind = ptr - cipher;
            x[i] = cipher[(ind + key) % strlen(cipher)];
        }
    }
}

void decrypt(char *y){
    int ylength = strlen(y), ybegin = 0;
    int i;
    for (int i = 1; i < ylength; i++){
        if (y[i] == '/' || y[i + 1] == '\0'){
            ybegin = i + 1;
            break;
        }
    }

    for (int i = strlen(y); i >= 0; i--){
        if (y[i] == '/'){
            break;
        }
        if (y[i] == '.' && i == (strlen(y) - 1)){
            ylength = strlen(y);
            break;
        }
        if (y[i] == '.' && i != (strlen(y) - 1)){
            ylength = i - 1;
            break;
        }
    }
    int ind;
    char *ptr;
    for (i = ybegin; i < ylength; i++){
        if (y[i] == '/'){
            continue;
        }
        ptr = strchr(cipher, y[i]);
        if (ptr){
            ind = ptr - cipher - key;
            if (ind < 0){
                ind = ind + strlen(cipher);
            }
            y[i] = cipher[ind];
        }
    }
}
```
Dalam pengenkripsian, akan perlu memanggil fungsi `xmp_readdir`, seperti berikut ini :
```
    while ((de = readdir(dp)) != NULL){
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0){
            continue;
        }
        char temp[1000];
        strcpy(temp, de->d_name);
        if (strncmp(path, "/encv1_", 7) == 0){
            encrypt(temp);
        }
        res = (filler(buf, temp, &st, 0));
        if (res != 0)
            break;
    }
```
Setelah dilakukan enkripsi, perlu untuk melakukan dekripsi, akan dipanggil pada setiap fungsi fuse, pengimplementasiannya sebagai berikut :
```
if (strncmp(path, "/encv1_", 7) == 0){
            decrypt(temp);
        }
```

## No 2 Program Enkripsi V2

## No 3 Program Sinkronisasi Directory Otomatis

```
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
```
Fungsi di atas adalah fungsi untuk men-sinkronkan antara 2 folder dengan perintah rsync. Untuk name1 dibelakangi dengan / agar folder itu sendiri tidak tercopy pada folder name2.

```
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
```
Fungsi Checkdir untuk cek directory yang sebelumnya sudah dicek bahwa folder kedua adalah folder sync dari folder 1. Dalam fungsi ini di cek, di dalam kedua folder tersebut isinya sama atau tidak, baik folder atau file. Kemudian dicek juga apabila file, maka dicek apakah last modified nya kurang dari 0.1 detik. Apabila sudah melewati semua tahap pemeriksaan, kedua directory artinya sama.

```
void sinkronisasi(char *fpath){
    int flag=0;
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
        // printf("%s\n",arr[j]->d_name);
        struct stat st;
        int sinc=0;
        char next[500],namadir[500],namadir2[500];
        sprintf(namadir,"sync_%s",arr[j]->d_name);
        sprintf(next,"%s/%s/",fpath,arr[j]->d_name);
        sprintf(namadir2,"%s/%s/",fpath,namadir);
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
                // printf("sink %s %s",next, namadir2);
                strcpy(simpan[flag],next);
                flag+=1;
                strcpy(simpan[flag],namadir2);
                flag++;
            }
        }
        if(arr[j]->d_type == 4){
            // printf("recur %s\n",next);
            sinkronisasi(next);
        }
        j++;
    }
}
```
Fungsi sinkronisasi digunakan untuk cek apakah folder tersebut mempunyai folder sync nya. Apabila ada, maka di checkdir keduanya apakah isinya benar-benar sama atau tidak. Apabila sama, kedua path folder tersebut disimpan dalam array simpan, untuk pengecekan pada pengubahan salah satu folder. Sinkronisasi ini dilakukan recursive.

```
char lokasi[500]; int i,temp=0;
strcpy(lokasi,fpath);
for(i=0;i<strlen(fpath);i++){
    if(lokasi[i]=='/') {
        temp++;
    }
    if(temp>=5) {
        lokasi[i+1]='\0';
    }
}
for(i=0;i<20;i++){
    if(strcmp(lokasi,simpan[i])==0&&i%2==0) rsinkronisasi(lokasi,simpan[i+1]);
    else if(strcmp(lokasi,simpan[i])==0&&i%2==1) rsinkronisasi(lokasi,simpan[i-1]);
}
```
Fungsi ini terdapat pada setiap fungsi yang melakukan pengubahan pada salah satu folder. Fungsi ini digunakan untuk mengecek, apakah path dari file/folder yang diubah merupakan folder yang mempunyai sync nya, oleh karena itu dicek pada array simpan yang pada awal program dijalankan. Apabila path tersebut mempunyai folder sync nya, maka dilakukan rsync pada kedua folder.

## No 4 Log System

```
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
```
Pertama, membuat atau menambahkan pada file fs.log yaitu di append. Kemudian dicek apabila rmdir atau unlink, maka level harus warning, selain itu info. Untuk tahun agar hanya 2 digit belakang, maka dikurangi 2000.


Print dalam file dengan format [LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC ...]

```
if(res == -1)
	return -errno;
else{
  char deskrip[1000];
  sprintf(deskrip,"%s",fpath);
  logs("CREAT",deskrip);
}
```
Fungsi dipanggil di dalam setiap fungsi yang memerlukan log. Log dicatat hanya saat fungsi berhasil dijalankan.
## Authors

Created by:

[Rifki Aulia Irawan 142](https://github.com/rifkiirawan)

[Evelyn Tjitrodjojo 99](https://github.com/marsellaeve)
