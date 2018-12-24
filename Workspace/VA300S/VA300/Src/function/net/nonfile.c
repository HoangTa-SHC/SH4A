/******************************************************************************
* NORTi Network File System (16-bit FAT)                                      *
*                                                                             *
*  Copyright (c) 1999-2005, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 07/Mar/1999                                                                 *
* 06/May/1999 'remove' is added.                                              *
* 23/May/1999 Append mode ("a") is added to fopen.                            *
* 23/May/1999 'feof' and 'rename' is added.                                   *
* 13/Jun/1999 'fseek' bug is fexied.                                          *
* 22/Sep/1999 resource ID auto assign added.                                  *
* 29/Nov/2000 18個以上のファイルを置けない不具合修正                          *
* 29/Nov/2000 fgets には '\n' を含む                                          *
* 01/May/2001 disk_iniにfile_ini完了チェック追加                              *
* 12/Api/2001 GAIOの予約語と衝突のため direct→_direct                        *
* 18/Jul/2001 get_disk()の追加, END_CLUSTER_NUMBER(+2)                        *
* 12/Sep/2001 f->bytenoを中心にfile_seek()の見直し                 M.Kawahara *
* 13/Sep/2001 SINGLE_TASK, USE_FSYS_ERCDマクロの追加修正           M.Kawahara *
* 13/Sep/2001 make_dir_list()バグp->d_inoの値をFAT1216と同じにした M.Kawahara *
* 14/Sep/2001 エラーコードの設定を中心に見直し、FSYS_ER(E_OK)は削除M.Kawahara *
* 15/Sep/2001 new_sector()new_cluster()==0のエラー処理を修正       M.Kawahara *
* 18/Dec/2001 get_disk()エラーコード消去処理を追加                 M.Kawahara *
* 17/Jan/2002 B*をchar*に修正、ワーニングの消去                    M.Kawahara *
* 08/Feb/2002 厳密すぎる型チェック回避 for CC32R                           OK *
* 27/Mar/2002 my_fopen()多重オープンロジックの見直し                       KA *
*               リードオープン中のファイルをライトオープンできないよう修正    *
* 19/JUN/2002 END_CLUSTER_NUMBERマクロの修正 SHC7.0.03バグ回避             KA *
* 20/Dec/2002 ID_FILE_SEM, ID_FILE_TSK, NFILE の型誤りを修正               KA *
* 03/Feb/2003 (get_le_h(p) == 0)をハンドオプティマイズ-> (*(UH*)p == 0)    KA *
* 06/Jun/2003 ID_FILE_TSK を削除、warnを減らすためだけの修正、             KA *
* 13/May/2005 Modified V3 -> V4                                            AK *
* 30/Apr/2009 Added the 'disk' variable initialization.                    FY *
* 19/May/2009 Modified the processing of 'donefini'.                       FY *
******************************************************************************/
/* #define SINGLE_TASK */
/* #define USE_FSYS_ERCD */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef GAIO
#include <memory.h>
#endif
#include "kernel.h"
#include "nonfile.h"
#include "nonitod.h"
#ifdef DEBUG
#include "nonet.h"
#include "nonteln.h"
#endif

/* Dummy System Call in the case of Single Task */
#ifdef SINGLE_TASK
static ER my_cre_sem(void)  { return E_OK; }
static ER my_vcre_sem(void) { return E_OK; }
static ER my_sig_sem(void)  { return E_OK; }
static ER my_wai_sem(void)  { return E_OK; }
#undef  cre_sem
#undef  acre_sem
#undef  sig_sem
#undef  wai_sem
#define cre_sem(id,csem)  my_cre_sem()
#define acre_sem(csem)    my_vcre_sem()
#define sig_sem(id)       my_sig_sem()
#define wai_sem(id)       my_wai_sem()
#endif

/* 繰り返しコードのサブルーチン化 */
static ER get_disk(const char** path, T_DISK **ppd);

/* Disk Parameter */
#define SECTORS_PER_CLUSTER     (d->sc)
#define RESERVED_SECTOR_COUNT   (d->rsc)
#define NUMBER_OF_FATS          (d->fn)
#define ROOT_DIR_ENTRYS         (d->rde)
#define TOTAL_SECTORS           (d->ts)
#define SECTORS_PER_FAT         (d->sf)
#define SECTORS_PER_DIR         ((ROOT_DIR_ENTRYS*DIR_ENTRY_SZ-1)/SECTOR_SIZE+1)
#define SYSTEM_SECTOR_AREA      (d->rsc+(d->sf)*(d->fn)+SECTORS_PER_DIR)
#define END_CLUSTER_NUMBER      ((TOTAL_SECTORS-SYSTEM_SECTOR_AREA+2)/SECTORS_PER_CLUSTER)
#define DIR_ENTRYS_PER_SECTOR   (SECTOR_SIZE/DIR_ENTRY_SZ)
#define FAT_ENTRYS_PER_SECTOR   (SECTOR_SIZE/2)

#ifndef _WIN32
int stricmp(const char *d, const char *s);
#endif

/* Local Variable */
static INT NFILE;
static T_FILE *file;
static T_DISK *disk;
static ID ID_FILE_SEM;
static const T_CSEM file_csem  = { TA_TFIFO, 0, 1, "file_sem" };
static B donefini;          /* file_ini完了フラグ */

#define DONEFINI_CODE       ((B)0xa5)

/* Referenceable Variable */
ER FSYS_ER(ER cd);
#ifdef USE_FSYS_ERCD
UH FSYS_ERCD = 0;
#define FSYS_ER(cd)  (FSYS_ERCD = (UH)(cd), (ER)cd)
#else
#define FSYS_ER(cd)  ((ER)cd)
#endif

#ifdef DEBUG
/************************************************/
/* For Debugging                                */
/************************************************/
static void file_debug(B *s, W d)
{
    B buf[32];

    ltod(buf, d, 8);
    print(s);
    puts(buf);
}
#endif

