#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#ifdef __MINGW32__
#undef main
#endif

int neogeo_convert(int argc, char** argv,SDL_Surface *image,char *adresse,int force,int noalpha,int ext,int *outoff);


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *image,*copy;
    int force = 0,mode = 0,i,ok = 0;
    char adresse[1000];
    adresse[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-' || argv[i][0] == '+')
        {
            if(strcmp(argv[i],"-palette") == 0) mode = 1;
            if(strcmp(argv[i],"-simplemap") == 0) mode = 2;
            if(strcmp(argv[i],"-nomap") == 0) mode = 3;
            ok = 0;
            if(strcmp(argv[i],"-o") == 0) ok = 1;


            if(argv[i][0] == '+') force = atoi(argv[i]);

        }else
        {
            if(ok == 0) strcpy(adresse,argv[i]);
            ok = 0;
        }
    }

    if(adresse[0] == 0)
    {
        printf("Enter a picture format .png,pcx,bmp, or jpg\n");
        printf("Exemple :\nneogeoconvert myimage\n");
        return 0;
    }

    image = IMG_Load(adresse);
    if(image == NULL)
    {
        printf("Image is not valide\n");
        return 0;
    }

    copy = SDL_CreateRGBSurface(0,image->w,image->h,24,0,0,0,0);
    SDL_BlitSurface(image,NULL,copy,NULL);

    int ext = 0,offset;
    ext = neogeo_convert(argc,argv,copy,adresse,force,mode,ext,&offset);
    if(ext == 1)
    {
        force = offset;
        neogeo_convert(argc,argv,copy,adresse,force,mode,ext,&offset);
    }
    SDL_FreeSurface(copy);
    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;

}

int recup_palette(SDL_Surface *image,unsigned char *palette)
{
    int i,l;
    unsigned char *pixel = image->pixels;
    int taille = image->w*image->h*image->format->BytesPerPixel;
    int n = 0,ok;
    //printf("%d %d = %d octet\n",image->w,image->h,taille);

    for(i = 0;i < 768;i++)
        palette[i] = 0;

    for(i = 0;i < taille;i += image->format->BytesPerPixel)
    {
        ok = 0;
        for(l = 0;l < 768;l+=3)
        {
            if(palette[l+0] == pixel[i+0] && palette[l+1] == pixel[i+1] && palette[l+2] == pixel[i+2])
            {
                ok = 0;
                break;
            }else
            {
                ok = 1;
            }
        }

        if(ok == 1)
        {
            palette[n+0] = pixel[i+0];
            palette[n+1] = pixel[i+1];
            palette[n+2] = pixel[i+2];
            n +=3;
            if(n > 768) break;
        }
    }

    return n;
}

void output_filename(char *adresse,char *schaine)
{
    int l = 0;
    int i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[l] = adresse[i];
        l++;

        if(adresse[i] == '/' || adresse[i] == '\\') l = 0;
        i++;
    }
    schaine[l] = 0;
}

void tri_palette(SDL_Surface *image,int casex,int casey,unsigned char *pixel,unsigned char *palette,int *tiles)
{
    int x,y,i,l;
    int n = 0;
    int r,v,b;


    for(y = casey;y < casey+8;y++)
    {
        for(x = casex;x < casex+8;x++)
        {
            i = (y*image->w*image->format->BytesPerPixel) + x*image->format->BytesPerPixel;
            r = pixel[i+0];
            v = pixel[i+1];
            b = pixel[i+2];
            //printf("i %d x:%d y:%d %d %d %d / ",i/4,x,y,r,v,b);

            for(l = 0;l < 768;l+=3)
            {
                if(palette[l+0] == r && palette[l+1] == v && palette[l+2] == b)
                    break;
            }


            tiles[n] = l/3;

            n++;
        }

        //printf("\n-------------------------------------------\n");
    }
}

