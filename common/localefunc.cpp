

#include <stdio.h>
#include <time.h>
#include <locale.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setLocale(void)
{
    printf("locale: %s \n ", setlocale( LC_ALL ,NULL))	;
    setlocale( LC_ALL , "en_US.utf8");
    printf("locale: %s \n ", setlocale( LC_ALL ,NULL))	;
    printf("a: %lf \n ",1.423);
    char chaine[1000]="1.2";
    double d;
    sscanf(chaine,"%lf\n",&d);
    printf("d: %lf \n ",d);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