/************************************************/
/* Little Endian Data Access                    */
/* リトルエンディアン のデータアクセス          */
/*   DOS形式のディスク上はリトルエンディアン    */
/************************************************/
static void set_le_h(VP p, UH d)
{
    UB *pp = (UB *)p;

    *pp++ = (UB)d; d >>= 8;
    *pp   = (UB)d;
}

static UH get_le_h(VP p)
{
    UB *pp = (UB *)p + 1;
    UH d;

    d = *pp--; d <<= 8;
    d |= *pp;
    return d;
}

static void set_le_w(VP p, UW d)
{
    UB *pp = (UB *)p;

    *pp++ = (UB)d; d >>= 8;
    *pp++ = (UB)d; d >>= 8;
    *pp++ = (UB)d; d >>= 8;
    *pp   = (UB)d;
}

static UW get_le_w(VP p)
{
    UB *pp = (UB *)p + 3;
    UW d;

    d  = *pp--; d <<= 8;
    d |= *pp--; d <<= 8;
    d |= *pp--; d <<= 8;
    d |= *pp;
    return d;
}

/************************************************/
/* Convert File Name to Internal Style          */
/* ファイル名を11文字の内部形式へ               */
/************************************************/
static UB *path_to_name(const char *path, UB *name)
{
    INT i;
    B c, dot;

    for (dot = FALSE, i = 0; i < 8+3; )
    {   c = *path++;
        if (c == ' ')
            continue;
        else if (c == '\0')
        {   for (; i < 8+3; )
                name[i++] = ' ';
            break;
        }
        else if (c == '.')
        {   if (dot)
                return NULL;
            dot = TRUE;
            for (; i < 8; )
                name[i++] = ' ';
        }
        else
        {   if (i >= 8 && !dot)
                return NULL;
            name[i++] = (UB)toupper(c);
        }
    }
    return name;
}

/************************************************/
/* Recover File Name from Internal Style        */
/************************************************/
static char *name_to_path(const UB *name, char* path)
{
    char* p;
    UB c;
    INT i;

    p = path;
    for (i = 0; i < 8+3; )
    {   if (i == 8)
            *p++ = '.';
        c = name[i];
        if (c == ' ')
        {   if (i >= 8)
                break;
            i = 8;
        }
        else
        {   *p++ = (char)c;
            i++;
        }
    }
    if (*(p - 1) == '.')
        p--;
    *p = '\0';
    return path;
}

/************************************************/
/* Convert Cluster Number to Sector Number      */
/* クラスタ番号→セクタ番号 変換                */
/************************************************/
static UW cluster_to_sector(T_DISK *d, UH clusno)
{
    return (UW)(clusno - 2) * SECTORS_PER_CLUSTER + SYSTEM_SECTOR_AREA;
}

/************************************************/
/* Convert Sector Number to Cluster Number      */
/* セクタ番号→クラスタ番号 変換                */
/************************************************/
static UH sector_to_cluster(T_DISK *d, UW sectno)
{
    return (UH)((sectno - SYSTEM_SECTOR_AREA) / SECTORS_PER_CLUSTER + 2);
}

/************************************************/
/* Read FAT Sector                              */
/* FATセクタ読み出し                            */
/************************************************/
static ER read_fat(T_DISK *d, INT fatno, UB **buf)
{
    /* Calculate FAT Sector Number */

    if (fatno >= (INT)SECTORS_PER_FAT)
        return EV_DISKFULL;
    fatno += RESERVED_SECTOR_COUNT;

    /* Read FAT Sector */
    return (*d->func)(d, TFN_READ_SECTOR, fatno, buf);
}

/************************************************/
/* Write FAT Sector                             */
/* FATセクタ書き込み                            */
/************************************************/
static ER write_fat(T_DISK *d, INT fatno, UB *buf)
{
    /* Calculate FAT Sector Number */

    if (fatno >= (INT)SECTORS_PER_FAT)
        return EV_DISKFULL;
    fatno += RESERVED_SECTOR_COUNT;

    /* Write FAT Sector */
    return (*d->func)(d, TFN_WRITE_SECTOR, fatno, buf);
}

/************************************************/
/* Read Root Directry Sector                    */
/* ルートディレクトリセクタの読み出し           */
/************************************************/
static ER read_dir(T_DISK *d, INT dirno, UB **buf)
{
    /* Calculate Root Directry Sector Number */

    dirno += RESERVED_SECTOR_COUNT + SECTORS_PER_FAT * NUMBER_OF_FATS;
    if (dirno >= (INT)SYSTEM_SECTOR_AREA)
        return EV_DISKFULL;

    /* Read Root Directry Sector */
    return (*d->func)(d, TFN_READ_SECTOR, dirno, buf);
}

/************************************************/
/* Write Root Directry Sector                   */
/* ルートディレクトリセクタの書き込み           */
/************************************************/
static ER write_dir(T_DISK *d, INT dirno, UB *buf)
{
    /* Calculate Root Directry Sector Number */

    dirno += RESERVED_SECTOR_COUNT + SECTORS_PER_FAT * NUMBER_OF_FATS;
    if (dirno >= (INT)SYSTEM_SECTOR_AREA)
        return EV_DISKFULL;

    /* Write Root Directry Sector */
    return (*d->func)(d, TFN_WRITE_SECTOR, dirno, buf);
}

/************************************************/
/* Get Next Cluster Number                      */
/* 次のクラスタ番号取得                         */
/************************************************/
static UH next_cluster(T_DISK *d, UH clusno)
{
    UH n, m;
    UB *p;
    ER ercd;

    m = clusno / FAT_ENTRYS_PER_SECTOR;
    n = clusno % FAT_ENTRYS_PER_SECTOR;
    ercd = read_fat(d, m, &p);
    if (ercd != E_OK)
        return 0;
    p += n * 2;
    n = get_le_h(p);
    if (n >= 0xfff7)
        return 0;
    return n;
}