int write_rom(int argc, char** argv,SDL_Surface *image,unsigned char *pixel,unsigned char *palette,int force,int ext,int *outoff)
{
    FILE *file1,*file2;
    int casex,casey;
    int tiles[64];
    int octet4[4];
    int offset = force;
    int n = 0,i,ok = 0;
    int quad = 0,quadx,quady,x,y;
    char chaine[500];

    casex = 0;
    casey = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-' || argv[i][0] == '+')
        {
            if(strcmp(argv[i],"-o") == 0) n = i+1;
        }
    }


    if(n == 0)
    {
        file1 = fopen("052-c1.bin","rb");
        file2 = fopen("052-c2.bin","rb");
    }else
    {

        ok = 0;
        strcpy(chaine,argv[n]);

        i = 0;
        while(argv[n][i] != 0)
        {
            if(argv[n][i] == 'c' && argv[n][i+1] == '1')
            {
                chaine[i+1] = '2';
                ok = 1;
            }

            if(argv[n][i] == 'c' && argv[n][i+1] == '3')
            {
                chaine[i+1] = '4';
                ok = 1;
            }

            if(argv[n][i] == 'c' && argv[n][i+1] == '5')
            {
                chaine[i+1] = '6';
                ok = 1;
            }

            if(argv[n][i] == 'c' && argv[n][i+1] == '7')
            {
                chaine[i+1] = '8';
                ok = 1;
            }

            i++;
        }


        if(ok == 0) return 1;

        printf("rom : %s %s\n",argv[n],chaine);
        file1 = fopen(argv[n],"rb+");
        file2 = fopen(chaine,"rb+");
    }



    if(file1 == NULL || file2 == NULL)
    {
        printf("Open failed c1/c2 or c3/c4 or c5/c6 or c7/c8\n");
        return 1;
    }



    //Begin
    fseek(file1,force*64,SEEK_SET);
    fseek(file2,force*64,SEEK_SET);


    while(1)
    {
        if(quad == 0)
        {
            quadx = casex+8;
            quady = casey;
        }

        if(quad == 1)
        {
            quadx = casex+8;
            quady = casey+8;
        }

        if(quad == 2)
        {
            quadx = casex;
            quady = casey;
        }

        if(quad == 3)
        {
            quadx = casex;
            quady = casey+8;
        }

        tri_palette(image,quadx,quady,pixel,palette,tiles);


        for(y = 0;y <8;y++)
        {
            octet4[0] = 0;
            octet4[1] = 0;
            octet4[2] = 0;
            octet4[3] = 0;

            for(x = 0;x < 8;x++)
            {
                i = tiles[x + (y*8)];

                if(ext == 0 && i < 16)
                {
                    octet4[0] += ( (i>>0) & 0x01 ) << (x);
                    octet4[1] += ( (i>>1) & 0x01 ) << (x);
                    octet4[2] += ( (i>>2) & 0x01 ) << (x);
                    octet4[3] += ( (i>>3) & 0x01 ) << (x);
                }

                if(ext == 1 && i > 15)
                {
                    i -= 15;
                    octet4[0] += ( (i>>0) & 0x01 ) << (x);
                    octet4[1] += ( (i>>1) & 0x01 ) << (x);
                    octet4[2] += ( (i>>2) & 0x01 ) << (x);
                    octet4[3] += ( (i>>3) & 0x01 ) << (x);
                }

            }

            fputc(octet4[0],file1);
            fputc(octet4[1],file1);

            fputc(octet4[2],file2);
            fputc(octet4[3],file2);
        }

        quad++;
        if(quad > 3)
        {
            quad = 0;
            casex += 16;
            if(casex+16 >image->w)
            {
                casex = 0;
                casey += 16;
            }
            offset++;
        }

        if(casey+8 >image->h) break;
    }

    printf("offset : %d/%d\n",force,offset);
    *outoff = offset;

    fclose(file1);
    fclose(file2);
    return 0;
}

int write_pal(char *schaine,unsigned char *palette,int ext)
{
    FILE *file;
    char chaine[1000];
    int i,n;
    int octet4[4];


    sprintf(chaine,"palette_%s.sng",schaine);
    if(ext == 1) sprintf(chaine,"paletteext_%s.sng",schaine);
    file = fopen(chaine,"w");


    fputs("\n",file);

    sprintf(chaine,"palette_%s:\n",schaine);
    if(ext == 1) sprintf(chaine,"paletteext_%s:\n",schaine);
    fputs(chaine,file);

    int nmode = 16;

    for(i = 0;i < nmode;i++)
    {
        n = i*3;
        if(ext == 1) n += 15*3;

        octet4[0] = palette[n+0]/8;
        octet4[1] = palette[n+1]/8;
        octet4[2] = palette[n+2]/8;

        n = ( (octet4[0]& 0x1E) >> 1) + ( (octet4[1]& 0x1E) << 3) + ( (octet4[2]& 0x1E) << 7);
        n += ( (octet4[0] & 0x01) << 12) + ((octet4[1] & 0x01) << 13) + ((octet4[0] & 0x01) << 14);


        if(i%16 == 0) fputs("    dc.w ",file);

        if(i%16 < 15) sprintf(chaine,"$%.4x,",n);
        else sprintf(chaine,"$%.4x",n);

        fputs(chaine,file);

        if(i%16 == 15) fputs("\n",file);

        //printf("%s %d %d %d\n",chaine ,palette[n+0],palette[n+1],palette[n+2]);
    }

    fputs("\n",file);
    fputs("\n",file);

    fclose(file);

    return 0;
}