/************************************************/
/* Delete Cluster and Get Next Cluster Number   */
/* クラスタ削除 & 次のクラスタ番号取得          */
/************************************************/
static UH del_cluster(T_DISK *d, UH clusno)
{
    UH n, m;
    UB *buf, *p;
    ER ercd;

    m = clusno / FAT_ENTRYS_PER_SECTOR;
    n = clusno % FAT_ENTRYS_PER_SECTOR;

    ercd = read_fat(d, m, &buf);
    if (ercd != E_OK)
        return 0;
    p = buf + n * 2;
    n = get_le_h(p);
    set_le_h(p, 0);
    ercd = write_fat(d, m, buf);
  #ifdef DEBUG
    file_debug("Erase Sector No.", cluster_to_sector(d, clusno));
  #endif
    if (ercd != E_OK)
        return 0;
    if (n >= 0xfff8)
        return 0;
    return n;
}

/************************************************/
/* Get Next Sector Number                       */
/* 次のセクタ番号取得                           */
/************************************************/
static UW next_sector(T_DISK *d, UW sectno)
{
    UH n;

    if (SECTORS_PER_CLUSTER > 1)
    {   if ((sectno + 1 - SYSTEM_SECTOR_AREA) % SECTORS_PER_CLUSTER != 0)
            return sectno + 1;
    }
    n = next_cluster(d, sector_to_cluster(d, sectno));
    if (n == 0)
        return 0;
    return cluster_to_sector(d, n);
}

/************************************************/
/* Allocate New Cluster                         */
/* with Cluster Data Clear (Directry)           */
/* 新しいクラスタの割当                         */
/*   ディレクトリ用に内容はクリアする           */
/************************************************/
static ER new_cluster(T_DISK *d, UH clusno, UH* pNewCluster)
{
    UB *buf;
    UB *p;
    UH n, end;      /* Cluster No */
    UH rem;         /* Cluster No in Sector*/
    UH nSecNo, mm;  /* Sector No */
    ER ercd;

    end = FAT_ENTRYS_PER_SECTOR * SECTORS_PER_FAT;
    if (end > (UH)END_CLUSTER_NUMBER)
        end = (UH)END_CLUSTER_NUMBER;

    n = d->saveclus + 1;
    if (n >= end)
        n = 0;
    nSecNo = n / FAT_ENTRYS_PER_SECTOR;
    rem = n - nSecNo * FAT_ENTRYS_PER_SECTOR;
    ercd = read_fat(d, nSecNo, &buf);
    if (ercd != E_OK)
        return ercd;
    if (n < 2)
    {
        n = 2;
        p = (UB*)((UH*)buf + n);
    }
    else
    {
        p = (UB*)((UH*)buf + rem);
    }

    while ( n != d->saveclus )
    {
        if (p >= buf + SECTOR_SIZE)
        {
            ercd = read_fat(d, ++nSecNo, &buf);
            if (ercd != E_OK)
                return ercd;
            p = buf;
        }
        if (*(UH*)p == 0)   /* (get_le_h(p) == 0)をオプティマイズ */
        {   /* 未使用セクタを見つけた */
            d->saveclus = n;
            /* 使用済みにして */
            set_le_h(p, 0xffff);
            /* 直ぐ書く */
            ercd = write_fat(d, nSecNo, buf);
            if (ercd != E_OK)
                return ercd;
            if (clusno != 0)
            {   /* 手前のクラスタとリンク */
                mm = clusno / FAT_ENTRYS_PER_SECTOR;
                if (mm != nSecNo)
                {
                    ercd = read_fat(d, mm, &buf);
                    if (ercd != E_OK)
                        return ercd;
                }
                p = buf + (clusno - mm * FAT_ENTRYS_PER_SECTOR) * 2;
                set_le_h(p, n);
                ercd = write_fat(d, mm, buf);
                if (ercd != E_OK)
                    return ercd;
            }
            *pNewCluster = n;
            return E_OK;
        }
        if (++n < end)
        {
            p += 2;
        }
        else
        {
            nSecNo = 0;
            ercd = read_fat(d, nSecNo, &buf);
            if (ercd != E_OK)
                return ercd;
            n = 2;
            p = (UB*)((UH*)buf + n);
        }
    }
    return EV_DISKFULL;
}

/************************************************/
/* Allocate New Sector                          */
/* 新しいセクタの割当                           */
/************************************************/
static ER new_sector(T_DISK *d, UW sectno, UW* pNewSectno)
{
    ER ercd;
    UH n;

    if (SECTORS_PER_CLUSTER > 1)
    {
        if ((sectno + 1 - SYSTEM_SECTOR_AREA) % SECTORS_PER_CLUSTER != 0)
        {
            *pNewSectno = sectno + 1;
            return E_OK;
        }
    }
    ercd = new_cluster(d, sector_to_cluster(d, sectno), &n);
    if (ercd != E_OK)
        return ercd;
    *pNewSectno = cluster_to_sector(d, n);
    return E_OK;
}

/************************************************/
/* Search Directry Entry by Name                */
/* ディレクトリエントリからファイル名を検索     */
/************************************************/
static ER find_dir_entry(T_DISK *d, INT *entrno, T_DIR_ENTRY *dir, const UB *name)
{
    T_DIR_ENTRY *p = NULL;
    UB *buf;
    INT n;
    ER ercd;

    for (n = 0; n < (INT)ROOT_DIR_ENTRYS; n++, p++)
    {
        if (n % DIR_ENTRYS_PER_SECTOR == 0)
        {   ercd = read_dir(d, n / DIR_ENTRYS_PER_SECTOR, &buf);
            if (ercd != E_OK)
                return ercd;
            p = (T_DIR_ENTRY *)buf;
        }
        if (name == NULL) /* Search Unused Entry */
        {   if (p->name[0] == 0x00 || p->name[0] == 0xe5)
                break;
        }
        else
        {   if (p->name[0] == 0x00) /* End of Entry Table */
            {   n = -1;
                break;
            }
            if (memcmp(p->name, name, 8+3) == 0)
                break;
        }
    }
    if (n == (INT)ROOT_DIR_ENTRYS)
        *entrno = -1;
    else
    {
        *entrno = n;
        memcpy(dir, p, DIR_ENTRY_SZ);
    }
    return E_OK;
}