int write_sMap(SDL_Surface *image,char *schaine,int force,int ext)
{
    FILE *file;
    char chaine[1000];
    int i,n;
    int octet4[4];
    int mp = 0;
    int casex,casey,x,y;


    sprintf(chaine,"sMap_%s.sng",schaine);
    if(ext == 1) sprintf(chaine,"sMapext_%s.sng",schaine);
    file = fopen(chaine,"w");

    fputs("\n",file);

    sprintf(chaine,"sMap_%s:\n",schaine);
    if(ext == 1) sprintf(chaine,"sMapext_%s:\n",schaine);
    fputs(chaine,file);

    casex = (image->w/16);
    casey = (image->h/16);


    fputs("    dc.w ",file);
    for(y = 0;y < casey;y++)
    {
        mp = force + y*casex;
        if(y < casey-1) sprintf(chaine,"$%.4x,",mp);
        else sprintf(chaine,"$%.4x\n",mp);

        fputs(chaine,file);
    }

    fputs("\n",file);
    fputs("\n",file);

    fclose(file);

    return 0;
}

int write_Map(SDL_Surface *image,char *schaine,int force,int ext)
{
    FILE *file;
    char chaine[1000];
    int i,n;
    int octet4[4];
    int mp = 0;
    int casex,casey,x,y;

    sprintf(chaine,"Map_%s.sng",schaine);
    if(ext == 1) sprintf(chaine,"Mapext_%s.sng",schaine);
    file = fopen(chaine,"w");

    fputs("\n",file);

    sprintf(chaine,"Map_%s:\n",schaine);
    if(ext == 1) sprintf(chaine,"Mapext_%s:\n",schaine);
    fputs(chaine,file);


    casex = (image->w/16);
    casey = (image->h/16);

    n = x*y;

    for(x = 0;x < casex;x++)
    {
        fputs("    dc.w ",file);
        for(y = 0;y < casey;y++)
        {
            mp = force + y*casex +x;
            if(y < casey-1) sprintf(chaine,"$%.4x,",mp);
            else sprintf(chaine,"$%.4x\n",mp);

            fputs(chaine,file);
        }


        //printf("%s %d %d %d\n",chaine ,palette[n+0],palette[n+1],palette[n+2]);
    }

    fputs("\n",file);
    fputs("\n",file);

    int bloc = 3;
    int l = 3;

    sprintf(chaine,"Map_flip_%s:\n",schaine);
    if(ext == 1) sprintf(chaine,"Mapext_flip_%s:\n",schaine);
    fputs(chaine,file);


    for(x = 0;x < casex;x++)
    {
        fputs("    dc.w ",file);
        for(y = 0;y < casey;y++)
        {
            mp = force + y*casex + bloc;
            if(y < casey-1) sprintf(chaine,"$%.4x,",mp);
            else sprintf(chaine,"$%.4x\n",mp);

            fputs(chaine,file);

        }

        l--;
        bloc--;

        if(l < 0)
        {
            l = 3;
            bloc = x+3+1;
        }

    }

    fputs("\n",file);
    fputs("\n",file);

    fclose(file);

    return 0;
}

int neogeo_convert(int argc, char** argv,SDL_Surface *image,char *adresse,int offset,int mode,int ext,int *outoff)
{
    char schaine[100];
    unsigned char palette[768];
    unsigned char *pixel = image->pixels;
    int taille = image->w*image->h*image->format->BytesPerPixel;
    int color;

    color = recup_palette(image,palette)/3;

    // info
    printf("\n%s\n",adresse);
    printf("color : %d\n",color);


    if(mode == 0 || mode == 2 || mode == 3)
    {
        if( write_rom(argc,argv,image,pixel,palette,offset,ext,outoff) == 1)
            return 0;
    }


    //------------------------
    output_filename(adresse,schaine);

    if(mode == 0 || mode == 1)
        write_pal(schaine,palette,ext);

    if(mode == 2)
        write_sMap(image,schaine,offset,ext);

    if(mode == 0)
        write_Map(image,schaine,offset,ext);






    //-------------------------



    if(color > 16) ext = 1;
    return ext;
}