/************************************************/
/* Delete Directry Entry                        */
/* ディレクトリエントリ削除                     */
/************************************************/
static ER del_dir_entry(T_DISK *d, INT entrno)
{
    T_DIR_ENTRY *p;
    UB *buf;
    UH n;
    INT m;
    ER ercd;

    m = entrno / DIR_ENTRYS_PER_SECTOR;
    ercd = read_dir(d, m, &buf);
    if (ercd != E_OK)
        return ercd;
    p = (T_DIR_ENTRY *)buf + entrno % DIR_ENTRYS_PER_SECTOR;
    n = get_le_h(&p->clusno);
    if (n != 0 && d->saveclus > n - 1)
        d->saveclus = n - 1;
    p->name[0] = 0xe5;
    ercd = write_dir(d, m, buf);
    if (ercd != E_OK)
        return ercd;
    while (n != 0)
        n = del_cluster(d, n);
    return E_OK;
}

/************************************************/
/* Set Directry Entry                           */
/* ディレクトリエントリセット                   */
/************************************************/
static ER set_dir_entry(T_DISK *d, INT entrno, T_DIR_ENTRY *dir)
{
    T_DIR_ENTRY *p;
    UB *buf;
    INT m;
    ER ercd;

    m = entrno / DIR_ENTRYS_PER_SECTOR;
    ercd = read_dir(d, m, &buf);
    if (ercd != E_OK)
        return ercd;
#ifdef TSTAMP
    tstamp( dir );
#endif
    p = (T_DIR_ENTRY *)buf + entrno % DIR_ENTRYS_PER_SECTOR;
    memcpy(p, dir, DIR_ENTRY_SZ);
    return write_dir(d, m, buf);
}

/************************************************/
/* Set File Size of Directry Entry              */
/* ディレクトリエントリにファイルサイズセット   */
/************************************************/
static ER set_filesize(T_DISK *d, INT entrno, UW size)
{
    T_DIR_ENTRY *dir;
    UB *buf;
    INT m;
    ER ercd;

    m = entrno / DIR_ENTRYS_PER_SECTOR;
    ercd = read_dir(d, m, &buf);
    if (ercd != E_OK)
        return ercd;
    dir = (T_DIR_ENTRY *)buf + entrno % DIR_ENTRYS_PER_SECTOR;
    set_le_w(&dir->size, size);
    return write_dir(d, m, buf);
}

/************************************************/
/* File System Initialize                       */
/* ファイルシステム初期化                       */
/************************************************/
ER file_ini(T_FILE *f, INT nfile, ID tskid, ID semid)
{
    ER ercd;

    memset(f, 0, sizeof (T_FILE) * nfile);
    file = f;
    NFILE = nfile;
    ID_FILE_SEM = semid;
    disk = NULL;

    if(ID_FILE_SEM){
        ercd = cre_sem(ID_FILE_SEM, &file_csem);
        if (ercd != E_OK)
            return ercd;
    } else {
        ID_FILE_SEM = acre_sem(&file_csem);
    }
    if ((ercd = sig_sem(ID_FILE_SEM)) == E_OK)
        donefini = DONEFINI_CODE;
    return ercd;
}

/************************************************/
/* Disk Drive Initialize                        */
/* ドライブ初期化(ドライバとの関連付け)         */
/************************************************/
/* RAM Disk                                     */
/*     addr: Data Address                       */
/*     par:  Data Size in bytes                 */
/* par [18...66051]* SECTOR_SIZE                */
/*     18... 66051 FAT16                        */
/*     66052... Not increase Disk space         */
/************************************************/
ER disk_ini(T_DISK *d, char* drivename, DISK_FP func, UW addr, UW par)
{
    T_DISK *p;
    ER ercd;

    if( donefini != DONEFINI_CODE){
        return FSYS_ER(-1);   /* file_ini完了チェック */
    }

    d->name = (B)toupper(drivename[0]);
    d->func = func;
    d->addr = addr;
    d->par = par;
    d->ch = 0;
    d->next = NULL;
    d->saveclus = 0;

    if (disk == NULL)
        disk = d;
    else
    {
        p = disk;
        d->ch++;
        while (p->next)
        {
            d->ch++;
            p = p->next;
        }
        p->next = d;
    }

    ercd = (*d->func)(d, TFN_HARDWARE_INIT, 0, NULL);/* 純然たるハードの初期化のみ   */
    if (ercd != E_OK)
        return ercd;
    return (*d->func)(d, TFN_IDENTIFY, 0, NULL);
}

/************************************************/
/* Make Directry List                           */
/* ディレクトリリストの生成                     */
/************************************************/
static ER make_dir_list(T_FILE *f)
{
    T_DIR_ENTRY *dir = NULL;
    T_DISK *d;
    UB *buf;
    struct _direct *p;
    UH n;
    INT i;
    ER ercd;

    d = f->d;
    p = (struct _direct *)f->buf;
    memset(p, 0, SECTOR_SIZE);
    n = f->entrno;
    ercd = read_dir(d, n / DIR_ENTRYS_PER_SECTOR, &buf);
    if (ercd != E_OK)
        return f->ercd = ercd;
    dir = (T_DIR_ENTRY *)buf;
    dir += n % DIR_ENTRYS_PER_SECTOR;

    for (i = 0; (n < (INT)ROOT_DIR_ENTRYS && i < (SECTOR_SIZE / sizeof (struct _direct))); n++, dir++, p++, i++)
    {
        if (n % DIR_ENTRYS_PER_SECTOR == 0)
        {   ercd = read_dir(d, n / DIR_ENTRYS_PER_SECTOR, &buf);
            if (ercd != E_OK)
                return f->ercd = ercd;
            dir = (T_DIR_ENTRY *)buf;
        }
        if (dir->name[0] == 0x00) /* End of Entry Table */
            break;
        name_to_path(dir->name, p->d_name);
        if (dir->name[0] == 0xe5)
            p->d_name[0] = '?';
        else {
            p->d_ino = (short)(0x1000 * d->ch + n + 1);
          #ifdef DIRECT_FSIZE
            lto_u(p->fsize, get_le_w(&dir->size), 8);
          #endif
        }
    }
    f->entrno = n;
    f->size += i * sizeof (struct _direct);
    return E_OK;
}

/************************************************/
/* File Seek                                    */
/* シーク                                       */
/************************************************/
static ER file_seek(T_FILE *f, UW pos)
{
    T_DISK *d;
    ER ercd;
    B *p;
    W m, i;
    UW n;
    UW nStartSv;     /* シーク開始セクタ */
    W nSv;          /* 前のセクタ */

    if (f->stat == 0)
        return f->ercd = EV_FSYS_PAR;
    if (pos > f->size)
        return f->ercd = EOF;
    d = f->d;
    if (f->byteno == (UH)(-1))
    {
        m = pos / SECTOR_SIZE;
        f->sectno = (UW)-1L;
        n = cluster_to_sector(d, f->start);
    }
    else
    {
        m = (W)(pos / SECTOR_SIZE - (f->pos - f->byteno) / SECTOR_SIZE);
        /* m = 0ならセクタ移動なし  */
        /* f->byteno は現在位置     */
        if (m != 0L && (f->stat & TFS_CHANGED))
        {   /* セクタ移動があり、バッファに更新がある */
            ercd = (*d->func)(d, TFN_WRITE_SECTOR, f->sectno, f->buf);
            if (ercd != E_OK)
                return f->ercd = ercd;
            f->stat &= ~TFS_CHANGED;
        }
        if (m >= 0)
            n = f->sectno;
        else
        {
            m = pos / SECTOR_SIZE;
            n = cluster_to_sector(d, f->start);
        }
    }

    nStartSv = n;
    for (i = 0L; i < m; i++)
    {
        nSv = n;
        n = next_sector(d, n);
        if (n == 0)
        {
            if (m - i == 1 && pos == f->size && (pos % SECTOR_SIZE == 0))
            {
                n = nSv;
            }
            else
            {
                if (nStartSv == cluster_to_sector(d, f->start))
                {
                    f->pos = (i+1) * SECTOR_SIZE;
                }else
                {
                    f->pos = (f->pos - f->byteno) + (i+1) * SECTOR_SIZE;
                }
                f->byteno = (UH)-1;
                return f->ercd = EV_FATLINK;
            }
            break;
        }
    }
    if (f->sectno != n)
    {
        ercd = (*d->func)(d, TFN_READ_SECTOR, n, &p);
        if (ercd != E_OK)
            return f->ercd = ercd;
        memcpy(f->buf, p, SECTOR_SIZE);
        f->sectno = n;
    }
    f->pos = pos;
    f->byteno = (UH)(pos % SECTOR_SIZE);
    if (f->byteno == 0 && pos == f->size && pos != 0)
    {
        f->byteno = SECTOR_SIZE;
    }
    return E_OK;
}

int my_fseek(FILE *fp, W pos, int mode)
{
    ER ercd;
    T_FILE* f = (T_FILE*)fp;

    if ( f == NULL ){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return -1;
    }
#ifdef XXDEBUG
    if (((f->pos - f->byteno) / 512 + 0x91) != f->sectno)
    {
        f++; f--;
    }
#endif
    wai_sem(ID_FILE_SEM);
    if (mode == SEEK_CUR)
        pos += f->pos;
    else if (mode == SEEK_END)
        pos += f->size;
    ercd = file_seek(f, pos);
    f->ercd = ercd;
    if (ercd != E_OK)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    sig_sem(ID_FILE_SEM);
    return 0;
}

/************************************************/
/* File Open                                    */
/* オープン                                     */
/************************************************/
static ER file_open(T_FILE *f, const char *path, const char *mode)
{
    T_DISK *d;
    T_DIR_ENTRY dir;
    ER ercd;
    INT entrno;
    UB name[8+3];
    UB *p;
    UH n = 0;

    ercd = get_disk(&path, &d);
    if (ercd != E_OK)
        return ercd;
    f->d = d;

    if (path[0] == '.') /* opendir */
    {   f->stat = TFS_RD_MODE;
        f->size = 0;
        f->entrno = 0;
        f->start = 0;
        f->byteno = 0;
        f->sectno = 0L;
        f->pos = 0L;
        return make_dir_list(f);
    }

    if (path_to_name(path, name) == NULL)
        return EV_NOFILE;
    ercd = find_dir_entry(d, &entrno, &dir, name);
    if (ercd != E_OK)
        return ercd;

    if (mode[0] == 'w' || mode[0] == 'a')
    {
        if (entrno != -1)
        {   if (mode[0] == 'w')
            {   ercd = del_dir_entry(d, entrno);
                if (ercd != E_OK)
                    return ercd;
            }
            else /* 'a' */
            {   n = get_le_h(&dir.clusno);
                ercd = (*d->func)(d, TFN_READ_SECTOR, cluster_to_sector(d, n), &p);
                if (ercd != E_OK)
                    return ercd;
                memcpy(f->buf, p, SECTOR_SIZE);
                f->stat = TFS_WR_MODE|TFS_OPENED;
                f->size = get_le_w(&dir.size);
                goto L1;
            }
        }
        ercd = find_dir_entry(d, &entrno, &dir, NULL);
        if (ercd != E_OK)
            return ercd;
        if( entrno == -1 )
            return EV_DISKFULL;
        ercd = new_cluster(d, 0, &n);
        if (ercd != E_OK)
            return ercd;
        memcpy(dir.name, name, 8+3);
        memset(dir.name + 8+3, 0, DIR_ENTRY_SZ - (8+3));
        set_le_h(&dir.clusno, n);
        ercd = set_dir_entry(d, entrno, &dir);
        if (ercd != E_OK)
            return ercd;
        memset(f->buf, 0, SECTOR_SIZE);
        f->stat = TFS_WR_MODE|TFS_OPENED;
        f->size = 0L;
    }
    else if (mode[0] == 'r')
    {   if (entrno == -1)
            return EV_NOFILE;
        n = get_le_h(&dir.clusno);
        ercd = (*d->func)(d, TFN_READ_SECTOR, cluster_to_sector(d, n), &p);
        if (ercd != E_OK)
            return ercd;
        memcpy(f->buf, p, SECTOR_SIZE);
        f->stat = TFS_RD_MODE|TFS_OPENED;
        f->size = get_le_w(&dir.size);
    }
    else
        return EV_FSYS_PAR;
L1:
    f->byteno = 0;
    f->pos = 0L;
    f->entrno = (UH)entrno;
    f->start = n;
    f->sectno = cluster_to_sector(d, n);
    if (mode[0] == 'a')
        return file_seek(f, f->size);
    return E_OK;
}

/* pathの正規化処理 */
ER NormalForm(char* fnbuf, const char* path)
{
    ER ercd;
    T_DISK *ppd;
    const char *f;
    char *t;

    if (path == NULL || path[0] == '\0')
        return EV_FSYS_NAME;

    ercd = get_disk(&path, &ppd);
    if (ercd)
        return ercd;
    fnbuf[0] = ppd->name;
    fnbuf[1] = ':';
    if (strlen(path) > FILENAME_MAX-2)
        return EV_FSYS_NAME;
    f = path;
    t = fnbuf +2;
    while(*f)
    {
        *t++ = (char)toupper(*f);
        f++;
    }
    *t++ = 0;
    return E_OK;
}

FILE *my_fopen(const char *path, const char *mode)
{
    T_FILE *f;
    ER ercd;
    INT n;
    const char* p;
    short wflag;
    char fnbuf[FILENAME_MAX+1];

    wai_sem(ID_FILE_SEM);

    /* 多重オープン防止ロジック */
    wflag = 0;
    for (p = mode; *p; p++)
    {
        if (strchr((char *)"+aAwW", *p) != 0)
        {
            wflag = 1;
            break;
        }
    }
    f = file;
    ercd = NormalForm(fnbuf, path);
    if (ercd)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return NULL;
    }
    for (n = 0; n < NFILE; n++, f++)
    {
        if (strcmp((char*)f->name, fnbuf) == 0)
        {
            if ((f->stat & TFS_WR_MODE) || wflag == 1)
            {
#ifdef USE_FSYS_ERCD
                FSYS_ERCD = (UH)(EV_ALREADY);
#endif
                sig_sem(ID_FILE_SEM);
                return NULL;
            }
            break;
        }
    }

    f = file;
    for (n = 0; n < NFILE; n++, f++)
    {   if (f->stat == 0)
        {   strncpy((char*)(f->name), fnbuf, FILENAME_MAX);
            ercd = file_open(f, fnbuf, mode);
            if (ercd != E_OK)
            {
#ifdef USE_FSYS_ERCD
                FSYS_ERCD = (UH)(ercd);
#endif
                memset( f->name, 0, FILENAME_MAX );
                sig_sem(ID_FILE_SEM);
                return NULL;
            }
            else
            {
                sig_sem(ID_FILE_SEM);
                return (FILE *)f;
            }
        }
    }
#ifdef USE_FSYS_ERCD
    FSYS_ERCD = (UH)(EV_NOFCB);
#endif
    sig_sem(ID_FILE_SEM);
    return NULL;
}

/************************************************/
/* File Close                                   */
/* クローズ                                     */
/************************************************/
static ER file_close(T_FILE *f)
{
    T_DISK *d;
    ER ercd;

    if (f->stat == 0)
        return f->ercd = EV_FSYS_PAR;
    d = f->d;

    if (f->stat & TFS_CHANGED)
    {
        ercd = (*d->func)(d, TFN_WRITE_SECTOR, f->sectno, f->buf);
        if (ercd != E_OK)
        {
            f->ercd = ercd;
            goto FINALIZE;  /* modefied  24/Aug/2000 */
        }
    }
    ercd = set_filesize(d, f->entrno, f->size);
    if (ercd != E_OK)
    {
       f->ercd = ercd;
        goto FINALIZE; /* modefied  24/Aug/2000 */
    }
FINALIZE:  /* modefied  24/Aug/2000 */
    f->stat = 0;
    memset( f->name, 0, FILENAME_MAX );
    return ercd;
}

int my_fclose(FILE *fp)
{
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return EOF;
    }
    wai_sem(ID_FILE_SEM);
    ercd = file_close(f);
    if (ercd != E_OK){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return  EOF;
    }
    sig_sem(ID_FILE_SEM);
    return  0;
}

/************************************************/
/* File Get Charactor                           */
/* １文字読み出し                               */
/************************************************/
static ER file_getc(T_FILE *f, UB *c)
{
    T_DISK *d;
    ER ercd;
    UB *p;
    W n;

    if (f->stat == 0)
        return E_OBJ;
    if (f->byteno == (UH)-1)
    {
        return f->ercd = EV_FATLINK;
    }
    d = f->d;

    if (f->byteno == SECTOR_SIZE)
    {
        if (f->start == 0) /* readdir */
        {   ercd = make_dir_list(f);
            if (ercd != E_OK)
                return ercd;
        }
        else
        {   if (f->stat & TFS_CHANGED)
            {   ercd = (*d->func)(d, TFN_WRITE_SECTOR, f->sectno, f->buf);
                if (ercd != E_OK)
                    return f->ercd = ercd;
                f->stat &= ~TFS_CHANGED;
            }
            n = next_sector(d, f->sectno);
            if (n == 0)
                return EOF;
            ercd = (*d->func)(d, TFN_READ_SECTOR, n, &p);
            if (ercd != E_OK)
                return f->ercd = ercd;
            memcpy(f->buf, p, SECTOR_SIZE);
            f->sectno = n;
        }
        f->byteno = 0;
    }
    if (f->pos >= f->size)
        return EOF;
    *c = *((UB *)f->buf + f->byteno++);
    f->pos++;
    return E_OK;
}

int my_fgetc(FILE *fp)
{
    ER ercd;
    UB c;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return EOF;
    }
    wai_sem(ID_FILE_SEM);
    ercd = file_getc(f, &c);
    if (ercd != E_OK){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return EOF;
    }
    sig_sem(ID_FILE_SEM);
    return  (int)c;
}

/************************************************/
/* File Put Charactor                           */
/* １文字書き込み                               */
/************************************************/
static ER file_putc(T_FILE *f, UB c)
{
    T_DISK *d;
    ER ercd;
    B *p;
    UW n;

    if (f->stat == 0)
        return f->ercd = EV_FSYS_PAR;
    if (f->byteno == (UH)-1)
    {
        return f->ercd = EV_FATLINK;
    }
    d = f->d;

    if (f->byteno == SECTOR_SIZE)
    {
        if (f->stat & TFS_CHANGED)
        {
            ercd = (*d->func)(d, TFN_WRITE_SECTOR, f->sectno, f->buf);
            if (ercd != E_OK)
                return f->ercd = ercd;
            f->stat &= ~TFS_CHANGED;
        }
        n = next_sector(d, f->sectno);
        if (n == 0)
        {
            ercd = new_sector(d, f->sectno, &n);
            if (ercd != E_OK)
                return ercd;
            memset(f->buf, 0, SECTOR_SIZE);
        }
        else
        {   ercd = (*d->func)(d, TFN_READ_SECTOR, n, &p);
            if (ercd != E_OK)
                return f->ercd = ercd;
            memcpy(f->buf, p, SECTOR_SIZE);
        }
        f->sectno = n;
        f->byteno = 0;
    }
    *((UB *)f->buf + f->byteno++) = c;
    if (++f->pos > f->size)
        f->size = f->pos;
    f->stat |= TFS_CHANGED;
    return E_OK;
}

int my_fputc(int c, FILE *fp)
{
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return EOF;
    }
    wai_sem(ID_FILE_SEM);
    ercd = file_putc(f, (UB)c);
    if (ercd != E_OK)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return EOF;
    }
    sig_sem(ID_FILE_SEM);
    return c;
}

/************************************************/
/* Put String                                   */
/* 文字列書き込み                               */
/************************************************/
int my_fputs(const char *buf, FILE *fp)
{
    int c;
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return EOF;
    }
    wai_sem(ID_FILE_SEM);
    while ((c = *buf++) != '\0')
    {   ercd = file_putc(f, (UB)c);
        if (ercd != E_OK){
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(ercd);
#endif
            sig_sem(ID_FILE_SEM);
            return EOF;   /* 07/Jul/2001 */
        }
    }
    sig_sem(ID_FILE_SEM);
    return c;
}

/************************************************/
/* Get String                                   */
/* 文字列読み出し                               */
/************************************************/
char *my_fgets(char *buf, int len, FILE *fp)
{
    INT i;
    UB c;
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return NULL;
    }
    if (--len < 0){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_FSYS_PAR);
#endif
        return NULL;
    }
    wai_sem(ID_FILE_SEM);
    for (i = c = 0; i < len; )
    {   ercd = file_getc(f, &c);
        if (ercd != E_OK)
        {
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(ercd);
#endif
            break;
        }
        if (c == '\r')  /* ignore CR */
            continue;
        buf[i++] = (B)c;
        if (c == '\n')  /* end if LF */
            break;
    }
    sig_sem(ID_FILE_SEM);

    buf[i] = '\0';
    if (i == 0 && len != 0)
        return NULL;    /* end of file */
    else
        return buf;
}

/************************************************/
/* File Read                                    */
/* 長さ指定読み出し                             */
/************************************************/

size_t my_fread(void *buf, size_t bsize, size_t n, FILE *fp)
{
    UW i, len;
    UB *b;
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    if (f == NULL)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return 0;
    }
    len = (UW)bsize * n;
    if (len == 0L)
        return 0;
    b = (UB *)buf;
    wai_sem(ID_FILE_SEM);
    for (i = 0; i < len; i++)
    {   ercd = file_getc(f, b++);
        if (ercd != E_OK)
        {
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(ercd);
#endif
            break;
        }
    }
    sig_sem(ID_FILE_SEM);
    return (size_t)(i / bsize);
}

/************************************************/
/* File Write                                   */
/************************************************/

size_t my_fwrite(const void *buf, size_t bsize, size_t n, FILE *fp)
{
    UW i, len;
    UB *b;
    ER ercd;
    T_FILE *f = (T_FILE *)fp;

    len = (UW)bsize * n;
    if (len == 0L)
        return 0;
    b = (UB *)buf;
    wai_sem(ID_FILE_SEM);
    for (i = 0; i < len; i++)
    {   ercd = file_putc(f, *b++);
        if (ercd != E_OK)
        {
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(ercd);
#endif
            break;
        }
    }
    sig_sem(ID_FILE_SEM);
    return (size_t)(i / bsize);
}

/************************************************/
/* Get Current Read/Write Position              */
/* ファイルポインタの取得                       */
/************************************************/
long my_ftell(FILE *fp)
{
    T_FILE *f = (T_FILE *)fp;
    W pos = -1;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return -1;
    }
    wai_sem(ID_FILE_SEM);
    if (f->stat != 0)
        pos = (W)f->pos;
    sig_sem(ID_FILE_SEM);
    return pos;
}

/************************************************/
/* Check End of File                            */
/* EOFチェック                                  */
/************************************************/

int my_feof(FILE *fp)
{
    T_FILE *f = (T_FILE *)fp;
    int r = 0;

    if (f == NULL){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(EV_NOOPEN);
#endif
        return 0;
    }
    wai_sem(ID_FILE_SEM);
    if (f->stat != 0)
        r = (f->pos == f->size);
    sig_sem(ID_FILE_SEM);
    return r;
}

/************************************************/
/* Remove a File                                */
/* 削除                                         */
/************************************************/
static ER file_delete(const char *path)
{
    T_DISK *d;
    T_DIR_ENTRY dir;
    ER ercd;
    INT entrno;
    UB name[8+3];

    ercd = get_disk(&path, &d);
    if (ercd != E_OK)
        return ercd;

    if (path_to_name(path, name) == NULL)
        return EV_NOFILE;
    ercd = find_dir_entry(d, &entrno, &dir, name);
    if (ercd != E_OK)
        return ercd;
    if (entrno == -1)
        return EV_NOFILE;
    ercd = del_dir_entry(d, entrno);
    return ercd;
}

int my_remove(const char* path)
{
    T_FILE *f;
    ER ercd;
    INT n;
    char fnbuf[FILENAME_MAX+1];

    wai_sem(ID_FILE_SEM);
    ercd = NormalForm(fnbuf, path);
    if (ercd)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    f = file;
    for (n = 0; n < NFILE; n++, f++)
    {   if ((f->stat != 0) && (strcmp((char*)f->name, fnbuf) == 0))
        {
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(EV_ALREADY);
#endif
            sig_sem(ID_FILE_SEM);
            return -1;
        }
    }

    ercd = file_delete(fnbuf);
    if( ercd != E_OK ){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    sig_sem(ID_FILE_SEM);
    return 0;
}

/************************************************/
/* Rename a File                                */
/* ファイル名変更                               */
/************************************************/
static ER file_rename(const char *oldpath, const char *newpath)
{
    T_DISK *d;
    T_DIR_ENTRY dir;
    ER ercd;
    INT entrno;
    UB name[8+3];
    UB newname[8+3];
    B c;

    ercd = get_disk(&oldpath, &d);
    if (ercd != E_OK)
        return ercd;
    if (newpath[1] == ':')
    {   c = (B)toupper(newpath[0]);
        if (d->name != c)
            return EV_DRIVE;
        newpath += 2;
    }
    if (newpath[0] == '\\')
        newpath += 1;

    if (path_to_name(newpath, newname) == NULL)
        return EV_NOFILE;
    ercd = find_dir_entry(d, &entrno, &dir, newname);
    if (ercd != E_OK)
        return ercd;
    if( entrno != -1 )
        return  EV_EXIST;       /* ?? */

    if (path_to_name(oldpath, name) == NULL)
        return EV_NOFILE;
    ercd = find_dir_entry(d, &entrno, &dir, name);
    if (ercd != E_OK)
        return ercd;
    if( entrno == -1 )
        return  EV_NOFILE;

    memcpy(dir.name, newname, 8+3);
    return set_dir_entry(d, entrno, &dir);
}

int my_rename(const char *oldpath, const char *newpath)
{
    T_FILE *f;
    ER ercd;
    INT n;
    char oldbuf[FILENAME_MAX+1];
    char newbuf[FILENAME_MAX+1];

    wai_sem(ID_FILE_SEM);

    ercd = NormalForm(oldbuf, oldpath);
    if (ercd)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    ercd = NormalForm(newbuf, newpath);
    if (ercd)
    {
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }

    f = file;
    for (n = 0; n < NFILE; n++, f++)
    {   if ((f->stat != 0) && (strcmp((char*)f->name, oldbuf) == 0 || strcmp((char*)f->name, newbuf) == 0))
        {
#ifdef USE_FSYS_ERCD
            FSYS_ERCD = (UH)(EV_ALREADY);
#endif
            sig_sem(ID_FILE_SEM);
            return -1;
        }
    }

    ercd = file_rename(oldbuf, newbuf);
    if(ercd != E_OK){
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    sig_sem(ID_FILE_SEM);
    return 0;
}

/************************************************/
/* Disk Fromat                                  */
/* フォーマット                                 */
/*   ★ドライバがサポートしている必要あり       */
/************************************************/
int my_dformat(const char* drivename)
{
    T_FILE *f;
    T_DISK *d;
    ER ercd;
    INT n;
    char path[3];

    wai_sem(ID_FILE_SEM);

    path[0] = drivename[0];
    path[1] = 0;
    if (path[0] != '\0')
        path[1] = ':';
    drivename = path;
    ercd = get_disk(&drivename, &d);
    if (ercd != E_OK)
        goto ERR;

    f = file;
    for (n = 0; n < NFILE; n++, f++)
    {
        if (f->stat != 0 && f->d == d)
        {
            ercd = EV_ALREADY;
            goto ERR;
        }
    }

    ercd = (*d->func)(d, TFN_FORMAT, 0, NULL);
    if (ercd != E_OK)
    {
ERR:
#ifdef USE_FSYS_ERCD
        FSYS_ERCD = (UH)(ercd);
#endif
        sig_sem(ID_FILE_SEM);
        return -1;
    }
    sig_sem(ID_FILE_SEM);
    return 0;
}

/************************************************/
/* Get T_DISK Address                           */
/************************************************/

static ER get_disk(const char** path, T_DISK **ppd)
{
    T_DISK *d;
    B c;

    if ((d = disk) == NULL)
        return FSYS_ER(EV_DRIVE);
    if ((*path)[1] == ':')
    {   c = (B)toupper((*path)[0]);
        while (d->name != c)
        {   d = d->next;
            if (d == NULL)
                return FSYS_ER(EV_DRIVE);
        }
        *path += 2;
    }
    if ((*path)[0] == '\\')
        *path += 1;
    *ppd = d;
    d->ercd = 0;        /* 18/Dec/2001 エラー処理を修正 M.Kawahara */
    return E_OK;
}

/* end */
